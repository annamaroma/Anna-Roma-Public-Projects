use std::collections::{VecDeque, HashMap};
use std::sync::{Arc, Mutex};
use eframe::egui;
use eframe::egui::Rounding;
use egui::{Color32, Stroke, Vec2, Pos2, Rect, FontId};
use tokio::{
    io::{AsyncBufReadExt, AsyncWriteExt, BufReader},
    net::TcpStream,
    runtime::Runtime,
    sync::Mutex as AsyncMutex,
};
use protocol::{ClientMessage, ServerMessage};

#[derive(Clone)]
struct GroupInfo {
    name: String,
    members: Vec<String>,
}

#[derive(Default)]
struct Unread {
    counts: HashMap<String, usize>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum ChatTarget {
    User(String),
    Group(String),
}

#[derive(PartialEq, Eq)]
enum Screen {
    Access,
    Menu,
    Chat,
    GroupCreation,
}

impl Default for Screen {
    fn default() -> Self {
        Screen::Access
    }
}

#[derive(Default)]
struct ChatApp {
    rt: Option<Runtime>,
    username: String,
    input_username: String,
    write_half: Option<Arc<AsyncMutex<tokio::net::tcp::OwnedWriteHalf>>>,
    read_half: Option<Arc<AsyncMutex<tokio::net::tcp::OwnedReadHalf>>>,
    current_screen: Screen,
    message_input: String,
    messages: Arc<Mutex<VecDeque<String>>>,
    current_target: Arc<Mutex<Option<ChatTarget>>>,
    error_message: Option<String>,
    search_filter: String,
    new_group_name: String,
    new_group_members: String,
    first_time_in_chat: bool,
    message_listener: Option<tokio::task::JoinHandle<()>>,
    groups: Arc<Mutex<Vec<GroupInfo>>>,
    group_creation_result: Arc<Mutex<Option<String>>>,
    creating_group: Arc<Mutex<bool>>,
    chats: Arc<Mutex<Vec<String>>>,
    status_message: Arc<Mutex<Option<String>>>,
    user_exists_check: Arc<Mutex<Option<bool>>>,
    checking_user: Arc<Mutex<bool>>,
    unread: Arc<Mutex<Unread>>,
    all_users: Arc<Mutex<Vec<String>>>,
    group_members_search: String,
    selected_members: Vec<String>,
    selected_users_for_group: Vec<String>,
}

impl ChatApp {


    // Send a ResetUnread request when a chat is opened
    fn mark_chat_as_read(&self, target: &ChatTarget) {
        let chat_id = match target {
            ChatTarget::User(user) => user.clone(),
            ChatTarget::Group(group) => format!("gruppo:{}", group),
        };

        self.send_message(ClientMessage::ResetUnread { chat_id: chat_id.clone() });

        // Reset local counter
        let mut unread_guard = self.unread.lock().unwrap();
        unread_guard.counts.remove(&chat_id);
    }

    // Get an unread counter for a chat
    fn get_unread_count(&self, target: &ChatTarget) -> usize {
        let chat_id = match target {
            ChatTarget::User(user) => user.clone(),
            ChatTarget::Group(group) => format!("gruppo:{}", group),
        };

        let unread_guard = self.unread.lock().unwrap();
        unread_guard.counts.get(&chat_id).copied().unwrap_or(0)
    }

    // Invio messaggi async con framing \n
    fn send_message(&self, message: ClientMessage) {
        if let Some(write_half) = &self.write_half {
            let mut data = serde_json::to_string(&message).unwrap();
            data.push('\n');
            let write_half = write_half.clone();
            self.rt.as_ref().unwrap().spawn(async move {
                if let Err(e) = write_half.lock().await.write_all(data.as_bytes()).await {
                    eprintln!("Errore durante l'invio: {:?}", e);
                }
            });
        }
    }

    // Invio messaggi bloccante
    fn send_message_blocking(&self, message: ClientMessage) -> Result<(), Box<dyn std::error::Error>> {
        if let Some(write_half) = &self.write_half {
            let mut data = serde_json::to_string(&message)?;
            data.push('\n');
            let write_half = write_half.clone();
            self.rt.as_ref().unwrap().block_on(async move {
                write_half.lock().await.write_all(data.as_bytes()).await
            })?;
        }
        Ok(())
    }

