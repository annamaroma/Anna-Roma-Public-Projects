use std::collections::HashMap;
use std::sync::Arc;
use tokio::sync::{mpsc, Mutex};
use tokio::net::{TcpListener, TcpStream};
use tokio::io::{AsyncReadExt, AsyncWriteExt, BufReader, AsyncBufReadExt};
use chrono::{Utc};
use protocol::{ClientMessage, Message, ServerMessage};
use sysinfo::{System, Pid};
use tokio::fs::{File, OpenOptions};
use std::path::Path;

const CHAT_PATH: &str = "chat_data.json";
const GROUP_PATH: &str = "group_data.json";

#[derive(Debug)]
struct UserInfo {
    online: bool,
    tx: Option<mpsc::Sender<ServerMessage>>,
}

type ChatLog = Arc<Mutex<HashMap<String, Vec<Message>>>>;
type GroupLog = Arc<Mutex<HashMap<String, (Vec<String>, Vec<Message>)>>>;
type Users = Arc<Mutex<HashMap<String, UserInfo>>>;
type UnreadCounts = Arc<Mutex<HashMap<String, HashMap<String, usize>>>>;

fn ordered_key(a: &str, b: &str) -> String {
    let mut v = vec![a, b];
    v.sort();
    v.join("_")
}


// Save chat on disk
async fn save_chat_log(chat_log: &ChatLog) {
    let guard = chat_log.lock().await;
    let data = serde_json::to_vec(&*guard).unwrap();
    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(CHAT_PATH)
        .await
        .unwrap();
    file.write_all(&data).await.unwrap();
}

// Retrrieve chat from disk
async fn load_chat_log() -> HashMap<String, Vec<Message>> {
    let mut file = match File::open(CHAT_PATH).await {
        Ok(f) => f,
        Err(_) => return HashMap::new(),
    };
    let mut data = Vec::new();
    file.read_to_end(&mut data).await.unwrap();
    serde_json::from_slice(&data).unwrap_or_default()
}

// Save groups on disk
async fn save_group_log(group_log: &GroupLog) {
    let guard = group_log.lock().await;
    let data = serde_json::to_vec(&*guard).unwrap();
    let mut file = OpenOptions::new()
        .create(true)
        .write(true)
        .truncate(true)
        .open(GROUP_PATH)
        .await
        .unwrap();
    file.write_all(&data).await.unwrap();
}

// Retrieve groups from disk
async fn load_group_log() -> HashMap<String, (Vec<String>, Vec<Message>)> {
    let mut file = match File::open(GROUP_PATH).await {
        Ok(f) => f,
        Err(_) => return HashMap::new(),
    };
    let mut data = Vec::new();
    file.read_to_end(&mut data).await.unwrap();
    serde_json::from_slice(&data).unwrap_or_default()
}

async fn send_message(tx: &mpsc::Sender<ServerMessage>, msg: ServerMessage) {
    if let Err(e) = tx.send(msg).await {
        eprintln!("Fallimento nell'invio del messaggio: {:?}", e);
    }
}

async fn increment_unread(unread_counts: &UnreadCounts, username: &str, chat_id: &str) {
    let mut counts = unread_counts.lock().await;
    let user_counts = counts.entry(username.to_string()).or_insert_with(HashMap::new);
    *user_counts.entry(chat_id.to_string()).or_insert(0) += 1;
}

async fn reset_unread(unread_counts: &UnreadCounts, username: &str, chat_id: &str) {
    let mut counts = unread_counts.lock().await;
    if let Some(user_counts) = counts.get_mut(username) {
        user_counts.remove(chat_id);
    }
}

