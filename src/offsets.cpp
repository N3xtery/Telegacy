/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

// Drafts & Topics
#define TL_DRAFT_MESSAGE_EMPTY              0x1b0c841a // draftMessageEmpty
#define TL_DRAFT_MESSAGE                    0x96eaa5eb // draftMessage
#define TL_INPUT_REPLY_TO_MSG               0x3bd4b7c2 // inputReplyToMessage
#define TL_INPUT_REPLY_TO_STORY             0x5881323a // inputReplyToStory
#define TL_FORUM_TOPIC                      0xfcdad815 // forumTopic
#define TL_FORUM_TOPIC_DELETED              0x023f109b // forumTopicDeleted

// Channel Emoji Status
#define TL_EMOJI_STATUS                     0xe7ff068a // emojiStatus
#define TL_EMOJI_STATUS_EMPTY               0x2de11aae // emojiStatusEmpty

// Media (Dice)
#define TL_MEDIA_DICE                       0x08cbec07 // messageMediaDice

// Keyboard Buttons
#define TL_KB_BUTTON                        0x7d170cff // keyboardButton
#define TL_KB_BUTTON_PHONE                  0x417efd8f // keyboardButtonRequestPhone
#define TL_KB_BUTTON_GEO                    0xaa40f94d // keyboardButtonRequestGeoLocation
#define TL_KB_BUTTON_GAME                   0x89c590f9 // keyboardButtonGame
#define TL_KB_BUTTON_BUY                    0x3fa53905 // keyboardButtonBuy
#define TL_KB_BUTTON_URL                    0xd80c25ec // keyboardButtonUrl
#define TL_KB_BUTTON_CALLBACK               0xe62bc960 // keyboardButtonCallback
#define TL_KB_BUTTON_WEBVIEW                0xe846b1a0 // keyboardButtonWebView
#define TL_KB_BUTTON_SIMPLE_WEBVIEW         0xe15c4370 // keyboardButtonSimpleWebView
#define TL_KB_BUTTON_COPY                   0xbcc4af10 // keyboardButtonCopy
#define TL_KB_BUTTON_SWITCH_INLINE          0x991399fc // keyboardButtonSwitchInline
#define TL_KB_BUTTON_URL_AUTH               0xf51006f9 // keyboardButtonUrlAuth
#define TL_INPUT_KB_BUTTON_URL_AUTH         0x68013e72 // inputKeyboardButtonUrlAuth
#define TL_KB_BUTTON_POLL                   0x7a11d782 // keyboardButtonRequestPoll
#define TL_INPUT_KB_BUTTON_USER             0x7d5e07c7 // inputKeyboardButtonUserProfile
#define TL_KB_BUTTON_USER                   0xc0fd5d09 // keyboardButtonUserProfile
#define TL_KB_BUTTON_REQUEST_PEER           0x5b0f15f5 // keyboardButtonRequestPeer
#define TL_INPUT_KB_BUTTON_REQUEST_PEER     0x02b78156 // inputKeyboardButtonRequestPeer

// Star Gifts & Attributes
#define TL_STAR_GIFT                        0x313a9547 // starGift
#define TL_STAR_GIFT_ATT_MODEL              0x565251e2 // starGiftAttributeModel
#define TL_STAR_GIFT_ATT_PATTERN            0x4e7085ea // starGiftAttributePattern
#define TL_STAR_GIFT_ATT_BACKDROP           0x9f2504e4 // starGiftAttributeBackdrop
#define TL_STAR_GIFT_ATT_ORIGINAL           0xe0bff26c // starGiftAttributeOriginalDetails

// Service Message Actions & Requests
#define TL_ACTION_WEBVIEW_DATA_SENT         0xb4c38cb5 // messageActionWebViewDataSent
#define TL_ACTION_GAME_SCORE                0x92a72876 // messageActionGameScore
#define TL_ACTION_GIVEAWAY_RESULTS          0x87e2f155 // messageActionGiveawayResults
#define TL_ACTION_PAYMENT_SENT_ME           0xffa00ccc // messageActionPaymentSentMe
#define TL_ACTION_PAYMENT_SENT              0xc624b16e // messageActionPaymentSent
#define TL_ACTION_BOT_ALLOWED               0xc516d679 // messageActionBotAllowed
#define TL_BOT_APP_NOT_MODIFIED             0x5da674b7 // botAppNotModified
#define TL_ACTION_SECURE_VALUES_SENT_ME     0x1b287353 // messageActionSecureValuesSentMe
#define TL_ACTION_SECURE_VALUES_SENT        0xd95c6154 // messageActionSecureValuesSent
#define TL_ACTION_GEO_PROXIMITY_REACHED     0x98e0d697 // messageActionGeoProximityReached
#define TL_ACTION_GROUP_CALL                0x7a0d7f42 // messageActionGroupCall
#define TL_ACTION_INVITE_TO_GROUP_CALL      0x502f92f7 // messageActionInviteToGroupCall
#define TL_ACTION_SET_MESSAGES_TTL          0x3c134d7b // messageActionSetMessagesTTL
#define TL_ACTION_GROUP_CALL_SCHEDULED      0xb3a07661 // messageActionGroupCallScheduled
#define TL_ACTION_WEBVIEW_DATA_SENT_ME      0x47dd8079 // messageActionWebViewDataSentMe
#define TL_ACTION_GIFT_PREMIUM              0x48e91302 // messageActionGiftPremium
#define TL_ACTION_SUGGEST_PROFILE_PHOTO     0x57de635e // messageActionSuggestProfilePhoto
#define TL_ACTION_REQUESTED_PEER            0x31518e9b // messageActionRequestedPeer
#define TL_ACTION_GIFT_CODE                 0x31c48347 // messageActionGiftCode
#define TL_ACTION_GIVEAWAY_LAUNCH           0xa80f51e4 // messageActionGiveawayLaunch
#define TL_ACTION_BOOST_APPLY               0xcc02aa6d // messageActionBoostApply
#define TL_ACTION_REQUESTED_PEER_SENT_ME    0x93b31848 // messageActionRequestedPeerSentMe
#define TL_REQUESTED_PEER_USER              0xd62ff46a // requestedPeerUser
#define TL_REQUESTED_PEER_CHANNEL           0x8ba403e4 // requestedPeerChannel
#define TL_ACTION_PAYMENT_REFUNDED          0x41b3e202 // messageActionPaymentRefunded
#define TL_ACTION_GIFT_STARS                0x45d5b021 // messageActionGiftStars
#define TL_ACTION_PRIZE_STARS               0xb00c47a2 // messageActionPrizeStars
#define TL_ACTION_STAR_GIFT                 0xea2c31d3 // messageActionStarGift
#define TL_ACTION_STAR_GIFT_UNIQUE          0xe6c31522 // messageActionStarGiftUnique

// Reply Headers
#define TL_REPLY_STORY_HEADER               0x0e5af939 // messageReplyStoryHeader

// Send Message Actions
#define TL_SEND_MSG_UPLOAD_VIDEO            0xe9763aec // sendMessageUploadVideoAction
#define TL_SEND_MSG_UPLOAD_AUDIO            0xf351d7ab // sendMessageUploadAudioAction
#define TL_SEND_MSG_UPLOAD_PHOTO            0xd1d34a26 // sendMessageUploadPhotoAction
#define TL_SEND_MSG_UPLOAD_ROUND            0x243e1c66 // sendMessageUploadRoundAction
#define TL_SEND_MSG_HISTORY_IMPORT          0xdbda9246 // sendMessageHistoryImportAction
#define TL_SEND_MSG_EMOJI_INTERACTION_SEEN  0xb665902e // sendMessageEmojiInteractionSeen
#define TL_SEND_MSG_EMOJI_INTERACTION       0x25972bcb // sendMessageEmojiInteraction

// Peer Notifications & Bot Menu & Invites & Reactions
#define TL_NOTIFICATION_SOUND_LOCAL         0x830b9ae4 // notificationSoundLocal
#define TL_NOTIFICATION_SOUND_RINGTONE      0xff6c8049 // notificationSoundRingtone
#define TL_BOT_MENU_BUTTON                  0xc7b57ce6 // botMenuButton
#define TL_CHAT_INVITE_EXPORTED             0xa22cbd96 // chatInviteExported
#define TL_CHAT_REACTIONS_SOME              0x661d4037 // chatReactionsSome
#define TL_REACTION_EMOJI                   0x1b2286b8 // reactionEmoji
#define TL_REACTION_CUSTOM_EMOJI            0x8935fc73 // reactionCustomEmoji
#define TL_CHAT_REACTIONS_NONE              0xeafc32bc // chatReactionsNone

// Core Input Peers & Users & Channels
#define TL_INPUT_PEER_CHAT                  0x35a95cb9 // inputPeerChat
#define TL_INPUT_PEER_USER                  0xdde8a54c // inputPeerUser
#define TL_INPUT_PEER_CHANNEL               0x27bcbbfc // inputPeerChannel
#define TL_INPUT_PEER_USER_FROM_MESSAGE     0xa87b0a1c // inputPeerUserFromMessage
#define TL_INPUT_PEER_CHANNEL_FROM_MESSAGE  0xbd2a0840 // inputPeerChannelFromMessage
#define TL_INPUT_USER                       0xf21158c6 // inputUser
#define TL_INPUT_USER_FROM_MESSAGE          0x1da448e2 // inputUserFromMessage
#define TL_INPUT_CHANNEL                    0xf35aec28 // inputChannel
#define TL_INPUT_CHANNEL_FROM_MESSAGE       0x5b934f9d // inputChannelFromMessage

// Geo & Empty Objects
#define TL_GEO_POINT                        0xb2a2f663 // geoPoint
#define TL_PHOTO_EMPTY                      0x2331b22d // photoEmpty
#define TL_DOCUMENT_EMPTY                   0x36f8c871 // documentEmpty

// RichText
#define TL_TEXT_EMPTY                       0xdc3d824f // textEmpty
#define TL_TEXT_PLAIN                       0x744694e0 // textPlain
#define TL_TEXT_URL                         0x3c2884c1 // textUrl
#define TL_TEXT_EMAIL                       0xde5a0dd6 // textEmail
#define TL_TEXT_PHONE                       0x1ccb966a // textPhone
#define TL_TEXT_ANCHOR                      0x35553762 // textAnchor
#define TL_TEXT_CONCAT                      0x7e6260d7 // textConcat
#define TL_TEXT_IMAGE                       0x081ccf4f // textImage