    // Login con logout precedente
    fn try_login(&mut self) {
        if !self.username.is_empty() {
            let _ = self.send_message_blocking(ClientMessage::Logout {
                username: self.username.clone(),
            });
            self.disconnect();
        }

        let username = self.input_username.trim().to_string();
        if username.is_empty() {
            self.error_message = Some("Username non può essere vuoto.".into());
            return;
        }

        self.rt = Some(Runtime::new().unwrap());
        let rt = self.rt.as_ref().unwrap();

        let stream = rt.block_on(async { TcpStream::connect("127.0.0.1:8080").await.ok() });
        if let Some(stream) = stream {
            let (read_half, write_half) = stream.into_split();
            self.read_half = Some(Arc::new(AsyncMutex::new(read_half)));
            self.write_half = Some(Arc::new(AsyncMutex::new(write_half)));

            let access_msg = ClientMessage::Access { username: username.clone() };
            if let Err(e) = self.send_message_blocking(access_msg) {
                self.error_message = Some(format!("Errore invio accesso: {}", e));
                return;
            }

            let read_half = self.read_half.as_ref().unwrap().clone();
            let result = rt.block_on(async {
                let mut buffer = Vec::new();
                let mut read_half_guard = read_half.lock().await;
                let mut reader = BufReader::new(&mut *read_half_guard);
                let n = reader.read_until(b'\n', &mut buffer).await.ok()?;
                if n == 0 { return None; }
                serde_json::from_slice::<ServerMessage>(&buffer[..n - 1]).ok()
            });

            match result {
                Some(ServerMessage::Ack { details }) => {
                    self.username = username;
                    self.input_username.clear();
                    self.error_message = None;
                    self.spawn_message_reader();
                    self.current_screen = Screen::Menu;
                    *self.status_message.lock().unwrap() = Some(details);

                    self.send_message(ClientMessage::ChatListRequest);
                }
                Some(ServerMessage::Error { message, .. }) => {
                    self.error_message = Some(message);
                }
                Some(_) => {
                    self.error_message = Some("Risposta inaspettata dal server".into());
                }
                None => {
                    self.error_message = Some("Errore durante il login".into());
                }
            }
        } else {
            self.error_message = Some("Connessione al server fallita.".into());
        }
    }


    fn reload(&self) {
        self.send_message(ClientMessage::UsersRequest);
    }


    // Disconnessione e reset stato
    fn disconnect(&mut self) {
        if !self.username.is_empty() {
            let _ = self.send_message_blocking(ClientMessage::Logout { username: self.username.clone() });
        }
        if let Some(listener) = self.message_listener.take() {
            listener.abort();
            // A small delay for a proper cleanup
            std::thread::sleep(std::time::Duration::from_millis(100));
        }

        if let Some(rt) = self.rt.take() {
            rt.shutdown_background();
        }

        self.username.clear();
        self.write_half = None;
        self.read_half = None;
        self.message_input.clear();
        self.messages.lock().unwrap().clear();
        *self.current_target.lock().unwrap() = None;
        self.error_message = None;
        *self.status_message.lock().unwrap() = None;
        self.groups.lock().unwrap().clear();
        self.chats.lock().unwrap().clear();
        *self.group_creation_result.lock().unwrap() = None;
        *self.creating_group.lock().unwrap() = false;
        *self.user_exists_check.lock().unwrap() = None;
        *self.checking_user.lock().unwrap() = false;
        self.new_group_members.clear();
        self.new_group_name.clear();
        self.search_filter.clear();
        self.unread.lock().unwrap().counts.clear();
        self.current_screen = Screen::Access;
    }

    // Invia messaggi a target
    fn try_send_message(&mut self) {
        let content = self.message_input.trim().to_string();
        if content.is_empty() {
            return;
        }
        let to = self.current_target.lock().unwrap().clone();
        if let Some(to) = to {
            let message = match to {
                ChatTarget::User(user) => ClientMessage::UserMessage { to: user, content },
                ChatTarget::Group(group) => ClientMessage::GroupMessage { to: group, content },
            };
            self.send_message(message);
            self.message_input.clear();
        }
    }

    // Richiesta storico chat
    fn request_chat_history(&mut self) {
        let to = self.current_target.lock().unwrap().clone();
        if let Some(ref target) = to {

            self.mark_chat_as_read(target);

            let message = match target {
                ChatTarget::User(user) => ClientMessage::UserHistory { with_user: user.clone() },
                ChatTarget::Group(group) => ClientMessage::GroupHistory { with_group: group.clone() },
            };
            self.send_message(message);
        }
    }

    // Creazione gruppo
    fn try_create_group(&mut self) {
        let group_name = self.new_group_name.trim().to_string();
        let members_text = self.new_group_members.trim().to_string();

        if group_name.is_empty() {
            self.error_message = Some("Nome gruppo non può essere vuoto".into());
            return;
        }

        if members_text.is_empty() {
            self.error_message = Some("Devi specificare almeno un membro".into());
            return;
        }

        let members: Vec<String> = members_text
            .split(',')
            .map(|s| s.trim().to_string())
            .filter(|s| !s.is_empty() && *s != self.username) // Escludi se stesso
            .collect();

        if members.is_empty() {
            self.error_message = Some("Devi specificare almeno un membro diverso da te".into());
            return;
        }

        *self.creating_group.lock().unwrap() = true;
        *self.group_creation_result.lock().unwrap() = None;

        // Usa ClientMessage::GroupCreate come previsto dal server
        let message = ClientMessage::GroupCreate {
            group_name: group_name.clone(),
            members,
        };
        self.send_message(message);

        self.current_screen = Screen::Menu;
        self.new_group_name.clear();
        self.new_group_members.clear();
        self.error_message = None;
    }

