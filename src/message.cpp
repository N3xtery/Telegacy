/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

int message_handler(bool to_front, BYTE* message, bool update_order, bool editing, bool rplhelper) {
	int msg_cons = read_le(message, 4);
	bool service = (msg_cons == 0xd3d28540) ? true : false;
	int offset_msg = 4;
	int flags_msg = read_le(message + offset_msg, 4);
	if (msg_cons == 0x90a6ca84) {
		offset_msg += 8;
		if (flags_msg & (1 << 0)) offset_msg += 12;
		return offset_msg;
	}
	int flags_msg2 = 0;
	if (!service) flags_msg2 = read_le(message + offset_msg + 4, 4);
	if (service) offset_msg -= 4;
	BYTE* msg_id = message + offset_msg + 8;
	offset_msg += 12;
	BYTE* chat_member_id = NULL;
	if (flags_msg & (1 << 8)) {
		chat_member_id = &message[offset_msg + 4];
		offset_msg += 12;
	}
	if (flags_msg & (1 << 29)) offset_msg += 4;
	offset_msg += 4;
	BYTE* peer_id = &message[offset_msg];

	Peer* peer = NULL;
	int index = 0;
	bool message_adding = (current_peer && memcmp(peer_id, current_peer->id, 8) == 0);
	if (message_adding) peer = current_peer;
	else for (int i = 0; i < peers_count; i++) if (memcmp(peers[i].id, peer_id, 8) == 0) {
		peer = &peers[i];
		index = i;
		break;
	}
	bool duplicate = (peer && read_le(msg_id, 4) <= peer->last_recv && !to_front);
	if (!duplicate && !to_front && peer) peer->last_recv = read_le(msg_id, 4);

	if (!duplicate && update_order) {
		char type = 0;
		int peer_cons = read_le(message + offset_msg - 4, 4);
		if (peer_cons == 0x36c6019a) type = 1;
		else if (peer_cons == 0xa2a5371e) type = 2;
		update_chats_order(message + offset_msg, msg_id, type);
		peer = &peers[0];
	}
	offset_msg += 8;
	if (flags_msg & (1 << 28)) offset_msg += 12;
	BYTE* msgfwd = NULL;
	if (flags_msg & (1 << 2)) {
		msgfwd = &message[offset_msg];
		offset_msg += msgfwd_offset(message + offset_msg);
	}
	if (flags_msg & (1 << 11)) offset_msg += 8;
	if (!service && flags_msg2 & (1 << 0)) offset_msg += 8;
	BYTE* msgrpl = NULL;
	if (flags_msg & (1 << 3)) {
		msgrpl = rplhelper ? NULL : &message[offset_msg];
		offset_msg += msgrpl_offset(message + offset_msg);
	}
	int date = read_le(message + offset_msg, 4);
	offset_msg += 4;
	wchar_t* service_msg = (wchar_t*)1;
	bool service_msg_allocated = false;

	if (service) {
		int service_cons = read_le(message + offset_msg, 4);
		if (!duplicate && service_cons == 0xaa786345 || service_cons == 0xb5a1ce5a || service_cons == 0x7fcb13a8 || service_cons == 0x95e3fbef) {
			if (service_cons == 0xaa786345 && peer && peer->full && date > peer->theme_set_time) {
				if (peer && peer->full && date > peer->theme_set_time) {
					if (tlstr_len(message + offset_msg + 4, false) == 0) {
						memset(peer->theme_id, 0, 8);
						if (message_adding) apply_theme(0);
					} else for (int i = 0; i < themes.size(); i++) {
						if (memcmp(themes[i].emoji_id, message + offset_msg + 4, tlstr_len(message + offset_msg + 4, true)) == 0) {
							memcpy(peer->theme_id, themes[i].id, 8);
							if (message_adding) apply_theme(i + 1);
							break;
						}
					}
					peer->theme_set_time = date;
				}
			} else if (service_cons == 0xb5a1ce5a && peer && date > peer->name_set_time) {
				free(peer->name);
				peer->name = read_string(message + offset_msg + 4, NULL);
				peer->name_set_time = date;
				update_name_in_list(index);
			} else if (service_cons == 0x7fcb13a8 && peer && date > peer->pfp_set_time) {
				memcpy(peer->photo, message + offset_msg + 12, 8);
				peer->pfp_set_time = date;
			} else if (service_cons == 0x95e3fbef && peer && date > peer->pfp_set_time) {
				memset(peer->photo, 0, 8);
				peer->pfp_set_time = date;
			} 
		}
		offset_msg = msgact_offset(message, offset_msg, message_adding ? &service_msg : NULL, &service_msg_allocated);
	}
	BYTE* msg_bytes = &message[offset_msg];
	if (!service) offset_msg += tlstr_len(message + offset_msg, true);
	if (!service && (flags_msg & (1 << 9))) offset_msg += messagemedia_offset(message + offset_msg);
	if (flags_msg & (1 << 6)) offset_msg += replymarkup_offset(message + offset_msg);
	std::vector<int> format_vecs[10];
	if (flags_msg & (1 << 7)) {
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int j = 0; j < count; j++) offset_msg += msgent_offset(message + offset_msg, message_adding ? &format_vecs[0] : NULL);
	}
	BYTE* views = NULL;
	if (flags_msg & (1 << 10)) {
		views = &message[offset_msg];
		offset_msg += 8;
	}
	if (flags_msg & (1 << 23)) {
		int flags_msgrep = read_le(message + offset_msg + 4, 4);
		offset_msg += 16;
		if (flags_msgrep & (1 << 1)) {
			int count = read_le(message + offset_msg + 4, 4);
			offset_msg += 8;
			for (int j = 0; j < count; j++) offset_msg += 12;
		}
		if (flags_msgrep & (1 << 0)) offset_msg += 8;
		if (flags_msgrep & (1 << 2)) offset_msg += 4;
		if (flags_msgrep & (1 << 3)) offset_msg += 4;
	}
	if (flags_msg & (1 << 15)) offset_msg += 4;
	if (flags_msg & (1 << 16)) offset_msg += tlstr_len(message + offset_msg, true);
	bool groupmed_end = false;
	bool footer = true;
	BYTE zero_arr[8] = {0};
	if (!duplicate && !rplhelper && !editing && (flags_msg & (1 << 17))) {
		if (to_front) {
			if (memcmp(message + offset_msg, group_id_tofront, 8) != 0) {
				if (memcmp(zero_arr, group_id_tofront, 8) != 0)  groupmed_end = true;
			} else footer = false;
			memcpy(group_id_tofront, message + offset_msg, 8);
		} else if (memcmp(message + offset_msg, group_id, 8) == 0) groupmed_end = true;
		else memcpy(group_id, message + offset_msg, 8);
	} else if (!rplhelper && to_front) {
		if (memcmp(zero_arr, group_id_tofront, 8) != 0) groupmed_end = true;
		memset(group_id_tofront, 0, 8);
	}
	if (flags_msg & (1 << 17)) offset_msg += 8;
	BYTE* reactions = NULL;
	if (flags_msg & (1 << 20)) {
		reactions = rplhelper ? NULL : (message + offset_msg + 12);
		offset_msg += msgreact_offset(message + offset_msg);
	}
	if (flags_msg & (1 << 22)) {
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int j = 0; j < count; j++) {
			offset_msg += 4;
			for (int k = 0; k < 3; k++) offset_msg += tlstr_len(message + offset_msg, true);
		}
	}
	if (flags_msg & (1 << 25)) offset_msg += 4;
	if (flags_msg & (1 << 30)) offset_msg += 4;
	if (flags_msg2 & (1 << 2)) offset_msg += 8;
	if (flags_msg2 & (1 << 3)) {
		int flags_facts = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		if (flags_facts & (1 << 1)) {
			offset_msg += tlstr_len(message + offset_msg, true);
			offset_msg += textwithent_offset(message + offset_msg);
		}
		offset_msg += 8;
	}
	if (flags_msg2 & (1 << 5)) offset_msg += 4;
	if (message_adding)
		message_adder(service, to_front, flags_msg, msg_id, service ? NULL : msg_bytes, service ? service_msg : NULL, NULL, chat_member_id, &format_vecs[0], reactions, msgrpl, msgfwd, views, groupmed_end, footer, editing, date);
	else if (!duplicate && peer && !(flags_msg & (1 << 1)) && !editing && !to_front && !rplhelper) new_msg_notification(peer, msg_bytes, groupmed_end);
	if (service_msg_allocated) free(service_msg);
	return offset_msg;
}

