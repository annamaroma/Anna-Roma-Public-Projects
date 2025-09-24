use serde::{Deserialize, Serialize};
use chrono::{DateTime, Utc};
use std::collections::{HashMap};

// Messagges sent from client to server
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "type")]
pub enum ClientMessage {
    // Access: if not subscribed, the system will keep track of the
    // new user, otherwise it will guarantee the access
    Access {
        username: String
    },

    // Logout advertisement of a user
    Logout {
        username: String
    },

    // Message sent to another user
    UserMessage {
        to: String,
        content: String
    },

    // Message sent to a group
    GroupMessage {
        to: String,
        content: String
    },

    // Request of username and group name related to an user
    ChatListRequest,

    UsersRequest,

    // Reset of the counter when a chat is opened
    ResetUnread {
        chat_id: String // username or "gruppo:group_name"
    },

    // History retrieve of a chat between two users
    UserHistory {
        with_user: String
    },

    // History retrieve of a chat with a group
    GroupHistory {
        with_group: String
    },

    // Group creation
    GroupCreate {
        group_name: String,
        members: Vec<String>
    },

    // Group leave
    GroupLeave {
        group_name: String
    },
}

/// Messaggi dal server al client
#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(tag = "type")]
pub enum ServerMessage {

    // Ack response
    Ack {
        details: String
    },

    Notify {
        from: String,
        to: String,
        content: String,
        timestamp: DateTime<Utc>
    },

    ChatList {
        users: Vec<String>,
        groups: Vec<String>,
        group_members: HashMap<String, Vec<String>>,
        unread_counts: HashMap<String, usize>,
        all_users: Vec<String>,
    },

    UserHistory {
        with_user: String,
        messages: Vec<Message>
    },

    GroupHistory {
        with_group: String,
        messages: Vec<Message>
    },

    Error {
        code: String,
        message: String
    },

    UsersList{
        all_users: Vec<String>,
    },

}


#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Message {
    pub from: String,
    pub to: String,
    pub content: String,
    pub timestamp: DateTime<Utc>,
}