#[tokio::main]
async fn main() {
    let listener = TcpListener::bind("0.0.0.0:8080").await.unwrap();
    println!("Server in ascolto su 0.0.0.0:8080");

    let chat_log: ChatLog = Arc::new(Mutex::new(load_chat_log().await));
    let group_log: GroupLog = Arc::new(Mutex::new(load_group_log().await));
    let users: Users = Arc::new(Mutex::new(HashMap::new()));
    let unread_counts: UnreadCounts = Arc::new(Mutex::new(HashMap::new()));

    if Path::new("server_cpu.log").exists() {
        match tokio::fs::remove_file("server_cpu.log").await {
            Ok(_) => println!("Vecchio file di log rimosso."),
            Err(e) => eprintln!("Errore rimuovendo il vecchio file di log: {}", e),
        };
    }

    // Crea file di log vuoto (truncando se esiste)
    match File::create("server_cpu.log").await {
        Ok(_) => println!("Nuovo file di log creato."),
        Err(e) => {
            eprintln!("Errore creando il file di log: {}", e);
            return; // abort se non puoi loggare
        }
    }

    // Start of CPU logging thread
    tokio::spawn(async move {
        let mut sys = System::new_all();
        let pid = Pid::from(std::process::id() as usize);

        loop {
            sys.refresh_process(pid);
            if let Some(proc) = sys.process(pid) {
                let cpu_usage = proc.cpu_usage(); // percentage of CPU used
                let mem_usage = proc.memory();    // memory usage in KB
                let timestamp = chrono::Utc::now();

                let log_line = format!(
                    "[{}] CPU: {:.2}% MEM: {} KB\n",
                    timestamp, cpu_usage, mem_usage
                );

                match OpenOptions::new()
                    .create(true)
                    .append(true)
                    .open("server_cpu.log")
                    .await
                {
                    Ok(mut file) => {
                        if let Err(e) = file.write_all(log_line.as_bytes()).await {
                            eprintln!("Errore scrittura log: {}", e);
                        }
                    }
                     Err(e) => {
                        eprintln!("Errore apertura log in append: {}", e);
                    }
                }
            }

            tokio::time::sleep(std::time::Duration::from_secs(120)).await;
        }
    });

    loop {
        let (socket, _) = listener.accept().await.unwrap();

        let chat_log = Arc::clone(&chat_log);
        let group_log = Arc::clone(&group_log);
        let users = Arc::clone(&users);
        let unread_counts = Arc::clone(&unread_counts);

        tokio::spawn(async move {
            handle_connection(socket, chat_log, group_log, users, unread_counts).await;
        });
    }
}

