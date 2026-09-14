/*
Copyright © 2026 N3xtery, nixxoq

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#ifndef TL_CONSTRUCTORS_H
#define TL_CONSTRUCTORS_H

// Core MTProto & RPC
#define TL_VECTOR                        0x1cb5c415 // vector
#define TL_RPC_RESULT                    0xf35c6d01 // rpc_result
#define TL_RPC_ERROR                     0x2144ca19 // rpc_error
#define TL_MSG_CONTAINER                 0x73f1f8dc // msg_container
#define TL_BAD_MSG_NOTIFICATION          0xa7eff811 // bad_msg_notification
#define TL_NEW_SESSION_CREATED           0x9ec20908 // new_session_created
#define TL_INVOKE_WITH_LAYER             0xda9b0d0d // invokeWithLayer
#define TL_INIT_CONNECTION               0xc1cd5ea9 // initConnection

// Updates
#define TL_UPDATES_COMBINED              0x725b04c3 // updatesCombined
#define TL_UPDATES                       0x74ae4240 // updates
#define TL_UPDATE_SHORT                  0x78d4dec1 // updateShort
#define TL_UPDATES_TOO_LONG              0xe317af7e // updatesTooLong

// Dialogs
#define TL_MESSAGES_DIALOGS              0x15ba6c40 // messages.dialogs
#define TL_MESSAGES_DIALOGS_SLICE        0x71e094f3 // messages.dialogsSlice
#define TL_MESSAGES_DIALOGS_NOT_MODIFIED 0xf0e3e596 // messages.dialogsNotModified
#define TL_DIALOG                        0xfc89f7f3 // dialog
#define TL_DIALOG_FOLDER                 0x714350a0 // dialogFolder

// DialogFilter flags
#define FILTER_CONTACTS(flags)           (((flags) & (1 << 0)) != 0)
#define FILTER_NON_CONTACTS(flags)       (((flags) & (1 << 1)) != 0)
#define FILTER_GROUPS(flags)             (((flags) & (1 << 2)) != 0)
#define FILTER_BROADCASTS(flags)         (((flags) & (1 << 3)) != 0)
#define FILTER_BOTS(flags)               (((flags) & (1 << 4)) != 0)
#define EXCLUDE_MUTED(flags)             (((flags) & (1 << 11)) != 0)
#define EXCLUDE_READ(flags)              (((flags) & (1 << 12)) != 0)

// Peer Types
#define TL_PEER_USER                     0x59511722 // peerUser
#define TL_PEER_CHAT                     0x36c6019a // peerChat
#define TL_PEER_CHANNEL                  0xa2a5371e // peerChannel

#define MTPROTO_LAYER                    225
#define TL_HELP_GET_CONFIG               0xc4f9186b // help.getConfig

// Users
#define TL_USER                          0x31774388 // user
#define TL_USER_EMPTY                    0xd3bc4b7a // userEmpty

// Chats
#define TL_CHAT                          0x41cbf256 // chat
#define TL_CHAT_FORBIDDEN                0x6592a1a7 // chatForbidden
#define TL_CHAT_EMPTY                    0x29562865 // chatEmpty

// Channels & Megagroups
#define TL_CHANNEL                       0x1c32b11c // channel
#define TL_CHANNEL_FORBIDDEN             0x17d493d5 // channelForbidden

// Messages
#define TL_MESSAGE                       0x95ef6f2b // message
#define TL_MESSAGE_SERVICE               0x7a800e0a // messageService
#define TL_MESSAGE_EMPTY                 0x90a6ca84 // messageEmpty

// Messages Response (messages.messages, messages.messagesSlice, messages.channelMessages)
#define TL_MESSAGES_MESSAGES             0x1d73e7ea // messages.messages
#define TL_MESSAGES_MESSAGES_SLICE       0x5f206716 // messages.messagesSlice
#define TL_MESSAGES_CHANNEL_MESSAGES     0xc776ba4e // messages.channelMessages

// Photos
#define TL_USER_PROFILE_PHOTO            0x82d1f706 // userProfilePhoto
#define TL_CHAT_PHOTO                    0x1c6e1c11 // chatPhoto

// Booleans
#define TL_BOOL_FALSE                    0xbc799737 // boolFalse
#define TL_BOOL_TRUE                     0x997275b5 // boolTrue

// Notifications
#define TL_PEER_NOTIFY_SETTINGS          0x99622c0c // peerNotifySettings
#define TL_MUTE_FOREVER                  2147483647 // means 0x7fffffff, indefinite
#define NOTIFY_HAS_SHOW_PREVIEWS(flags)  (((flags) & (1 << 0)) != 0)
#define NOTIFY_HAS_SILENT(flags)         (((flags) & (1 << 1)) != 0)
#define NOTIFY_HAS_MUTE_UNTIL(flags)     (((flags) & (1 << 2)) != 0)

// Service Message Actions (MessageAction)
#define TL_ACTION_CHAT_JOINED_BY_LINK    0x031224c3 // messageActionChatJoinedByLink
#define TL_ACTION_TOPIC_CREATE           0x0d999256 // messageActionTopicCreate
#define TL_ACTION_CHAT_ADD_USER          0x15cefd00 // messageActionChatAddUser
#define TL_ACTION_SCREENSHOT_TAKEN       0x4792929b // messageActionScreenshotTaken
#define TL_ACTION_SET_CHAT_WALLPAPER     0x5060a3f4 // messageActionSetChatWallPaper
#define TL_ACTION_CHAT_EDIT_PHOTO        0x7fcb13a8 // messageActionChatEditPhoto
#define TL_ACTION_PHONE_CALL             0x80e11a7f // messageActionPhoneCall
#define TL_ACTION_PIN_MESSAGE            0x94bd38ed // messageActionPinMessage
#define TL_ACTION_CHANNEL_CREATE         0x95d2ac92 // messageActionChannelCreate
#define TL_ACTION_CHAT_DELETE_PHOTO      0x95e3fbef // messageActionChatDeletePhoto
#define TL_ACTION_HISTORY_CLEAR          0x9fbab604 // messageActionHistoryClear
#define TL_ACTION_CHAT_DELETE_USER       0xa43f30cc // messageActionChatDeleteUser
#define TL_ACTION_SET_CHAT_THEME         0xaa786345 // messageActionSetChatTheme
#define TL_ACTION_CHAT_EDIT_TITLE        0xb5a1ce5a // messageActionChatEditTitle
#define TL_ACTION_CHAT_CREATE            0xbd47cbad // messageActionChatCreate
#define TL_ACTION_TOPIC_EDIT             0xc0944820 // messageActionTopicEdit
#define TL_ACTION_CHAT_MIGRATE_TO        0xe1037f92 // messageActionChatMigrateTo
#define TL_ACTION_CHANNEL_MIGRATE_FROM   0xea3948e9 // messageActionChannelMigrateFrom
#define TL_ACTION_CUSTOM_ACTION          0xfae69f56 // messageActionCustomAction

// Message Media
#define TL_MEDIA_PHOTO                   0xe216eb63 // messageMediaPhoto
#define TL_MEDIA_DOCUMENT                0x52d8ccd9 // messageMediaDocument
#define TL_MEDIA_GEO                     0x56e0d474 // messageMediaGeo
#define TL_MEDIA_CONTACT                 0x70322949 // messageMediaContact
#define TL_MEDIA_POLL                    0x773f4e66 // messageMediaPoll

// Document Attributes (DocumentAttribute)
#define TL_ATT_IMAGE_SIZE                0x6c37c15c // documentAttributeImageSize
#define TL_ATT_ANIMATED                  0x11b58939 // documentAttributeAnimated
#define TL_ATT_FILENAME                  0x15590068 // documentAttributeFilename
#define TL_ATT_VIDEO                     0x43c57c48 // documentAttributeVideo
#define TL_ATT_STICKER                   0x6319d612 // documentAttributeSticker
#define TL_ATT_AUDIO                     0x9852f9c6 // documentAttributeAudio
#define TL_ATT_CUSTOM_EMOJI              0xfd149899 // documentAttributeCustomEmoji

// PhotoSize
#define TL_PHOTO_SIZE_PROGRESSIVE        0xfa3efb95 // photoSizeProgressive

// Other
#define TL_SUGGESTED_POST                0x0e8e37e5 // suggestedPost
#define TL_STARS_AMOUNT                  0xbbb6b4a3 // starsAmount

#endif // TL_CONSTRUCTORS_H