    fn leave_current_group(&mut self) {
        let target = self.current_target.lock().unwrap().clone();
        if let Some(ChatTarget::Group(group_name)) = target {
            self.send_message(ClientMessage::GroupLeave { group_name });
            self.current_screen = Screen::Menu;
            *self.current_target.lock().unwrap() = None;
        }
    }

    // Task di lettura messaggi dal server con framing \n
    fn spawn_message_reader(&mut self) {
        if self.message_listener.is_some() {
            return;
        }
        let read_half = self.read_half.as_ref().unwrap().clone();
        let messages_arc = Arc::clone(&self.messages);
        let my_username = self.username.clone();
        let current_target = self.current_target.clone();
        let groups_arc = self.groups.clone();
        let group_creation_arc = self.group_creation_result.clone();
        let creating_group_arc = self.creating_group.clone();
        let chats_arc = self.chats.clone();
        let status_arc = self.status_message.clone();
        let user_exists_arc = self.user_exists_check.clone();
        let checking_user_arc = self.checking_user.clone();
        let unread_arc = self.unread.clone();
        let all_users_arc = self.all_users.clone();

        let listener = self.rt.as_ref().unwrap().spawn(async move {
            let mut buf = Vec::new();
            loop {
                let mut read_half_guard = read_half.lock().await;
                let mut reader = BufReader::new(&mut *read_half_guard);
                buf.clear();
                let n = match reader.read_until(b'\n', &mut buf).await {
                    Ok(n) if n > 0 => n,
                    _ => continue,
                };
                let response: ServerMessage = match serde_json::from_slice(&buf[..n - 1]) {
                    Ok(msg) => msg,
                    Err(e) => {
                        eprintln!("Errore parsing messaggio server: {:?}", e);
                        continue;
                    }
                };
                match response {
                    ServerMessage::Ack { details } => {
                        *status_arc.lock().unwrap() = Some(details.clone());

                        // Gestione creazione gruppo
                        if details.contains("Aggiunto al gruppo:") {
                            if let Some(group_part) = details.split("Aggiunto al gruppo: ").nth(1) {
                                if let Some((group_name, members_part)) = group_part.split_once(" con membri: ") {
                                    if let Ok(members) = serde_json::from_str::<Vec<String>>(members_part) {
                                        let mut groups = groups_arc.lock().unwrap();
                                        let group_info = GroupInfo {
                                            name: group_name.to_string(),
                                            members,
                                        };
                                        groups.retain(|g| g.name != group_info.name);
                                        groups.push(group_info);
                                    }
                                }
                            }
                            *group_creation_arc.lock().unwrap() = Some(details.clone());
                            *creating_group_arc.lock().unwrap() = false;
                            if let Some(group_part) = details.split("Aggiunto al gruppo: ").nth(1) {
                                if let Some((group_name, _)) = group_part.split_once(" con membri: ") {
                                    //*current_target.lock().unwrap() = Some(ChatTarget::Group(group_name.to_string()));
                                }
                            }
                        }

                        if details.contains("Uscito dal gruppo") {
                            // Update group_list removing the one left
                            if let Some(group_name) = details.split("Uscito dal gruppo: ").nth(1) {
                                let mut groups = groups_arc.lock().unwrap();
                                groups.retain(|g| g.name != group_name);
                            }
                        }

                        // Reset controllo utente
                        if *checking_user_arc.lock().unwrap() {
                            *user_exists_arc.lock().unwrap() = Some(true);
                            *checking_user_arc.lock().unwrap() = false;
                        }
                    }
                    ServerMessage::Notify { from, to, content, timestamp } => {
                        let current_target_guard = current_target.lock().unwrap();
                        let current_target_name = current_target_guard.clone();
                        drop(current_target_guard);

                        let is_relevant = match current_target_name.as_ref() {
                            Some(ChatTarget::Group(group_name)) if to.starts_with("gruppo:") => {
                                to == format!("gruppo:{}", group_name) || to == *group_name
                            }
                            Some(ChatTarget::User(user_name)) if !to.starts_with("gruppo:") => {
                                (from == *user_name && to == my_username) || (from == my_username && to == *user_name)
                            }
                            _ => false,
                        };

                        let should_increment_unread = from != my_username && !is_relevant;

                        if is_relevant {
                            let time_str = timestamp.format("%H:%M:%S").to_string();
                            let formatted_msg = format!("[{}] {}: {}", time_str, from, content);
                            let mut messages = messages_arc.lock().unwrap();
                            messages.push_back(formatted_msg);
                        }

                        // Update the counter of unread if the notify is not for the current
                        // chat
                        if should_increment_unread {
                            let chat_id = if to.starts_with("gruppo:") {
                                to.clone()
                            } else {
                                from.clone()
                            };

                            let mut unread_guard = unread_arc.lock().unwrap();
                            *unread_guard.counts.entry(chat_id).or_insert(0) += 1;
                        }

                        // CORREZIONE: Aggiorna lista chat per utenti che ti scrivono
                        if !to.starts_with("gruppo:") && from != my_username {
                            let mut chats = chats_arc.lock().unwrap();
                            if !chats.iter().any(|c| *c == from) {
                                chats.push(from);
                            }
                        }
                    }
                    ServerMessage::ChatList { users, groups, group_members, unread_counts, all_users } => {
                        let mut chats = chats_arc.lock().unwrap();
                        chats.clear();
                        chats.extend(users);

                        let mut groups_vec = groups_arc.lock().unwrap();
                        groups_vec.clear();
                        for group_name in groups {
                            let members = group_members.get(&group_name).cloned().unwrap_or_default();
                            groups_vec.push(GroupInfo {
                                name: group_name,
                                members,
                            });
                        }

                        let mut unread_guard = unread_arc.lock().unwrap();
                        unread_guard.counts = unread_counts;

                        let mut all_users_guard = all_users_arc.lock().unwrap();
                        *all_users_guard = all_users;
                    }
                    ServerMessage::UserHistory { with_user, messages: hist_messages } => {
                        // Solo aggiorna i messaggi se è la chat attiva
                        let current_target_guard = current_target.lock().unwrap();
                        let is_current_chat = matches!(current_target_guard.as_ref(), Some(ChatTarget::User(user)) if user == &with_user);
                        drop(current_target_guard);

                        if is_current_chat {
                            let mut messages = messages_arc.lock().unwrap();
                            messages.clear();
                            for msg in hist_messages {
                                let time_str = msg.timestamp.format("%H:%M:%S").to_string();
                                let formatted_msg = format!("[{}] {}: {}", time_str, msg.from, msg.content);
                                messages.push_back(formatted_msg);
                            }
                        }

                        // CORREZIONE: Sempre aggiungi alla lista chat se non presente
                        let mut chats = chats_arc.lock().unwrap();
                        if !chats.iter().any(|c| *c == with_user) {
                            chats.push(with_user);
                        }

                        // Reset controllo utente se in corso
                        if *checking_user_arc.lock().unwrap() {
                            *user_exists_arc.lock().unwrap() = Some(true);
                            *checking_user_arc.lock().unwrap() = false;
                        }
                    }
                    ServerMessage::GroupHistory { with_group, messages: hist_messages } => {
                        // Solo aggiorna i messaggi se è la chat attiva
                        let current_target_guard = current_target.lock().unwrap();
                        let is_current_chat = matches!(current_target_guard.as_ref(), Some(ChatTarget::Group(group)) if group == &with_group);
                        drop(current_target_guard);

                        if is_current_chat {
                            let mut messages = messages_arc.lock().unwrap();
                            messages.clear();
                            for msg in hist_messages {
                                let time_str = msg.timestamp.format("%H:%M:%S").to_string();
                                let formatted_msg = format!("[{}] {}: {}", time_str, msg.from, msg.content);
                                messages.push_back(formatted_msg);
                            }
                        }

                        // CORREZIONE: Verifica se il gruppo esiste già in groups, altrimenti crealo
                        let mut groups = groups_arc.lock().unwrap();
                        if !groups.iter().any(|g| g.name == with_group) {
                            // Il server dovrebbe fornire i membri, ma per ora usiamo una lista vuota
                            groups.push(GroupInfo {
                                name: with_group.clone(),
                                members: vec![], // I membri verranno aggiornati quando disponibili
                            });
                        }
                    }
                    ServerMessage::Error { code: _, message } => {
                        let error_msg = format!("Errore: {}", message);
                        *status_arc.lock().unwrap() = Some(error_msg.clone());

                        // Non aggiungere errori ai messaggi della chat attiva, solo al log

                        if *creating_group_arc.lock().unwrap() {
                            *group_creation_arc.lock().unwrap() = Some(message);
                            *creating_group_arc.lock().unwrap() = false;
                        }

                        // Gestione errore controllo utente
                        if *checking_user_arc.lock().unwrap() {
                            *user_exists_arc.lock().unwrap() = Some(false);
                            *checking_user_arc.lock().unwrap() = false;
                        }
                    }
                    ServerMessage::UsersList { all_users } => {
                        let mut all_users_guard = all_users_arc.lock().unwrap();
                        *all_users_guard = all_users;
                    }
                }
            }
        });
        self.message_listener = Some(listener);
    }