// Message Entities
#define TL_ENTITY_MENTION_NAME              0xdc7b1140 // messageEntityMentionName
#define TL_ENTITY_INPUT_MENTION_NAME        0x208e68c9 // inputMessageEntityMentionName
#define TL_ENTITY_BOLD                      0xbd610bc9 // messageEntityBold
#define TL_ENTITY_ITALIC                    0x826f8b60 // messageEntityItalic
#define TL_ENTITY_UNDERLINE                 0x9c4e7e8b // messageEntityUnderline
#define TL_ENTITY_STRIKE                    0xbf0693d4 // messageEntityStrike
#define TL_ENTITY_BLOCKQUOTE                0xf1ccaaac // messageEntityBlockquote
#define TL_ENTITY_CODE                      0x28a20571 // messageEntityCode
#define TL_ENTITY_PRE                       0x73924be0 // messageEntityPre
#define TL_ENTITY_URL                       0x6ed02538 // messageEntityUrl
#define TL_ENTITY_TEXT_URL                  0x76a6d327 // messageEntityTextUrl
#define TL_ENTITY_SPOILER                   0x32ca960f // messageEntitySpoiler
#define TL_ENTITY_CUSTOM_EMOJI              0xc8cf05f8 // messageEntityCustomEmoji

// Extended Media, Secure File, Wallpaper
#define TL_MSG_EXTENDED_MEDIA_PREVIEW       0xad628cc8 // messageExtendedMediaPreview
#define TL_SECURE_FILE_EMPTY                0x64199744 // secureFileEmpty
#define TL_WALLPAPER                        0xa437c3ed // wallPaper

// Reply Markups & Request Peer Types & Keyboard Styles
#define TL_REPLY_KEYBOARD_HIDE              0xa03e5b85 // replyKeyboardHide
#define TL_REPLY_KEYBOARD_FORCE_REPLY       0x86b40b08 // replyKeyboardForceReply
#define TL_REPLY_KEYBOARD_MARKUP            0x85dd99d1 // replyKeyboardMarkup
#define TL_REPLY_INLINE_MARKUP              0x48a30254 // replyInlineMarkup
#define TL_REQUEST_PEER_TYPE_USER           0x5f3b8a00 // requestPeerTypeUser
#define TL_REQUEST_PEER_TYPE_CHAT           0xc9f06e1b // requestPeerTypeChat
#define TL_REQUEST_PEER_TYPE_BROADCAST      0x339bef6c // requestPeerTypeBroadcast
#define TL_KB_BUTTON_STYLE                  0x4fdd3430 // keyboardButtonStyle

// Input Sticker Sets
#define TL_INPUT_STICKER_SET_ID             0x9de7a269 // inputStickerSetID
#define TL_INPUT_STICKER_SET_SHORT_NAME     0x861cc8a0 // inputStickerSetShortName
#define TL_INPUT_STICKER_SET_DICE           0xe67f520e // inputStickerSetDice

// Photo & Video Sizes
#define TL_PHOTO_SIZE                       0x75c78e60 // photoSize
#define TL_PHOTO_CACHED_SIZE                0x021e1ad6 // photoCachedSize
#define TL_PHOTO_STRIPPED_SIZE              0xe0b0bc2e // photoStrippedSize
#define TL_PHOTO_PATH_SIZE                  0xd8214d41 // photoPathSize
#define TL_VIDEO_SIZE                       0xde33b094 // videoSize
#define TL_VIDEO_SIZE_EMOJI_MARKUP          0xf85c413c // videoSizeEmojiMarkup
#define TL_VIDEO_SIZE_STICKER_MARKUP        0x0da082fe // videoSizeStickerMarkup

// Story & Privacy & Media Areas
#define TL_STORY_ITEM_DELETED               	0x51e6ee4f // storyItemDeleted
#define TL_STORY_ITEM_SKIPPED               	0xffadc913 // storyItemSkipped
#define TL_PRIVACY_ALLOW_USERS              	0xb8905fb2 // privacyValueAllowUsers
#define TL_PRIVACY_DISALLOW_USERS           	0xe4621141 // privacyValueDisallowUsers
#define TL_PRIVACY_ALLOW_CHAT_PARTICIPANTS  	0x6b134e8e // privacyValueAllowChatParticipants
#define TL_PRIVACY_DISALLOW_CHAT_PARTICIPANTS 	0x41c87565 // privacyValueDisallowChatParticipants
#define TL_MEDIA_AREA_VENUE                 	0xbe82db9c // mediaAreaVenue
#define TL_MEDIA_AREA_GEO_POINT             	0xcad5452d // mediaAreaGeoPoint
#define TL_MEDIA_AREA_SUGGESTED_REACTION    	0x14455871 // mediaAreaSuggestedReaction
#define TL_MEDIA_AREA_CHANNEL_POST          	0x770416af // mediaAreaChannelPost
#define TL_INPUT_MEDIA_AREA_VENUE           	0xb282217f // inputMediaAreaVenue
#define TL_INPUT_MEDIA_AREA_CHANNEL_POST    	0x2271f2bf // inputMediaAreaChannelPost
#define TL_MEDIA_AREA_URL                   	0x37381085 // mediaAreaUrl
#define TL_MEDIA_AREA_WEATHER               	0x49a6549c // mediaAreaWeather

// Message Media
#define TL_MEDIA_VENUE                      0x2ec0533f // messageMediaVenue
#define TL_MEDIA_GEO_LIVE                   0xb940c666 // messageMediaGeoLive
#define TL_MEDIA_WEB_PAGE                   0xddf10c3b // messageMediaWebPage
#define TL_MEDIA_GAME                       0xfdb19008 // messageMediaGame
#define TL_MEDIA_INVOICE                    0xf6a548d3 // messageMediaInvoice
#define TL_MEDIA_STORY                      0x68cb6283 // messageMediaStory
#define TL_MEDIA_GIVEAWAY                   0xaa073beb // messageMediaGiveaway
#define TL_MEDIA_GIVEAWAY_RESULTS           0xceaa3ea1 // messageMediaGiveawayResults
#define TL_MEDIA_PAID_MEDIA                 0xa8852491 // messageMediaPaidMedia
#define TL_WEB_PAGE_EMPTY                   0x211a1788 // webPageEmpty
#define TL_WEB_PAGE_PENDING                 0xb0d13e47 // webPagePending
#define TL_WEB_PAGE                         0xe89c45b2 // webPage
#define TL_WEB_PAGE_NOT_MODIFIED            0x7311ca11 // webPageNotModified
#define TL_WEB_PAGE_ATT_THEME               0x54b56617 // webPageAttributeTheme
#define TL_WEB_PAGE_ATT_STORY               0x2e94c3e7 // webPageAttributeStory
#define TL_WEB_PAGE_ATT_STICKER_SET         0x50cc03d3 // webPageAttributeStickerSet
#define TL_WEB_DOCUMENT                     0x1c570ed1 // webDocument
#define TL_POLL                             0x966e2dbf // poll
#define TL_POLL_ANSWER                      0x4b7d786a // pollAnswer
#define TL_POLL_ANSWER_VOTERS               0x3645230a // pollAnswerVoters
#define TL_POLL_RESULTS                     0xba7bb15e // pollResults

// Page Blocks (Instant View)
#define TL_PAGE_BLOCK_UNSUPPORTED           0x13567e8a // pageBlockUnsupported
#define TL_PAGE_BLOCK_DIVIDER               0xdb20b188 // pageBlockDivider
#define TL_PAGE_BLOCK_AUTHOR_DATE           0xbaafe5e0 // pageBlockAuthorDate
#define TL_PAGE_BLOCK_PREFORMATTED          0xc070d93e // pageBlockPreformatted
#define TL_PAGE_BLOCK_ANCHOR                0xce0d37b0 // pageBlockAnchor
#define TL_PAGE_BLOCK_LIST                  0xe4e88011 // pageBlockList
#define TL_PAGE_LIST_ITEM_TEXT              0xb92fb6cd // pageListItemText
#define TL_PAGE_BLOCK_BLOCKQUOTE            0x263d7c26 // pageBlockBlockquote
#define TL_PAGE_BLOCK_PULLQUOTE             0x4f4456d3 // pageBlockPullquote
#define TL_PAGE_BLOCK_PHOTO                 0x1759c560 // pageBlockPhoto
#define TL_PAGE_BLOCK_VIDEO                 0x7c8fe7b6 // pageBlockVideo
#define TL_PAGE_BLOCK_COVER                 0x39f23300 // pageBlockCover
#define TL_PAGE_BLOCK_EMBED                 0xa8718dc5 // pageBlockEmbed
#define TL_PAGE_BLOCK_EMBED_POST            0xf259a80b // pageBlockEmbedPost
#define TL_PAGE_BLOCK_COLLAGE               0x65a0fa4d // pageBlockCollage
#define TL_PAGE_BLOCK_SLIDESHOW             0x031f9590 // pageBlockSlideshow
#define TL_PAGE_BLOCK_CHANNEL               0xef1751b5 // pageBlockChannel
#define TL_PAGE_BLOCK_AUDIO                 0x804361ea // pageBlockAudio
#define TL_PAGE_BLOCK_DETAILS               0x76768bed // pageBlockDetails
#define TL_PAGE_BLOCK_RELATED_ARTICLES      0x16115a96 // pageBlockRelatedArticles
#define TL_PAGE_BLOCK_MAP                   0xa44f3ef6 // pageBlockMap
#define TL_PAGE_BLOCK_ORDERED_LIST          0x9a8ae1e1 // pageBlockOrderedList
#define TL_PAGE_LIST_ORDERED_ITEM_TEXT      0x5e068047 // pageListOrderedItemText
#define TL_PAGE_BLOCK_TABLE                 0xbf4dea82 // pageBlockTable