async fn handle_connection(
    socket: TcpStream,
    chat_log: ChatLog,
    group_log: GroupLog,
    users: Users,
    unread_counts: UnreadCounts) {

    let (read_half, write_half) = tokio::io::split(socket);
    let mut reader = BufReader::new(read_half);
    let write_half = Arc::new(Mutex::new(write_half));
    let mut username = String::from("<sconosciuto>");

    // Channel to receive messages for this user
    let (tx, mut rx) = mpsc::channel::<ServerMessage>(100);

    // Task to forward messages from other users
    let write_half_clone = Arc::clone(&write_half);

    tokio::spawn(async move {
        while let Some(msg) = rx.recv().await {
            let mut json_msg = serde_json::to_vec(&msg).unwrap();
            json_msg.push(b'\n');
            let mut writer = write_half_clone.lock().await;
            let _ = writer.write_all(&json_msg).await;
        }
    });

    let mut buf = Vec::new();

    loop {
        buf.clear();

        let n = match reader.read_until(b'\n', &mut buf).await {
            Ok(0) => {
                println!("Client {} disconnesso", username);

                let mut users_guard = users.lock().await;

                if let Some(user_info) = users_guard.get_mut(&username) {
                    user_info.online = false;
                    user_info.tx = None;
                }

                break;
            }
            Ok(n) => n,
            Err(e) => {
                eprintln!("Errore di lettura: {}", e);

                let mut users_guard = users.lock().await;

                if let Some(user_info) = users_guard.get_mut(&username) {
                    user_info.online = false;
                    user_info.tx = None;
                }

                break;
            }
        };

        if n == 0 {
            continue;
        }

        let client_msg: ClientMessage = match serde_json::from_slice(&buf[..n-1]) {
            Ok(m) => m,
            Err(e) => {
                eprintln!("Errore deserializzazione: {}", e);
                let error_msg = ServerMessage::Error {
                    code: "PARSE_ERROR".to_string(),
                    message: format!("Formato del messaggio non valido: {}", e),
                };
                send_message(&tx, error_msg).await;
                continue;
            }
        };

        println!("Ricevuto: {:?}", client_msg);

        match client_msg {
            ClientMessage::Access { username: user } => {

                let mut users_guard = users.lock().await;

                // Check if user already exists
                let entry = users_guard.entry(user.clone()).or_insert(UserInfo {
                    online: false,
                    tx: None
                });

                // Check if user is already online
                if entry.online {
                    let error_msg = ServerMessage::Error {
                        code: "USER_ALREADY_ONLINE".to_string(),
                        message: format!("Utente {} è già connesso", user),
                    };
                    send_message(&tx, error_msg).await;
                    continue;
                }

                // Set the user online and save the channel
                entry.online = true;
                entry.tx = Some(tx.clone());
                username = user.clone();

                let details = format!("Benvenuto, {}!", username);

                let ack_msg = ServerMessage::Ack { details };
                send_message(&tx, ack_msg).await;

                // Send 1-to-1 chat history per new messages
                let chat_guard = chat_log.lock().await;
                for (key, messages) in chat_guard.iter() {
                    if key.contains(&username) {
                        let other_user = key.split('_')
                            .find(|u| *u != username)
                            .unwrap_or("")
                            .to_string();

                        // Send the last 50 messages
                        let recent_messages: Vec<_> = messages.iter()
                            .rev()
                            .take(50)
                            .cloned()
                            .collect();

                        if !recent_messages.is_empty() {
                            send_message(&tx, ServerMessage::UserHistory {
                                with_user: other_user,
                                messages: recent_messages.into_iter().rev().collect(),
                            }).await;
                        }
                    }
                }

                // Send group history for groups the user belongs to
                let groups_guard = group_log.lock().await;
                for (group_name, (members, messages)) in groups_guard.iter() {
                    if members.contains(&username) {
                        // Send the last 50 messages
                        let recent_messages: Vec<_> = messages.iter()
                            .rev()
                            .take(50)
                            .cloned()
                            .collect();

                        if !recent_messages.is_empty() {
                            send_message(&tx, ServerMessage::GroupHistory {
                                with_group: group_name.clone(),
                                messages: recent_messages.into_iter().rev().collect()
                            }).await;
                        }
                    }
                }

            }

            ClientMessage::Logout { username } => {
                println!("Logout richiesto da {}", username);

                let mut users_guard = users.lock().await;
                if let Some(user_info) = users_guard.get_mut(&username) {
                    user_info.online = false;
                    user_info.tx = None;
                }

                let ack_msg = ServerMessage::Ack {
                    details: format!("Logout effettuato per {}", username)
                };

                send_message(&tx, ack_msg).await;
            }

            ClientMessage::UserMessage { to, content } => {
                let users_guard = users.lock().await;

                if !users_guard.contains_key(&to) {
                    let error_msg = ServerMessage::Error {
                        code: "USER_NOT_FOUND".to_string(),
                        message: format!("Utente {} non trovato", to),
                    };
                    send_message(&tx, error_msg).await;
                    continue;
                }

                let message = Message {
                    from: username.clone(),
                    to: to.clone(),
                    content,
                    timestamp: Utc::now(),
                };

                // Store message
                let mut log = chat_log.lock().await;
                let key = ordered_key(&username, &to);
                log.entry(key).or_default().push(message.clone());

                //save_chat_log(&chat_log).await;

                increment_unread(&unread_counts, &to, &username).await;

                // Send notification to recipient
                if let Some(dest_tx) = users_guard.get(&to).and_then(|u| u.tx.as_ref()) {
                    let notify_msg = ServerMessage::Notify {
                        from: message.from.clone(),
                        to: message.to.clone(),
                        content: message.content.clone(),
                        timestamp: message.timestamp,
                    };
                    send_message(dest_tx, notify_msg).await;
                }

                // Send notification to sender (echo)
                let notify_msg = ServerMessage::Notify {
                    from: message.from.clone(),
                    to: message.to.clone(),
                    content: message.content.clone(),
                    timestamp: message.timestamp,
                };

                send_message(&tx, notify_msg).await;
            }

            ClientMessage::GroupMessage { to: group_name, content } => {
                let mut log = group_log.lock().await;
                match log.get_mut(&group_name) {
                    Some((members, messages)) => {
                        if !members.contains(&username) {
                            let error_msg = ServerMessage::Error {
                                code: "NOT_GROUP_MEMBER".to_string(),
                                message: format!("Non sei un membro del gruppo {}", group_name),
                            };
                            send_message(&tx, error_msg).await;
                            continue;
                        }

                        let message = Message {
                            from: username.clone(),
                            to: group_name.clone(),
                            content,
                            timestamp: Utc::now(),
                        };

                        messages.push(message.clone());

                        //save_group_log(&group_log).await;

                        // Send notification to all group members
                        let users_guard = users.lock().await;
                        let group_chat_id = format!("gruppo:{}", group_name);

                        for member in members.iter() {

                            if member != &username {
                                increment_unread(&unread_counts, member, &group_chat_id).await;
                            }

                            if let Some(member_tx) = users_guard.get(member).and_then(|u| u.tx.as_ref()) {
                                let notify_msg = ServerMessage::Notify {
                                    from: message.from.clone(),
                                    to: format!("gruppo:{}", message.to),
                                    content: message.content.clone(),
                                    timestamp: message.timestamp,
                                };
                                send_message(member_tx, notify_msg).await;
                            }
                        }
                    }
                    None => {
                        let error_msg = ServerMessage::Error {
                            code: "GROUP_NOT_FOUND".to_string(),
                            message: format!("Gruppo {} non trovato", group_name),
                        };
                        send_message(&tx, error_msg).await;
                    }
                }
            }

            ClientMessage::ResetUnread { chat_id } => {
                reset_unread(&unread_counts, &username, &chat_id).await;
            }

            ClientMessage::ChatListRequest => {
                let chat_guard = chat_log.lock().await;
                let mut user_chats = Vec::new();

                for key in chat_guard.keys() {
                    if key.contains(&username) {
                        let other_user = key.split("_")
                            .find(|u| *u != username)
                            .unwrap_or("")
                            .to_string();
                        user_chats.push(other_user);
                    }
                }

                let group_guard = group_log.lock().await;
                let mut user_groups = Vec::new();
                let mut group_members_map = HashMap::new();

                for (group_name, (members, _)) in group_guard.iter() {
                    if members.contains(&username) {
                        user_groups.push(group_name.clone());
                        group_members_map.insert(group_name.clone(), members.clone());
                    }
                }

                let users_guard = users.lock().await;
                let all_users: Vec<String> = users_guard.keys().cloned().collect();
                let counts_guard = unread_counts.lock().await;
                let user_unread = counts_guard.get(&username).cloned().unwrap_or_default();

                let chatlist = ServerMessage::ChatList {
                    users: user_chats,
                    groups: user_groups,
                    group_members: group_members_map,
                    unread_counts: user_unread,
                    all_users,
                };

                send_message(&tx, chatlist).await;
            }

            ClientMessage::UserHistory { with_user } => {
                let users_guard = users.lock().await;
                if !users_guard.contains_key(&with_user) {
                    let error_msg = ServerMessage::Error {
                        code: "USER_NOT_FOUND".to_string(),
                        message: format!("Utente {} non trovato", with_user),
                    };
                    send_message(&tx, error_msg).await;
                    continue;
                }

                reset_unread(&unread_counts, &username, &with_user).await;

                let log = chat_log.lock().await;
                let key = ordered_key(&username, &with_user);
                let messages = log.get(&key).cloned().unwrap_or_default();

                let history_msg = ServerMessage::UserHistory {
                    with_user,
                    messages,
                };

                send_message(&tx, history_msg).await;
            }

            ClientMessage::GroupHistory { with_group } => {
                let log = group_log.lock().await;
                match log.get(&with_group) {
                    Some((members, messages)) => {
                        if !members.contains(&username) {
                            let error_msg = ServerMessage::Error {
                                code: "NOT_GROUP_MEMBER".to_string(),
                                message: format!("Non sei un membro del gruppo {}", with_group),
                            };
                            send_message(&tx, error_msg).await;
                            continue;
                        }

                        let group_chat_id = format!("groppo:{}", with_group);
                        reset_unread(&unread_counts, &username, &group_chat_id).await;

                        let history_msg = ServerMessage::GroupHistory {
                            with_group,
                            messages: messages.clone(),
                        };
                        send_message(&tx, history_msg).await;
                    }
                    None => {
                        let error_msg = ServerMessage::Error {
                            code: "GROUP_NOT_FOUND".to_string(),
                            message: format!("Gruppo {} non trovato", with_group),
                        };
                        send_message(&tx, error_msg).await;
                    }
                }
            }

            ClientMessage::GroupCreate { group_name, members } => {

                // Verify the name of the group was provided
                if group_name.trim().is_empty() {
                    let error_msg = ServerMessage::Error {
                        code: "INVALID_GROUP_NAME".to_string(),
                        message: "Il nome del gruppo non può essere vuoto".to_string(),
                    };
                    send_message(&tx, error_msg).await;
                    continue;
                }


                // Verify all members exist
                let users_guard = users.lock().await;
                let invalid_members: Vec<_> = members
                    .iter()
                    .filter(|member| !users_guard.contains_key(*member))
                    .cloned()
                    .collect();

                if !invalid_members.is_empty() {
                    let error_msg = ServerMessage::Error {
                        code: "INVALID_MEMBERS".to_string(),
                        message: format!("Utente non trovato: {:?}", invalid_members),
                    };
                    send_message(&tx, error_msg).await;
                    continue;
                }
                drop(users_guard);

                let mut log = group_log.lock().await;

                // Check if the group already exists
                if log.contains_key(&group_name) {
                    let error_msg = ServerMessage::Error {
                        code: "GROUP_EXISTS".to_string(),
                        message: format!("Il gruppo {} esiste già", group_name),
                    };
                    send_message(&tx, error_msg).await;
                    continue;
                }

                // Add the creator to members if not already included
                let mut all_members = members.clone();
                if !all_members.contains(&username) {
                    all_members.push(username.clone());
                }

                log.insert(group_name.clone(), (all_members.clone(), Vec::new()));
                // save_group_log(&group_log).await;
                drop(log);

                let users_guard = users.lock().await;
                let member_txs: Vec<_> = all_members
                    .iter()
                    .filter_map(|member| users_guard.get(member).and_then(|u| u.tx.clone()))
                    .collect();
                drop(users_guard);

                // Send ack message
                let ack_msg = ServerMessage::Ack {
                    details: format!("Aggiunto al gruppo: {} con membri: {:?}", group_name, all_members),
                };

                // Send confirm to all members
                for member_tx in member_txs {
                    send_message(&member_tx, ack_msg.clone()).await;
                }
            }

            ClientMessage::GroupLeave { group_name } => {
                let mut log = group_log.lock().await;
                match log.get_mut(&group_name) {
                    Some((members, _)) => {
                        if let Some(pos) = members.iter().position(|m| m == &username) {
                            members.remove(pos);
                            //save_group_log(&group_log).await;

                            let group_chat_id = format!("gruppo:{}", group_name);
                            reset_unread(&unread_counts, &username, &group_chat_id).await;

                            let ack_msg = ServerMessage::Ack {
                                details: format!("Uscito dal gruppo: {}", group_name),
                            };

                            send_message(&tx, ack_msg).await;

                            // If group is empty, remove it
                            if members.is_empty() {
                                log.remove(&group_name);
                            }

                        } else {
                            let error_msg = ServerMessage::Error {
                                code: "NOT_GROUP_MEMBER".to_string(),
                                message: format!("Non sei membro del gruppo {}", group_name),
                            };
                            send_message(&tx, error_msg).await;
                        }
                    }
                    None => {
                        let error_msg = ServerMessage::Error {
                            code: "GROUP_NOT_FOUND".to_string(),
                            message: format!("Groppo {} non trovato", group_name),
                        };
                        send_message(&tx, error_msg).await;
                    }
                }
            }

            ClientMessage::UsersRequest => {
                let users_guard = users.lock().await;
                let all_users: Vec<String> = users_guard.keys().cloned().collect();

                let userslist = ServerMessage::UsersList {
                    all_users,
                };

                send_message(&tx, userslist).await;
            }
        }
    }
}