    pub fn setup_visuals(ctx: &egui::Context) {
        let mut visuals = egui::Visuals::dark();
        visuals.override_text_color = Some(Color32::from_rgb(245, 245, 255));
        visuals.window_fill = Color32::from_rgb(36, 44, 58);
        visuals.panel_fill = Color32::from_rgb(30, 34, 41);
        visuals.widgets.inactive.bg_fill = Color32::from_rgb(48, 54, 73);
        visuals.widgets.hovered.bg_fill = Color32::from_rgb(0, 180, 150);
        visuals.selection.bg_fill = Color32::from_rgb(0, 163, 113);
        visuals.faint_bg_color = Color32::from_rgb(42, 42, 54);
        visuals.widgets.active.bg_fill = Color32::from_rgb(109, 191, 184);
        visuals.extreme_bg_color = Color32::from_rgb(20, 20, 32);
        ctx.set_visuals(visuals);
    }

    fn rounded_button(ui: &mut egui::Ui, text: &str, color: Color32, size: Vec2) -> bool {
        let font_id = FontId::proportional(16.0);
        let galley = ui.fonts(|f| f.layout_no_wrap(text.into(), font_id.clone(), Color32::WHITE));

        let desired_size = Vec2::new(
            size.x.max(galley.size().x + 30.0),
            size.y.max(galley.size().y + 16.0)
        );

        let (rect, response) = ui.allocate_exact_size(desired_size, egui::Sense::click());

        let mut button_color = color;
        if response.hovered() {
            button_color = Color32::from_rgb(
                (color.r() as f32 * 1.2).min(255.0) as u8,
                (color.g() as f32 * 1.2).min(255.0) as u8,
                (color.b() as f32 * 1.2).min(255.0) as u8,
            );
        }
        if response.clicked() {
            button_color = Color32::from_rgb(
                (color.r() as f32 * 0.8) as u8,
                (color.g() as f32 * 0.8) as u8,
                (color.b() as f32 * 0.8) as u8,
            );
        }

        ui.painter().rect_filled(rect, 12.0, button_color);
        ui.painter().text(
            rect.center(),
            egui::Align2::CENTER_CENTER,
            text,
            font_id,
            Color32::WHITE,
        );

        response.clicked()
    }