int draftmessage_offset(BYTE* buf) {
	int cons = read_le(buf, 4);
	if (cons == TL_DRAFT_MESSAGE_EMPTY) {
		int flags = read_le(buf + 4, 4);
		return 8 + ((flags & 1) ? 4 : 0);
	} else if (cons == TL_DRAFT_MESSAGE) {
		int flags = read_le(buf + 4, 4);
		int offset = 8;
		if (flags & (1 << 4)) { // reply_to:InputReplyTo
			int rcons = read_le(buf + offset, 4);
			offset += 4;
			if (rcons == TL_INPUT_REPLY_TO_MSG) {
				int rflags = read_le(buf + offset, 4);
				offset += 8;
				if (rflags & (1 << 0)) offset += 4;
				if (rflags & (1 << 1)) offset += 12;
				if (rflags & (1 << 2)) offset += tlstr_len(buf + offset, true);
				if (rflags & (1 << 3)) {
					int qcount = read_le(buf + offset + 4, 4);
					offset += 8;
					for (int k = 0; k < qcount; k++) offset += msgent_offset(buf + offset, NULL);
				}
				if (rflags & (1 << 4)) offset += 4;
				if (rcons == TL_INPUT_REPLY_TO_MSG) {
					if (rflags & (1 << 5)) offset += inputpeer_offset(buf + offset);
					if (rflags & (1 << 6)) offset += 4;
					if (rflags & (1 << 7)) offset += tlstr_len(buf + offset, true);
				}
			} else if (rcons == TL_INPUT_REPLY_TO_STORY) offset += 16;
		}
		offset += tlstr_len(buf + offset, true);
		if (flags & (1 << 3)) {
			int ecount = read_le(buf + offset + 4, 4);
			offset += 8;
			for (int k = 0; k < ecount; k++) offset += msgent_offset(buf + offset, NULL);
		}
		if (flags & (1 << 5)) offset += place_inputmedia(buf + offset, NULL, 0);
		offset += 4;
		if (flags & (1 << 7)) offset += 8;
		if (flags & (1 << 8)) offset += 4;
		return offset;
	}
	return 4;
}

int forumtopic_offset(BYTE* unenc_response, ForumTopic* topic) {
	int offset = 0;
	int cons = read_le(unenc_response, 4);
	offset += 4;
	if (cons == TL_FORUM_TOPIC_DELETED) {
		if (topic) topic->id = read_le(unenc_response + offset, 4);
		offset += 4;
		return offset;
	}
	if (cons == TL_FORUM_TOPIC) {
		int flags = read_le(unenc_response + offset, 4);
		offset += 4;
		if (topic) {
			topic->closed = (flags & (1 << 2)) ? true : false;
			topic->pinned = (flags & (1 << 3)) ? true : false;
			topic->hidden = (flags & (1 << 6)) ? true : false;
			topic->title_missing = (flags & (1 << 7)) ? true : false;
			topic->id = read_le(unenc_response + offset, 4);
		}
		offset += 4; // id
		offset += 4; // date
		offset += 12; // peer:Peer
		if (topic) {
			topic->title = read_string(unenc_response + offset, NULL);
		}
		offset += tlstr_len(unenc_response + offset, true); // title:string
		if (topic) {
			topic->icon_color = read_le(unenc_response + offset, 4);
		}
		offset += 4; // icon_color:int
		if (flags & (1 << 0)) {
			if (topic) topic->icon_emoji_id = read_le(unenc_response + offset, 8);
			offset += 8; // icon_emoji_id:long
		} else if (topic) topic->icon_emoji_id = 0;
		if (topic) topic->top_message = read_le(unenc_response + offset, 4);
		offset += 4; // top_message:int
		if (topic) topic->read_inbox_max_id = read_le(unenc_response + offset, 4);
		offset += 4; // read_inbox_max_id:int
		offset += 4; // read_outbox_max_id:int
		if (topic) topic->unread_count = read_le(unenc_response + offset, 4);
		offset += 4; // unread_count:int
		if (topic) topic->unread_mentions_count = read_le(unenc_response + offset, 4);
		offset += 4; // unread_mentions_count:int
		if (topic) topic->unread_reactions_count = read_le(unenc_response + offset, 4);
		offset += 4; // unread_reactions_count:int
		offset += 4; // unread_poll_votes_count:int
		offset += 12; // from_id:Peer
		offset += peernotifyset_offset(unenc_response + offset); // notify_settings:PeerNotifySettings
		if (flags & (1 << 4)) {
			offset += draftmessage_offset(unenc_response + offset); // draft:DraftMessage
		}
		return offset;
	}
	return 4;
}

int msgfwd_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int flags_msgfwd = read_le(unenc_response + offset_msg + 4, 4);
	offset_msg += 8;
	if (flags_msgfwd & (1 << 0)) offset_msg += 12;
	if (flags_msgfwd & (1 << 5)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
	offset_msg += 4;
	if (flags_msgfwd & (1 << 2)) offset_msg += 4;
	if (flags_msgfwd & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
	if (flags_msgfwd & (1 << 4)) offset_msg += 16;
	if (flags_msgfwd & (1 << 8)) offset_msg += 12;
	if (flags_msgfwd & (1 << 9)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
	if (flags_msgfwd & (1 << 10)) offset_msg += 4;
	if (flags_msgfwd & (1 << 6)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
	return offset_msg;
}

int inputstickerset_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int sticker_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (sticker_cons) {
	case TL_INPUT_STICKER_SET_ID:
		offset_msg += 16;
		break;
	case TL_INPUT_STICKER_SET_SHORT_NAME:
	case TL_INPUT_STICKER_SET_DICE:
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	return offset_msg;
}

int photo_video_size_offset(BYTE* unenc_response, bool photo, bool photo_vec, bool video) {
	int offset_msg = 0;
	if (photo) {
		int count = 1;
		if (photo_vec) {
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
		}
		for (int j = 0; j < count; j++) {
			int photosize_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			switch (photosize_cons) {
			case TL_PHOTO_SIZE:
				offset_msg += 12;
				break;
			case TL_PHOTO_CACHED_SIZE:
				offset_msg += 8;
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				break;
			case TL_PHOTO_STRIPPED_SIZE:
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				break;
			case TL_PHOTO_SIZE_PROGRESSIVE:
				offset_msg += 12;
				offset_msg += read_le(unenc_response + offset_msg, 4) * 4 + 4;
				break;
			case TL_PHOTO_PATH_SIZE:
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				break;
			}
		}
	}
	if (video) {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int j = 0; j < count; j++) {
			int videosize_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			switch (videosize_cons) {
			case TL_VIDEO_SIZE: {
				int flags_videosize = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				offset_msg += 12;
				if (flags_videosize & (1 << 0)) offset_msg += 8;
				break;
			}
			case TL_VIDEO_SIZE_EMOJI_MARKUP:
				offset_msg += 12;
				offset_msg += read_le(unenc_response + offset_msg, 4) * 4 + 4;
				break;
			case TL_VIDEO_SIZE_STICKER_MARKUP:
				offset_msg += inputstickerset_offset(unenc_response + offset_msg);
				offset_msg += 12;
				offset_msg += read_le(unenc_response + offset_msg, 4) * 4 + 4;
				break;
			}
		}
	}
	return offset_msg;
}

int docatt_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int docatt_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (docatt_cons) {
	case TL_ATT_IMAGE_SIZE:
		offset_msg += 8;
		break;
	case TL_ATT_STICKER: {
		int flags_docattstick = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += inputstickerset_offset(unenc_response + offset_msg);
		if (flags_docattstick & (1 << 0)) offset_msg += 32;
		break;
	}
	case TL_ATT_VIDEO: {
		int flags_docattvid = read_le(unenc_response + offset_msg, 4);
		offset_msg += 20;
		if (flags_docattvid & (1 << 2)) offset_msg += 4;
		if (flags_docattvid & (1 << 4)) offset_msg += 8;
		if (flags_docattvid & (1 << 5)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case TL_ATT_AUDIO: {
		int flags_docattaud = read_le(unenc_response + offset_msg, 4);
		offset_msg += 8;
		if (flags_docattaud & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_docattaud & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_docattaud & (1 << 2)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case TL_ATT_FILENAME:
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case TL_ATT_CUSTOM_EMOJI:
		offset_msg += 4;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += inputstickerset_offset(unenc_response + offset_msg);
		break;
	}
	return offset_msg;
}

int doc_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int doc_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (doc_cons == TL_DOCUMENT_EMPTY) offset_msg += 8;
	else {
		int flags_doc = read_le(unenc_response + offset_msg, 4);
		offset_msg += 20;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 4;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 8;
		offset_msg += photo_video_size_offset(unenc_response + offset_msg, (flags_doc & (1 << 0)) ? true : false, true, (flags_doc & (1 << 1)) ? true : false);
		offset_msg += 4;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int j = 0; j < count; j++) offset_msg += docatt_offset(unenc_response + offset_msg);
	}
	return offset_msg;
}

int photo_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	if (read_le(unenc_response + offset_msg, 4) == TL_PHOTO_EMPTY)
		offset_msg += 12;
	else {
		int flags_photo = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 24;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 4;
		offset_msg += photo_video_size_offset(unenc_response + offset_msg, true, true, (flags_photo & (1 << 1)) ? true : false);
		offset_msg += 4;
	}
	return offset_msg;
}

int richtext_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int richtext_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (richtext_cons) {
	case TL_TEXT_EMPTY:
		break;
	case TL_TEXT_PLAIN:
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case TL_TEXT_URL:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 8;
		break;
	case TL_TEXT_EMAIL:
	case TL_TEXT_PHONE:
	case TL_TEXT_ANCHOR:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case TL_TEXT_CONCAT: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += richtext_offset(unenc_response + offset_msg);
		break;
	}
	case TL_TEXT_IMAGE:
		offset_msg += 16;
		break;
	default:
		offset_msg += richtext_offset(unenc_response + offset_msg);
	}
	return offset_msg;
}

int pagecaption_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	offset_msg += 4;
	offset_msg += richtext_offset(unenc_response + offset_msg);
	offset_msg += richtext_offset(unenc_response + offset_msg);
	return offset_msg;
}