void message_adder(bool service, bool to_front, int flags, BYTE* msg_id, BYTE* msg_bytes, wchar_t* service_msg, EDITSTREAM* es, BYTE* chat_member_id, std::vector<int>* format_vecs, BYTE* reactions, BYTE* msgrpl, BYTE* msgfwd, BYTE* views, bool groupmed_end, bool footer, bool editing, int date) {
	Message message;
	message.id = NULL;
	if (msg_id) message.id = read_le(msg_id, 4);
	message.outgoing = (flags & (1 << 1)) != 0 || memcmp(current_peer->id, myself.id, 8) == 0;
	message.seen = ((to_front && !message.outgoing) || (message.outgoing && message.id != NULL && message.id <= current_peer->last_read)) ? true : false;
	if ((flags & (1 << 4)) && message.outgoing && !to_front && !editing) read_react_ment(false);
	
	SCROLLINFO si = {0};
	si.cbSize = sizeof(si);
	si.fMask = SIF_RANGE | SIF_TRACKPOS | SIF_PAGE | SIF_POS;
	GetScrollInfo(chat, SB_VERT, &si);
	if (ischatscrolling) {
		si.nPos = si.nTrackPos;
		ischatscrolling = false;
	}
	CHARRANGE cr;
	SendMessage(chat, EM_EXGETSEL, 0, (LPARAM)&cr);
	if (drawchat) SendMessage(chat, WM_SETREDRAW, FALSE, 0);

	int editing_index = -1;
	int pos = to_front ? 0 : -1;
	bool same_photo = false;
	EDITSTREAM es_photo = {0};
	StreamData sd = {0};
	Document document;
	document.photo_msg_id[0] = 1;
	BYTE* doc = (!service && (flags & (1 << 9))) ? msg_bytes + tlstr_len(msg_bytes, true) : NULL;
	if (editing) for (int i = messages.size() - 1; i >= 0; i--) {
		if (message.id == messages[i].id) {
			editing_index = i;
			message.seen = messages[editing_index].seen;
			for (int j = documents.size() - 1; j >= 0; j--) {
				if (documents[j].min >= messages[i].start_char && documents[j].max <= messages[i].end_footer) {
					if (documents[j].photo_size && doc && memcmp(documents[j].access_hash, doc + 24, 8) == 0) {
						same_photo = true;
						SendMessage(chat, EM_SETSEL, documents[j].min, documents[j].max);
						es_photo.dwCookie = (DWORD_PTR)&sd;
						es_photo.pfnCallback = StreamOutCallback;
						SendMessage(chat, EM_STREAMOUT, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es_photo);
						document = documents[j];
					} else {
						free(documents[j].filename);
						free(documents[j].file_reference);
					}
					documents.erase(documents.begin() + j);
				} else if (documents[j].max < messages[i].start_char) break;
			}
			for (j = links.size() - 1; j >= 0; j--) {
				if (links[j].chrg.cpMin >= messages[i].start_char && links[j].chrg.cpMax <= messages[i].end_footer) {
					free(links[j].lpstrText);
					links.erase(links.begin() + j);
				}
			}
			break;
		}
	}
	if (editing && editing_index == -1) return;

	bool group_media = (flags & (1 << 17)) ? true : false;
	bool header = (group_media && (to_front || (!to_front && (groupmed_end || (editing && messages[editing_index-1].end_char == messages[editing_index-1].end_footer))))) ? false : true;
	if (editing && !header) footer = false;
	bool groupmed_end_nottofront = false;
	if (!to_front && groupmed_end) {
		groupmed_end = false;
		groupmed_end_nottofront = true;
	}
	
	bool chat_member_found = true;
	wchar_t* sender = current_peer->name;
	if (message.outgoing) sender = L"You";
	if (current_peer->type == 1 && !message.outgoing) {
		chat_member_found = false;
		for (int i = 0; i < current_peer->chat_users->size(); i++) {
			if (memcmp(chat_member_id, current_peer->chat_users->at(i).id, 8) == 0) {
				chat_member_found = true;
				sender = current_peer->chat_users->at(i).name;
				break;
			}
		}
	}
	if (!chat_member_found || (chat_member_id && !message.outgoing && current_peer->type == 2)) {
		if (msg_bytes - chat_member_id == 16) sender = L"Unknown";
		else {
			char type = -1;
			BYTE* peer_bytes = find_peer(chat_member_id + 8, chat_member_id - 4, true, &type);
			peer_set_name(peer_bytes, &sender, type);
		}
	}

	wchar_t* msg;
	int msg_len = 0;
	if (service_msg) msg_len += wcslen(service_msg);
	else if (msg_bytes) msg_len += tlstr_to_str_len(msg_bytes);
	int sender_len = header ? wcslen(sender) : 0;
	int header_len = header ? (sender_len + ((current_peer->perm.cansendmsg || msg_len == 0)  ? 2 : 3)) : 0;
	bool msghastext = msg_bytes && tlstr_to_str_len(msg_bytes) > 0;
	if (groupmed_end) msg_len += wcslen(last_tofront_sender) + 3;
	msg_len += service ? sender_len : header_len;
	if (!message.outgoing && !to_front && ((current_peer->type == 0 && current_peer->online != - 1) || (current_peer->type != 0 && wcsncmp(sender, status_str, wcslen(sender)) == 0)) ) SetTimer(hMain, 0, 0, NULL);

	if (service) {
		if (service_msg[0] != L'%' && message.outgoing) sender = L"you";
		msg = (wchar_t*)malloc((msg_len + 1)*2);
		swprintf(msg, service_msg, sender);
	} else {
		msg = (wchar_t*)malloc((msg_len + 1)*2);
		if (header) {
			wcsncpy(msg, sender, sender_len);
			msg[sender_len] = ':';
			msg[sender_len+1] = ' ';
			if (header_len == sender_len + 3) msg[sender_len+2] = '\n';
		}
		int offset = header_len;
		if (msg_bytes) read_string(msg_bytes, msg + offset);
		if (es) msg[offset] = 0;
	}

	if (!editing && !to_front && !header) {
		int last_msg = messages.size() - 1;
		SendMessage(chat, EM_SETSEL, messages[last_msg].end_char, messages[last_msg].end_footer);
		SendMessage(chat, EM_REPLACESEL, 0, 0);
		messages[last_msg].end_footer = messages[last_msg].end_char;
	}

	bool another_footer = false;
	int last_group_msg = (editing_index == -1) ? 0 : editing_index;
	if ((to_front && !footer) || (editing && messages[editing_index].end_char == messages[editing_index].end_footer)) {
		another_footer = true;
		while (messages[last_group_msg].end_char == messages[last_group_msg].end_footer) last_group_msg++;
	}

	bool editmsgisup = false;
	EDITSTREAM es_rep = {0};
	if (editing) {
		RECT rect;
		SendMessage(chat, EM_GETRECT, 0, (LPARAM)&rect);
		POINTL point;
		point.x = rect.left;
		point.y = rect.top;
		int first_visible_char = SendMessage(chat, EM_CHARFROMPOS, 0, (LPARAM)&point);
		if (first_visible_char > messages[editing_index].end_footer) editmsgisup = true;

		if (msgrpl) {
			FINDTEXT ft;
			ft.chrg.cpMin = messages[last_group_msg].end_char + 1;
			ft.chrg.cpMax = messages[last_group_msg].end_footer;
			ft.lpstrText = L"\r";
			SendMessage(chat, EM_SETSEL, messages[last_group_msg].end_char, SendMessage(chat, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft));
			StreamData sd = {0};
			es_rep.dwCookie = (DWORD_PTR)&sd;
			es_rep.pfnCallback = StreamOutCallback;
			SendMessage(chat, EM_STREAMOUT, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es_rep);
		}
	}

	SendMessage(chat, EM_SETSEL, editing ? messages[editing_index].start_char : pos, editing ? (footer ? messages[editing_index].end_footer : messages[editing_index].end_char) : pos);
	int written_groupmedend = 0;
	if (to_front) {
		if (!groupmed_end || (documents.size() > 0 && documents[0].min == 0)) {
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"\n");
			SendMessage(chat, EM_SETSEL, pos, pos);
		} else written_groupmedend--;
		if (groupmed_end) {
			written_groupmedend += riched_write(chat, last_tofront_sender);
			written_groupmedend += riched_write(chat, L": ");
			if (!current_peer->perm.cansendmsg) written_groupmedend += riched_write(chat, L"\n");
			written_groupmedend++;
			int deleted_wchars = 0;
			for (int i = 0; i < wcslen(last_tofront_sender); i++) i = emoji_adder(i, last_tofront_sender, pos, 15, chat, &deleted_wchars);
			written_groupmedend -= deleted_wchars;
			SendMessage(chat, EM_SETSEL, pos, pos);
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"\n");
			SendMessage(chat, EM_SETSEL, pos, pos);
			messages[0].start_char -= written_groupmedend;
		}
	}

	CHARRANGE cr_startmsg;
	SendMessage(chat, EM_EXGETSEL, 0, (LPARAM)&cr_startmsg);

	int written = riched_write(chat, msg);
	if (es) written += SendMessage(chat, EM_STREAMIN, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)es);
	bool added_doc = false, added_photo = false;
	if (doc != NULL && read_le(doc, 4) == 0xdd570bd5 && (read_le(doc + 4, 4) & (1 << 0))) { // messageMediaDocument
		added_doc = true;
		wchar_t duration_str[15] = {0};
		bool voice = false, gif = false, round = false, sticker = false;
		if (!same_photo) {
			int offset = 12;
			int doc_flags = read_le(doc + offset, 4);
			offset += 4;
			document.filename = NULL;
			document.visible = true;
			document.photo_size = 0;
			memcpy(document.id, doc + offset, 8);
			offset += 8;
			memcpy(document.access_hash, doc + offset, 8);
			offset += 8;
			int fileref_len = tlstr_len(doc + offset, true);
			document.file_reference = (BYTE*)malloc(fileref_len);
			memcpy(document.file_reference, doc + offset, fileref_len);
			offset += fileref_len + 4;
			int mime_offset = offset;
			offset += tlstr_len(doc + offset, true);
			document.size = read_le(doc + offset, 8);
			offset += 8;
			offset += photo_video_size_offset(doc + offset, (doc_flags & (1 << 0)) ? true : false, true, (doc_flags & (1 << 1)) ? true : false);
			document.dc = read_le(doc + offset, 4);
			offset += 12;
			int att_count = read_le(doc + offset - 4, 4);
			for (int i = 0; i < att_count; i++) {
				int att_cons = read_le(doc + offset, 4);
				if (att_cons == 0x15590068) document.filename = read_string(doc + offset + 4, NULL);
				else if (att_cons == 0x9852f9c6 || att_cons == 0x43c57c48) {
					int att_flags = read_le(doc + offset + 4, 4);
					int duration;
					if (att_cons == 0x9852f9c6) {
						duration = read_le(doc + offset + 8, 4);
						voice = (att_flags & (1 << 10)) ? true : false;
					}
					else {
						double duration_double;
						memcpy(&duration_double, doc + offset + 8, 8);
						duration = (int)duration_double;
						round = (att_flags & (1 << 0)) ? true : false;
					}
					if (duration < 3600) swprintf(duration_str, L" (%02d:%02d)", duration / 60, duration % 60);
					else swprintf(duration_str, L" (%02d:%02d:%02d)", duration / 3600, (duration / 60) % 60, duration % 60);
				} else if (att_cons == 0x11b58939) gif = true;
				else if (att_cons == 0x6319d612) sticker = true;
				offset += docatt_offset(doc + offset);
			}
			if (document.filename == NULL) {
				document.filename = (wchar_t*)malloc(22*2);
				wchar_t mime_type[256];
				read_string(doc + mime_offset, &mime_type[0]);
				wchar_t* slash = wcsrchr(mime_type, L'/');
				slash[0] = 0;
				wchar_t* type = mime_type;
				if (voice) type = L"voice";
				else if (gif) type = L"gif";
				else if (round) type = L"round";
				int id = read_le(document.access_hash + 4, 4);
				swprintf(document.filename, L"%s%08X.%s", type, id, slash + 1);
			}
		}
		if (header || msghastext) {
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"\n");
			written++;
		}
		if (sticker || same_photo) {
			document.photo_size = 1;
			memset(document.photo_msg_id, 0, 8);
			document.min = cr_startmsg.cpMin + written;
			if (same_photo) {
				es_photo.pfnCallback = StreamInCallback;
				SendMessage(chat, EM_STREAMIN, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es_photo);
				StreamData* sd = (StreamData*)es_photo.dwCookie;
				free(sd->buf);
			} else {
				wchar_t placeholder[] = {0xFE0F, 0};
				SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)placeholder);
				if (!to_front && IMAGELOADPOLICY == 2) get_photo(NULL, &document, &dcInfoMain);
			}
			written++;
			document.max = cr_startmsg.cpMin + written;
		} else {
			double size = (double)document.size;
			int measure = 0;
			while (size >= 1024) {
				size /= 1024.0;
				measure++;
			}
			wchar_t size_str[12];
			const wchar_t* measures[] = {L"B", L"KB", L"MB", L"GB"};
			const wchar_t* formats[] = {L" (%.0f %s)", L" (%.1f %s)"};
			swprintf(size_str, measure == 0 ? formats[0] : formats[1], size, measures[measure]);

			document.min = cr_startmsg.cpMin + written;
			written += riched_write(chat, document.filename);
			document.max = cr_startmsg.cpMin + written;
			if (duration_str[0] == ' ') written += riched_write(chat, &duration_str[0]);
			written += riched_write(chat, &size_str[0]);
		}
	} else if (doc != NULL && read_le(doc, 4) == 0x695150d7 && (read_le(doc + 4, 4) & (1 << 0))) {
		added_doc = true;
		HBITMAP hClone;
		if (!same_photo) {
			int flags = read_le(doc + 12, 4);
			memcpy(document.id, doc + 16, 8);
			memcpy(document.access_hash, doc + 24, 8);
			memset(document.photo_msg_id, 0, 8);
			document.filename = (wchar_t*)malloc(2*18);
			swprintf(document.filename, L"photo%08X.jpg", read_le(document.access_hash + 4, 4));
			int fileref_len = tlstr_len(doc + 32, true);
			document.file_reference = (BYTE*)malloc(fileref_len);
			memcpy(document.file_reference, doc + 32, fileref_len);
			char size_main = 0;
			int offset = 44 + fileref_len;
			int count = read_le(doc + offset - 4, 4);
			for (int i = 0; i < count; i++) {
				int photosize_cons = read_le(doc + offset, 4);
				char size = (char)doc[offset + 5];
				if (size == 'i' && IMAGELOADPOLICY) {
					const char jpg_header[] = "\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49\x46\x00\x01\x01\x00\x00\x01\x00\x01\x00\x00\xff\xdb\x00\x43\x00\x28\x1c"
						"\x1e\x23\x1e\x19\x28\x23\x21\x23\x2d\x2b\x28\x30\x3c\x64\x41\x3c\x37\x37\x3c\x7b\x58\x5d\x49\x64\x91\x80\x99\x96\x8f\x80\x8c\x8a\xa0\xb4\xe6\xc3"
						"\xa0\xaa\xda\xad\x8a\x8c\xc8\xff\xcb\xda\xee\xf5\xff\xff\xff\x9b\xc1\xff\xff\xff\xfa\xff\xe6\xfd\xff\xf8\xff\xdb\x00\x43\x01\x2b\x2d\x2d\x3c\x35"
						"\x3c\x76\x41\x41\x76\xf8\xa5\x8c\xa5\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8"
						"\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xf8\xff\xc0\x00\x11\x08\x00\x00\x00\x00\x03\x01\x22\x00"
						"\x02\x11\x01\x03\x11\x01\xff\xc4\x00\x1f\x00\x00\x01\x05\x01\x01\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08"
						"\x09\x0a\x0b\xff\xc4\x00\xb5\x10\x00\x02\x01\x03\x03\x02\x04\x03\x05\x05\x04\x04\x00\x00\x01\x7d\x01\x02\x03\x00\x04\x11\x05\x12\x21\x31\x41\x06"
						"\x13\x51\x61\x07\x22\x71\x14\x32\x81\x91\xa1\x08\x23\x42\xb1\xc1\x15\x52\xd1\xf0\x24\x33\x62\x72\x82\x09\x0a\x16\x17\x18\x19\x1a\x25\x26\x27\x28"
						"\x29\x2a\x34\x35\x36\x37\x38\x39\x3a\x43\x44\x45\x46\x47\x48\x49\x4a\x53\x54\x55\x56\x57\x58\x59\x5a\x63\x64\x65\x66\x67\x68\x69\x6a\x73\x74\x75"
						"\x76\x77\x78\x79\x7a\x83\x84\x85\x86\x87\x88\x89\x8a\x92\x93\x94\x95\x96\x97\x98\x99\x9a\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xb2\xb3\xb4\xb5\xb6"
						"\xb7\xb8\xb9\xba\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xe1\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xf1\xf2\xf3\xf4"
						"\xf5\xf6\xf7\xf8\xf9\xfa\xff\xc4\x00\x1f\x01\x00\x03\x01\x01\x01\x01\x01\x01\x01\x01\x01\x00\x00\x00\x00\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08"
						"\x09\x0a\x0b\xff\xc4\x00\xb5\x11\x00\x02\x01\x02\x04\x04\x03\x04\x07\x05\x04\x04\x00\x01\x02\x77\x00\x01\x02\x03\x11\x04\x05\x21\x31\x06\x12\x41"
						"\x51\x07\x61\x71\x13\x22\x32\x81\x08\x14\x42\x91\xa1\xb1\xc1\x09\x23\x33\x52\xf0\x15\x62\x72\xd1\x0a\x16\x24\x34\xe1\x25\xf1\x17\x18\x19\x1a\x26"
						"\x27\x28\x29\x2a\x35\x36\x37\x38\x39\x3a\x43\x44\x45\x46\x47\x48\x49\x4a\x53\x54\x55\x56\x57\x58\x59\x5a\x63\x64\x65\x66\x67\x68\x69\x6a\x73\x74"
						"\x75\x76\x77\x78\x79\x7a\x82\x83\x84\x85\x86\x87\x88\x89\x8a\x92\x93\x94\x95\x96\x97\x98\x99\x9a\xa2\xa3\xa4\xa5\xa6\xa7\xa8\xa9\xaa\xb2\xb3\xb4"
						"\xb5\xb6\xb7\xb8\xb9\xba\xc2\xc3\xc4\xc5\xc6\xc7\xc8\xc9\xca\xd2\xd3\xd4\xd5\xd6\xd7\xd8\xd9\xda\xe2\xe3\xe4\xe5\xe6\xe7\xe8\xe9\xea\xf2\xf3\xf4"
						"\xf5\xf6\xf7\xf8\xf9\xfa\xff\xda\x00\x0c\x03\x01\x00\x02\x11\x03\x11\x00\x3f\x00";
					int jpg_size = tlstr_len(doc + offset + 8, false) - 3;
					int myjpg_size = 623 + jpg_size + 2;

					BYTE* myjpg = (BYTE*)malloc(myjpg_size);
					memcpy(myjpg, jpg_header, 623);
					if (doc[offset + 8] == 254) offset += 3;
					memcpy(myjpg + 623, doc + offset + 12, jpg_size);
					myjpg[623 + jpg_size] = '\xff';
					myjpg[623 + jpg_size + 1] = '\xd9';
					myjpg[164] = doc[offset + 10];
					myjpg[166] = doc[offset + 11];
					hClone = jpg_to_bmp(myjpg, myjpg_size);
					free(myjpg);
				} else if (size > size_main && photosize_cons == 0x75c78e60) {
					size_main = size;
					document.size = read_le(doc + offset + 16, 4);
				}
				offset += photo_video_size_offset(doc + offset, true, false, false);
			}
			document.photo_size = size_main;
			if (flags & (1 << 1)) offset += photo_video_size_offset(doc + offset, false, false, true);
			document.dc = read_le(doc + offset, 4);
			if (!to_front && IMAGELOADPOLICY == 2) get_photo(NULL, &document, &dcInfoMain);
		}
		if (header || msghastext) {
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"\n");
			written++;
		}
		if (!to_front && !header && !msghastext) {
			int index = editing ? editing_index - 1 : messages.size() - 1;
			if (editing) update_positions(-1, messages[index].start_char, 0);
			SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + written - 1, cr_startmsg.cpMin + written);
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"");
			messages[index].end_char--;
			messages[index].end_footer--;
			messages[index].start_reactions--;
			cr_startmsg.cpMin--;
			cr_startmsg.cpMax--;
		}
		if (editing && !header && msghastext && messages[editing_index].end_char - messages[editing_index].end_header <= 2) {
			update_positions(1, messages[editing_index - 1].start_char, 0);
			SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin, cr_startmsg.cpMin);
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"\n");
			messages[editing_index - 1].end_char++;
			messages[editing_index - 1].end_footer++;
			messages[editing_index - 1].start_reactions++;
			cr_startmsg.cpMin++;
			cr_startmsg.cpMax++;
			SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + written, cr_startmsg.cpMin + written);
		}
		document.min = cr_startmsg.cpMin + written;
		if (same_photo) {
			es_photo.pfnCallback = StreamInCallback;
			SendMessage(chat, EM_STREAMIN, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es_photo);
			StreamData* sd = (StreamData*)es_photo.dwCookie;
			free(sd->buf);
		} else if (IMAGELOADPOLICY) {
			insert_image(chat, NULL, hClone);
			DeleteObject(hClone);
		} else {
			wchar_t placeholder[] = {0xFE0F, 0};
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)placeholder);
		}
		written++;
		document.max = cr_startmsg.cpMin + written;
		if (to_front && !footer && messages[0].end_char - messages[0].end_header <= 2) {
			SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + written, cr_startmsg.cpMin + written + 1);
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"");
			written--;
		}
	}

	bool addnewline = (editing && added_photo && group_media && editing_index != messages.size() - 1 && messages[editing_index + 1].start_char == messages[editing_index + 1].end_header) ? false : true;
	if (!addnewline && another_footer && messages[editing_index + 1].end_char - messages[editing_index + 1].end_header > 2) addnewline = true;

	message.reply_needed = 0;
	int written_info = 0;
	message.start_reactions = 0;
	if (footer || to_front) {
		Message* message_footer = &message;
		wchar_t info[100] = {0};
		if (another_footer) {
			messages[last_group_msg].reacted.clear();
			int diff = written + 2;
			if (editing) diff -= (messages[editing_index].end_char - messages[editing_index].start_char + 1);
			SendMessage(chat, EM_SETSEL, messages[last_group_msg].end_char + diff - 1, messages[last_group_msg].end_footer + diff - 3);
			written_info += messages[last_group_msg].end_char - (editing ? messages[editing_index].end_char : 0) + 1;
			if (!addnewline) written_info--;
			message_footer = &messages[last_group_msg];
		} else written_info += riched_write(chat, L"\n");
		
		int replying_msg_id = 0;
		if (es) replying_msg_id = ::replying_msg_id;
		BYTE* quote_text = NULL;
		BYTE* msgrpl_another_chat = NULL;
		if (msgrpl) {
			int flags = read_le(msgrpl + 4, 4);
			if (flags & (1 << 4)) replying_msg_id = read_le(msgrpl + 8, 4);
			if (flags & (1 << 6)) {
				int offset = 8;
				if (flags & (1 << 4)) offset += 4;
				if (flags & (1 << 0)) offset += 12;
				if (flags & (1 << 5)) {
					msgrpl_another_chat = &msgrpl[offset];
					offset += msgfwd_offset(msgrpl + offset);
				}
				if (flags & (1 << 8)) offset += messagemedia_offset(msgrpl + offset);
				if (flags & (1 << 1)) offset += 4;
				quote_text = &msgrpl[offset];
			}
		}
		if (msgrpl_another_chat && quote_text) {
			written_info += riched_write(chat, L"replying to ");
			format_vecs[1].push_back(written - header_len + written_info);
			written_info += msgfwd_addname(msg_bytes, msgrpl_another_chat, cr_startmsg.cpMin + written + written_info, msg_bytes - msg_id == 12);
			written_info += riched_write(chat, L": ");
			written_info += set_reply(-1, cr_startmsg.cpMin + written + written_info, quote_text, false);
			format_vecs[1].push_back(written - header_len + written_info);
			written_info += riched_write(chat, L"\n");
		} else {
			if (to_front && msgrpl) {
				written_info += riched_write(chat, L"replying to ");
				if (quote_text) {
					format_vecs[1].push_back(written - header_len + written_info);
					written_info += set_reply(-1, cr_startmsg.cpMin + written + written_info, quote_text, false);
					format_vecs[1].push_back(written - header_len + written_info);
				}
				message_footer->reply_needed = replying_msg_id;
				written_info += riched_write(chat, L"\n");
			}
			if (!to_front && replying_msg_id && !editing) {
				if (!format_vecs) format_vecs = new std::vector<int>[9];
				for (int i = messages.size() - 1; i >= -1; i--) {
					int id = messages[i].id;
					if (i == -1 || messages[i].id == replying_msg_id) break;
				}
				written_info += riched_write(chat, L"replying to ");
				format_vecs[1].push_back(written - header_len + written_info);
				if (i == -1) {
					if (quote_text) written_info += set_reply(-1, cr_startmsg.cpMin + written + written_info, quote_text, false);
					message_footer->reply_needed = replying_msg_id;
					written_info += riched_write(chat, L"\n");
					get_message(replying_msg_id, current_peer);
				} else {
					written_info += set_reply(i, cr_startmsg.cpMin + written + written_info, quote_text, false);
					written_info += riched_write(chat, L"\n");
				}
				format_vecs[1].push_back(written - header_len + written_info - 1);
			} else if (replying_msg_id && editing) {
				es_rep.pfnCallback = StreamInCallback;
				written_info += SendMessage(chat, EM_STREAMIN, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es_rep);
				StreamData* sd = (StreamData*)es_rep.dwCookie;
				free(sd->buf);
				written_info += riched_write(chat, L"\n");
			}
		}

		if (msgfwd) {
			written_info += riched_write(chat, L"forwarded from ");
			written_info += msgfwd_addname(msg_bytes, msgfwd, cr_startmsg.cpMin + written + written_info, msg_bytes - msg_id == 12);
			written_info += riched_write(chat, L"\n");
		}

		get_date(info + wcslen(info), date, false);
		if ((flags & (1 << 15)) && !(flags & (1 << 21))) wcscat(info, L" | edited");
		if (message.outgoing && memcmp(current_peer->id, myself.id, 8) != 0) {
			if (message.seen) wcscat(info, L" | seen");
			else if (msg_id == NULL) wcscat(info, L" | sending");
			else wcscat(info, L" | delivered");
		}
		if (views != NULL) {
			int views_int = read_le(views, 4);
			if (views_int > 1) swprintf(info, L"%s | %d views", info, views_int);
			else swprintf(info, L"%s | %d view", info, views_int);
		}
		written_info += riched_write(chat, info);
		if (reactions != NULL) {
			message.start_reactions = written_info - 1;
			written_info += set_reactions(reactions, message_footer, format_vecs, written - header_len + written_info, message.id);
			if (!to_front && message.outgoing) read_react_ment(true);
		}
		if (another_footer) {
			SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + written, cr_startmsg.cpMin + written);
			footer = false;
			written_info -= (messages[last_group_msg].end_char - (to_front ? 0 : messages[editing_index].end_char) - 1);
			if (!addnewline) written_info++;
			int diff = written_info - (messages[last_group_msg].end_footer - messages[last_group_msg].end_char);
			if (cr.cpMin > messages[last_group_msg].end_footer) {
				cr.cpMin += diff;
				cr.cpMax += diff;
			}
			if (diff) update_positions(diff, messages[last_group_msg].start_char, 0);
			messages[last_group_msg].end_footer += diff;
			if (message.start_reactions) {
				message.start_reactions -= (messages[last_group_msg].end_char - (to_front ? 0 : messages[editing_index].end_char));
				messages[last_group_msg].start_reactions = messages[last_group_msg].end_char + message.start_reactions;
				message.start_reactions = 0;
			} else messages[last_group_msg].start_reactions = messages[last_group_msg].end_footer;
			written_info = 0;
		} else {
			written_info += riched_write(chat, L"\n");
			written += written_info;
		}
	}

	if (!to_front && addnewline) SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"\n");
	if (addnewline) written++;

	SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin, cr_startmsg.cpMin + written + written_groupmedend);

	PARAFORMAT2 pf;
	pf.cbSize = sizeof(pf);
	pf.dwMask = PFM_ALIGNMENT;
	pf.wAlignment = service ? PFA_CENTER : PFA_LEFT;
	SendMessage(chat, EM_SETPARAFORMAT, 0, (LPARAM)&pf);

	CHARFORMAT2 cf;
	cf.cbSize = sizeof(cf);
	cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_SIZE | CFM_LINK | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_FACE | CFM_BACKCOLOR | CFM_WEIGHT;
	LOGFONT lf = {0};
	GetObject(hFonts[0], sizeof(lf), &lf);
	wcscpy(cf.szFaceName, lf.lfFaceName);
	cf.wWeight = lf.lfWeight;
	cf.dwEffects = 0;
	cf.crTextColor = 0;
	cf.crBackColor = RGB(255, 255, 255);
	cf.yHeight = MulDiv(-lf.lfHeight, 144, dpi) * 10;
	if (es) SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + written - written_info, cr_startmsg.cpMin + written);
	SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	if (es) SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin, cr_startmsg.cpMin + written);
    
	cf.dwMask = CFM_COLOR | CFM_BOLD;
	if (!service && header) {
		SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin, cr_startmsg.cpMin + sender_len);
		cf.crTextColor = RGB(message.outgoing ? 255 : 0, 0, message.outgoing ? 0 : 255);
		cf.dwEffects = CFE_BOLD;
		SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
	if (groupmed_end) {
		SendMessage(chat, EM_SETSEL, written, written + written_groupmedend - 2);
		cf.crTextColor = RGB(wcscmp(last_tofront_sender, L"You") == 0 ? 255 : 0, 0, wcscmp(last_tofront_sender, L"You") == 0 ? 0 : 255);
		cf.dwEffects = CFE_BOLD;
		SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}
	if (footer) {
		SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + written - written_info, cr_startmsg.cpMin + written - 1);
		cf.dwMask = CFM_SIZE;
		cf.yHeight = 160;
		SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		if (current_peer->perm.cansendmsg) {
			SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + written - 1, cr_startmsg.cpMin + written);
			cf.yHeight = 80;
			SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	if (added_doc && document.photo_size != 1) {
		cf.dwMask = CFM_LINK;
		cf.dwEffects = CFE_LINK;
		SendMessage(chat, EM_SETSEL, document.min, document.max);
		SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
	}

	int format_values[9] = {CFM_BOLD, CFM_ITALIC, CFM_UNDERLINE, CFM_STRIKEOUT, CFM_COLOR, CFM_FACE, CFM_BACKCOLOR, CFM_LINK, CFM_LINK};
	int new_links = 0;
	if (format_vecs) for (int i = 0; i < 9; i++) {
		if (!SPOILERS && i == 6) continue;
		for (int j = 0; j < format_vecs[i].size(); j+=2) {
			SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + header_len + format_vecs[i][j], cr_startmsg.cpMin + header_len + format_vecs[i][j+1]);
			cf.dwMask = format_values[i];
			if (i < 4 || i > 6) cf.dwEffects = format_values[i];
			else cf.dwEffects = 0;
			if (i == 4) cf.crTextColor = RGB(169, 169, 169);
			else if (i == 5) wcscpy(cf.szFaceName, L"Courier New");
			else if (i == 6) {
				cf.dwMask |= CFM_COLOR;
				cf.crTextColor = 0;
				cf.crBackColor = 0;
			} else if (i == 8) {
				new_links++;
				for (int k = 0; k < links.size(); k++) if (links[k].chrg.cpMin == -1) break;
				links[k].chrg.cpMin = cr_startmsg.cpMin + header_len + format_vecs[i][j];
				links[k].chrg.cpMax = cr_startmsg.cpMin + header_len + format_vecs[i][j+1];
			}
			SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		}
	}
	if (es && format_vecs) delete[] format_vecs;

	if (!es) {
		int deleted_wchars = 0;
		if (!message.outgoing) {
			for (int i = 0; i < header_len; i++) i = emoji_adder(i, msg, cr_startmsg.cpMin, 15, chat, &deleted_wchars);
			header_len -= deleted_wchars;
		}
		int deleted_wchars_cpy = deleted_wchars;
		int header_len_old = header_len + deleted_wchars;
		int links_count = links.size();
		int newlinks_to_check = new_links;
		for (int i = header_len_old; i < msg_len; i++) {
			deleted_wchars_cpy = deleted_wchars;
			__int64 custom_emoji_id = 0;
			for (int j = 0; j < format_vecs[9].size(); j += 4) {
				if (i == format_vecs[9][j] + header_len_old) {
					wchar_t file_name[MAX_PATH];
					custom_emoji_id = ((__int64)format_vecs[9][j + 3] << 32) | format_vecs[9][j + 2];
					swprintf(file_name, L"%s\\%016I64X.ico", get_path(appdata_path, L"custom_emojis"), custom_emoji_id);
					SendMessage(chat, EM_SETSEL, cr_startmsg.cpMin + i - deleted_wchars, cr_startmsg.cpMin + i - deleted_wchars + format_vecs[9][j + 1]);
					if (!insert_emoji(file_name, 15, chat)) {
						wchar_t placeholder[] = {0xFE0F, 0};
						SendMessage(chat, EM_REPLACESEL, 0, (LPARAM)placeholder);
						unknown_custom_emoji_solver(message.id, i - deleted_wchars - header_len, 15, custom_emoji_id, false);
					}
					deleted_wchars += format_vecs[9][j + 1] - 1;
					i += format_vecs[9][j + 1] - 1;
					break;
				}
			}
			if (!custom_emoji_id) i = emoji_adder(i, msg, cr_startmsg.cpMin, 15, chat, &deleted_wchars);
			else custom_emoji_id = 0;
			if (newlinks_to_check > 0 && deleted_wchars_cpy != deleted_wchars) {
				int diff = deleted_wchars - deleted_wchars_cpy;
				for (int j = links_count - newlinks_to_check; j < links_count; j++) {
					if (links[j].chrg.cpMin > cr_startmsg.cpMin + i - deleted_wchars) links[j].chrg.cpMin -= diff;
					if (links[j].chrg.cpMax > cr_startmsg.cpMin + i - deleted_wchars) links[j].chrg.cpMax -= diff;
					else newlinks_to_check--;
				}
			}
		}
		written -= deleted_wchars;
		if (added_doc) {
			document.min -= deleted_wchars;
			document.max -= deleted_wchars;
		}
		
	}
	free(msg);

	message.start_char = cr_startmsg.cpMin;
	message.end_header = message.start_char + header_len;
	message.end_char = message.start_char + written - written_info;
	message.end_footer = message.end_char + written_info;
	message.start_reactions = message.start_reactions ? (message.end_char + message.start_reactions) : message.end_footer;
	written += written_groupmedend;
	if (editing) {
		written -= (messages[editing_index].end_footer - messages[editing_index].start_char);
		if (!footer && messages[editing_index].end_char != messages[editing_index].end_footer) {
			written += messages[editing_index].end_footer - messages[editing_index].end_char;
			message.reacted = messages[editing_index].reacted;
			message.end_footer = messages[editing_index].end_footer + written;
		}
		update_positions(written, cr_startmsg.cpMin, new_links);
	}

	if (cr.cpMin != cr.cpMax) {
		if (to_front || (editing && cr.cpMin > messages[editing_index].end_footer)) {
			cr.cpMin += written;
			cr.cpMax += written;
		}
		SendMessage(chat, EM_EXSETSEL, 0, (LPARAM)&cr);
	} else SendMessage(chat, EM_SETSEL, -1, -1);

	bool notify = false;
	if (to_front || editmsgisup) {
		if (to_front) update_positions(written, 0x80000000, new_links);
		if (drawchat) {
			SCROLLINFO si_new = {0};
			si_new.cbSize = sizeof(si_new);
			si_new.fMask = SIF_RANGE;
			GetScrollInfo(chat, SB_VERT, &si_new);
			if (si_new.nMax > si.nMax) {
				if (si.nPos < (int)(si.nMax - si.nPage) - 15) SendMessage(chat, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, si_new.nMax - si.nMax + si.nPos), 0);
				else SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
			}
		}
		if (!message.outgoing && messages.size() == 0) make_seen(&message);
	} else {
		if (si.nPos >= (int)(si.nMax - si.nPage) - 15) {
			SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
			if (!message.outgoing && !editing) {
				if (!IsIconic(hMain) && IsWindowVisible(hMain)) {
					make_seen(&message);
					if (sound_paths[1][0] && !groupmed_end_nottofront) PlaySound(sound_paths[1], NULL, SND_FILENAME | SND_ASYNC);
				}
				else if (!message.outgoing && !editing) new_msg_notification(current_peer, msg_bytes, groupmed_end_nottofront);
			}
		} else {
			SendMessage(chat, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, si.nPos), 0);
			if (!message.outgoing && !editing) new_msg_notification(current_peer, msg_bytes, groupmed_end_nottofront);
		}
		if (message.outgoing && !editing && sound_paths[2][0] && !groupmed_end_nottofront) PlaySound(sound_paths[2], NULL, SND_FILENAME | SND_ASYNC);
	}

	if (drawchat) {
		SendMessage(chat, WM_SETREDRAW, TRUE, 0);
		InvalidateRect(chat, NULL, TRUE);
	}

	if (editing) {
		messages[editing_index] = message;
		if (added_doc) {
			for (int i = documents.size() - 1; i >= -1; i--) if (i == -1 || documents[i].max < document.min) break;
			if (i == -1) documents.push_front(document);
			else documents.insert(documents.begin() + i + 1, document);
		}
	}
	else {
		if (to_front) messages.push_front(message);
		else messages.push_back(message);
		if (to_front) last_tofront_sender = sender;
		if (added_doc) {
			if (to_front) documents.push_front(document);
			else documents.push_back(document);
		}
	}
	if (!chat_member_found && msg_bytes - chat_member_id != 16) free(sender);
	if (!to_front) get_unknown_custom_emojis();
}