    fn rounded_text_edit(ui: &mut egui::Ui, text: &mut String, hint: &str, width: f32) -> egui::Response {
    let height = 40.0;
    let (rect, response) = ui.allocate_exact_size(Vec2::new(width, height), egui::Sense::focusable_noninteractive());

    let bg_color = Color32::from_rgb(48, 54, 73);
    ui.painter().rect_filled(rect, 12.0, bg_color);

    let text_rect = Rect::from_min_size(rect.min + Vec2::new(12.0, 8.0), rect.size() - Vec2::new(18.0, 16.0));
    ui.allocate_ui_at_rect(text_rect, |ui| {
        ui.add(
            egui::TextEdit::singleline(text)
                .hint_text(hint)
                .desired_width(text_rect.width())
                .frame(false)
        );
    });

    response
}
    fn draw_unread_badge(ui: &mut egui::Ui, count: usize, pos: Pos2) {
        if count > 0 {
            let badge_radius = 12.0;
            let badge_color = Color32::GREEN;

            ui.painter().circle_filled(pos, badge_radius, badge_color);
            ui.painter().text(
                pos,
                egui::Align2::CENTER_CENTER,
                if count > 99 { "99+".to_string() } else { count.to_string() },
                egui::FontId::proportional(10.0),
                Color32::WHITE,
            );
        }
    }

    fn ui_access(&mut self, ui: &mut egui::Ui) {
        ui.vertical_centered(|ui| {
            ui.add_space(100.0);

            ui.heading("RUGGINE");
            ui.add_space(40.0);

            ui.label("Inserisci il tuo username:");
            ui.add_space(10.0);

            Self::rounded_text_edit(ui, &mut self.input_username, "Username...", 300.0);
            ui.add_space(20.0);

            if Self::rounded_button(ui, "Accedi", Color32::from_rgb(100, 150, 200), Vec2::new(200.0, 45.0)) {
                self.try_login();
            }

            ui.add_space(20.0);

            if let Some(err) = &self.error_message {
                ui.label(egui::RichText::new(err).color(Color32::RED).size(14.0));
            }
        });
    }