int inputpeer_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int inputpeer_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (inputpeer_cons) {
	case TL_INPUT_PEER_CHAT:
		offset_msg += 8;
		break;
	case TL_INPUT_PEER_USER:
	case TL_INPUT_PEER_CHANNEL:
		offset_msg += 16;
		break;
	case TL_INPUT_PEER_USER_FROM_MESSAGE:
	case TL_INPUT_PEER_CHANNEL_FROM_MESSAGE:
		offset_msg += inputpeer_offset(unenc_response + offset_msg);
		offset_msg += 12;
		break;
	}
	return offset_msg;
}

int chatphoto_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int chatphoto_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (chatphoto_cons == TL_CHAT_PHOTO || chatphoto_cons == TL_USER_PROFILE_PHOTO) {
		int flags_chatphoto = read_le(unenc_response + offset_msg, 4);
		offset_msg += 12;
		if (flags_chatphoto & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 4;
	}
	return offset_msg;
}

int geo_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int geo_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (geo_cons == TL_GEO_POINT) {
		int flags_geo = read_le(unenc_response + offset_msg, 4);
		offset_msg += 28;
		if (flags_geo & (1 << 0)) offset_msg += 4;
	}
	return offset_msg;
}

int inputchannel_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int inputchannel_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (inputchannel_cons == TL_INPUT_CHANNEL) offset_msg += 16;
	else if (inputchannel_cons == TL_INPUT_CHANNEL_FROM_MESSAGE) {
		offset_msg += inputpeer_offset(unenc_response + offset_msg);
		offset_msg += 12;
	}
	return offset_msg;
}

int pageblock_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int pageblock_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (pageblock_cons) {
	case TL_PAGE_BLOCK_UNSUPPORTED:
	case TL_PAGE_BLOCK_DIVIDER:
		break;
	case TL_PAGE_BLOCK_AUTHOR_DATE:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += 4;
		break;
	case TL_PAGE_BLOCK_PREFORMATTED:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case TL_PAGE_BLOCK_ANCHOR:
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case TL_PAGE_BLOCK_LIST: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int pageblock_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (pageblock_cons == TL_PAGE_LIST_ITEM_TEXT) offset_msg += richtext_offset(unenc_response + offset_msg);
			else {
				int count2 = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int j = 0; j < count2; j++) offset_msg += pageblock_offset(unenc_response + offset_msg);
			}
		}
		break;
	}
	case TL_PAGE_BLOCK_BLOCKQUOTE:
	case TL_PAGE_BLOCK_PULLQUOTE:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += richtext_offset(unenc_response + offset_msg);
		break;
	case TL_PAGE_BLOCK_PHOTO: {
		int flags = read_le(unenc_response + offset_msg, 4);
		offset_msg += 12;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		if (flags & (1 << 0)) {
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 8;
		}
		break;
	}
	case TL_PAGE_BLOCK_VIDEO:
		offset_msg += 12;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	case TL_PAGE_BLOCK_COVER:
		offset_msg += pageblock_offset(unenc_response + offset_msg);
		break;
	case TL_PAGE_BLOCK_EMBED: {
		int flags = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags & (1 << 2)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags & (1 << 4)) offset_msg += 8;
		if (flags & (1 << 5)) offset_msg += 8;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	}
	case TL_PAGE_BLOCK_EMBED_POST: {
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 16;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 4;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += pageblock_offset(unenc_response + offset_msg);
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	}
	case TL_PAGE_BLOCK_COLLAGE:
	case TL_PAGE_BLOCK_SLIDESHOW: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += pageblock_offset(unenc_response + offset_msg);
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	}
	case TL_PAGE_BLOCK_CHANNEL: {
		int chat_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		switch (chat_cons) {
		case TL_CHAT_EMPTY:
			offset_msg += 8;
			break;
		case TL_CHAT: {
			int flags = read_le(unenc_response + offset_msg, 4);
			offset_msg += 12;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += chatphoto_offset(unenc_response + offset_msg);
			offset_msg += 12;
			if (flags & (1 << 6)) offset_msg += inputchannel_offset(unenc_response + offset_msg);
			if (flags & (1 << 14)) offset_msg += 8;
			if (flags & (1 << 18)) offset_msg += 12;
			break;
		}
		case TL_CHAT_FORBIDDEN:
			offset_msg += 8;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		case TL_CHANNEL: {
			int flags = read_le(unenc_response + offset_msg, 4);
			int flags2 = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 16;
			if (flags & (1 << 13)) offset_msg += 8;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags & (1 << 6)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += chatphoto_offset(unenc_response + offset_msg);
			offset_msg += 4;
			if (flags & (1 << 9)) {
				int count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int i = 0; i < count; i++) {
					offset_msg += 4;
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
				}
			}
			if (flags & (1 << 14)) offset_msg += 8;
			if (flags & (1 << 15)) offset_msg += 12;
			if (flags & (1 << 18)) offset_msg += 12;
			if (flags & (1 << 17)) offset_msg += 4;
			if (flags2 & (1 << 0)) {
				int count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int i = 0; i < count; i++) {
					offset_msg += 8;
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
				}
			}
			if (flags2 & (1 << 4)) offset_msg += 4;
			int peercolor_count = 0;
			if (flags2 & (1 << 7)) peercolor_count++;
			if (flags2 & (1 << 8)) peercolor_count++;
			for (int i = 0; i < peercolor_count; i++) {
				int flags_peercolor = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				if (flags_peercolor & (1 << 0)) offset_msg += 4;
				if (flags_peercolor & (1 << 1)) offset_msg += 8;
			}
			if (flags2 & (1 << 9)) {
				int emoji_cons = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				if (emoji_cons == TL_EMOJI_STATUS) {
					int flags_em = read_le(unenc_response + offset_msg, 4);
					offset_msg += 12; // flags (4) + document_id (8)
					if (flags_em & (1 << 0)) offset_msg += 4; // until:int
				} else if (emoji_cons == TL_EMOJI_STATUS_EMPTY) {
					// empty
				}
			}
			if (flags2 & (1 << 10)) offset_msg += 4;
			if (flags2 & (1 << 11)) offset_msg += 4;
			if (flags2 & (1 << 13)) offset_msg += 8;
			if (flags2 & (1 << 14)) offset_msg += 8;
			if (flags2 & (1 << 18)) offset_msg += 8;
			if (flags2 & (1 << 20)) offset_msg += 8;
			break;
		}
		case TL_CHANNEL_FORBIDDEN: {
			int flags = read_le(unenc_response + offset_msg, 4);
			offset_msg += 20;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags & (1 << 16)) offset_msg += 4;
			break;
		}
		}
		break;
	}
	case TL_PAGE_BLOCK_AUDIO:
		offset_msg += 8;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	case TL_PAGE_BLOCK_TABLE: {
		offset_msg += 4;
		offset_msg += richtext_offset(unenc_response + offset_msg);
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			offset_msg += 4;
			int count2 = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int j = 0; j < count2; j++) {
				offset_msg += 4;
				int flags = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				if (flags & (1 << 7)) offset_msg += richtext_offset(unenc_response + offset_msg);
				if (flags & (1 << 1)) offset_msg += 4;
				if (flags & (1 << 2)) offset_msg += 4;
			}
		}
		break;
	}
	case TL_PAGE_BLOCK_ORDERED_LIST: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int list_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (list_cons == TL_PAGE_LIST_ORDERED_ITEM_TEXT) offset_msg += richtext_offset(unenc_response + offset_msg);
			else {
				int count2 = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int j = 0; j < count2; j++) offset_msg += pageblock_offset(unenc_response + offset_msg);
			}
		}
		break;
	}
	case TL_PAGE_BLOCK_DETAILS: {
		offset_msg += 4;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += pageblock_offset(unenc_response + offset_msg);
		offset_msg += richtext_offset(unenc_response + offset_msg);
		break;
	}
	case TL_PAGE_BLOCK_RELATED_ARTICLES: {
		offset_msg += richtext_offset(unenc_response + offset_msg);
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++)	{
			offset_msg += 4;
			int flags = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 8;
			if (flags & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags & (1 << 2)) offset_msg += 8;
			if (flags & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags & (1 << 4)) offset_msg += 4;
		}
		break;
	}
	case TL_PAGE_BLOCK_MAP:
		offset_msg += geo_offset(unenc_response + offset_msg);
		offset_msg += 12;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	default:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		break;
	}
	return offset_msg;
}

int wallset_offset(BYTE* unenc_response) {
	int offset_msg = 4;
	int flags_wallset = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (flags_wallset & (1 << 0)) offset_msg += 4;
	if (flags_wallset & (1 << 4)) offset_msg += 4;
	if (flags_wallset & (1 << 5)) offset_msg += 4;
	if (flags_wallset & (1 << 6)) offset_msg += 4;
	if (flags_wallset & (1 << 3)) offset_msg += 4;
	if (flags_wallset & (1 << 4)) offset_msg += 4;
	if (flags_wallset & (1 << 7)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
	return offset_msg;
}

int medareacoords_offset(BYTE* unenc_response) {
	int offset_msg = 4;
	int flags = read_le(unenc_response + offset_msg, 4);
	offset_msg += 40;
	if (flags & (1 << 0)) offset_msg += 8;
	return offset_msg;
}

int reaction_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int reaction_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (reaction_cons == TL_REACTION_EMOJI) offset_msg += tlstr_len(unenc_response + offset_msg, true);
	else if (reaction_cons == TL_REACTION_CUSTOM_EMOJI) offset_msg += 8;
	return offset_msg;
}

int msgextmed_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int msgextmed_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (msgextmed_cons == TL_MSG_EXTENDED_MEDIA_PREVIEW) {
		int flags = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += 8;
		if (flags & (1 << 1)) offset_msg += photo_video_size_offset(unenc_response + offset_msg, true, false, false);
		if (flags & (1 << 2)) offset_msg += 4;
	} else {
		offset_msg += messagemedia_offset(unenc_response + offset_msg);
	}
	return offset_msg;
}

int inputuser_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int inputuser_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (inputuser_cons == TL_INPUT_USER) offset_msg += 16;
	else if (inputuser_cons == TL_INPUT_USER_FROM_MESSAGE) {
		offset_msg += inputpeer_offset(unenc_response + offset_msg);
		offset_msg += 12;
	}
	return offset_msg;
}

int msgent_offset(BYTE* unenc_response, std::vector<int>* format_vecs) {
	int offset_msg = 0;
	int msgent_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (msgent_cons) {
	case TL_ENTITY_MENTION_NAME:
		offset_msg += 16;
		break;
	case TL_ENTITY_INPUT_MENTION_NAME: {
		offset_msg += 8;
		offset_msg += inputuser_offset(unenc_response + offset_msg);
		break;
	}
	case TL_ENTITY_BOLD:
	case TL_ENTITY_ITALIC:
	case TL_ENTITY_UNDERLINE:
	case TL_ENTITY_STRIKE:
	case TL_ENTITY_BLOCKQUOTE:
	case TL_ENTITY_CODE:
	case TL_ENTITY_SPOILER:
	case TL_ENTITY_URL:
	case TL_ENTITY_TEXT_URL:
	case TL_ENTITY_PRE:
	case TL_ENTITY_CUSTOM_EMOJI: {
		if (msgent_cons == TL_ENTITY_BLOCKQUOTE) offset_msg += 4;
		int format_start = read_le(unenc_response + offset_msg, 4);
		int format_length = read_le(unenc_response + offset_msg + 4, 4);
		int index = 0;
		if (msgent_cons == TL_ENTITY_ITALIC) index = 1;
		else if (msgent_cons == TL_ENTITY_UNDERLINE) index = 2;
		else if (msgent_cons == TL_ENTITY_STRIKE) index = 3;
		else if (msgent_cons == TL_ENTITY_BLOCKQUOTE) index = 4;
		else if (msgent_cons == TL_ENTITY_CODE) index = 5;
		else if (msgent_cons == TL_ENTITY_PRE) {
			index = 5;
			offset_msg += tlstr_len(unenc_response + offset_msg + 8, true);
		} else if (msgent_cons == TL_ENTITY_SPOILER) index = 6;
		else if (msgent_cons == TL_ENTITY_URL) index = 7;
		else if (msgent_cons == TL_ENTITY_TEXT_URL) {
			if (format_vecs) {
				index = 8;
				wchar_t* url = read_string(unenc_response + offset_msg + 8, NULL);
				TEXTRANGE tr;
				tr.lpstrText = url;
				tr.chrg.cpMin = -1;
				links.push_back(tr);
			}
			offset_msg += tlstr_len(unenc_response + offset_msg + 8, true);
		}
		if (msgent_cons == TL_ENTITY_CUSTOM_EMOJI) {
			if (format_vecs) {
				format_vecs[9].push_back(format_start);
				format_vecs[9].push_back(format_length);
				int id_1 = read_le(unenc_response + offset_msg + 8, 4);
				int id_2 = read_le(unenc_response + offset_msg + 12, 4);
				format_vecs[9].push_back(id_1);
				format_vecs[9].push_back(id_2);
			}
			offset_msg += 8;
		}
		else if (format_vecs) {
			format_vecs[index].push_back(format_start);
			format_vecs[index].push_back(format_start + format_length);
		}
		offset_msg += 8;
		break;
	}
	default:
		offset_msg += 8;
		break;
	}
	return offset_msg;
}

int textwithent_offset(BYTE* unenc_response) {
	int offset_msg = 4;
	offset_msg += tlstr_len(unenc_response + offset_msg, true);
	int count = read_le(unenc_response + offset_msg + 4, 4);
	offset_msg += 8;
	for (int i = 0; i < count; i++) offset_msg += msgent_offset(unenc_response + offset_msg, NULL);
	return offset_msg;
}

int story_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int story_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (story_cons == TL_STORY_ITEM_DELETED) offset_msg += 4;
	else if (story_cons == TL_STORY_ITEM_SKIPPED) offset_msg += 16;
	else {
		int flags_story = read_le(unenc_response + offset_msg, 4);
		offset_msg += 12;
		if (flags_story & (1 << 18)) offset_msg += 12;
		if (flags_story & (1 << 17)) {
			offset_msg += 4;
			int flags_storyfwd = read_le(unenc_response + offset_msg, 4);
			if (flags_storyfwd & (1 << 0)) offset_msg += 12;
			if (flags_storyfwd & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_storyfwd & (1 << 2)) offset_msg += 4;
		}
		offset_msg += 4;
		if (flags_story & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_story & (1 << 1)) {
			int count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int k = 0; k < count; k++) msgent_offset(unenc_response + offset_msg, NULL);
		}
		offset_msg += messagemedia_offset(unenc_response + offset_msg);
		if (flags_story & (1 << 14)) {
			int count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int k = 0; k < count; k++) {
				int msgarea_cons = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				switch (msgarea_cons) {
				case TL_MEDIA_AREA_VENUE: {
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += geo_offset(unenc_response + offset_msg);
					for (int l = 0; l < 5; l++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
					break;
				}
				case TL_INPUT_MEDIA_AREA_VENUE:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += 8;
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
					break;
				case TL_MEDIA_AREA_GEO_POINT: {
					int flags_medareageo = read_le(unenc_response + offset_msg, 4);
					offset_msg += 4;
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += geo_offset(unenc_response + offset_msg);
					if (flags_medareageo & (1 << 0)) {
						offset_msg += 4;
						int flags_geoadd = read_le(unenc_response + offset_msg, 4);
						offset_msg += tlstr_len(unenc_response + offset_msg, true);
						if (flags_geoadd & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
						if (flags_geoadd & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
						if (flags_geoadd & (1 << 2)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
					}
					break;
				}
				case TL_MEDIA_AREA_SUGGESTED_REACTION:
					offset_msg += 4;
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += reaction_offset(unenc_response + offset_msg);
					break;
				case TL_MEDIA_AREA_CHANNEL_POST:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += 12;
					break;
				case TL_INPUT_MEDIA_AREA_CHANNEL_POST:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += inputchannel_offset(unenc_response + offset_msg);
					offset_msg += 4;
					break;
				case TL_MEDIA_AREA_URL:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
					break;
				case TL_MEDIA_AREA_WEATHER:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
					offset_msg += 12;
					break;
				}
			}
		}
		if (flags_story & (1 << 2)) {
			int count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int k = 0; k < count; k++) {
				int prvrule_cons = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				switch (prvrule_cons) {
				case TL_PRIVACY_ALLOW_USERS:
				case TL_PRIVACY_DISALLOW_USERS:
				case TL_PRIVACY_ALLOW_CHAT_PARTICIPANTS:
				case TL_PRIVACY_DISALLOW_CHAT_PARTICIPANTS: {
					int count2 = read_le(unenc_response + offset_msg + 4, 4);
					offset_msg += 8;
					for (int l = 0; l < count2; l++) offset_msg += 8;
				}
				}

			}
		}
		if (flags_story & (1 << 3)) {
			offset_msg += 4;
			int flags_storyviews = read_le(unenc_response + offset_msg, 4);
			offset_msg += 8;
			if (flags_storyviews & (1 << 2)) offset_msg += 4;
			if (flags_storyviews & (1 << 3)) {
				int count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int k = 0; k < count; k++) {
					offset_msg += 4;
					int flags_reactcount = read_le(unenc_response + offset_msg, 4);
					offset_msg += 4;
					if (flags_reactcount & (1 << 0)) offset_msg += 4;
					offset_msg += reaction_offset(unenc_response + offset_msg);
					offset_msg += 4;
				}
			}
			if (flags_storyviews & (1 << 4)) offset_msg += 4;
			if (flags_storyviews & (1 << 0)) {
				int count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int k = 0; k < count; k++) offset_msg += 8;
			}
		}
		if (flags_story & (1 << 15)) offset_msg += reaction_offset(unenc_response + offset_msg);
	}
	return offset_msg;
}