    // MODIFICATO: ui_menu con badge unread e membri gruppo
// UI principale del menu
fn ui_menu(&mut self, ui: &mut egui::Ui) {
    ui.vertical(|ui| {
        ui.vertical_centered(|ui| {
            ui.heading(format!("Ciao, {}!", self.username));
            if Self::rounded_button(ui, "Disconnetti", Color32::from_rgb(200, 100, 100), Vec2::new(120.0, 35.0)) {
                self.disconnect();
            }
        });

        ui.add_space(12.0);

        ui.horizontal(|ui| {
            ui.vertical(|ui| {
                Self::rounded_text_edit(ui, &mut self.search_filter, "Nome utente...", 250.0);

                let all_users = self.all_users.lock().unwrap().clone();
                let filter = self.search_filter.to_lowercase();
                let matching_users: Vec<_> = all_users.into_iter()
                    .filter(|u| u != &self.username && (filter.is_empty() || u.to_lowercase().contains(&filter)))
                    .collect();

                if !matching_users.is_empty() {
                    let max_visible = 3;
                    let item_height = 35.0;
                    let total_height = (matching_users.len().min(max_visible) as f32) * item_height;

                    egui::Frame::none()
                        .fill(Color32::from_rgb(48, 54, 73))
                        .stroke(egui::Stroke::new(1.0, Color32::from_rgb(100, 100, 110)))
                        .rounding(egui::Rounding::same(8.0))
                        .show(ui, |ui| {
                            ui.set_min_width(250.0);
                            egui::ScrollArea::vertical()
                                .max_height(total_height)
                                .show(ui, |ui| {
                                    for user in matching_users.iter() {
                                        let response = ui.add_sized(
                                            [250.0, item_height],
                                            egui::Button::new(format!("👤 {}", user))
                                                .fill(Color32::TRANSPARENT)
                                                .stroke(egui::Stroke::NONE)
                                        );
                                        if response.clicked() {
                                            *self.current_target.lock().unwrap() = Some(ChatTarget::User(user.clone()));
                                            self.first_time_in_chat = true;
                                            self.current_screen = Screen::Chat;
                                            self.search_filter.clear();
                                            break;
                                        }
                                    }
                                });
                        });
                }

                ui.add_space(10.0);
            });

            let button_size = 24.0; // puoi regolare
            if ui.add_sized(
                [button_size, button_size],
                egui::Button::new("🔄") // o un'icona SVG
                    .rounding(Rounding::same(button_size / 2.0)) // tondo
                    .stroke(Stroke::new(1.0, Color32::WHITE)) // bordo se vuoi
                    .fill(Color32::from_rgb(70, 70, 80)), // colore sfondo
            ).clicked() {
                // azione al click, ad es. ricaricare lista
                self.reload();
            }

        });
        ui.add_space(18.0);

        // Bottone Crea Gruppo (apre creazione gruppo)
        if Self::rounded_button(ui, "Crea Gruppo", Color32::from_rgb(0, 163, 113), Vec2::new(180.0, 40.0)) {
            self.current_screen = Screen::GroupCreation;
            self.error_message = None;
        }

        ui.add_space(30.0);

        // Lista chat e gruppi con badge
        ui.label(egui::RichText::new("Le tue chat:").size(18.0).strong());
        ui.add_space(10.0);

        egui::ScrollArea::vertical().max_height(300.0).show(ui, |ui| {
            let chats_snapshot = self.chats.lock().unwrap().clone();
            let groups_snapshot = self.groups.lock().unwrap().clone();

            // Chat utenti
            for chat in chats_snapshot.iter() {
                ui.horizontal(|ui| {
                    let target = ChatTarget::User(chat.clone());
                    let unread_count = self.get_unread_count(&target);
                    let (rect, response) = ui.allocate_exact_size(Vec2::new(ui.available_width() - 20.0, 50.0), egui::Sense::click());


                    let bg_color = if response.hovered() {
                        Color32::from_rgb(70, 70, 80)
                    } else {
                        Color32::from_rgb(60, 60, 70)
                    };

                    ui.painter().rect_filled(rect, 12.0, bg_color);
                    ui.painter().rect_stroke(rect, 12.0, Stroke::new(1.0, Color32::from_rgb(100, 100, 110)));
                    ui.painter().text(rect.min + Vec2::new(20.0, rect.height() / 2.0), egui::Align2::LEFT_CENTER, format!("👤 {}", chat), FontId::proportional(16.0), Color32::WHITE);


                    if unread_count > 0 {
                        let badge_pos = rect.right_center() - Vec2::new(25.0, 0.0);
                        ChatApp::draw_unread_badge(ui, unread_count, badge_pos);
                    }

                    if response.clicked() {
                        *self.current_target.lock().unwrap() = Some(target);
                        self.messages.lock().unwrap().clear();
                        self.first_time_in_chat = true;
                        self.current_screen = Screen::Chat;
                    }
                });
                ui.add_space(5.0);
            }

            // Chat gruppi
            for group in groups_snapshot.iter() {
                ui.horizontal(|ui| {
                    let target = ChatTarget::Group(group.name.clone());
                    let unread_count = self.get_unread_count(&target);
                    let (rect, response) = ui.allocate_exact_size(Vec2::new(ui.available_width() - 20.0, 60.0), egui::Sense::click());

                    let selected = *self.current_target.lock().unwrap() == Some(target.clone());
                    let bg_color = if selected {
                        Color32::from_rgb(150, 100, 200)
                    } else if response.hovered() {
                        Color32::from_rgb(70, 70, 80)
                    } else {
                        Color32::from_rgb(60, 60, 70)
                    };

                    ui.painter().rect_filled(rect, 12.0, bg_color);
                    ui.painter().rect_stroke(rect, 12.0, Stroke::new(1.0, Color32::from_rgb(100, 100, 110)));

                    ui.painter().text(rect.min + Vec2::new(20.0, 12.0), egui::Align2::LEFT_TOP, format!("👥 {}", group.name), FontId::proportional(16.0), Color32::WHITE);

                    let members_preview = if group.members.is_empty() {
                        "membri sconosciuti".to_string()
                    } else {
                        let preview = group.members.iter().take(3).cloned().collect::<Vec<_>>().join(", ");
                        if group.members.len() > 3 {
                            format!("{} e altri {}", preview, group.members.len() - 3)
                        } else {
                            preview
                        }
                    };

                    ui.painter().text(rect.min + Vec2::new(20.0, 35.0), egui::Align2::LEFT_TOP, members_preview, FontId::proportional(12.0), Color32::LIGHT_GRAY);

                    if unread_count > 0 {
                        let badge_pos = rect.right_center() - Vec2::new(25.0, 0.0);
                        ChatApp::draw_unread_badge(ui, unread_count, badge_pos);
                    }

                    if response.clicked() {
                        *self.current_target.lock().unwrap() = Some(target);
                        self.messages.lock().unwrap().clear();
                        self.first_time_in_chat = true;
                        self.current_screen = Screen::Chat;
                    }
                });
                ui.add_space(5.0);
            }
        });

        if let Some(err) = &self.error_message {
            ui.add_space(20.0);
            ui.label(egui::RichText::new(err).color(Color32::RED).size(14.0));
        }
    });
}

// UI creazione gruppo
fn ui_group_creation(&mut self, ui: &mut egui::Ui) {
    ui.vertical(|ui| {
        ui.horizontal(|ui| {
            if Self::rounded_button(ui, "Indietro", Color32::from_rgb(120, 120, 120), Vec2::new(110.0, 35.0)) {
                self.current_screen = Screen::Menu;
                self.new_group_name.clear();
                self.group_members_search.clear();
                self.selected_members.clear();
                self.error_message = None;
                return;
            }
        });

        ui.horizontal(|ui| {
            Self::rounded_text_edit(ui, &mut self.new_group_name, "Nome gruppo", 280.0);
        });

        ui.add_space(12.0);

        ui.label("Aggiungi membri:");
        Self::rounded_text_edit(ui, &mut self.group_members_search, "Cerca utente", 280.0);

        let all_users = self.all_users.lock().unwrap().clone();
        let filter = self.group_members_search.to_lowercase();
        let filtered: Vec<_> = all_users.iter()
            .filter(|u| *u != &self.username)
            .filter(|u| filter.is_empty() || u.to_lowercase().contains(&filter))
            .collect();

        let item_height = 35.0;
        let max_visible = 5;
        let total_height = (filtered.len().min(max_visible) as f32) * item_height;

        egui::Frame::none()
            .fill(Color32::from_rgb(48, 54, 73))
            .stroke(egui::Stroke::new(1.0, Color32::from_rgb(100, 100, 110)))
            .rounding(egui::Rounding::same(8.0))
            .show(ui, |ui| {
                ui.set_min_width(280.0);
                egui::ScrollArea::vertical()
                    .max_height(total_height)
                    .show(ui, |ui| {
                        for user in filtered {
                            let selected = self.selected_members.iter().any(|m| m == user);

                            let response = ui.add_sized(
                                [280.0, item_height],
                                egui::Button::new(format!("👤 {}", user))
                                    .fill(if selected { Color32::from_rgb(0, 163, 113) } else { Color32::from_rgb(48, 54, 73) })
                                    .stroke(egui::Stroke::new(1.0, Color32::from_rgb(100, 100, 110)))
                            );

                            if response.clicked() {
                                if selected {
                                    self.selected_members.retain(|m| m != user);
                                } else {
                                    self.selected_members.push(user.clone());
                                }
                            }
                        }
                    });
            });

        ui.add_space(12.0);

        // Bottone crea gruppo
        if Self::rounded_button(ui, "Crea Gruppo", Color32::from_rgb(0, 163, 113), Vec2::new(150.0, 45.0)) {
            let name = self.new_group_name.trim().to_string();
            if name.is_empty() || self.selected_members.is_empty() {
                self.error_message = Some("Nome gruppo e almeno 1 membro!".to_string());
            } else {
                self.send_message(ClientMessage::GroupCreate {
                    group_name: name,
                    members: self.selected_members.clone(),
                });
                self.current_screen = Screen::Menu;
                self.new_group_name.clear();
                self.selected_members.clear();
                self.error_message = None;
            }
        }

        // Mostra eventuale errore
        if let Some(err) = &self.error_message {
            ui.add_space(10.0);
            ui.label(egui::RichText::new(err).color(Color32::RED).size(14.0));
        }
    });
}