int messagemedia_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int msgmed_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (msgmed_cons) {
	case TL_MEDIA_PHOTO: {
		int flags_medphoto = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_medphoto & (1 << 0)) offset_msg += photo_offset(unenc_response + offset_msg);
		if (flags_medphoto & (1 << 2)) offset_msg += 4;
		if (flags_medphoto & (1 << 4)) offset_msg += doc_offset(unenc_response + offset_msg);
		break;
	}
	case TL_MEDIA_GEO: {
		offset_msg += geo_offset(unenc_response + offset_msg);
		break;
	}
	case TL_MEDIA_CONTACT: {
		for (int j = 0; j < 4; j++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 8;
		break;
	}
	case TL_MEDIA_DOCUMENT: {
		int flags_doc1 = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_doc1 & (1 << 0)) offset_msg += doc_offset(unenc_response + offset_msg);
		if (flags_doc1 & (1 << 5)) {
			int count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int j = 0; j < count; j++) offset_msg += doc_offset(unenc_response + offset_msg);
		}
		if (flags_doc1 & (1 << 9)) offset_msg += photo_offset(unenc_response + offset_msg);
		if (flags_doc1 & (1 << 10)) offset_msg += 4;
		if (flags_doc1 & (1 << 2)) offset_msg += 4;
		break;
	}
	case TL_MEDIA_WEB_PAGE: {
		offset_msg += 4;
		int web_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		int flags_web = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		switch (web_cons) {
		case TL_WEB_PAGE_EMPTY:
			offset_msg += 8;
			if (flags_web & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		case TL_WEB_PAGE_PENDING:
			offset_msg += 8;
			if (flags_web & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			break;
		case TL_WEB_PAGE: {
			offset_msg += 8;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			if (flags_web & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_web & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_web & (1 << 2)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_web & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_web & (1 << 4)) offset_msg += photo_offset(unenc_response + offset_msg);
			if (flags_web & (1 << 5)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_web & (1 << 5)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_web & (1 << 6)) offset_msg += 4;
			if (flags_web & (1 << 6)) offset_msg += 4;
			if (flags_web & (1 << 7)) offset_msg += 4;
			if (flags_web & (1 << 8)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_web & (1 << 9)) offset_msg += doc_offset(unenc_response + offset_msg);
			if (flags_web & (1 << 10)) {
				int flags_page = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				int count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				int j;
				for (j = 0; j < count; j++) offset_msg += pageblock_offset(unenc_response + offset_msg);
				count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (j = 0; j < count; j++) offset_msg += photo_offset(unenc_response + offset_msg);
				count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (j = 0; j < count; j++) offset_msg += doc_offset(unenc_response + offset_msg);
				if (flags_page & (1 << 3)) offset_msg += 4;
			}
			if (flags_web & (1 << 12)) {
				int count = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int j = 0; j < count; j++) {
					int webatt_cons = read_le(unenc_response + offset_msg, 4);
					offset_msg += 4;
					switch (webatt_cons) {
					case TL_WEB_PAGE_ATT_THEME: {
						int flags_webatt = read_le(unenc_response + offset_msg, 4);
						offset_msg += 4;
						if (flags_webatt & (1 << 0)) {
							int count2 = read_le(unenc_response + offset_msg + 4, 4);
							offset_msg += 8;
							for (int k = 0; k < count2; k++) offset_msg += doc_offset(unenc_response + offset_msg);
						}
						if (flags_webatt & (1 << 1)) {
							offset_msg += 4;
							int flags_theme = read_le(unenc_response + offset_msg, 4);
							offset_msg += 12;
							if (flags_theme & (1 << 3)) offset_msg += 4;
							if (flags_theme & (1 << 0)) {
								int count2 = read_le(unenc_response + offset_msg + 4, 4);
								offset_msg += 8;
								for (int k = 0; k < count2; k++) offset_msg += 4;
							}
							if (flags_theme & (1 << 1)) offset_msg += wallpaper_offset(unenc_response + offset_msg);
						}
						break;
					}
					case TL_WEB_PAGE_ATT_STORY: {
						int flags_webatt = read_le(unenc_response + offset_msg, 4);
						offset_msg += 20;
						if (flags_webatt & (1 << 0)) offset_msg += story_offset(unenc_response + offset_msg);
						break;
					}
					case TL_WEB_PAGE_ATT_STICKER_SET: {
						offset_msg += 4;
						int count = read_le(unenc_response + offset_msg + 4, 4);
						offset_msg += 8;
						for (int k = 0; k < count; k++) offset_msg += doc_offset(unenc_response + offset_msg);
						break;
					}
					}
				}
			}
			break;
		}
		case TL_WEB_PAGE_NOT_MODIFIED: {
			int flags_web = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_web & (1 << 0)) offset_msg += 4;
			break;
		}
		}
		break;
	}
	case TL_MEDIA_VENUE: {
		offset_msg += geo_offset(unenc_response + offset_msg);
		for (int i = 0; i < 5; i++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case TL_MEDIA_GAME: {
		offset_msg += 4;
		int flags_game = read_le(unenc_response + offset_msg, 4);
		offset_msg += 20;
		for (int i = 0; i < 3; i++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += photo_offset(unenc_response + offset_msg);
		if (flags_game & (1 << 0)) offset_msg += doc_offset(unenc_response + offset_msg);
		break;
	}
	case TL_MEDIA_INVOICE: {
		int flags_invoice = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		for (int i = 0; i < 2; i++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_invoice & (1 << 0)) {
			int webdoc_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += (webdoc_cons == TL_WEB_DOCUMENT) ? 12 : 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			int count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += docatt_offset(unenc_response + offset_msg);
		}
		if (flags_invoice & (1 << 2)) offset_msg += 4;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 8;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_invoice & (1 << 4)) offset_msg += msgextmed_offset(unenc_response + offset_msg);
		break;
	}
	case TL_MEDIA_GEO_LIVE: {
		int flags_geolive = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		offset_msg += geo_offset(unenc_response + offset_msg);
		if (flags_geolive & (1 << 0)) offset_msg += 4;
		offset_msg += 4;
		if (flags_geolive & (1 << 1)) offset_msg += 4;
		break;
	}
	case TL_MEDIA_POLL: {
		int flags_medpoll = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		int poll_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		offset_msg += 8;
		int flags_poll = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		offset_msg += textwithent_offset(unenc_response + offset_msg);
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int pa_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			int pa_flags = 0;
			if (pa_cons == TL_POLL_ANSWER) {
				pa_flags = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
			}
			offset_msg += textwithent_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (pa_cons == TL_POLL_ANSWER) {
				if (pa_flags & (1 << 0)) offset_msg += messagemedia_offset(unenc_response + offset_msg);
				if (pa_flags & (1 << 1)) offset_msg += 16;
			}
		}
		if (flags_poll & (1 << 4)) offset_msg += 4;
		if (flags_poll & (1 << 5)) offset_msg += 4;
		if (poll_cons == TL_POLL) {
			if (flags_poll & (1 << 12)) {
				int ccount = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int c = 0; c < ccount; c++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			}
			offset_msg += 8;
		}
		int pollres_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		int flags_pollres = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_pollres & (1 << 1)) {
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) {
				int pav_cons = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				int pav_flags = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				if (pav_cons == TL_POLL_ANSWER_VOTERS) {
					if (pav_flags & (1 << 2)) {
						offset_msg += 4;
						int rv_count = read_le(unenc_response + offset_msg + 4, 4);
						offset_msg += 8;
						offset_msg += rv_count * 12;
					}
				} else {
					offset_msg += 4;
				}
			}
		}
		if (flags_pollres & (1 << 2)) offset_msg += 4;
		if (flags_pollres & (1 << 3)) {
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += 12;
		}
		if (flags_pollres & (1 << 4)) {
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += msgent_offset(unenc_response + offset_msg, NULL);
		}
		if (pollres_cons == TL_POLL_RESULTS && (flags_pollres & (1 << 5))) {
			offset_msg += messagemedia_offset(unenc_response + offset_msg);
		}
		if (flags_medpoll & (1 << 0)) {
			offset_msg += messagemedia_offset(unenc_response + offset_msg);
		}
		break;
	}
	case TL_MEDIA_DICE: {
		int flags_dice = read_le(unenc_response + offset_msg, 4);
		offset_msg += 8;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_dice & (1 << 0)) {
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 16;
		}
		break;
	}
	case TL_MEDIA_STORY: {
		int flags_story = read_le(unenc_response + offset_msg, 4);
		offset_msg += 20;
		if (flags_story & (1 << 0)) offset_msg += story_offset(unenc_response + offset_msg);
		break;
	}
	case TL_MEDIA_GIVEAWAY: {
		int flags_give = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 8;
		if (flags_give & (1 << 1)) {
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		}
		if (flags_give & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 4;
		if (flags_give & (1 << 4)) offset_msg += 4;
		if (flags_give & (1 << 5)) offset_msg += 8;
		offset_msg += 4;
		break;
	}
	case TL_MEDIA_GIVEAWAY_RESULTS: {
		int flags_give = read_le(unenc_response + offset_msg, 4);
		offset_msg += 12;
		if (flags_give & (1 << 3)) offset_msg += 4;
		offset_msg += 12;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 8;
		if (flags_give & (1 << 4)) offset_msg += 4;
		if (flags_give & (1 << 5)) offset_msg += 8;
		if (flags_give & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 4;
		break;
	}
	case TL_MEDIA_PAID_MEDIA: {
		offset_msg += 8;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += msgextmed_offset(unenc_response + offset_msg);
		break;
	}
	}
	return offset_msg;
}

int requestpeertype_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int rpt_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	int flags_rpt = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (rpt_cons) {
	case TL_REQUEST_PEER_TYPE_USER:
		if (flags_rpt & (1 << 0)) offset_msg += 4;
		if (flags_rpt & (1 << 1)) offset_msg += 4;
		break;
	case TL_REQUEST_PEER_TYPE_CHAT:
		if (flags_rpt & (1 << 3)) offset_msg += 4;
		if (flags_rpt & (1 << 4)) offset_msg += 4;
		if (flags_rpt & (1 << 1)) offset_msg += 8;
		if (flags_rpt & (1 << 2)) offset_msg += 8;
		break;
	case TL_REQUEST_PEER_TYPE_BROADCAST:
		if (flags_rpt & (1 << 3)) offset_msg += 4;
		if (flags_rpt & (1 << 1)) offset_msg += 8;
		if (flags_rpt & (1 << 2)) offset_msg += 8;
		break;
	}
	return offset_msg;
}

int keyboardbuttonstyle_offset(BYTE* unenc_response) {
	int s_cons = read_le(unenc_response, 4);
	if (s_cons == TL_KB_BUTTON_STYLE) {
		int s_flags = read_le(unenc_response + 4, 4);
		int off = 8;
		if (s_flags & (1 << 3)) off += 8; // icon:long
		return off;
	}
	return 4;
}

int keybutrow_offset(BYTE* unenc_response) {
	int offset_msg = 4;
	int count = read_le(unenc_response + offset_msg + 4, 4);
	offset_msg += 8;
	if (count <= 0 || count > 100) return offset_msg;
	for (int i = 0; i < count; i++) {
		int keybut_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		switch (keybut_cons) {
		case TL_KB_BUTTON:
		case TL_KB_BUTTON_PHONE:
		case TL_KB_BUTTON_GEO:
		case TL_KB_BUTTON_GAME:
		case TL_KB_BUTTON_BUY: {
			int flags_kb = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_kb & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		}
		case TL_KB_BUTTON_URL:
		case TL_KB_BUTTON_CALLBACK:
		case TL_KB_BUTTON_WEBVIEW:
		case TL_KB_BUTTON_SIMPLE_WEBVIEW:
		case TL_KB_BUTTON_COPY: {
			int flags_kbc = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_kbc & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		}
		case TL_KB_BUTTON_SWITCH_INLINE: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_keybut & (1 << 1)) {
				int count2 = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int j = 0; j < count2; j++) offset_msg += 4;
			}
			break;
		}
		case TL_KB_BUTTON_URL_AUTH: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_keybut & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			break;
		}
		case TL_INPUT_KB_BUTTON_URL_AUTH: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_keybut & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += inputuser_offset(unenc_response + offset_msg);
			break;
		}
		case TL_KB_BUTTON_POLL: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			if (flags_keybut & (1 << 0)) offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		}
		case TL_INPUT_KB_BUTTON_USER: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += inputuser_offset(unenc_response + offset_msg);
			break;
		}
		case TL_KB_BUTTON_USER: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 8;
			break;
		}
		case TL_KB_BUTTON_REQUEST_PEER: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			offset_msg += requestpeertype_offset(unenc_response + offset_msg);
			offset_msg += 4;
			break;
		}
		case TL_INPUT_KB_BUTTON_REQUEST_PEER: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 10)) offset_msg += keyboardbuttonstyle_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			offset_msg += requestpeertype_offset(unenc_response + offset_msg);
			offset_msg += 4;
			break;
		}
		}
	}
	return offset_msg;
}

int replymarkup_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int reply_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (reply_cons) {
	case TL_REPLY_KEYBOARD_HIDE:
		offset_msg += 4;
		break;
	case TL_REPLY_KEYBOARD_FORCE_REPLY: {
		int flags_reply = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_reply & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case TL_REPLY_KEYBOARD_MARKUP: {
		int flags_reply = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		if (count > 0 && count < 100) {
			for (int i = 0; i < count; i++) offset_msg += keybutrow_offset(unenc_response + offset_msg);
		}
		if (flags_reply & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case TL_REPLY_INLINE_MARKUP: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		if (count > 0 && count < 100) {
			for (int i = 0; i < count; i++) offset_msg += keybutrow_offset(unenc_response + offset_msg);
		}
		break;
	}
	}
	return offset_msg;
}

int msgreact_offset(BYTE* unenc_response) {
	int offset_msg = 4;
	int flags_msgreact = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	int count = read_le(unenc_response + offset_msg + 4, 4);
	offset_msg += 8;
	int i;
	for (i = 0; i < count; i++) {
		int flags_reactc = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		if (flags_reactc & (1 << 0)) offset_msg += 4;
		offset_msg += reaction_offset(unenc_response + offset_msg);
		offset_msg += 4;
	}
	if (flags_msgreact & (1 << 1)) {
		count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (i = 0; i < count; i++) {
			offset_msg += 24;
			offset_msg += reaction_offset(unenc_response + offset_msg);
		}
	}
	if (flags_msgreact & (1 << 4)) {
		count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (i = 0; i < count; i++) {
			int flags_reactor = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			if (flags_reactor & (1 << 3)) offset_msg += 12;
			offset_msg += 4;
		}
	}
	return offset_msg;
}

int securefile_offset(BYTE* message) {
	int offset_msg = 4;
	if (read_le(message + offset_msg - 4, 4) == TL_SECURE_FILE_EMPTY) return offset_msg;
	offset_msg += 32;
	for (int j = 0; j < 2; j++) offset_msg += tlstr_len(message + offset_msg, true);
	return offset_msg;
}

int wallpaper_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int wallpaper_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (wallpaper_cons == TL_WALLPAPER) {
		offset_msg += 8;
		int flags_wallpaper = read_le(unenc_response + offset_msg, 4);
		offset_msg += 12;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += doc_offset(unenc_response + offset_msg);
		if (flags_wallpaper & (1 << 2)) offset_msg += wallset_offset(unenc_response + offset_msg);
	} else {
		offset_msg += 8;
		int flags_wallpaper = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_wallpaper & (1 << 2)) offset_msg += wallset_offset(unenc_response + offset_msg);
	}
	return offset_msg;
}

int stargift_offset(BYTE* message) {
	int offset_msg = 0;
	int stargift_cons = read_le(message + offset_msg, 4);
	if (stargift_cons == TL_STAR_GIFT) {
		int stargift_flags = read_le(message + offset_msg + 4, 4);
		offset_msg += 16;
		offset_msg += doc_offset(message + offset_msg);
		offset_msg += 16;
		if (stargift_flags & (1 << 0)) offset_msg += 8;
		if (stargift_flags & (1 << 4)) offset_msg += 16;
		if (stargift_flags & (1 << 1)) offset_msg += 8;
		if (stargift_flags & (1 << 3)) offset_msg += 8;
		if (stargift_flags & (1 << 5)) offset_msg += tlstr_len(message + offset_msg, true);
		if (stargift_flags & (1 << 6)) offset_msg += 12;
		if (stargift_flags & (1 << 8)) offset_msg += 8;
		if (stargift_flags & (1 << 9)) offset_msg += 4;
	} else {
		offset_msg += 12;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 12;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int stargiftatt_cons = read_le(message + offset_msg, 4);
			offset_msg += 4;
			if (stargiftatt_cons == TL_STAR_GIFT_ATT_MODEL || stargiftatt_cons == TL_STAR_GIFT_ATT_PATTERN) {
				offset_msg += tlstr_len(message + offset_msg, true);
				offset_msg += doc_offset(message + offset_msg);
				offset_msg += 4;
			} else if (stargiftatt_cons == TL_STAR_GIFT_ATT_BACKDROP) {
				offset_msg += tlstr_len(message + offset_msg, true);
				offset_msg += 20;
			} else if (stargiftatt_cons == TL_STAR_GIFT_ATT_ORIGINAL) {
				int startgiftatt_flags = read_le(message + offset_msg, 4);
				offset_msg += 4;
				if (startgiftatt_flags & (1 << 0)) offset_msg += 8;
				offset_msg += 12;
				if (startgiftatt_flags & (1 << 1)) offset_msg += textwithent_offset(message + offset_msg);
			}
		}
		offset_msg += 8;
	}
	return offset_msg;
}

int msgact_offset(BYTE* message) {
	int msgact_cons = read_le(message, 4);
	int offset_msg = 4;
	switch (msgact_cons) {
	case TL_ACTION_CHAT_CREATE:
	case TL_ACTION_CHAT_EDIT_TITLE: {
		offset_msg += tlstr_len(message + offset_msg, true);
		if (msgact_cons == TL_ACTION_CHAT_CREATE) {
			int count = read_le(message + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += 8;
		}
		break;
	}
	case TL_ACTION_CHANNEL_CREATE: {
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case TL_ACTION_CHAT_EDIT_PHOTO:
		offset_msg += photo_offset(message + offset_msg);
		break;
	case TL_ACTION_CHAT_DELETE_PHOTO:
		break;
	case TL_ACTION_CHAT_ADD_USER: {
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 8;
		break;
	}
	case TL_ACTION_CHAT_DELETE_USER:
		offset_msg += 8;
		break;
	case TL_ACTION_CHAT_JOINED_BY_LINK:
		offset_msg += 8;
		break;
	case TL_ACTION_CUSTOM_ACTION:
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case TL_ACTION_WEBVIEW_DATA_SENT:
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case TL_ACTION_CHAT_MIGRATE_TO:
		offset_msg += 8;
		break;
	case TL_ACTION_CHANNEL_MIGRATE_FROM:
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		break;
	case TL_ACTION_GAME_SCORE:
	case TL_ACTION_GIVEAWAY_RESULTS:
		offset_msg += 12;
		break;
	case TL_ACTION_PAYMENT_SENT_ME: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 0)) {
			int flags_pri = read_le(message + offset_msg + 4, 4);
			offset_msg += 8;
			if (flags_pri & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
			if (flags_pri & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
			if (flags_pri & (1 << 2)) offset_msg += tlstr_len(message + offset_msg, true);
			if (flags_pri & (1 << 3)) {
				offset_msg += 4;
				for (int i = 0; i < 6; i++) offset_msg += tlstr_len(message + offset_msg, true);
			}
		}
		if (flags & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 4)) offset_msg += 4;
		break;
	}
	case TL_ACTION_PAYMENT_SENT: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 4)) offset_msg += 4;
		break;
	}
	case TL_ACTION_PHONE_CALL: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 12;
		if (flags & (1 << 0)) offset_msg += 4;
		if (flags & (1 << 1)) offset_msg += 4;
		break;
	}
	case TL_ACTION_BOT_ALLOWED: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 2)) {
			int bot_cons = read_le(message + offset_msg, 4);
			offset_msg += 4;
			if (bot_cons == TL_BOT_APP_NOT_MODIFIED) break;
			int flags_bot = read_le(message + offset_msg, 4);
			offset_msg += 20;
			for (int i = 0; i < 3; i++) offset_msg += tlstr_len(message + offset_msg, true);
			offset_msg += photo_offset(message + offset_msg);
			if (flags_bot & (1 << 0)) offset_msg += doc_offset(message + offset_msg);
			offset_msg += 8;
		}
		break;
	}
	case TL_ACTION_SECURE_VALUES_SENT_ME: {
		int i;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (i = 0; i < count; i++) {
			offset_msg += 4;
			int flags = read_le(message + offset_msg, 4);
			offset_msg += 8;
			if (flags & (1 << 0)) {
				offset_msg += 4;
				for (int j = 0; j < 3; j++) offset_msg += tlstr_len(message + offset_msg, true);
			}
			for (int k = 1; k < 4; k++) if (flags & (1 << k)) offset_msg += securefile_offset(message + offset_msg);
			for (int l = 0; l < 2; l++) {
				if ((l == 0 && (flags & (1 << 6))) || (l == 1 && (flags & (1 << 4)))) {
					int count2 = read_le(message + offset_msg + 4, 4);
					offset_msg += 8;
					for (int j = 0; j < count2; j++) offset_msg += securefile_offset(message + offset_msg);
				}
			}
			if (flags & (1 << 5)) {
				offset_msg += 4;
				offset_msg += tlstr_len(message + offset_msg, true);
			}
			offset_msg += tlstr_len(message + offset_msg, true);
		}
		offset_msg += 4;
		for (i = 0; i < 3; i++) offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case TL_ACTION_SECURE_VALUES_SENT: {
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 4;
		break;			 
	}
	case TL_ACTION_GEO_PROXIMITY_REACHED:
		offset_msg += 28;
		break;
	case TL_ACTION_GROUP_CALL: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 24;
		if (flags & (1 << 0)) offset_msg += 4;
		break;
	}
	case TL_ACTION_INVITE_TO_GROUP_CALL: {
		offset_msg += 20;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 8;
		break;
	}
	case TL_ACTION_SET_MESSAGES_TTL: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 8;
		if (flags & (1 << 0)) offset_msg += 8;
		break;
	}
	case TL_ACTION_GROUP_CALL_SCHEDULED:
		offset_msg += 24;
		break;
	case TL_ACTION_SET_CHAT_THEME:
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case TL_ACTION_WEBVIEW_DATA_SENT_ME: {
		for (int i = 0; i < 2; i++) offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case TL_ACTION_GIFT_PREMIUM: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 12;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 0)) offset_msg += 8;
		if (flags & (1 << 1)) offset_msg += textwithent_offset(message + offset_msg);
		break;
	}
	case TL_ACTION_TOPIC_CREATE: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += 8;
		break;
	}
	case TL_ACTION_TOPIC_EDIT: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 1)) offset_msg += 8;
		if (flags & (1 << 2)) offset_msg += 4;
		if (flags & (1 << 3)) offset_msg += 4;
		break;
	}
	case TL_ACTION_SUGGEST_PROFILE_PHOTO:
		offset_msg += photo_offset(message + offset_msg);
		break;
	case TL_ACTION_REQUESTED_PEER: {
		offset_msg += 4;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 12;
		break;
	}
	case TL_ACTION_SET_CHAT_WALLPAPER: {
		offset_msg += 4;
		offset_msg += wallpaper_offset(message + offset_msg);
		break;
	}
	case TL_ACTION_GIFT_CODE: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 1)) offset_msg += 12;
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 2)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 2)) offset_msg += 8;
		if (flags & (1 << 3)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 3)) offset_msg += 8;
		if (flags & (1 << 4)) offset_msg += textwithent_offset(message + offset_msg);
		break;
	}
	case TL_ACTION_GIVEAWAY_LAUNCH: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += 8;
		break;
	}
	case TL_ACTION_BOOST_APPLY:
		offset_msg += 4;
		break;
	case TL_ACTION_REQUESTED_PEER_SENT_ME: {
		offset_msg += 4;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int rpeer_cons = read_le(message + offset_msg, 4);
			offset_msg += 4;
			int flags = read_le(message + offset_msg, 4);
			offset_msg += 12;
			if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
			if (rpeer_cons == TL_REQUESTED_PEER_USER) {
				if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
				if (flags & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
			} else if (rpeer_cons == TL_REQUESTED_PEER_CHANNEL) {
				if (flags & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
			}
			if (flags & (1 << 2)) offset_msg += photo_offset(message + offset_msg);
		}
		break;
	}
	case TL_ACTION_PAYMENT_REFUNDED: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 16;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 4;
		for (int i = 0; i < 2; i++) offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case TL_ACTION_GIFT_STARS: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 16;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 0)) offset_msg += 8;
		if (flags & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case TL_ACTION_PRIZE_STARS:
		offset_msg += 12;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 16;
		break;
	case TL_ACTION_STAR_GIFT: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += stargift_offset(message + offset_msg);
		if (flags & (1 << 1)) offset_msg += textwithent_offset(message + offset_msg);
		if (flags & (1 << 4)) offset_msg += 8;
		if (flags & (1 << 5)) offset_msg += 4;
		if (flags & (1 << 8)) offset_msg += 8;
		break;
	}
	case TL_ACTION_STAR_GIFT_UNIQUE: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += stargift_offset(message + offset_msg);
		if (flags & (1 << 3)) offset_msg += 4;
		if (flags & (1 << 4)) offset_msg += 8;
		break;
	}
	} 
	return offset_msg;
}