    // MODIFICATO: ui_chat con pulsante "Esci dal gruppo"
    fn ui_chat(&mut self, ui: &mut egui::Ui) {
        let target_opt = self.current_target.lock().unwrap().clone();
        if let Some(target) = target_opt {
            ui.horizontal(|ui| {
                if Self::rounded_button(ui, "← Indietro", Color32::from_rgb(120, 120, 120), Vec2::new(100.0, 35.0)) {
                    *self.current_target.lock().unwrap() = None;
                    self.current_screen = Screen::Menu;
                }

                ui.add_space(20.0);

                match &target {
                    ChatTarget::User(user) => {
                        ui.heading(format!("Chat con {}", user));
                    }
                    ChatTarget::Group(group_name) => {
                        let groups = self.groups.lock().unwrap();
                        if let Some(group) = groups.iter().find(|g| g.name == *group_name) {
                            let members_text = if group.members.is_empty() {
                                "membri sconosciuti".to_string()
                            } else {
                                group.members.join(", ")
                            };
                            ui.heading(format!("👥 Gruppo: {}", group_name));
                            ui.label(egui::RichText::new(format!("Membri: {}", members_text)).size(12.0).color(Color32::LIGHT_GRAY));
                        } else {
                            ui.heading(format!("👥 Gruppo: {}", group_name));
                        }
                    }
                }

                // NUOVO: Pulsante "Esci dal gruppo" per i gruppi
                if matches!(&target, ChatTarget::Group(_)) {
                    ui.with_layout(egui::Layout::right_to_left(egui::Align::Center), |ui| {
                        if Self::rounded_button(ui, "Esci dal gruppo", Color32::from_rgb(200, 100, 100), Vec2::new(120.0, 35.0)) {
                            self.leave_current_group();
                        }
                    });
                }
            });

            ui.separator();
            ui.add_space(10.0);

            if self.first_time_in_chat {
                self.request_chat_history();
                self.first_time_in_chat = false;
            }

            let messages = self.messages.lock().unwrap().clone();
            egui::ScrollArea::vertical()
                .max_height(400.0)
                .stick_to_bottom(true)
                .show(ui, |ui| {
                    for msg in messages {
                        let is_my_message = if let Some(content_start) = msg.find("] ") {
                            if let Some(username_end) = msg[content_start + 2..].find(": ") {
                                let username = &msg[content_start + 2..content_start + 2 + username_end];
                                username == self.username
                            } else {
                                false
                            }
                        } else {
                            false
                        };

                        ui.horizontal(|ui| {
                            if is_my_message {
                                ui.with_layout(egui::Layout::right_to_left(egui::Align::TOP), |ui| {
                                    let (rect, _) = ui.allocate_exact_size(
                                        Vec2::new(ui.available_width().min(400.0), 40.0),
                                        egui::Sense::hover()
                                    );

                                    ui.painter().rect_filled(rect, 12.0, Color32::from_rgb(100, 150, 200));
                                    ui.painter().text(
                                        rect.min + Vec2::new(15.0, rect.height() / 2.0),
                                        egui::Align2::LEFT_CENTER,
                                        &msg,
                                        FontId::proportional(14.0),
                                        Color32::WHITE,
                                    );
                                });
                            } else {
                                let (rect, _) = ui.allocate_exact_size(
                                    Vec2::new(ui.available_width().min(400.0), 40.0),
                                    egui::Sense::hover()
                                );

                                ui.painter().rect_filled(rect, 12.0, Color32::from_rgb(80, 80, 90));
                                ui.painter().text(
                                    rect.min + Vec2::new(15.0, rect.height() / 2.0),
                                    egui::Align2::LEFT_CENTER,
                                    &msg,
                                    FontId::proportional(14.0),
                                    Color32::WHITE,
                                );
                            }
                        });
                        ui.add_space(5.0);
                    }
                });

            ui.separator();
            ui.add_space(10.0);

            ui.horizontal(|ui| {
                Self::rounded_text_edit(ui, &mut self.message_input, "Scrivi un messaggio...", ui.available_width() - 100.0);
                ui.add_space(10.0);

                if Self::rounded_button(ui, "Invia", Color32::from_rgb(100, 200, 150), Vec2::new(80.0, 40.0)) ||
                   ui.input(|i| i.key_pressed(egui::Key::Enter)) {
                    self.try_send_message();
                }
            });
        } else {
            ui.vertical_centered(|ui| {
                ui.add_space(100.0);
                ui.heading("Seleziona una chat");
                ui.label("Torna al menu per selezionare una chat o gruppo");

                ui.add_space(20.0);
                if Self::rounded_button(ui, "Torna al Menu", Color32::from_rgb(120, 120, 120), Vec2::new(150.0, 40.0)) {
                    self.current_screen = Screen::Menu;
                }
            });
        }
    }

}
impl eframe::App for ChatApp {
    fn update(&mut self, ctx: &egui::Context, _frame: &mut eframe::Frame) {
        ChatApp::setup_visuals(ctx);

        egui::CentralPanel::default().show(ctx, |ui| {
            match self.current_screen {
                Screen::Access => {
                    self.ui_access(ui);
                }
                Screen::Menu => {
                    self.ui_menu(ui);
                }
                Screen::Chat => {
                    self.ui_chat(ui);
                }
                Screen::GroupCreation => {
                    self.ui_group_creation(ui);
                }
            }
        });

        ctx.request_repaint_after(std::time::Duration::from_millis(100));
    }
}

fn main() {
    let options = eframe::NativeOptions {
        viewport: egui::ViewportBuilder::default()
            .with_inner_size([900.0, 650.0])
            .with_min_inner_size([600.0, 400.0])
            .with_title("RUGGINE Chat"),
        ..Default::default()
    };

    eframe::run_native(
        "RUGGINE",
        options,
        Box::new(|cc| {
            ChatApp::setup_visuals(&cc.egui_ctx);
            Box::new(ChatApp::default())
        }),
    ).unwrap();
}