int msgrpl_offset(BYTE* message) {
	int offset_msg = 0;
	if (read_le(message + offset_msg, 4) == TL_REPLY_STORY_HEADER) offset_msg += 20;
	else {
		int flags_msgrpl = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		if (flags_msgrpl & (1 << 4)) offset_msg += 4;
		if (flags_msgrpl & (1 << 0)) offset_msg += 12;
		if (flags_msgrpl & (1 << 5)) offset_msg += msgfwd_offset(message + offset_msg);
		if (flags_msgrpl & (1 << 8)) offset_msg += messagemedia_offset(message + offset_msg);
		if (flags_msgrpl & (1 << 1)) offset_msg += 4;
		if (flags_msgrpl & (1 << 6)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags_msgrpl & (1 << 7)) {
			int count = read_le(message + offset_msg + 4, 4);
			offset_msg += 8;
			for (int j = 0; j < count; j++) offset_msg += msgent_offset(message + offset_msg, NULL);
		}
		if (flags_msgrpl & (1 << 10)) offset_msg += 4;
		if (flags_msgrpl & (1 << 11)) offset_msg += 4;
		if (flags_msgrpl & (1 << 12)) offset_msg += tlstr_len(message + offset_msg, true);
	}
	return offset_msg;
}

int sendmsgaction_offset(BYTE* message) {
	int offset_msg = 4;
	int cons = read_le(message, 4);
	switch (cons) {
	case TL_SEND_MSG_UPLOAD_VIDEO:
	case TL_SEND_MSG_UPLOAD_AUDIO:
	case TL_SEND_MSG_UPLOAD_PHOTO:
	case TL_SEND_MSG_UPLOAD_ROUND:
	case TL_SEND_MSG_HISTORY_IMPORT:
		offset_msg += 4;
		break;
	case TL_SEND_MSG_EMOJI_INTERACTION_SEEN:
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case TL_SEND_MSG_EMOJI_INTERACTION:
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	return offset_msg;
}

int peernotifyset_offset(BYTE* unenc_response) {
	int offset = 0;
	int flags_pns = read_le(unenc_response + offset + 4, 4);
	offset += 8;
	for (int j = 0; j < 11; j++) {
		if (flags_pns & (1 << j)) {
			if (j < 3 || j == 6 || j== 7) offset += 4;
			else {
				int notsound_cons = read_le(unenc_response + offset, 4);
				offset += 4;
				if (notsound_cons == TL_NOTIFICATION_SOUND_LOCAL) {
					offset += tlstr_len(unenc_response + offset, true);
					offset += tlstr_len(unenc_response + offset, true);
				} else if (notsound_cons == TL_NOTIFICATION_SOUND_RINGTONE) offset += 8;
			}
		}
	}
	return offset;
}

int botinfo_offset(BYTE* unenc_response) {
	int offset = 0;
	int flags_bots = read_le(unenc_response + offset + 4, 4);
	offset += 8;
	if (flags_bots & (1 << 0)) offset += 8;
	if (flags_bots & (1 << 1)) offset += tlstr_len(unenc_response + offset, true);
	if (flags_bots & (1 << 4)) offset += photo_offset(unenc_response + offset);
	if (flags_bots & (1 << 4)) offset += doc_offset(unenc_response + offset);
	if (flags_bots & (1 << 2)) {
		int count = read_le(unenc_response + offset + 4, 4);
		offset += 8;
		for (int k = 0; k < count; k++) {
			offset += 4;
			offset += tlstr_len(unenc_response + offset, true);
			offset += tlstr_len(unenc_response + offset, true);
		}
	}
	if (flags_bots & (1 << 3)) {
		int botmenu_cons = read_le(unenc_response + offset, 4);
		offset += 4;
		if (botmenu_cons == TL_BOT_MENU_BUTTON) {
			offset += tlstr_len(unenc_response + offset, true);
			offset += tlstr_len(unenc_response + offset, true);
		}
	}
	if (flags_bots & (1 << 7)) offset += tlstr_len(unenc_response + offset, true);
	if (flags_bots & (1 << 8)) {
		int flags_botapp = read_le(unenc_response + offset + 4, 4);
		offset += 8;
		if (flags_botapp & (1 << 0)) offset += tlstr_len(unenc_response + offset, true);
		for (int k = 1; k < 5; k++) if (flags_botapp & (1 << k)) offset += 4;
	}
	if (flags_bots & (1 << 9)) {
		int flags_botver = read_le(unenc_response + offset + 4, 4);
		offset += 16;
		offset += tlstr_len(unenc_response + offset, true);
		if (flags_botver & (1 << 0)) offset += tlstr_len(unenc_response + offset, true);
	}
	return offset;
}

int exchatinv_offset(BYTE* unenc_response) {
	int offset = 0;
	int exchatinv_cons = read_le(unenc_response + offset, 4);
	offset += 4;
	if (exchatinv_cons == TL_CHAT_INVITE_EXPORTED) {
		int flags_exchatinv = read_le(unenc_response + offset, 4);
		offset += 4;
		offset += tlstr_len(unenc_response + offset, true);
		offset += 12;
		for (int j = 1; j < 11; j++) {
			if (j == 5 || j == 6 || j == 8 || j == 9) continue;
			if (flags_exchatinv & (1 << j)) offset += 4;
		}
		if (flags_exchatinv & (1 << 8)) offset += tlstr_len(unenc_response + offset, true);
		if (flags_exchatinv & (1 << 9)) offset += 16;
	}
	return offset;
}

int chatreactions_offset(BYTE* unenc_response, Peer* peer) {
	int reaction_cons = read_le(unenc_response, 4);
	int offset = 4;
	if (reaction_cons == TL_CHAT_REACTIONS_SOME) {
		int count = read_le(unenc_response + offset + 4, 4);
		offset += 8;
		if (peer) peer->reaction_list = new std::vector<wchar_t*>();
		for (int k = 0; k < count; k++) {
			int reactiontype_cons = read_le(unenc_response + offset, 4);
			offset += 4;
			if (reactiontype_cons == TL_REACTION_EMOJI) {
				if (peer) {
					wchar_t emoji_str[10] = {0};
					read_string(unenc_response + offset, emoji_str);
					wchar_t file_name[50];
					file_name[0] = 0;
					wemoji_to_path(emoji_str, file_name, false);
					wchar_t* emoji = _wcsdup(file_name);
					peer->reaction_list->push_back(emoji);
				}
				offset += tlstr_len(unenc_response + offset, true);
			} else if (reactiontype_cons == TL_REACTION_CUSTOM_EMOJI) {
				if (peer) {
					__int64 document_id = read_le(unenc_response + offset, 8);
					wchar_t path[36];
					swprintf(path, L"/%016I64X", document_id);
					path[0] = 1;
					wchar_t* emoji = _wcsdup(path);
					peer->reaction_list->push_back(emoji);
				}
				offset += 8;
			}
		}
	} else if (reaction_cons == TL_CHAT_REACTIONS_NONE && peer) peer->reaction_list = NULL;
	else offset += 4;
	return offset;
}