/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "telegacy.h"

prng_state prng;
hash_state md;

DCInfo dcInfoMain = {0};
int time_diff = 0;
int pts = 0;
int qts = 0;
int date = 0;

int database_version = 0;
bool test_server = false;
bool drawchat = true;
bool closing = false;
HWND current_dialog = NULL, current_notification = NULL, current_info = NULL, current_about = NULL;
HWND dlgPic, infoLabel;
HBITMAP infoBmp = NULL, infoBmpMask = NULL;
int width = 500;
int height = 450;
bool maximized = false;
wchar_t appdata_path[MAX_PATH];
wchar_t exe_path[MAX_PATH];

BYTE* phone_number_bytes = NULL;
BYTE* phone_code_hash = NULL;
BYTE* qrCodeToken = NULL;

HWND hComboBoxChats, hComboBoxFolders, msgInput, chat, hMain, hStatus, hToolbar, tbSeparatorHider, hTabs, emojiStatic, emojiScroll, hOverlayTabs, reactionStatic, splitter;
HWND hNumber = NULL, hNumberBtn = NULL, hCode = NULL, hCodeBtn = NULL, hQRCode = NULL, h2FA = NULL, hPass = NULL, h2FAHint = NULL;
IActiveIMMApp* g_pAIMM = NULL;
HMENU hMenuBar;
HFONT hDefaultFont = NULL;

Peer myself = {0};
Peer* peers = NULL;
int peers_count = 0;
int total_peers_count = 0;
ChatsFolder* folders;
Peer* current_peer = NULL;
Peer* old_peer = NULL;
ChatsFolder* current_folder = NULL;
int folders_count = 1;
std::deque<Document> documents;
std::vector<Document> downloading_docs;
std::vector<TEXTRANGE> links;
std::deque<Message> messages;
std::deque<wchar_t*> fav_emojis;
std::vector<wchar_t*> reaction_list;
int reaction_hash = 0;
__int64 theme_hash = 0;
int sel_msg_id;
int editing_msg_id = 0;
int replying_msg_id = 0;
int forwarding_msg_id = 0;
BYTE forwarding_peer_id[8];
std::vector<wchar_t*> files;
std::vector<Theme> themes;
HBRUSH theme_brush = NULL;
BYTE* sendMultiMedia = NULL;
BYTE pfp_msgid[8];
BYTE handle_msgid[8];
Peer dlg_peer;
HWND writing_str_from = NULL;
wchar_t status_str[100];
std::vector<RequestedCustomEmoji> rces;
std::list<DCInfo> active_dcs;
int muted_types[3] = {-1, -1, -1};
int total_unread_msgs_count = 0;
BYTE notification_peer_id[8];
BYTE difference_msg_id[8];
std::vector<UnmuteTimer> unmuteTimers;
bool dragging = false;
int edits_border_offset = 0;
bool dontlosefocus = false;
bool needtosetuplogin = false;
bool minimized = false;
bool ischatscrolling = false;
bool ie4 = true;
bool ie3 = true;
bool nt3 = false;
bool hint_needed = false;
wchar_t* hint = NULL;
bool no_more_msgs = false;

bool balloon_notifications_available = false;
bool balloon_notifications = true;
bool SENDMEDIAASFILES = false;
bool CLOSETOTRAY = true;
int IMAGELOADPOLICY = 2;
bool EMOJIS = true;
bool SPOILERS = true;
wchar_t sound_paths[3][MAX_PATH];
int SAMPLERATE;
int BITSPERSAMPLE;
int CHANNELS;

WNDPROC oldMsgInputProc, oldChatProc, oldEmojiScrollProc, oldEmojiStaticProc, oldReactionStaticProc, oldEmojiScrollFallbackProc, oldSplitterProc, oldReactionButtonProc;
LRESULT CALLBACK WndProcMsgInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcChat(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcEmojiScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcEmojiScrollFallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcEmojiStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcReactionStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcReactionButton(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProcSplitter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcLogin(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc2FA(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcInfo(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcOptions(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

wchar_t* last_tofront_sender;
BYTE group_id_tofront[8] = {0};
BYTE group_id[8] = {0};
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
	SETTEXTEX st;
	st.flags = ST_SELECTION;
	st.codepage = 1200;
	int written_groupmedend = 0;
	if (to_front) {
		if (!groupmed_end || (documents.size() > 0 && documents[0].min == 0)) {
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"\n");
			SendMessage(chat, EM_SETSEL, pos, pos);
		} else written_groupmedend--;
		if (groupmed_end) {
			written_groupmedend += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)last_tofront_sender);
			written_groupmedend += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L": ");
			if (!current_peer->perm.cansendmsg) written_groupmedend += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
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

	int written = SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)msg);
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
				SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)placeholder);
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
			written += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)document.filename);
			document.max = cr_startmsg.cpMin + written;
			if (duration_str[0] == ' ') written += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)&duration_str);
			written += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)&size_str);
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
			SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)placeholder);
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
		} else written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
		
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
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"replying to ");
			format_vecs[1].push_back(written - header_len + written_info);
			written_info += msgfwd_addname(msg_bytes, msgrpl_another_chat, cr_startmsg.cpMin + written + written_info, msg_bytes - msg_id == 12);
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L": ");
			written_info += set_reply(-1, cr_startmsg.cpMin + written + written_info, quote_text, false);
			format_vecs[1].push_back(written - header_len + written_info);
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
		} else {
			if (to_front && msgrpl) {
				written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"replying to ");
				if (quote_text) {
					format_vecs[1].push_back(written - header_len + written_info);
					written_info += set_reply(-1, cr_startmsg.cpMin + written + written_info, quote_text, false);
					format_vecs[1].push_back(written - header_len + written_info);
				}
				message_footer->reply_needed = replying_msg_id;
				written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
			}
			if (!to_front && replying_msg_id && !editing) {
				if (!format_vecs) format_vecs = new std::vector<int>[9];
				for (int i = messages.size() - 1; i >= -1; i--) {
					int id = messages[i].id;
					if (i == -1 || messages[i].id == replying_msg_id) break;
				}
				written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"replying to ");
				format_vecs[1].push_back(written - header_len + written_info);
				if (i == -1) {
					if (quote_text) written_info += set_reply(-1, cr_startmsg.cpMin + written + written_info, quote_text, false);
					message_footer->reply_needed = replying_msg_id;
					written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
					get_message(replying_msg_id, current_peer);
				} else {
					written_info += set_reply(i, cr_startmsg.cpMin + written + written_info, quote_text, false);
					written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
				}
				format_vecs[1].push_back(written - header_len + written_info - 1);
			} else if (replying_msg_id && editing) {
				es_rep.pfnCallback = StreamInCallback;
				written_info += SendMessage(chat, EM_STREAMIN, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es_rep);
				StreamData* sd = (StreamData*)es_rep.dwCookie;
				free(sd->buf);
				written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
			}
		}

		if (msgfwd) {
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"forwarded from ");
			written_info += msgfwd_addname(msg_bytes, msgfwd, cr_startmsg.cpMin + written + written_info, msg_bytes - msg_id == 12);
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
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
		written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)info);
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
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"\n");
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
	cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_SIZE | CFM_LINK | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_FACE | CFM_BACKCOLOR;
	wcscpy(cf.szFaceName, L"Arial");
	cf.dwEffects = 0;
	cf.crTextColor = 0;
	cf.crBackColor = RGB(255, 255, 255);
	cf.yHeight = 200;
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

int folder_handler(BYTE* unenc_response, ChatsFolder* folder, int i, bool update) {
	int flags = read_le(unenc_response, 4);
	int offset = 4;
	memcpy(folder->id, unenc_response + offset, 4);
	offset += 8;
	if (update) {
		wchar_t* new_name = read_string(unenc_response + offset, NULL);
		if (wcscmp(new_name, folder->name) != 0) {
			free(folder->name);
			folder->name = new_name;
			SendMessage(hComboBoxFolders, CB_DELETESTRING, i, NULL);
			SendMessage(hComboBoxFolders, CB_INSERTSTRING, i, (LPARAM)folder->name);
			SendMessage(hComboBoxFolders, CB_SETITEMDATA, i, (LPARAM)folder);
			if (folder == current_folder) SendMessage(hComboBoxFolders, CB_SETCURSEL, i, 0);
		} else free(new_name);
	} else folder->name = read_string(unenc_response + offset, NULL);
	offset += tlstr_len(unenc_response + offset, true);
	int msgent_count = read_le(unenc_response + offset + 4, 4);
	offset += 8;
	for (int j = 0; j < msgent_count; j++) offset += msgent_offset(unenc_response, NULL);
	if (!update) SendMessage(hComboBoxFolders, CB_ADDSTRING, 0, (LPARAM)folder->name);
	if (!update) SendMessage(hComboBoxFolders, CB_SETITEMDATA, i, (LPARAM)folder);
	if (flags & (1 << 25)) offset += tlstr_len(unenc_response + offset, true);
	if (flags & (1 << 27)) offset += 4;
	offset += 4;
	folder->count = read_le(unenc_response + offset, 4);
	folder->pinned_count = folders[i].count;
	BYTE vector_cons[4];
	write_le(vector_cons, 0x1cb5c415, 4);
	folder->count += read_le(unenc_response+offset+array_find(unenc_response+offset, vector_cons, 4, 1)+4, 4);
	offset += 4;
	
	std::vector<int> peers_temp(folders[i].count);
	for (j = 0; j < folder->count; j++) {
		int inputpeer = read_le(unenc_response + offset, 4);
		if (inputpeer == 0x1cb5c415) offset += 8;
		offset += 4;
		bool found = false;
		for (int k = 0; k < peers_count; k++) {
			if (memcmp(unenc_response + offset, peers[k].id, 8) == 0) {
				peers_temp[j] = k;
				found = true;
				break;
			}
		}
		offset += (inputpeer == 0x35a95cb9) ? 8 : 16;
		if (!found) {
			folder->count--;
			j--;
		}
	}
	std::sort(peers_temp.begin() + folder->pinned_count, peers_temp.begin() + folder->count);
	if (update) {
		free(folder->peers);
		if (folder == current_folder) SendMessage(hComboBoxChats, CB_RESETCONTENT, 0, 0);
	}
	folder->peers = (int*)malloc(sizeof(int*)*folder->count);
	for (j = 0; j < folder->count; j++) {
		folder->peers[j] = peers_temp[j];
		if (update && folder == current_folder) {
			SendMessage(hComboBoxChats, CB_ADDSTRING, 0, (LPARAM)peers[folder->peers[j]].name);
			SendMessage(hComboBoxChats, CB_SETITEMDATA, j, (LPARAM)&peers[folder->peers[j]]);
			if (&peers[folder->peers[j]] == current_peer) SendMessage(hComboBoxChats, CB_SETCURSEL, j, 0);
		}
	}
	return offset;
}

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

int update_handler(BYTE* update) {
	int bytes_count = 4;
	unsigned int update_constructor = read_le(update, 4);
	switch (update_constructor) {
	case 0xc01e857f: // updateUserTyping
	case 0x83487af0: // updateChatUserTyping
	case 0x8c88c923: { // updateChannelUserTyping
		wchar_t* name = current_peer ? current_peer->name : NULL;
		bytes_count += 8;
		bool changing_status_bar = false;
		bool name_allocated = false;
		if (update_constructor == 0xc01e857f) {
			if (current_peer && memcmp(update + 4, current_peer->id, 8) == 0) changing_status_bar = true;
		} else if (update_constructor == 0x83487af0) {
			if (current_peer && memcmp(update + 4, current_peer->id, 8) == 0) changing_status_bar = true;
			if (changing_status_bar && memcmp(current_peer->id, update + 16, 8) != 0) {
				for (int i = 0; i < current_peer->chat_users->size(); i++) {
					if (memcmp(current_peer->chat_users->at(i).id, update + 16, 8) == 0) {
						name = current_peer->chat_users->at(i).name;
						break;
					}
				}
			}
			bytes_count += 12;
		} else {
			if (current_peer && memcmp(update + 8, current_peer->id, 8) == 0) changing_status_bar = true;
			bytes_count += 4;
			if (update[4]) bytes_count += 4;

			if (changing_status_bar && memcmp(current_peer->id, update + bytes_count + 4, 8) != 0) {
				char type = -1;
				BYTE* peer_bytes = find_peer(update + bytes_count + 12, update + bytes_count, true, &type);
				peer_set_name(peer_bytes, &name, type);
				name_allocated = true;
			}

			bytes_count += 12;
		}
		if (changing_status_bar) {
			KillTimer(hMain, 0);
			int sendmsgaction_cons = read_le(update + bytes_count, 4);
			switch (sendmsgaction_cons) {
			case 0xfd5ec8f5: // sendMessageCancelAction
				status_bar_status(current_peer);
				break;
			case 0x16bf744e:
				swprintf(status_str, L"%s is typing...", name);
				break;
			case 0xa187d66f:
				swprintf(status_str, L"%s is recording a video...", name);
				break;
			case 0xe9763aec:
				swprintf(status_str, L"%s is uploading a video...", name);
				break;
			case 0xd52f73f7:
				swprintf(status_str, L"%s is recording a voice message...", name);
				break;
			case 0xf351d7ab:
				swprintf(status_str, L"%s is uploading a voice message...", name);
				break;
			case 0xd1d34a26:
				swprintf(status_str, L"%s is uploading a photo...", name);
				break;
			case 0xaa0cd9e4:
				swprintf(status_str, L"%s is uploading a file...", name);
				break;
			case 0x88f27fbc:
				swprintf(status_str, L"%s is recording a round video...", name);
				break;
			case 0x243e1c66:
				swprintf(status_str, L"%s is uploading a round video...", name);
				break;
			case 0xb05ac6b1:
				swprintf(status_str, L"%s is choosing a sticker...", name);
				break;
			}
			if (sendmsgaction_cons != 0xfd5ec8f5) {
				SendMessage(hStatus, SB_SETTEXTA, 0 | SBT_OWNERDRAW, wcslen(name));
				SetTimer(hMain, 0, 6000, NULL);
			}
			if (name_allocated) free(name);
		}
		bytes_count += sendmsgaction_offset(update + bytes_count);
		break;
	}
	case 0xe5bdf8de: { // updateUserStatus
		if (read_le(update + 4, 4) == 0x9d05049) bytes_count += 4;
		else bytes_count += 8;
		for (int i = 0; i < peers_count; i++) {
			if (memcmp(peers[i].id, update + 4, 8) == 0) {
				user_status_updated(update + 12, &peers[i]);
				break;
			}
		}
		break;
	}
	case 0xb75f99a9: // updateReadChannelOutbox
	case 0x2f2f21bf: { // updateReadHistoryOutbox
		int offset = update_constructor == 0xb75f99a9 ? 4 : 8;
		for (int i = 0; i < peers_count; i++) {
			if (memcmp(peers[i].id, update + offset, 8) == 0) {
				peers[i].last_read = read_le(update + offset + 8, 4);
				if (current_peer == &peers[i]) for (int j = messages.size() - 1; j >= 0; j--) {
					if (messages[j].outgoing) {
						if (messages[j].seen) break;
						else {
							messages[j].seen = true;
							FINDTEXTEX ft;
							ft.chrg.cpMin = messages[j].end_char;
							ft.chrg.cpMax = messages[j].end_footer;
							ft.lpstrText = L"delivered";
							int diff = replace_in_chat(&ft, NULL, L"seen", NULL, NULL, NULL, NULL);
							messages[j].end_footer += diff;
						}
					}
				}
				break;
			}
		}
		if (update_constructor == 0x2f2f21bf) {
			update_pts(read_le(update + 20, 4));
			bytes_count += 24;
		} else bytes_count += 12;
		break;
	}
	case 0x922e6e10: // updateReadChannelInbox
	case 0x9c974fdf: { // updateReadHistoryInbox
		int flags = read_le(update + 4, 4);
		bytes_count += 4;
		if (flags & (1 << 0)) bytes_count += 4;
		if (update_constructor == 0x9c974fdf) bytes_count += 4;
		int index = -1;
		for (int i = 0; i < peers_count; i++) {
			if (memcmp(peers[i].id, update + bytes_count, 8) == 0) {
				int unread_count_old = peers[i].unread_msgs_count;
				peers[i].unread_msgs_count = read_le(update + bytes_count + 12, 4);
				if (unread_count_old != peers[i].unread_msgs_count && !peers[i].mute_until && !muted_types[peers[i].type]) update_total_unread_msgs_count(peers[i].unread_msgs_count - unread_count_old);
				if (memcmp(peers[i].id, notification_peer_id, 8) == 0 && !peers[i].unread_msgs_count) remove_notification();
				index = i;
				break;
			}
		}
		bytes_count += 20;
		if (update_constructor == 0x9c974fdf) {
			update_pts(read_le(update + bytes_count - 4, 4));
			bytes_count += 4;
		} else if (index != -1) peers[index].channel_pts = read_le(update + bytes_count - 4, 4);
		break;
	}
	case 0x7761198: { // updateChatParticipants
		int cons = read_le(update + 4, 4);
		if (cons == 0x3cbc93f8) {
			for (int i = peers_count - 1; i >= 0; i--) {
				if (memcmp(peers[i].id, update + 8, 4) == 0) {
					if (!peers[i].full) break;
					for (int j = 0; j < peers[i].chat_users->size(); j++) {
						free(peers[i].chat_users->at(j).name);
						if (peers[i].chat_users->at(j).handle) free(peers[i].chat_users->at(j).handle);
					}
					peers[i].chat_users->clear();
					int chat_users_count = read_le(update + 20, 4);
					int offset = 24;
					BYTE user_cons[4];
					write_le(user_cons, 0x4b46c37e, 4);
					int user_pos = offset;
					for (j = 0; j < chat_users_count; j++) {
						user_pos += array_find(update + user_pos + 4, user_cons, 4, 1) + 4;
						Peer peer;
						peer.type = 0;
						set_peer_info(update + user_pos, &peer, false);
						peers[i].chat_users->push_back(peer);
						offset += (read_le(update + offset, 4) == 0xe46bcee4) ? 12 : 24;
					}
					if (current_peer == &peers[i]) status_bar_status(current_peer);
				}
			}
		}
		break;
	}
	case 0xe40370a3: // updateEditMessage
		bytes_count += message_handler(false, update + 4, false, true, false);
		update_pts(read_le(update + bytes_count, 4));
		bytes_count += 8;
		break;
	case 0x1b3f4df7: { // updateEditChannelMessage
		bytes_count += message_handler(false, update + 4, false, true, false);
		BYTE* id;
		get_peerid_from_msg(update + 4, &id, NULL);
		for (int i = 0; i < peers_count; i++) {
			if (memcmp(peers[i].id, id, 8) == 0) {
				peers[i].channel_pts = read_le(update + bytes_count, 4);
				break;
			}
		}
		bytes_count += 8;
		break;
	}
	case 0xf226ac08: // updateChannelMessageViews
		bytes_count += 16;
		if (current_peer && memcmp(current_peer->id, update + 4, 8) == 0) {
			for (int i = messages.size() - 1; i >= 0; i--) {
				if (memcmp(&messages[i].id, update + 12, 4) == 0) {
					FINDTEXTEX ft;
					ft.chrg.cpMin = messages[i].end_char;
					ft.chrg.cpMax = messages[i].end_footer;
					ft.lpstrText = L"view";
					SendMessage(chat, EM_FINDTEXTEX, 0, (LPARAM)&ft);
					ft.chrg.cpMax = ft.chrgText.cpMax;
					ft.lpstrText = L"|";
					ft.chrgText.cpMin = SendMessage(chat, EM_FINDTEXT, 0, (LPARAM)&ft) + 2;
					ft.chrg.cpMin = ft.chrgText.cpMin;
					ft.chrg.cpMax = messages[i].end_footer;
					ft.chrgText.cpMax = SendMessage(chat, EM_FINDTEXT, FR_DOWN, (LPARAM)&ft) - 2;
					int views_count = read_le(update + 16, 4);
					wchar_t views[20];
					swprintf(views, L"%d views", views_count);
					int diff = replace_in_chat(NULL, &ft.chrgText, views, NULL, NULL, NULL, NULL);
					messages[i].end_footer += diff;
					break;
				}
			}
		}
		break;
	case 0x5e1b3cb8: { // updateMessageReactions
		bytes_count += 36;
		if (current_peer && memcmp(current_peer->id, update + 12, 8) == 0) {
			for (int i = 0; i < messages.size(); i++) {
				if (memcmp(&messages[i].id, update + 20, 4) == 0) {
					while (messages[i].end_char == messages[i].end_footer) i++;
					messages[i].reacted.clear();
					int flags = read_le(update + 4, 4);
					if (flags & (1 << 0)) bytes_count += 4;
					if (flags & (1 << 1)) bytes_count += 12;
					if (flags) break;
					CHARRANGE cr;
					cr.cpMin = messages[i].start_reactions;
					cr.cpMax = messages[i].end_footer - 2;
					memcpy(update + bytes_count - 12, &messages[i].id, 4);
					memcpy(update + bytes_count - 8, &i, 4);
					int diff = replace_in_chat(NULL, &cr, NULL, NULL, update + bytes_count - 4, NULL, NULL);
					messages[i].end_footer += diff;
					get_unknown_custom_emojis();
					break;
				}
			}
		}
		break;
	}
	case 0xa20db0e5: { // updateDeleteMessages
		int count = read_le(update + 8, 4);
		bytes_count += 8;
		bool deleted = false;
		for (int i = 0; i < count; i++) {
			bytes_count += 4;
			int deleted_msg_id = read_le(update + 12 + 4 * i, 4);
			for (int j = 0; j < messages.size(); j++) {
				if (deleted_msg_id == messages[j].id) {
					deleted = true;
					delete_message(j, false);
					break;
				}
			}
		}
		if (deleted) SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
		update_pts(read_le(update + bytes_count, 4));
		bytes_count += 8;
		break;			 
	}
	case 0x54c01850: { // updateChatDefaultBannedRights
		for (int i = 0; i < peers_count; i++) {
			if (memcmp(peers[i].id, update + 8, 8) == 0) {
				if (!peers[i].amadmin) set_permissions(update + 16, &peers[i]); 
				break;
			}
		}
		bytes_count += 28;
		break;
	}
	case 0xa7848924: { // updateUserName
		bytes_count += 8;
		for (int i = -1; i < peers_count; i++) {
			Peer* peer = (i == -1) ? &myself : &peers[i];
			if (memcmp(peer->id, update + 4, 8) == 0) {
				free(peer->name);
				bytes_count += set_name(update + 12, &peer->name);
				if (i > -1) update_name_in_list(i);
				int count = read_le(update + bytes_count + 4, 4);
				bytes_count += 8;
				for (int j = 0; j < count; j++) {
					int flags = read_le(update + bytes_count + 4, 4);
					bytes_count += 8;
					if (flags & (1 << 1)) {
						if (peer->handle) free(peer->handle);
						peer->handle = read_string(update + bytes_count, NULL);
					}
					bytes_count += tlstr_len(update + bytes_count, true);
				}
				break;
			}
		}
		break;
	}
	case 0x1f2b0afd: { // updateNewMessage
		bytes_count += message_handler(false, update + 4, true, false, false);
		update_pts(read_le(update + bytes_count, 4));
		bytes_count += 8;
		break;				 
	}
	case 0x62ba04d9: { // updateNewChannelMessage
		bytes_count += message_handler(false, update + 4, true, false, false);
		BYTE* id;
		get_peerid_from_msg(update + 4, &id, NULL);
		for (int i = 0; i < peers_count; i++) {
			if (memcmp(peers[i].id, id, 8) == 0) {
				peers[i].channel_pts = read_le(update + bytes_count, 4);
				break;
			}
		}
		bytes_count += 8;
		break;			 
	}
	case 0x26ffde7d: { // updateDialogFilter
		int flags = read_le(update + 4, 4);
		if (flags & (1 << 0)) {
			bool found = false;
			for (int i = 1; i < folders_count; i++) {
				if (memcmp(folders[i].id, update + 8, 4) == 0) {
					folder_handler(update + 16, &folders[i], i, true);
					found = true;
					break;
				}
			}
			if (!found) {
				folders_count++;
				int current_folder_pos = current_folder-folders;
				folders = (ChatsFolder*)realloc(folders, folders_count * sizeof(ChatsFolder));
				current_folder = &folders[current_folder_pos];
				for (int i = 0; i < folders_count; i++) SendMessage(hComboBoxFolders, CB_SETITEMDATA, i, (LPARAM)&folders[i]);
				folder_handler(update + 16, &folders[folders_count-1], folders_count-1, false);
			}
		} else {
			for (int i = 1; i < folders_count; i++) {
				if (memcmp(folders[i].id, update + 8, 4) == 0) {
					free(folders[i].peers);
					free(folders[i].name);
					SendMessage(hComboBoxFolders, CB_DELETESTRING, i, 0);
					for (int j = i + 1; j < folders_count; j++) folders[i-1] = folders[i];
					folders_count--;
					int current_folder_pos = current_folder-folders;
					folders = (ChatsFolder*)realloc(folders, folders_count * sizeof(ChatsFolder));
					current_folder = &folders[current_folder_pos];
					for (j = 0; j < folders_count; j++) SendMessage(hComboBoxFolders, CB_SETITEMDATA, j, (LPARAM)&folders[j]);
					break;
				}
			}
		}
		break;
	}
	case 0xa5d72105: { // updateDialogFilterOrder
		ChatsFolder temp;
		int current_folder_index = 0;
		bytes_count += 8 + read_le(update + 8, 4) * 4;
		for (int i = 0; i < folders_count; i++) {
			temp = folders[i];
			for (int j = i + 1; j < folders_count; j++) {
				if (memcmp(update + 12 + i * 4, folders[j].id, 4) == 0) {
					folders[i] = folders[j];
					folders[j] = temp;
					if (current_folder == &folders[i]) {
						current_folder = &folders[j];
						current_folder_index = j;
					} else if (current_folder == &folders[j]) {
						current_folder = &folders[i];
						current_folder_index = i;
					}
					break;
				}
			}
		}
		SendMessage(hComboBoxFolders, CB_RESETCONTENT, 0, 0);
		for (i = 0; i < folders_count; i++) {
			SendMessage(hComboBoxFolders, CB_ADDSTRING, 0, (LPARAM)folders[i].name);
			SendMessage(hComboBoxFolders, CB_SETITEMDATA, i, (LPARAM)&folders[i]);
		}
		SendMessage(hComboBoxFolders, CB_SETCURSEL, current_folder_index, 0);
		break;
	}
	case 0xf89a6a4e: // updateChat
	case 0x635b4c09: // updateChannel
	case 0x20529438: { // updateUser
		bytes_count += 8;
		Peer* peer = memcmp(update + 4, myself.id, 8) == 0 ? &myself : NULL; 
		if (!peer) for (int i = 0; i < peers_count; i++) {
			if (memcmp(update + 4, peers[i].id, 8) == 0) {
				peer = &peers[i];
				break;
			}
		}
		if (peer) {
			BYTE* peer_bytes = find_peer(update + bytes_count, &peer->id[0] - 4, true, &peer->type);
			if (peer->type != 0 && (peer_bytes[4] & (1 << 2))) remove_peer(peer);
			else {
				if (peer->full) get_full_peer(peer);
				free(peer->name);
				if (peer->handle) free(peer->handle);
				if (peer->full && peer->about) free(peer->about);
				set_peer_info(peer_bytes, peer, false);
			}
		} else {
			char type = 0;
			if (update_constructor == 0x635b4c09) type = 2;
			else if (update_constructor == 0xf89a6a4e) type = 1;
			update_chats_order(update + 4, update + 4, type);
		}
		break;
	}
	case 0x564fe691: // updateLoginToken
		if (current_dialog && !dcInfoMain.authorized) SetTimer(current_dialog, 1, 0, NULL);
		break;
	case 0x8e5e9873: // updateDcOptions
		bytes_count += save_dcs(update + 8, (BYTE*)&dcInfoMain.dc) + 4;
		break;
	case 0xbec268ef: { // updateNotifySettings
		bytes_count += 24;
		int notify_cons = read_le(update + 4, 4);
		int type = -1;
		switch (notify_cons) {
		case 0x9fd40bd8: {
			for (int i = 0; i < peers_count; i++) {
				if (memcmp(peers[i].id, update + 12, 8) == 0) {
					int mute_until_old = peers[i].mute_until;
					apply_notifysettings(update + 20, &peers[i].mute_until, read_le(peers[i].id, 8));
					if ((!mute_until_old && peers[i].mute_until) || (mute_until_old && !peers[i].mute_until)) {
						if (peers[i].unread_msgs_count && !muted_types[peers[i].type])
							update_total_unread_msgs_count(mute_until_old ? peers[i].unread_msgs_count : (0 - peers[i].unread_msgs_count));
						if (&peers[i] == current_peer) {
							HMENU hMenuChat = GetSubMenu(hMenuBar, 1);
							ModifyMenu(hMenuChat, 1, MF_BYPOSITION | MF_STRING, 41, mute_until_old ? L"Mute this chat" : L"Unmute this chat");
						}
					}
					break;
				}
			}
			break;
		}
		case 0xb4c83b4c:
			type = 0;
		case 0xc007cec3:
			if (type == -1) type = 1;
		case 0xd612e8ef:
			if (type == -1) type = 2;
			int muted_type_old = muted_types[type];
			apply_notifysettings(update + 8, &muted_types[type], type);
			if ((!muted_type_old && muted_types[type]) || (muted_type_old && !muted_types[type])) {
				for (int i = 0; i < peers_count; i++) {
					if (peers[i].type == type && peers[i].unread_msgs_count && !peers[i].mute_until)
						update_total_unread_msgs_count(muted_type_old ? peers[i].unread_msgs_count : (0 - peers[i].unread_msgs_count));
				}
				change_mute_all(type, muted_type_old ? true : false);
			}
			break;
		}
		break;
	}
	case 0x108d941f: { // updateChannelTooLong
		bytes_count += 12;
		int flags = read_le(update + 4, 4);
		if (flags & (1 << 0)) bytes_count += 4;
		for (int i = 0; i < peers_count; i++) {
			if (memcmp(peers[i].id, update + 8, 8) == 0) {
				get_channel_difference(&peers[i]);
				break;
			}
		}
		break;
	}
	case 0x3354678f: // updatePtsChanged
		get_state(false);
		break;
	case 0x4e90bfd6: // updateMessageID
		bytes_count += 12;
		break;
	case 0x4b46c37e: // user
	case 0x41cbf256: // chat
	case 0xe00998b7: // channel
		bytes_count = 1000000;
		break;
	/*default:
		wchar_t text[15];
		wsprintf(text, L"%x", update_constructor);
		MessageBox(NULL, text, L"Update", MB_OK | MB_ICONINFORMATION);*/
	}
	date = current_time();
	return bytes_count;
}

int set_peer_info(BYTE* unenc_response, Peer* peer, bool just_update) {
	if (!just_update) {
		peer->reaction_list = &reaction_list;
		peer->full = false;
	} else {
		free(peer->name);
		if (peer->handle) free(peer->handle);
	}
	peer->last_recv = 0;
	peer->name_set_time = current_time();
	peer->pfp_set_time = current_time();
	peer->status_updated = true;
	int flags = read_le(unenc_response + 4, 4);
	int offset = 8;
	if (peer->type == 0) {
		bool min = (flags & (1 << 20)) ? true : false;
		peer->amadmin = false;
		offset += 4;
		memcpy(peer->id, unenc_response + offset, 8);
		offset += 8;
		if (flags & (1 << 0)) {
			if (!min) memcpy(peer->access_hash, unenc_response + offset, 8);
			offset += 8;
		}
		if ((flags & (1 << 10)) && peer->online != -2 && !min) {
			peer->name = _wcsdup(L"Saved Messages");
			if (flags & (1 << 2)) offset += set_name(unenc_response + offset, NULL);
			else offset += tlstr_len(unenc_response + offset, true);
		}
		else if (flags & (1 << 2)) offset += set_name(unenc_response + offset, min ? NULL : &peer->name);
		else if (flags & (1 << 13)) peer->name = _wcsdup(L"Deleted User");
		else if (!min) {
			peer->name = read_string(unenc_response + offset, NULL);
			offset += tlstr_len(unenc_response + offset, true);
		}
		if (flags & (1 << 3)) {
			if (!min) peer->handle = read_string(unenc_response + offset, NULL);
			offset += tlstr_len(unenc_response + offset, true);
		} else peer->handle = NULL;
		if (flags & (1 << 4)) offset += tlstr_len(unenc_response + offset, true);
		if (flags & (1 << 5)) {
			if (read_le(unenc_response + offset, 4) == 0x82d1f706) {
				if (!min || (flags & (1 << 25))) memcpy(peer->photo, unenc_response + offset + 8, 8);
				offset += chatphoto_offset(unenc_response + offset);
				if (!min || (flags & (1 << 25))) peer->photo_dc = read_le(unenc_response + offset - 4, 4);
			} else {
				if (!min || (flags & (1 << 25))) memset(peer->photo, 0, 8);
				offset += 4;
			}
		} else memset(peer->photo, 0, 8);
		memset(&peer->perm, 1, sizeof(Permissions));
		if (!(flags & (1 << 10))) peer->perm.canchangedesc = false;
		if (!min && (flags & (1 << 6)) && !(flags & (1 << 10))) user_status_updated(unenc_response + offset, peer);
		else if (!min) peer->online = -1;
	} else if (peer->type == 1) {
		if (flags & (1 << 0)) peer->amadmin = true;
		else peer->amadmin = false;
		peer->handle = NULL;
		memcpy(peer->id, unenc_response + offset, 8);
		offset += 8;
		peer->name = read_string(unenc_response + offset, NULL);
		offset += tlstr_len(unenc_response + offset, true);
		if (read_le(unenc_response + offset, 4) == 0x1c6e1c11) {
			memcpy(peer->photo, unenc_response + offset + 8, 8);
			offset += chatphoto_offset(unenc_response + offset);
			peer->photo_dc = read_le(unenc_response + offset - 4, 4);
		} else {
			memset(peer->photo, 0, 8);
			offset += 4;
		}
		offset += 12;
		if (flags & (1 << 6)) offset += inputchannel_offset(unenc_response + offset);
		if (flags & (1 << 14)) offset += 8;
		if (!peer->amadmin && (flags & (1 << 18))) set_permissions(unenc_response + offset, peer);
		else memset(&peer->perm, 1, sizeof(Permissions));
		offset += 12;
	} else {
		memset(peer->channel_msg_id, 0, 8);
		bool min = (flags & (1 << 12)) ? true : false;
		if (flags & (1 << 0)) peer->amadmin = true;
		else peer->amadmin = false;
		offset += 4;
		memcpy(peer->id, unenc_response + offset, 8);
		offset += 8;
		if (flags & (1 << 13)) {
			if (!min) memcpy(peer->access_hash, unenc_response + offset, 8);
			offset += 8;
		}
		if (!min) peer->name = read_string(unenc_response + offset, NULL);
		offset += tlstr_len(unenc_response + offset, true);
		if (flags & (1 << 6)) {
			if (!min) peer->handle = read_string(unenc_response + offset, NULL);
			offset += tlstr_len(unenc_response + offset, true);
		} else peer->handle = NULL;
		if (read_le(unenc_response + offset, 4) == 0x1c6e1c11) {
			memcpy(peer->photo, unenc_response + offset + 8, 8);
			offset += chatphoto_offset(unenc_response + offset);
			peer->photo_dc = read_le(unenc_response + offset - 4, 4);
		} else {
			memset(peer->photo, 0, 8);
			offset += 4;
		}
		offset += 4;
		if (flags & (1 << 9)) {
			int count = read_le(unenc_response + offset + 4, 4);
			offset += 8;
			for (int i = 0; i < count; i++) {
				offset += 4;
				for (int j = 0; j < 3; j++) offset += tlstr_len(unenc_response + offset, true);
			}
		}
		if (flags & (1 << 14)) offset += 8;
		if (flags & (1 << 15)) offset += 12;
		if (!peer->amadmin && (flags & (1 << 5))) memset(&peer->perm, 0, sizeof(Permissions));
		else if (!peer->amadmin && (flags & (1 << 18))) set_permissions(unenc_response + offset, peer);
		else memset(&peer->perm, 1, sizeof(Permissions));
	}
	return offset;
}

void update_chats_order(BYTE* id, BYTE* msg_id, char type) {
	for (int i = 0; i < peers_count + 1; i++) {
		bool not_found = (i == peers_count) ? true : false;
		if (not_found || memcmp(peers[i].id, id, 8) == 0) {
			if (i == 0 || (!not_found && peers[i].name == NULL)) break;
			if (not_found) {
				BYTE* peer_bytes = msg_id;
				if (msg_id == id) {
					peer_bytes = find_peer(peer_bytes, id - 4, true, &type);
					if (type != 0 && (peer_bytes[4] & (1 << 2))) return;
				}
				peers_count++;
				int current_peer_pos = current_peer-peers;
				peers = (Peer*)realloc(peers, peers_count * sizeof(Peer));
				memcpy(peers[peers_count-1].id, id, 8);
				folders[0].peers = (int*)realloc(folders[0].peers, peers_count * sizeof(int));
				folders[0].count++;
				folders[0].peers[folders[0].count-1] = folders[0].count-1;
				peers[peers_count-1].type = type;
				peers[peers_count-1].last_read = 0;
				peers[peers_count-1].mute_until = 0;
				peers[peers_count-1].unread_msgs_count = 0;
				if (current_peer != NULL) current_peer = &peers[current_peer_pos];
				if (msg_id != id) {
					peers[peers_count - 1].name = NULL;
					get_message(read_le(msg_id, 4), &peers[peers_count-1]);
				} else {
					set_peer_info(peer_bytes, &peers[peers_count-1], false);
					not_found = false;
				}
			}
			Peer peer = peers[i];
			for (int j = i; j > 0; j--) memcpy(&peers[j], &peers[j-1], sizeof(Peer));
			memcpy(&peers[0], &peer, sizeof(Peer));
			if (current_peer == &peers[i]) current_peer = &peers[0];
			else if (current_peer != NULL && current_peer - peers < i) current_peer++;
			for (j = 0; j < folders_count; j++) {
				for (int k = 0; k < folders[j].count; k++) {
					if (folders[j].peers[k] < i) {
						folders[j].peers[k]++;
						if (&folders[j] == current_folder) SendMessage(hComboBoxChats, CB_SETITEMDATA, k, (LPARAM)&peers[folders[j].peers[k]]);
					} else if (folders[j].peers[k] == i) {
						if (k < folders[j].pinned_count) {
							folders[j].peers[k] = 0;
							if (&folders[j] == current_folder) SendMessage(hComboBoxChats, CB_SETITEMDATA, k, (LPARAM)&peers[0]);
						} else {
							if (!not_found && &folders[j] == current_folder) SendMessage(hComboBoxChats, CB_DELETESTRING, k, 0);
							for (int l = k; l > folders[j].pinned_count; l--) folders[j].peers[l] = folders[j].peers[l-1];
							folders[j].peers[folders[j].pinned_count] = 0;
							if (!not_found && &folders[j] == current_folder) SendMessage(hComboBoxChats, CB_INSERTSTRING, folders[j].pinned_count, (LPARAM)peers[0].name);
							if (!not_found && &folders[j] == current_folder) SendMessage(hComboBoxChats, CB_SETITEMDATA, folders[j].pinned_count, (LPARAM)&peers[0]);
							if (!not_found && &folders[j] == current_folder && current_peer == &peers[0]) SendMessage(hComboBoxChats, CB_SETCURSEL, folders[j].pinned_count, 0);
							break;
						}
					} else if (k >= folders[j].pinned_count && folders[j].peers[k] > i) break;
				}
			}
			break;
		}
	}
}

void download_file(DCInfo* dcInfo, Document* document) {
	BYTE unenc_query[144];
	BYTE enc_query[168];
	internal_header(dcInfo, unenc_query, true);
	memcpy(document, unenc_query + 16, 8);
	
	write_le(unenc_query + 32, 0xbe5335be, 4);
	memset(unenc_query + 36, 0, 4);
	if (document->photo_size && document->photo_size != 1) write_le(unenc_query + 40, 0x40181ffe, 4);
	else write_le(unenc_query + 40, 0xbad07584, 4);
	memcpy(unenc_query + 44, document->id, 8);
	memcpy(unenc_query + 52, document->access_hash, 8);
	int fileref_len = tlstr_len(document->file_reference, true);
	memcpy(unenc_query + 60, document->file_reference, fileref_len);
	int offset = 60 + fileref_len;
	memset(unenc_query + offset, 0, 4);
	if (document->photo_size) {
		unenc_query[offset] = 1;
		unenc_query[offset + 1] = document->photo_size;
	}
	offset += 4;
	HANDLE h = CreateFile(document->filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	__int64 file_size = 0;
	if (h != INVALID_HANDLE_VALUE) {
		unsigned long low, high;
		low = GetFileSize(h, &high);
		file_size = ((__int64)high << 32) | low;
		CloseHandle(h);
		write_le(unenc_query + offset, file_size, 8);
	} else memset(unenc_query + offset, 0, 8);
	offset += 8;
	write_le(unenc_query + offset, 1048576, 4);
	offset += 4;
	write_le(unenc_query + 28, offset - 32, 4);
	int padding_len = get_padding(offset);
	fortuna_read(unenc_query + offset, padding_len, &prng);
	offset += padding_len;
	convert_message(dcInfo, unenc_query, enc_query, offset, 0);
	send_query(dcInfo, enc_query, offset + 24);
	wchar_t status_str[100];
	int percentage = (int)((double)file_size / (double)document->size * 100.0);
	swprintf(status_str, L"Downloading %s... %d%%", document->filename, percentage);
	SendMessage(hStatus, SB_SETTEXTA, 1 | SBT_OWNERDRAW, (LPARAM)status_str);
}

struct uploadThreadEvent {
	HANDLE event;
	BYTE lastId[8];
};
std::vector<uploadThreadEvent*> uploadEvents;
BYTE last_rpcresult_msgid[8];
void response_handler(DCInfo* dcInfo, BYTE* unenc_response, bool acknowledgement, int length) {
	unsigned int constructor = read_le(unenc_response, 4);
	switch (constructor) {
	case 0x9ec20908: // new_session_created
		break;
	case 0x347773c5: // pong
		acknowledgement = false;
		if (needtosetuplogin) {
			wchar_t* version = NULL;
			OSVERSIONINFO osvi;
			osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			GetVersionEx(&osvi);
			bool server = false;
			if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT && (osvi.dwMajorVersion > 4 || wcscmp(osvi.szCSDVersion, L"Service Pack 6") == 0)) {
				OSVERSIONINFOEX osviex;
				osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
				GetVersionEx((OSVERSIONINFO*)&osviex);
				if (osviex.wProductType == VER_NT_SERVER) server = true;
			}
			if (osvi.dwMajorVersion == 3) {
				if (osvi.dwMinorVersion == 10) version = L"Windows NT 3.1";
				else if (osvi.dwMinorVersion == 50) version = L"Windows NT 3.5";
				else if (osvi.dwMinorVersion == 51) version = L"Windows NT 3.51";
			} else if (osvi.dwMajorVersion == 4) {
				if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
					if (server) version = L"Windows NT 4.0 Server";
					else version = L"Windows NT 4.0";
				} else if (osvi.dwMinorVersion == 0) version = L"Windows 95";
				else if (osvi.dwMinorVersion == 10) {
					if (wcscmp(osvi.szCSDVersion, L" A ") != 0) version = L"Windows 98 FE";
					else version = L"Windows 98 SE";
				} else if (osvi.dwMinorVersion == 90) version = L"Windows ME";
			} else if (osvi.dwMajorVersion == 5) {
				if (osvi.dwMinorVersion == 0) {
					if (server) version = L"Windows 2000 Server";
					else version = L"Windows 2000";
				} else if (osvi.dwMinorVersion == 1) version = L"Windows XP";
				else if (osvi.dwMinorVersion == 2) version = L"Windows Server 2003";
			} else if (osvi.dwMajorVersion == 6) {
				if (osvi.dwMinorVersion == 0) {
					if (server) version = L"Windows Server 2008";
					else version = L"Windows Vista";
				} else if (osvi.dwMinorVersion == 1) {
					if (server) version = L"Windows Server 2008 R2";
					else version = L"Windows 7";
				} else if (osvi.dwMinorVersion == 2) {
					if (server) version = L"Windows Server 2012 or above";
					else version = L"Windows 8 or above";
				}
			}
			if (version == NULL) {
				if (server) version = L"Windows Server";
				else version = L"Windows";
			}

			BYTE unenc_query[208];
			BYTE enc_query[232];
			internal_header(unenc_query, true);
			write_le(unenc_query + 32, 0xda9b0d0d, 4);
			write_le(unenc_query + 36, 196, 4);
			write_le(unenc_query + 40, 0xc1cd5ea9, 4);
			memset(unenc_query + 44, 0, 4);
			write_le(unenc_query + 48, 27752131, 4);
			wchar_t pc_name[MAX_COMPUTERNAME_LENGTH + 1];
			DWORD size = sizeof(pc_name);
			GetComputerName(pc_name, &size);
			write_string(unenc_query + 52, pc_name);
			int offset = 52 + tlstr_len(unenc_query + 52, true);
			write_string(unenc_query + offset, version);
			offset += tlstr_len(unenc_query + offset, true);
			write_string(unenc_query + offset, L"1.0.0");
			write_string(unenc_query + offset + 8, L"en");
			write_string(unenc_query + offset + 12, L"tdesktop");
			write_string(unenc_query + offset + 24, L"en");
			if (qrCodeToken) {
				write_le(unenc_query + offset + 28, 0x95ac5ce4, 4);
				int token_len = tlstr_len(qrCodeToken, true);
				memcpy(unenc_query + offset + 32, qrCodeToken, token_len);
				offset += 32 + token_len;
				free(qrCodeToken);
				qrCodeToken = NULL;
			} else {
				write_le(unenc_query + offset + 28, 0xb7e085fe, 4);
				write_le(unenc_query + offset + 32, 27752131, 4);
				write_string(unenc_query + offset + 36, L"f60c7955c55ad59d438d007f1fd59c0d");
				write_le(unenc_query + offset + 72, 0x1cb5c415, 4);
				memset(unenc_query + offset + 76, 0, 4);
				offset += 80;
				place_dialog_center(current_dialog, false);
				SetForegroundWindow(current_dialog);
				if (current_info) ShowWindow(current_info, SW_HIDE);
			}
			write_le(unenc_query + 28, offset - 32, 4);
			int padding_len = get_padding(offset);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			offset += padding_len;
			convert_message(unenc_query, enc_query, offset, 0);
			send_query(enc_query, offset + 24);
			needtosetuplogin = false;
		}
		break;
	case 0xcc1a241e: { // config
		save_dcs(unenc_response + 28, unenc_response + 20);
		break;
	}
	case 0x5e002502: { // auth.sentCode
		if (phone_code_hash != NULL) free(phone_code_hash);
		phone_code_hash = (BYTE*)malloc(20);
		memcpy(phone_code_hash, unenc_response + 16, 20);
		EnableWindow(hCode, TRUE);
		EnableWindow(hCodeBtn, TRUE);
		SetFocus(hCode);
		break;
	}
	case 0x957b50fb: { // account.password
		int flags = read_le(unenc_response + 4, 4);
		if (flags & (1 << 2)) {
			if (hint_needed) {
				hint_needed = false;
				if (flags & (1 << 3)) {
					int offset = tlstr_len(unenc_response + 12, true);
					offset += tlstr_len(unenc_response + 12 + offset, true) + 4;
					offset += tlstr_len(unenc_response + 12 + offset, true);
					offset += tlstr_len(unenc_response + 12 + offset, true) + 8;
					hint = read_string(unenc_response + 12 + offset, NULL);
					MessageBox(current_dialog, hint, L"Hint", MB_OK | MB_ICONINFORMATION);
				} else {
					MessageBox(current_dialog, L"No hint!", L"Hint", MB_OK | MB_ICONERROR);
					EnableWindow(h2FAHint, FALSE);
				}
			} else {
				if (!(flags & (1 << 3))) EnableWindow(h2FAHint, FALSE);
				int wpassword_len = GetWindowTextLength(h2FA);
				wchar_t wpassword[64];
				GetWindowText(h2FA, wpassword, wpassword_len + 1);
				BYTE u8password[260];
				write_string(u8password, wpassword);
				BYTE* password = &u8password[u8password[0] == 254 ? 4 : 1];
				int password_len = tlstr_len(u8password, false);

				ShowWindow(current_dialog, SW_HIDE);
				SetWindowText(infoLabel, L"Going through the SRP protocol...");
				ShowWindow(current_info, SW_SHOW);
				UpdateWindow(current_info);
				mp_int g, a, p, k, u, g_a, g_b, x, v, k_v, t, s_a, temp;
				mp_init_multi(&g, &a, &p, &k, &u, &g_a, &g_b, &x, &v, &k_v, &t, &s_a, &temp, NULL);

				int salt1_len = unenc_response[12];
				BYTE* salt1 = &unenc_response[13];
				int offset = 12 + 1 + salt1_len + (4 - (salt1_len + 1) % 4) % 4;
				int salt2_len = unenc_response[offset];
				BYTE* salt2 = &unenc_response[offset + 1];
				offset += 1 + salt2_len + (4 - (salt2_len + 1) % 4) % 4;
				
				int g_int = read_le(unenc_response + offset, 4);
				mp_set_int(&g, g_int);
				int g_size = mp_ubin_size(&g);
				offset += 4;

				int p_size = tlstr_len(unenc_response + offset, false);
				BYTE* p_bytes = &unenc_response[offset + 4];
				mp_read_unsigned_bin(&p, unenc_response + offset + 4, p_size);
				offset += 4 + p_size + (4 - (p_size + 4) % 4) % 4;

				int g_b_size = tlstr_len(unenc_response + offset, false);
				BYTE* g_b_bytes = &unenc_response[offset + 4];
				mp_read_unsigned_bin(&g_b, unenc_response + offset + 4, g_b_size);
				offset += 4 + g_b_size + (4 - (g_b_size + 4) % 4) % 4;

				BYTE buf[640] = {0};
				unsigned out;
				mp_to_ubin(&p, buf + 256 - p_size, p_size, &out);
				mp_to_ubin(&g, buf + 512 - g_size, g_size, &out);

				BYTE sha256_res[32];
				sha256_init(&md);
				sha256_process(&md, buf, 512);
				sha256_done(&md, sha256_res);
				mp_read_unsigned_bin(&k, sha256_res, 32);

				BYTE a_bytes[256];
				fortuna_read(a_bytes, 256, &prng);
				mp_read_unsigned_bin(&a, a_bytes, 256);
				mp_exptmod(&g, &a, &p, &g_a);
				int g_a_size = mp_ubin_size(&g_a);

				memset(buf, 0, 512);
				mp_to_ubin(&g_a, buf + 256 - g_a_size, g_a_size, &out);
				memcpy(buf + 512 - g_b_size, g_b_bytes, g_b_size);

				sha256_init(&md);
				sha256_process(&md, buf, 512);
				sha256_done(&md, sha256_res);
				mp_read_unsigned_bin(&u, sha256_res, 32);

				memset(buf, 0, 512);
				memcpy(buf, salt1, salt1_len);
				memcpy(buf + salt1_len, password, password_len);
				memcpy(buf + salt1_len + password_len, salt1, salt1_len);
				sha256_init(&md);
				sha256_process(&md, buf, salt1_len * 2 + password_len);
				sha256_done(&md, sha256_res);

				memcpy(buf, salt2, salt2_len);
				memcpy(buf + salt2_len, sha256_res, 32);
				memcpy(buf + salt2_len + 32, salt2, salt2_len);
				sha256_init(&md);
				sha256_process(&md, buf, salt2_len * 2 + 32);
				sha256_done(&md, sha256_res);

				BYTE sha512_res[64];
				unsigned long len = 64;
				int reg = register_hash(&sha512_desc);
				int hash = find_hash("sha512");
				int res = pkcs_5_alg2(sha256_res, 32, salt1, salt1_len, 100000, hash, sha512_res, &len);
				memcpy(buf + salt2_len, sha512_res, 64);
				memcpy(buf + salt2_len + 64, salt2, salt2_len);
				sha256_init(&md);
				sha256_process(&md, buf, salt2_len * 2 + 64);
				sha256_done(&md, sha256_res);
				mp_read_unsigned_bin(&x, sha256_res, 32);
				mp_exptmod(&g, &x, &p, &v);
				mp_mul(&k, &v, &temp);
				mp_mod(&temp, &p, &k_v);

				mp_sub(&g_b, &k_v, &temp);
				mp_mod(&temp, &p, &t);
				
				mp_mul(&u, &x, &temp);
				mp_add(&a, &temp, &s_a);
				mp_copy(&s_a, &temp);
				mp_exptmod(&t, &temp, &p, &s_a);
				int s_a_size = mp_ubin_size(&s_a);
				mp_to_ubin(&s_a, buf, s_a_size, &out);
				sha256_init(&md);
				sha256_process(&md, buf, s_a_size);
				sha256_done(&md, buf + 608);

				memset(buf, 0, 512);
				sha256_init(&md);
				sha256_process(&md, p_bytes, p_size);
				sha256_done(&md, buf);
				mp_to_ubin(&g, buf + 512 - g_size, g_size, &out);
				sha256_init(&md);
				sha256_process(&md, buf + 256, 256);
				sha256_done(&md, sha256_res);
				memset(buf + 256, 0, 256);
				for (int i = 0; i < 32; i++) buf[i] = buf[i] ^ sha256_res[i];
				sha256_init(&md);
				sha256_process(&md, salt1, salt1_len);
				sha256_done(&md, buf + 32);
				sha256_init(&md);
				sha256_process(&md, salt2, salt2_len);
				sha256_done(&md, buf + 64);
				mp_to_ubin(&g_a, buf + 352 - g_a_size, g_a_size, &out);
				memcpy(buf + 608 - g_b_size, g_b_bytes, g_b_size);

				sha256_init(&md);
				sha256_process(&md, buf, 640);
				sha256_done(&md, sha256_res);

				BYTE enc_query[392];
				internal_header(buf, true);
				write_le(buf + 28, 312, 4);
				write_le(buf + 32, 0xd18b4d16, 4);
				write_le(buf + 36, 0xd27ff082, 4);
				memcpy(buf + 40, unenc_response + offset, 8);
				memset(buf + 48, 0, 296);
				buf[48] = 254;
				buf[50] = 1;
				mp_to_ubin(&g_a, buf + 308 - g_a_size, g_a_size, &out);
				buf[308] = 32;
				memcpy(buf + 309, sha256_res, 32);
				fortuna_read(buf + 344, 24, &prng);
				convert_message(buf, enc_query, 368, 0);
				send_query(enc_query, 392);
				mp_clear_multi(&g, &a, &p, &k, &u, &g_a, &g_b, &x, &v, &k_v, &t, &s_a, &temp, NULL);
			}
		}
		break;
	}
	case 0x390d5c5e: // auth.loginTokenSuccess
		response_handler(dcInfo, unenc_response + 4, false, length - 4);
		break;
	case 0x2ea2c0d4: { // auth.authorization
		if (!dcInfo->authorized) {
			if (dcInfo == &dcInfoMain) {
				free(phone_number_bytes);
				free(phone_code_hash);
				FILE* f = _wfopen(get_path(appdata_path, L"session.dat"), L"wb");
				fwrite(dcInfoMain.auth_key, 1, 256, f);
				fwrite(dcInfoMain.auth_key_id, 1, 8, f);
				fwrite(dcInfoMain.server_salt, 1, 8, f);
				fwrite(dcInfoMain.session_id, 1, 8, f);
				fwrite(&dcInfoMain.current_seq_no, 4, 1, f);
				fwrite(&pts, 4, 1, f);
				fwrite(&qts, 4, 1, f);
				fwrite(&date, 4, 1, f);
				int logged_flags = 0;
				fwrite(&logged_flags, 4, 1, f);
				fclose(f);
				DestroyWindow(current_dialog);
				current_dialog = NULL;
				SetWindowText(infoLabel, L"Logged in, getting data...");
				ShowWindow(current_info, SW_SHOW);

				// updates.getState
				get_state(false);

				int flags = read_le(unenc_response + 4, 4);
				int offset = 8;
				if (flags & (1 << 1)) offset += 4;
				if (flags & (1 << 0)) offset += 4;
				if (flags & (1 << 2)) offset += tlstr_len(unenc_response + offset, true);
				myself.type = 0;
				myself.online = -2;
				set_peer_info(unenc_response + offset, &myself, false);
				get_full_peer(&myself);
			} else {
				FILE* f = _wfopen(get_path(appdata_path, L"session.dat"), L"rb+");
				fseek(f, 296, SEEK_SET);
				int logged_flags = 0;
				fread(&logged_flags, 4, 1, f);
				fseek(f, 296, SEEK_SET);
				logged_flags += (int)pow(2, dcInfo->dc);
				fwrite(&logged_flags, 4, 1, f);

				fseek(f, 0, SEEK_END);
				dcInfo->dc_file_index = ftell(f);
				fwrite(&dcInfo->dc, 4, 1, f);
				fwrite(dcInfo->auth_key, 1, 256, f);
				fwrite(dcInfo->auth_key_id, 1, 8, f);
				fwrite(dcInfo->server_salt, 1, 8, f);
				fwrite(dcInfo->session_id, 1, 8, f);
				fwrite(&dcInfo->current_seq_no, 4, 1, f);
				fclose(f);
			}
			get_future_salt(dcInfo);
			dcInfo->authorized = true;
		}
		break;
	}
	case 0xb434e2b8: { // auth.exportedAuthorization
		for (std::list<DCInfo>::iterator it = active_dcs.begin(); it != active_dcs.end(); it++) {
			DCInfo* active_dc = &(*it);
			if (memcmp(active_dc->future_salt, last_rpcresult_msgid, 8) == 0) {
				BYTE unenc_query[320];
				BYTE enc_query[344];
				internal_header(active_dc, unenc_query, true);
				write_le(unenc_query + 32, 0xda9b0d0d, 4);
				write_le(unenc_query + 36, 196, 4);
				write_le(unenc_query + 40, 0xc1cd5ea9, 4);
				memset(unenc_query + 44, 0, 4);
				write_le(unenc_query + 48, 27752131, 4);
				write_string(unenc_query + 52, L"com");
				write_string(unenc_query + 56, L"win");
				write_string(unenc_query + 60, L"1.0.0");
				write_string(unenc_query + 68, L"en");
				write_string(unenc_query + 72, L"tdesktop");
				write_string(unenc_query + 84, L"en");
				write_le(unenc_query + 88, 0xa57a7dad, 4);
				memcpy(unenc_query + 92, unenc_response + 4, 8 + tlstr_len(unenc_response + 12, true));
				int offset = 100 + tlstr_len(unenc_response + 12, true);
				write_le(unenc_query + 28, offset - 32, 4);
				int padding_len = get_padding(offset);
				fortuna_read(unenc_query + offset, padding_len, &prng);
				offset += padding_len;
				convert_message(active_dc, unenc_query, enc_query, offset, 0);
				send_query(active_dc, enc_query, offset + 24);
				SendMessage(hStatus, SB_SETTEXTA, 1 | SBT_OWNERDRAW, (LPARAM)L"Logged in to the needed file's datacenter!");
				if (uploadEvents.size() == 0 && downloading_docs.size() == 0) SetTimer(hMain, 2, 3000, NULL);
				break;
			}
		}
		break;
	}
	case 0xc3a2835f: { // auth.loggedOut
		acknowledgement = false;
		logout_cleanup();
		break;
	}
	case 0xedab447b: { // bad_server_salt
		memcpy(dcInfo->server_salt, unenc_response + 20, 8);
		get_future_salt(dcInfo);
		acknowledgement = false;
		break;
	}
	case 0xa7eff811: { // bad_msg_notification
		int error_code = read_le(unenc_response + 16, 4);
		wchar_t error_message[14];
		wsprintf(error_message, L"Error code %d", error_code);
		MessageBox(NULL, error_message, L"bad_msg_notification", MB_OK | MB_ICONERROR);
		if (error_code == 16 || error_code == 17) {
			time_diff = read_le(unenc_response - 12, 4) - time(NULL);
			if (read_le(dcInfoMain.future_salt, 8) == 0) get_future_salt(dcInfo);
		} else if (error_code == 32) dcInfoMain.current_seq_no++;
		acknowledgement = false;
		break;				 
	}
	case 0x73f1f8dc: { // msg_container
		int msg_count = read_le(unenc_response + 4, 4);
		int pos = 8;
		for (int i = 0; i < msg_count; i++) {
			pos += 16;
			int len = read_le(unenc_response + pos - 4, 4);
			memcpy(dcInfoMain.last_msg_id, unenc_response + pos - 16, 8);
			response_handler(dcInfo, unenc_response + pos, true, len);
			pos += len;
		}
		acknowledgement = false;
		break;
	}
	case 0xf35c6d01: { // rpc_result
		if (memcmp(last_rpcresult_msgid, unenc_response + 4, 8) == 0) break;
		memcpy(last_rpcresult_msgid, unenc_response + 4, 8);
		response_handler(dcInfo, unenc_response + 12, false, length - 12);
		if (memcmp(last_rpcresult_msgid, difference_msg_id, 8) == 0) {
			DestroyWindow(current_info);
			ShowWindow(hMain, maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
			set_tray_icon();
		}
		if (!dcInfo->dc) acknowledgement = false;
		break;
	}
	case 0x2144ca19: { // rpc_error
		int error_code = read_le(unenc_response + 4, 4);
		wchar_t error_message[50];
		read_string(unenc_response + 8, error_message);
		if (error_code == 303) {
			if (error_message[0] == L'P') {
				ShowWindow(current_dialog, SW_HIDE);
				SetWindowText(infoLabel, L"Account is on a different DC! Reconnecting...");
				ShowWindow(current_info, SW_SHOW);
				UpdateWindow(current_info);
				dcInfoMain.dc = error_message[14] - L'0';
				FILE* f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"rb+");
				fseek(f, 0, SEEK_SET);
				fwrite(&dcInfoMain.dc, 4, 1, f);
				fclose(f);
				reconnect(&dcInfoMain);
				unsigned threadID;
				_beginthreadex(NULL, 0, SocketWorker, (void*)&dcInfoMain, 0, &threadID);
				SendMessage(current_dialog, WM_INITDIALOG, 0, 10);
				SendMessage(current_dialog, WM_COMMAND, MAKEWPARAM(3, 0), 0);
				return;
			} else {
				int dc = error_message[13] - L'0';
				if (dc == dcInfoMain.dc) break;
				bool found = false;
				DCInfo* active_dc = NULL;
				for (std::list<DCInfo>::iterator it = active_dcs.begin(); it != active_dcs.end(); it++) {
					active_dc = &(*it);
					if (active_dc->dc == dc) {
						found = true;
						break;
					}
				}
				if (found) {
					if (active_dc->ready) {
						found = false;
						if (rces.size() && memcmp(rces[0].msg_id, last_rpcresult_msgid, 8) == 0) {
							get_photo(&rces[0], NULL, active_dc);
							break;
						}
						for (int j = 0; j < downloading_docs.size(); j++) {
							if (memcmp(&downloading_docs[j], last_rpcresult_msgid, 8) == 0) {
								download_file(active_dc, &downloading_docs[j]);
								found = true;
								break;
							}
						}
						if (found) break;
						for (j = documents.size() - 1; j >= 0; j--) {
							if (memcmp(documents[j].photo_msg_id, last_rpcresult_msgid, 8) == 0) {
								get_photo(NULL, &documents[j], active_dc);
								break;
							}
						}
						if (memcmp(pfp_msgid, last_rpcresult_msgid, 8) == 0) {
							get_pfp(active_dc, current_peer);
							break;
						}
					}
					break;
				}
				FILE* f = _wfopen(get_path(appdata_path, L"session.dat"), L"rb");
				fseek(f, 296, SEEK_SET);
				int logged_flags;
				fread(&logged_flags, 4, 1, f);
				
				DCInfo dcInfo = {0};
				dcInfo.dc = dc;
				dcInfo.ready = false;
				if (logged_flags & (1 << dc)) {
					int dc_to_compare = 0;
					while (true) {
						fread(&dc_to_compare, 4, 1, f);
						if (dc_to_compare != dc) fseek(f, 284, SEEK_CUR);
						else break;
					}
					dcInfo.dc_file_index = ftell(f) - 4;
					fread(dcInfo.auth_key, 1, 256, f);
					fread(dcInfo.auth_key_id, 1, 8, f);
					fread(dcInfo.server_salt, 1, 8, f);
					fread(dcInfo.session_id, 1, 8, f);
					fread(&dcInfo.current_seq_no, 4, 1, f);
					dcInfo.authorized = true;
					
				} else {
					dcInfo.current_seq_no = 0;
					dcInfo.authorized = false;
				}
				fclose(f);
				active_dcs.push_back(dcInfo);
				unsigned threadID;
				_beginthreadex(NULL, 0, SocketWorker, (void*)&active_dcs.back(), 0, &threadID);
			}
		} else if (error_code == 401 && wcscmp(error_message, L"SESSION_PASSWORD_NEEDED") == 0) {
			BYTE buffer[24] = {0};
			LONG dlgUnits = GetDialogBaseUnits();
			DLGTEMPLATE *dlg = (DLGTEMPLATE*)buffer;
			dlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME;
			dlg->cx = MulDiv(175, 4, LOWORD(dlgUnits));
			dlg->cy = MulDiv(95, 8, HIWORD(dlgUnits));
			if (current_dialog) DestroyWindow(current_dialog);
			current_dialog = CreateDialogIndirect(GetModuleHandle(NULL), dlg, NULL, DlgProc2FA);
		} else if (error_code == 400) {
			if (wcscmp(error_message, L"AUTH_TOKEN_EXPIRED") == 0) {
				MessageBox(NULL, L"Auth token expired! Try again.", L"Error", MB_OK | MB_ICONERROR);
				if (qrCodeToken) {
					free(qrCodeToken);
					qrCodeToken = NULL;
				}
				SendMessage(current_dialog, WM_INITDIALOG, 0, 10);
			} else if (wcscmp(error_message, L"PASSWORD_HASH_INVALID") == 0) {
				MessageBox(NULL, L"Incorrect password!", L"Error", MB_OK | MB_ICONERROR);
				ShowWindow(current_info, SW_HIDE);
				ShowWindow(current_dialog, SW_SHOW);
			} else if (wcscmp(error_message, L"SRP_ID_INVALID") == 0) {
				MessageBox(NULL, L"Wasn't able to finish in time! If you just used your hint, you can try again. If not, your CPU is likely too slow for 2FA :(", L"Error", MB_OK | MB_ICONERROR);
				ShowWindow(current_info, SW_HIDE);
				ShowWindow(current_dialog, SW_SHOW);
			}
		} else if (error_code == 401 && wcscmp(error_message, L"AUTH_KEY_UNREGISTERED") == 0) {
			MessageBox(NULL, L"Not logged in! Cleaning up and exiting...", L"Error", MB_OK | MB_ICONERROR);
			logout_cleanup();
		} else {
			wchar_t error_message2[50];
			swprintf(error_message2, L"Error code %d: %s", error_code, error_message);
			MessageBox(NULL, error_message2, L"rpc_error", MB_OK | MB_ICONERROR);
		}
		break;
	}
	case 0x68e9916: { // auth.loginTokenMigrateTo
		ShowWindow(current_dialog, SW_HIDE);
		SetWindowText(infoLabel, L"Account is on a different DC! Reconnecting...");
		ShowWindow(current_info, SW_SHOW);
		UpdateWindow(current_info);
		dcInfoMain.dc = read_le(unenc_response + 4, 4);
		qrCodeToken = (BYTE*)malloc(tlstr_len(unenc_response + 8, true));
		memcpy(qrCodeToken, unenc_response + 8, tlstr_len(unenc_response + 8, true));
		FILE* f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"rb+");
		fseek(f, 0, SEEK_SET);
		fwrite(&dcInfoMain.dc, 4, 1, f);
		fclose(f);
		reconnect(&dcInfoMain);
		unsigned threadID;
		_beginthreadex(NULL, 0, SocketWorker, (void*)&dcInfoMain, 0, &threadID);
		SendMessage(current_dialog, WM_INITDIALOG, 0, 10);
		break;
	}
	case 0x3072cfa1: { // gzip_packed
		int str_len = tlstr_len(unenc_response + 4, false) - 10;
		int str_start = (str_len >= 244 ? 4 : 1) + 14;
		unsigned int out_len;
		void* out_data = tinfl_decompress_mem_to_heap(unenc_response + str_start,
			str_len, &out_len, 0);
		response_handler(dcInfo, (BYTE*)out_data, false, out_len);
		free(out_data);
		break;			 
	}
	case 0xae500895: { // future_salts
		if (dcInfo == &dcInfoMain && read_le(dcInfo->future_salt, 8) == 0) {
			update_own_status(true);
			FILE* f = _wfopen(get_path(appdata_path, L"database.dat"), L"rb");
			if (f) {
				fseek(f, 0, SEEK_END);
				int size = ftell(f);
				int this_database_version;
				bool rip = false;
				
				if (size == 0) rip = true;
				else {
					rewind(f);
					fread(&this_database_version, 4, 1, f);
					if (this_database_version != database_version) rip = true;
				}
				if (rip) {
					fclose(f);
					f = NULL;
					DeleteFile(L"database.dat");
				}
			}
			if (f) {
				fread(&muted_types, 4, 3, f);
				HMENU hMenuMute = GetSubMenu(GetSubMenu(hMenuBar, 0), 1);
				if (muted_types[0]) ModifyMenu(hMenuMute, 0, MF_BYPOSITION | MF_STRING, 38, L"Unmute all chats");
				if (muted_types[1]) ModifyMenu(hMenuMute, 1, MF_BYPOSITION | MF_STRING, 39, L"Unmute all groups");
				if (muted_types[2]) ModifyMenu(hMenuMute, 2, MF_BYPOSITION | MF_STRING, 40, L"Unmute all channels");
				fread(&peers_count, 4, 1, f);
				peers = (Peer*)malloc(peers_count*sizeof(Peer));
				int new_unread_msgs_count = 0;
				for (int i = -1; i < peers_count; i++) {
					Peer* peer = (i == -1) ? &myself : &peers[i];
					fread(peer->id, 1, 8, f);
					fread(peer->access_hash, 1, 8, f);
					int name_len = 0;
					fread(&name_len, 4, 1, f);
					peer->name = (wchar_t*)malloc(name_len*2);
					fread(peer->name, 2, name_len, f);
					int handle_len = 0;
					fread(&handle_len, 4, 1, f);
					if (handle_len) {
						peer->handle = (wchar_t*)malloc(handle_len*2);
						fread(peer->handle, 2, handle_len, f);
					} else peer->handle = NULL;
					fread(&peer->online, 4, 1, f);
					fread(peer->photo, 1, 8, f);
					fread(&peer->photo_dc, 4, 1, f);
					fread(&peer->last_read, 4, 1, f);
					fread(&peer->unread_msgs_count, 4, 1, f);
					fread(&peer->mute_until, 4, 1, f);
					fread(&peer->perm, sizeof(Permissions), 1, f);
					fread(&peer->amadmin, 1, 1, f);
					fread(&peer->full, 1, 1, f);
					fread(&peer->type, 1, 1, f);
					if (peer->type != 0) {
						fread(&peer->name_set_time, 4, 1, f);
						fread(&peer->pfp_set_time, 4, 1, f);
					}
					peer->reaction_list = &reaction_list;
					peer->status_updated = false;
					if (peer->full) {
						int about_len = 0;
						fread(&about_len, 4, 1, f);
						if (about_len) {
							peer->about = (wchar_t*)malloc(about_len*2);
							fread(peer->about, 2, about_len, f);
						} else peer->about = NULL;
						if (peer->type == 0) fread(peer->birthday, 1, 4, f);
						fread(&peer->theme_id, 1, 8, f);
						fread(&peer->theme_set_time, 4, 1, f);
						if (peer->type == 1) {
							int chat_users_count;
							fread(&chat_users_count, 4, 1, f);
							peer->chat_users = new std::vector<Peer>();
							for (int j = 0; j < chat_users_count; j++) {
								Peer peer_participant;
								fread(peer_participant.id, 1, 8, f);
								fread(peer_participant.access_hash, 1, 8, f);
								int name_len;
								fread(&name_len, 4, 1, f);
								peer_participant.name = (wchar_t*)malloc(name_len*2);
								fread(peer_participant.name, 2, name_len, f);
								peer_participant.handle = NULL;
								peer->chat_users->push_back(peer_participant);
							}
						}
						if (peer->type != 0) {
							int reaction_count, reaction_len;
							fread(&reaction_count, 4, 1, f);
							if (reaction_count == 0) peer->reaction_list = NULL;
							else if (reaction_count == 0xFFFFFFFF) peer->reaction_list = &reaction_list;
							else {
								peer->reaction_list = new std::vector<wchar_t*>(reaction_count);
								for (int j = 0; j < reaction_count; j++) {
									fread(&reaction_len, 4, 1, f);
									wchar_t* reaction = (wchar_t*)malloc(reaction_len * 2);
									fread(reaction, 2, reaction_len, f);
									peer->reaction_list->at(j) = reaction;
								}
							}
						}
						if (peer->type == 2) fread(&peer->chat_users, 4, 1, f);
					}
					if (peer->type == 2) {
						fread(&peer->channel_pts, 4, 1, f);
						memset(peer->channel_msg_id, 0, 8);
					}
					if (peer->mute_until && peer->mute_until != 2147483647) {
						if (peer->mute_until - current_time() <= 0) peer->mute_until = 0;
						else {
							UnmuteTimer ut;
							memcpy((BYTE*)&ut.peer_id, peer->id, 8);
							ut.timer_id = 50 + unmuteTimers.size();
							unmuteTimers.push_back(ut);
							SetTimer(hMain, ut.timer_id, (peer->mute_until - current_time()) * 1000, NULL);
						}
					}
					if (peer->unread_msgs_count && !peer->mute_until && !muted_types[peer->type])
						new_unread_msgs_count += peer->unread_msgs_count;
					peer->last_recv = 0;
				}
				if (new_unread_msgs_count) update_total_unread_msgs_count(new_unread_msgs_count);
				fread(&folders_count, 4, 1, f);
				folders = (ChatsFolder*)malloc(folders_count*sizeof(ChatsFolder));
				for (i = 0; i < folders_count; i++) {
					fread(folders[i].id, 1, 4, f);
					fread(&folders[i].count, 4, 1, f);
					fread(&folders[i].pinned_count, 4, 1, f);
					folders[i].peers = (int*)malloc(folders[i].count * 4);
					fread(folders[i].peers, 4, folders[i].count, f);
					int name_len = 0;
					fread(&name_len, 4, 1, f);
					folders[i].name = (wchar_t*)malloc(name_len*2);
					fread(folders[i].name, 2, name_len, f);
					SendMessage(hComboBoxFolders, CB_ADDSTRING, 0, (LPARAM)folders[i].name);
					SendMessage(hComboBoxFolders, CB_SETITEMDATA, i, (LPARAM)&folders[i]);
				}
				fread(&reaction_hash, 4, 1, f);
				int reactions_count, reaction_len;
				fread(&reactions_count, 4, 1, f);
				for (i = 0; i < reactions_count; i++) {
					fread(&reaction_len, 4, 1, f);
					wchar_t* reaction = (wchar_t*)malloc(reaction_len * 2);
					fread(reaction, 2, reaction_len, f);
					reaction_list.push_back(reaction);
				}
				fread(&theme_hash, 8, 1, f);
				int themes_count, theme_len;
				fread(&themes_count, 4, 1, f);
				for (i = 0; i < themes_count; i++) {
					Theme theme;
					fread(theme.id, 1, 8, f);
					fread(&theme.color, 4, 1, f);
					fread(&theme_len, 4, 1, f);
					theme.emoji_id = (BYTE*)malloc(theme_len);
					fread(theme.emoji_id, 1, theme_len, f);
					themes.push_back(theme);
				}
				fclose(f);

				register_themes();
				SendMessage(hComboBoxFolders, CB_SETCURSEL, 0, 0);
				current_folder = &folders[0];
				for (i = 0; i < peers_count; i++) {
					SendMessage(hComboBoxChats, CB_ADDSTRING, 0, (LPARAM)peers[folders[0].peers[i]].name);
					SendMessage(hComboBoxChats, CB_SETITEMDATA, i, (LPARAM)&peers[folders[0].peers[i]]);
				}

				// updates.getDifference
				if (pts > 0) {
					BYTE unenc_query[64];
					BYTE enc_query[88];
					internal_header(unenc_query, true);
					memcpy(difference_msg_id, unenc_query + 16, 8);
					write_le(unenc_query + 28, 20, 4);
					write_le(unenc_query + 32, 0x19c2f763, 4);
					memset(unenc_query + 36, 0, 4);
					write_le(unenc_query + 40, pts, 4);
					write_le(unenc_query + 44, date, 4);
					write_le(unenc_query + 48, qts, 4);
					fortuna_read(unenc_query + 52, 12, &prng);
					convert_message(unenc_query, enc_query, 64, 0);
					send_query(enc_query, 88);
				} else get_state(true);
			} else {
				//messages.getDialogs
				BYTE unenc_query[80];
				BYTE enc_query[104];
				internal_header(unenc_query, true);
				write_le(unenc_query + 28, 32, 4);
				write_le(unenc_query + 32, 0xa0f4cb4f, 4);
				memset(unenc_query + 36, 0, 12);
				write_le(unenc_query + 48, 0x7f3b18ea, 4);
				memset(unenc_query + 52, 0, 12);
				fortuna_read(unenc_query + 64, 16, &prng);
				convert_message(unenc_query, enc_query, 80, 0);
				send_query(enc_query, 104);

				// users.getUsers (get current user)
				internal_header(unenc_query, true);
				write_le(unenc_query + 28, 16, 4);
				write_le(unenc_query + 32, 0xd91a548, 4);
				write_le(unenc_query + 36, 0x1cb5c415, 4);
				write_le(unenc_query + 40, 1, 4);
				write_le(unenc_query + 44, 0xf7c1b13f, 4);
				fortuna_read(unenc_query + 48, 16, &prng);
				convert_message(unenc_query, enc_query, 64, 0);
				send_query(enc_query, 88);
			}

			if (dcInfo == &dcInfoMain) {
				// messages.getAvailableReactions
				BYTE unenc_query[64];
				BYTE enc_query[88];
				internal_header(unenc_query, true);
				write_le(unenc_query + 28, 8, 4);
				write_le(unenc_query + 32, 0x18dea0ac, 4);
				write_le(unenc_query + 36, reaction_hash, 4);
				fortuna_read(unenc_query + 40, 24, &prng);
				convert_message(unenc_query, enc_query, 64, 0);
				send_query(enc_query, 88);

				// account.getChatThemes
				internal_header(unenc_query, true);
				write_le(unenc_query + 28, 12, 4);
				write_le(unenc_query + 32, 0xd638de89, 4);
				write_le(unenc_query + 36, theme_hash, 8);
				fortuna_read(unenc_query + 48, 16, &prng);
				convert_message(unenc_query, enc_query, 64, 0);
				send_query(enc_query, 88);

				if (muted_types[0] == -1) {
					// account.getNotifySettings
					internal_header(unenc_query, true);
					write_le(unenc_query + 28, 8, 4);
					write_le(unenc_query + 32, 0x12b3ad31, 4);
					write_le(unenc_query + 36, 0x193b4417, 4);
					fortuna_read(unenc_query + 40, 24, &prng);
					convert_message(unenc_query, enc_query, 64, 0);
					send_query(enc_query, 88);
				}

				FILE* f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"rb");
				if (!f) get_config();
				else fclose(f);
			}
		}
		int offset = 0;
		if (memcmp(unenc_response + 28, dcInfo->server_salt, 8) == 0) offset += 16;
		SetTimer(hMain, 10 + dcInfo->dc, (read_le(unenc_response + 20 + offset, 4) - current_time() + 60) * 1000, NULL);
		memcpy(dcInfo->future_salt, unenc_response + 28 + offset, 8);

		if (dcInfo != &dcInfoMain && !dcInfo->ready) {
			dcInfo->ready = true;
			if (rces.size() && rces[0].dc == dcInfo->dc) get_photo(&rces[0], NULL, dcInfo);
			for (int j = 0; j < downloading_docs.size(); j++) if (downloading_docs[j].dc == dcInfo->dc) download_file(dcInfo, &downloading_docs[j]);
			for (j = documents.size() - 1; j >= 0; j--) if (documents[j].photo_size && documents[j].dc == dcInfo->dc) get_photo(NULL, &documents[j], dcInfo);
			if (current_dialog && current_peer && current_peer->photo_dc == dcInfo->dc) get_pfp(dcInfo, current_peer);
		}
		break;		 
	}
	case 0x99622c0c: { // peerNotifySettings
		int type = -1;
		if (muted_types[0] == -1) type = 0;
		else if (muted_types[1] == -1) type = 1;
		else if (muted_types[2] == -1) type = 2;
		if (type != -1) {
			apply_notifysettings(unenc_response, &muted_types[type], type);
			if (!muted_types[type]) {
				int new_unread_msgs_count = 0;
				for (int i = 0; i < peers_count; i++)
					if (peers[i].type == type && !peers[i].mute_until && peers[i].unread_msgs_count) new_unread_msgs_count += peers[i].unread_msgs_count;
				if (new_unread_msgs_count) update_total_unread_msgs_count(new_unread_msgs_count);
			} else change_mute_all(type, false);
			if (type != 2) {
				BYTE unenc_query[64];
				BYTE enc_query[88];
				internal_header(unenc_query, true);
				write_le(unenc_query + 28, 8, 4);
				write_le(unenc_query + 32, 0x12b3ad31, 4);
				write_le(unenc_query + 36, type == 0 ? 0x4a95e84e : 0xb1db7c7e, 4);
				fortuna_read(unenc_query + 40, 24, &prng);
				convert_message(unenc_query, enc_query, 64, 0);
				send_query(enc_query, 88);
			}
		}
		break;
	}
	case 0xbc799737: // boolFalse
	case 0x997275b5: { // boolTrue
		for (int i = 0; i < uploadEvents.size(); i++) if (memcmp(uploadEvents[i]->lastId, last_rpcresult_msgid, 8) == 0) SetEvent(uploadEvents[i]->event);
		if (memcmp(handle_msgid, last_rpcresult_msgid, 8) == 0) {
			for (int i = 0; i < peers_count; i++) {
				if (memcmp(dlg_peer.id, peers[i].id, 8) == 0) {
					if (constructor == 0x997275b5) {
						if (peers[i].handle) free(peers[i].handle);
						peers[i].handle = dlg_peer.handle;
					} else free(dlg_peer.handle);
					break;
				}
			}
		}
		break;
	}
	case 0xa56c2a3e: { // updates.State
		update_pts(read_le(unenc_response + 4, 4));
		qts = read_le(unenc_response + 8, 4);
		date = read_le(unenc_response + 12, 4);
		break;
	}
	case 0xa8fb1981: // updates.differenceSlice
	case 0x2064674e: // updates.channelDifference
	case 0xf49ca0: { // updates.difference
		if (dcInfo != &dcInfoMain) break;
		int offset = 4;
		Peer* channel = NULL;
		if (constructor == 0x2064674e) {
			for (int i = 0; i < peers_count; i++) {
				if (peers[i].type == 2 && memcmp(last_rpcresult_msgid, peers[i].channel_msg_id, 8) == 0) {
					channel = &peers[i];
					break;
				}
			}
			if (!channel) break;
			int flags = read_le(unenc_response + offset, 4);
			channel->channel_pts = read_le(unenc_response + offset + 4, 4);
			offset += 8;
			if (flags & (1 << 1)) {
				if (channel == current_peer) {
					int timeout = read_le(unenc_response + offset, 4);
					SetTimer(hMain, 3, timeout * 1000, NULL);
				}
				offset += 4;
			}
			if (!(flags & (1 << 0))) get_channel_difference(channel);
		}
		int msg_count = read_le(unenc_response + offset + 4, 4);
		offset += 8;
		for (int i = 0; i < msg_count; i++) offset += message_handler(false, unenc_response + offset, true, false, false);
		offset += 8;
		for (i = offset; i < length - 40;) i += update_handler(unenc_response + i);
		if (constructor != 0x2064674e) {
			update_pts(read_le(unenc_response + length - 20, 4));
			qts = read_le(unenc_response + length - 16, 4);
			date = read_le(unenc_response + length - 12, 4);
		}
		if (constructor == 0xa8fb1981) {
			BYTE unenc_query[64];
			BYTE enc_query[88];
			internal_header(unenc_query, true);
			memcpy(difference_msg_id, unenc_query + 16, 8);
			write_le(unenc_query + 28, 20, 4);
			write_le(unenc_query + 32, 0x19c2f763, 4);
			memset(unenc_query + 36, 0, 4);
			write_le(unenc_query + 40, pts, 4);
			write_le(unenc_query + 44, date, 4);
			write_le(unenc_query + 48, qts, 4);
			fortuna_read(unenc_query + 52, 12, &prng);
			convert_message(unenc_query, enc_query, 64, 0);
			send_query(enc_query, 88);
		}
		if (channel) memset(channel->channel_msg_id, 0, 8);
		break;
	}
	case 0xa4bcc6fe: { // updates.channelDifferenceTooLong
		for (int i = 0; i < peers_count; i++) {
			if (peers[i].type == 2 && memcmp(last_rpcresult_msgid, peers[i].channel_msg_id, 8) == 0) {
				int flags1 = read_le(unenc_response + 4, 4);
				int offset = 8;
				if (flags1 & (1 << 1)) offset += 4; 
				int flags = read_le(unenc_response + offset + 4, 4);
				offset += 28;
				peers[i].last_read = read_le(unenc_response + offset, 4);
				offset += 16;
				apply_notifysettings(unenc_response + offset, &peers[i].mute_until, read_le(peers[i].id, 8));
				int unread_count_old = peers[i].unread_msgs_count;
				offset -= 12;
				peers[i].unread_msgs_count = read_le(unenc_response + 40, 4);
				if (unread_count_old != peers[i].unread_msgs_count && !peers[i].mute_until && !muted_types[peers[i].type]) update_total_unread_msgs_count(peers[i].unread_msgs_count - unread_count_old);
				if (flags & (1 << 0)) {
					offset += 12;
					offset += peernotifyset_offset(unenc_response + 52);
					peers[i].channel_pts = read_le(unenc_response + offset, 4);
				}
				memset(peers[i].channel_msg_id, 0, 8);
				break;
			}
		}
		break;
	}
	case 0x5d75a138: // updates.differenceEmpty
		break;
	case 0x3e11affb: { // updates.channelDifferenceEmpty
		for (int i = 0; i < peers_count; i++) {
			if (peers[i].type == 2 && memcmp(last_rpcresult_msgid, peers[i].channel_msg_id, 8) == 0) {
				memset(peers[i].channel_msg_id, 0, 8);
				break;
			}
		}
		break;
	}
	case 0x725b04c3: // updatesCombined
	case 0x74ae4240: { // updates
		if (dcInfo == &dcInfoMain) for (int i = 12; i < length;) i += update_handler(unenc_response + i);
		break;
	}
	case 0x78d4dec1: { // updateShort
		if (dcInfo == &dcInfoMain) update_handler(unenc_response + 4);
		break;		 
	}
	case 0x313bc7f8: // updateShortMessage
	case 0x4d6deea5: { // updateShortChatMessage
		if (peers_count == 0 || dcInfo != &dcInfoMain) break;
		bool chatMsg = (constructor == 0x4d6deea5) ? true : false;
		int offset = chatMsg ? 8 : 0;
		update_chats_order(unenc_response + 12 + offset, unenc_response + 8, chatMsg ? 1 : 0);
		int flags = read_le(unenc_response + 4, 4);
		BYTE* msg_bytes = &unenc_response[20 + offset];
		bool message_adding = (current_peer && memcmp(unenc_response + 12 + offset, current_peer->id, 8) == 0);
		offset += tlstr_len(unenc_response + 20 + offset, true) + 20;
		update_pts(read_le(unenc_response + offset, 4));
		offset += 8;
		date = read_le(unenc_response + offset, 4);
		if (message_adding) {
			offset += 4;
			BYTE* msgfwd = NULL;
			if (flags & (1 << 2)) {
				msgfwd = &unenc_response[offset];
				offset += msgfwd_offset(unenc_response + offset);
			}
			if (flags & (1 << 11)) offset += 8;
			BYTE* msgrpl = NULL;
			if (flags & (1 << 3)) {
				msgrpl = &unenc_response[offset];
				offset += msgrpl_offset(unenc_response + offset);
			}
			std::vector<int> format_vecs[10];
			if (flags & (1 << 7)) {
				int count = read_le(unenc_response + offset + 4, 4);
				offset += 8;
				for (int i = 0; i < count; i++) offset += msgent_offset(unenc_response + offset, &format_vecs[0]);
			}
			message_adder(false, false, flags, unenc_response + 8, msg_bytes, NULL, NULL, unenc_response + 12, &format_vecs[0], NULL, msgrpl, msgfwd, NULL, false, true, false, date);
		} else if (!(flags & (1 << 1))) {
			for (int i = 0; i < peers_count; i++) if (memcmp(peers[i].id, msg_bytes - 8, 8) == 0) {
				new_msg_notification(&peers[i], msg_bytes, false);
				break;
			}
		}
		break;			 
	}
	case 0x9015e101: { // updateShortSentMessage
		if (dcInfo != &dcInfoMain) break;
		for (int i = messages.size() - 1; i >= 0; i--) {
			Message message = messages[i];
			if (!messages[i].seen && messages[i].outgoing && read_le(last_rpcresult_msgid, 4) == (unsigned int)messages[i].id) {
				messages[i].id = read_le(unenc_response + 8, 4);
				FINDTEXTEX ft;
				ft.chrg.cpMin = messages[i].end_char;
				ft.chrg.cpMax = messages[i].end_footer;
				ft.lpstrText = L"sending";
				int diff = replace_in_chat(&ft, NULL, L"delivered", NULL, NULL, NULL, NULL);
				messages[i].end_footer += diff;
				break;
			}
		}
		update_pts(read_le(unenc_response + 12, 4));
		date = read_le(unenc_response + 20, 4);
		break;
	}
	case 0xe317af7e: // updatesTooLong
		break;
	case 0x84d19185: // messages.affectedMessages
		break;
	case 0x71e094f3: // messages.dialogsSlice
		total_peers_count = read_le(unenc_response + 4, 4);
		write_le(unenc_response + 4, 0x15ba6c40, 4);
		response_handler(dcInfo, unenc_response + 4, false, length-4);
		break;
	case 0xf0e3e596: // messages.dialogsNotModified
		break;
	case 0x15ba6c40: { // messages.dialogs
		if (!read_le(dcInfo->future_salt, 8)) break;
		if (!read_le(unenc_response + 8, 4)) {
			get_folders();
			break;
		}
		BYTE dialog_cons[4], message_cons[8], user_cons[4], group_cons[4], channel_cons[4];
		write_le(dialog_cons, 0xd58a08c6, 4);
		write_le(message_cons, 0x96fdbbe9, 4);
		write_le(message_cons + 4, 0xd3d28540, 4);
		write_le(user_cons, 0x4b46c37e, 4);
		write_le(group_cons, 0x41cbf256, 4);
		write_le(channel_cons, 0xe00998b7, 4);
		int offset = 0, offset_msg = 0, offset_user = 0, offset_group = 0, offset_channel = 0;

		offset_msg = array_find(unenc_response, message_cons, 4, 2);
		int peers_count_old = peers_count;
		peers_count += read_le(unenc_response + offset_msg - 4, 4);

		if (!peers) {
			if (!total_peers_count) total_peers_count = peers_count;
			folders = (ChatsFolder*)malloc(sizeof(ChatsFolder));
			folders[0].name = L"All chats";
			folders[0].pinned_count = 0;
			memset(folders[0].id, 0, 4);
			SendMessage(hComboBoxFolders, CB_ADDSTRING, 0, (LPARAM)folders[0].name);
			SendMessage(hComboBoxFolders, CB_SETITEMDATA, 0, (LPARAM)&folders[0]);
			SendMessage(hComboBoxFolders, CB_SETCURSEL, 0, 0);
			current_folder = &folders[0];

			peers = (Peer*)malloc(sizeof(Peer) * total_peers_count);
			folders[0].peers = (int*)malloc(sizeof(int) * total_peers_count);
		}

		folders[0].count = peers_count;
		int old_pinned_count = folders[0].pinned_count;
		int pinned_peers_count = 0;
		BYTE* pinned_peers_ids = NULL;
		int lowest_date = 2147483647;
		for (int i = peers_count_old; i < peers_count; i++) {
			offset_msg += array_find(unenc_response + offset_msg, message_cons, 4, 2) + 4;
			int flags_msg = read_le(unenc_response + offset_msg, 4);
			int flags2_msg = 0;
			if (memcmp(unenc_response + offset_msg - 4, message_cons, 4) == 0) {
				flags2_msg = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 12;
			} else offset_msg += 8;
			if (flags_msg & (1 << 8)) offset_msg += 12;
			if (flags_msg & (1 << 29)) offset_msg += 4;
			memcpy(peers[i].id, unenc_response + offset_msg + 4, 8);
			if (peers_count < total_peers_count) {
				int offset_msg2 = offset_msg + 12;
				if (flags_msg & (1 << 28)) offset_msg2 += 12;
				if (flags_msg & (1 << 2)) offset_msg2 += msgfwd_offset(unenc_response + offset_msg2);
				if (flags_msg & (1 << 11)) offset_msg2 += 8;
				if (flags2_msg & (1 << 0)) offset_msg2 += 8;
				if (flags_msg & (1 << 3)) offset_msg2 += msgrpl_offset(unenc_response + offset_msg2);
				int date = read_le(unenc_response + offset_msg2, 4);
				if (date < lowest_date) lowest_date = date;
			}

			int offset_dlg = array_find(unenc_response, peers[i].id, 8, 1) - 8;
			int flags = read_le(unenc_response + offset_dlg, 4);
			if (flags & (1 << 2)) {
				pinned_peers_count++;
				if (pinned_peers_ids) pinned_peers_ids = (BYTE*)malloc(pinned_peers_count * 8);
				else pinned_peers_ids = (BYTE*)realloc(pinned_peers_ids, pinned_peers_count * 8);
				memcpy(pinned_peers_ids + (pinned_peers_count-1)*8, unenc_response + offset_dlg + 8, 8);
			}
			peers[i].last_read = read_le(unenc_response + offset_dlg + 24, 4);
			peers[i].unread_msgs_count = read_le(unenc_response + offset_dlg + 28, 4);
			apply_notifysettings(unenc_response + offset_dlg + 40, &peers[i].mute_until, read_le(peers[i].id, 8));
			
			switch (read_le(unenc_response + offset_msg, 4)) {
			case 0x59511722: {
				if (offset_user == 0) offset_user = offset_msg + array_find(unenc_response + offset_msg, user_cons, 4, 1);
				peers[i].type = 0;
				set_peer_info(unenc_response + offset_user + array_find(unenc_response + offset_user, peers[i].id, 8, 1) - 12, &peers[i], false);
				break;
			}
			case 0x36c6019a: {
				if (offset_group == 0) offset_group = offset_msg + array_find(unenc_response + offset_msg, group_cons, 4, 1);
				peers[i].type = 1;
				set_peer_info(unenc_response + offset_group + array_find(unenc_response + offset_group, peers[i].id, 8, 1) - 8, &peers[i], false);
				break;
			}
			case 0xa2a5371e: {
				if (offset_channel == 0) offset_channel = offset_msg + array_find(unenc_response + offset_msg, channel_cons, 4, 1);
				peers[i].type = 2;
				set_peer_info(unenc_response + offset_channel + array_find(unenc_response + offset_channel, peers[i].id, 8, 1) - 12, &peers[i], false);
				if (flags & (1 << 0)) {
					offset_dlg += 40 + peernotifyset_offset(unenc_response + offset_dlg + 40);
					peers[i].channel_pts = read_le(unenc_response + offset_dlg, 4);
				}
				break;
			}
			}
			if ((flags & (1 << 2)) || pinned_peers_count != folders[0].pinned_count) {
				for (int j = 0; j < pinned_peers_count * 8; j += 8)
				if (memcmp(pinned_peers_ids + j, peers[i].id, 8) == 0) {
					folders[0].peers[folders[0].pinned_count] = i;
					folders[0].pinned_count++;
					break;
				}
			}
			if (peers[i].unread_msgs_count && !peers[i].mute_until && !muted_types[peers[i].type]) update_total_unread_msgs_count(peers[i].unread_msgs_count);
		}
		if (pinned_peers_ids) free(pinned_peers_ids);
		int diff = 0;
		for (i = peers_count_old; i < peers_count; i++) {
			bool found = false;
			for (int j = 0; j < folders[0].pinned_count; j++) {
				if (i == folders[0].peers[j]) {
					diff++;
					found = true;
					break;
				}
			}
			if (found) continue;
			folders[0].peers[i-diff+folders[0].pinned_count-old_pinned_count] = i;
		}	
		for (i = peers_count_old; i < peers_count; i++) {
			SendMessage(hComboBoxChats, CB_ADDSTRING, 0, (LPARAM)peers[folders[0].peers[i]].name);
			SendMessage(hComboBoxChats, CB_SETITEMDATA, i, (LPARAM)&peers[folders[0].peers[i]]);
		}

		if (peers_count < total_peers_count) {
			// messages.getDialogs
			BYTE unenc_query[96];
			BYTE enc_query[120];
			internal_header(unenc_query, true);
			write_le(unenc_query + 32, 0xa0f4cb4f, 4);
			memset(unenc_query + 36, 0, 12);
			write_le(unenc_query + 40, lowest_date, 4);
			write_le(unenc_query + 48, 0x7f3b18ea, 4);
			int offset = 52;
			//int offset = 48 + place_peer(unenc_query + 48, &peers[peers_count - 1], true);
			memset(unenc_query + offset, 0, 12);
			offset += 12;
			write_le(unenc_query + 28, offset - 32, 4);
			int padding_len = get_padding(offset);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			offset += padding_len;
			convert_message(unenc_query, enc_query, offset, 0);
			send_query(enc_query, offset + 24);
		} else get_folders();
		break;			 
	}
	case 0x2ad93719: { // messages.dialogFilters (folders)
		if (peers_count == 0) break;
		folders_count = read_le(unenc_response + 12, 4);
		if (folders_count == 1) break;
		folders = (ChatsFolder*)realloc(folders, sizeof(ChatsFolder)*folders_count);
		SendMessage(hComboBoxFolders, CB_SETITEMDATA, 0, (LPARAM)&folders[0]);
		current_folder = &folders[0];
		int offset = 24;

		BYTE folder_cons[4];
		write_le(folder_cons, 0xaa472651, 4);

		for (int i = 1; i < folders_count; i++) {
			offset += folder_handler(unenc_response + offset, &folders[i], i, false);
			if (i != folders_count-1) offset += array_find(unenc_response + offset, folder_cons, 4, 1) + 4;
		}

		DestroyWindow(current_info);
		ShowWindow(hMain, maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
		set_tray_icon();
		break;
	}
	case 0x1cb5c415: { // vector
		unsigned int vec_constructor = read_le(unenc_response + 8, 4);
		switch (vec_constructor) {
		case 0xa384b779: // received messages ack
			break;
		case 0x4b46c37e: // user (myself)
			myself.type = 0;
			myself.online = -2;
			set_peer_info(unenc_response + 8, &myself, false);
			get_full_peer(&myself);
			break;
		case 0x8fd4c4d8: { // document (custom emojis)
			int offset = 8;
			int count = read_le(unenc_response + 4, 4);
			for (int i = 0; i < count; i++) {
				for (int j = 0; j < rces.size(); j++) if (memcmp(&rces[j].id, unenc_response + offset + 8, 8) == 0) {
					int flags_doc = read_le(unenc_response + offset + 4, 4);
					memcpy(&rces[j].access_hash, unenc_response + offset + 16, 8);
					memcpy(rces[j].file_reference, unenc_response + offset + 24, tlstr_len(unenc_response + offset + 24, true));
					offset += 28 + tlstr_len(unenc_response + offset + 24, true);
					offset += 8 + tlstr_len(unenc_response + offset, true);
					offset += photo_video_size_offset(unenc_response + offset, (flags_doc & (1 << 0)) ? true : false, true, (flags_doc & (1 << 1)) ? true : false);
					rces[j].dc = read_le(unenc_response + offset, 4);
					offset += 4;
					int count2 = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					for (int k = 0; k < count2; k++) offset += docatt_offset(unenc_response + offset);
					break;
				}
			}
			if (rces.size() && read_le(rces[0].msg_id, 8) == 0) get_photo(&rces[0], NULL, &dcInfoMain);
			break;
		}
		}
		break;
	}
	case 0x4b46c37e: // user
		break;
	case 0x3b6d152e: { // users.userFull
		for (int i = -1; i < peers_count; i++) {
			Peer* peer = (i == -1) ? &myself : &peers[i];
			if (memcmp(peer->id, unenc_response + 16, 8) == 0) {
				peer->full = true;
				int flags = read_le(unenc_response + 8, 4);
				int flags2 = read_le(unenc_response + 12, 4);
				int offset = 24;
				if (flags & (1 << 1)) {
					peer->about = read_string(unenc_response + offset, NULL);
					offset += tlstr_len(unenc_response + offset, true);
				} else peer->about = NULL;
				int peerset_flags = read_le(unenc_response + offset + 4, 4);
				offset += 8;
				if (peerset_flags & (1 << 6)) offset += 4;
				if (peerset_flags & (1 << 9)) offset += tlstr_len(unenc_response + offset, true) + 4;
				if (peerset_flags & (1 << 13)) offset += tlstr_len(unenc_response + offset + 8, true) + 8;
				if (peerset_flags & (1 << 14)) offset += 8;
				if (peerset_flags & (1 << 15)) offset += tlstr_len(unenc_response + offset, true);
				if (peerset_flags & (1 << 16)) offset += tlstr_len(unenc_response + offset + 8, true) + 8;
				if (peerset_flags & (1 << 17)) offset += 4;
				if (peerset_flags & (1 << 18)) offset += 8;
				if (flags & (1 << 21)) offset += photo_offset(unenc_response + offset);
				if (flags & (1 << 2)) {
					memcpy(peer->photo, unenc_response + offset + 8, 8);
					offset += photo_offset(unenc_response + offset);
				} else memset(peer->photo, 0, 8);
				if (flags & (1 << 22)) offset += photo_offset(unenc_response + offset);
				offset += peernotifyset_offset(unenc_response + offset);
				if (flags & (1 << 3)) offset += botinfo_offset(unenc_response + offset);
				if (flags & (1 << 6)) offset += 4;
				offset += 4;
				if (flags & (1 << 11)) offset += 4;
				if (flags & (1 << 14)) offset += 4;
				if (flags & (1 << 15)) {
					for (int j = 0; j < themes.size(); j++) {
						if (memcmp(themes[j].emoji_id, unenc_response + offset, tlstr_len(unenc_response + offset, true)) == 0) {
							memcpy(peer->theme_id, themes[j].id, 8);
							break;
						}
					}
					offset += tlstr_len(unenc_response + offset, true);
				} else memset(peer->theme_id, 0, 8);
				peer->theme_set_time = current_time();
				if (flags & (1 << 16)) offset += tlstr_len(unenc_response + offset, true);
				if (flags & (1 << 17)) offset += 8;
				if (flags & (1 << 18)) offset += 8;
				if (flags & (1 << 19)) {
					int count = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					for (int j = 0; j < count; j++) {
						int flags_gift = read_le(unenc_response + offset + 4, 4);
						offset += 12;
						offset += tlstr_len(unenc_response + offset, true);
						offset += 8;
						offset += tlstr_len(unenc_response + offset, true);
						if (flags_gift & (1 << 0)) offset += tlstr_len(unenc_response + offset, true);
					}
				}
				if (flags & (1 << 24)) offset += wallpaper_offset(unenc_response + offset);
				if (flags & (1 << 25)) {
					int flags_story = read_le(unenc_response + offset + 4, 4);
					offset += 20;
					if (flags_story & (1 << 0)) offset += 4;
					int count = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					for (int j = 0; j < count; j++) offset += story_offset(unenc_response + offset);
				}
				if (flags2 & (1 << 0)) {
					offset += 8;
					offset += tlstr_len(unenc_response + offset, true);
					int count = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					for (int j = 0; j < count; j++) offset += 12;
				}
				if (flags2 & (1 << 1)) {
					int flags_busgeo = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					if (flags_busgeo & (1 << 0)) offset += geo_offset(unenc_response + offset);
					offset += tlstr_len(unenc_response + offset, true);
				}
				if (flags2 & (1 << 2)) {
					int flags_busrec = read_le(unenc_response + offset + 12, 4);
					offset += 16;
					if (flags_busrec & (1 << 4)) {
						int count = read_le(unenc_response + offset + 4, 4);
						offset += 8;
						for (int j = 0; j < count; j++) offset += 8;
					}
					offset += 4;
				}
				if (flags2 & (1 << 3)) {
					offset += 12;
					int sche_cons = read_le(unenc_response + offset, 4);
					if (sche_cons == 0xcc4d9ecc) offset += 12;
					else offset += 4;
					int flags_busrec = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					if (flags_busrec & (1 << 4)) {
						int count = read_le(unenc_response + offset + 4, 4);
						offset += 8;
						for (int j = 0; j < count; j++) offset += 8;
					}
				}
				if (flags2 & (1 << 4)) {
					int flags_busint = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					offset += tlstr_len(unenc_response + offset, true);
					offset += tlstr_len(unenc_response + offset, true);
					if (flags_busint & (1 << 0)) offset += doc_offset(unenc_response + offset);
				}
				if (flags2 & (1 << 5)) {
					int flags_birthday = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					peer->birthday[0] = unenc_response[offset];
					offset += 4;
					peer->birthday[1] = unenc_response[offset];
					offset += 4;
					if (flags_birthday & (1 << 0)) {
						memcpy(peer->birthday + 2, unenc_response + offset, 2);
						offset += 4;
					} else memset(peer->birthday + 2, 0, 2);
				} else memset(peer->birthday, 0, 4);
				if (current_peer == peer) SendMessage(hMain, WM_COMMAND, MAKEWPARAM(3, CBN_SELCHANGE), NULL);
				if (i != -1) break;
			}
		}
		break;
	}
	case 0xe5d7d19c: { // messages.chatFull
		switch (read_le(unenc_response + 4, 4)) {
		case 0x2633421b: { // chatFull
			for (int i = 0; i < peers_count; i++) {
				if (memcmp(peers[i].id, unenc_response + 12, 8) == 0) {
					peers[i].chat_users = new std::vector<Peer>();
					peers[i].full = true;
					int flags = read_le(unenc_response + 8, 4);
					int offset = tlstr_len(unenc_response + 20, true);
					if (unenc_response[20]) peers[i].about = read_string(unenc_response + 20, NULL);
					else peers[i].about = NULL;
					int chat_users_count = read_le(unenc_response + offset + 36, 4);
					BYTE user_cons[4];
					write_le(user_cons, 0x4b46c37e, 4);
					offset += 40;
					int user_pos = offset; 
					for (int j = 0; j < chat_users_count; j++) {
						user_pos += array_find(unenc_response + user_pos + 4, user_cons, 4, 1) + 4;
						Peer peer;
						peer.type = 0;
						set_peer_info(unenc_response + user_pos, &peer, false);
						peers[i].chat_users->push_back(peer);
						offset += (read_le(unenc_response + offset, 4) == 0xe46bcee4) ? 12 : 24;
					}
					offset += 4;
					if (flags & (1 << 2)) offset += photo_offset(unenc_response + offset);
					offset += peernotifyset_offset(unenc_response + offset);
					if (flags & (1 << 13)) offset += exchatinv_offset(unenc_response + offset);
					if (flags & (1 << 3)) {
						int count = read_le(unenc_response + offset + 4, 4);
						offset += 8;
						for (int j = 0; j < count; j++) offset += botinfo_offset(unenc_response + offset);
					}
					if (flags & (1 << 6)) offset += 4;
					if (flags & (1 << 11)) offset += 4;
					if (flags & (1 << 12)) offset += 20;
					if (flags & (1 << 14)) offset += 4;
					if (flags & (1 << 15)) offset += 12;
					if (flags & (1 << 16)) {
						int size = tlstr_len(unenc_response + offset, true);
						for (int j = 0; j < themes.size(); j++) {
							if (size != tlstr_len(themes[j].emoji_id, true)) continue;
							if (memcmp(themes[j].emoji_id, unenc_response + offset, size) == 0) {
								memcpy(peers[i].theme_id, themes[j].id, 8);
								break;
							}
						}
						offset += size;
					} else memset(peers[i].theme_id, 0, 8);
					peers[i].theme_set_time = current_time();
					if (flags & (1 << 17)) {
						offset += 4;
						int count = read_le(unenc_response + offset + 4, 4);
						offset += 8;
						for (int j = 0; j < count; j++) offset += 8;
					}
					if (flags & (1 << 18)) offset += chatreactions_offset(unenc_response + offset, &peers[i]);
					if (current_peer == &peers[i]) SendMessage(hMain, WM_COMMAND, MAKEWPARAM(3, CBN_SELCHANGE), NULL);
					break;
				}
			}
			break;				 
		}
		case 0x9ff3b858: { // channelFull
			for (int i = 0; i < peers_count; i++) {
				if (memcmp(peers[i].id, unenc_response + 16, 8) == 0) {
					peers[i].full = true;
					int flags = read_le(unenc_response + 8, 4);
					int offset = 24 + tlstr_len(unenc_response + 24, true);
					if (unenc_response[24]) peers[i].about = read_string(unenc_response + 24, NULL);
					else peers[i].about = NULL;
					if (flags & (1 << 0)) {
						peers[i].chat_users = (std::vector<Peer>*)read_le(unenc_response + offset, 4);
						offset += 4;
					}
					if (flags & (1 << 1)) offset += 4;
					if (flags & (1 << 2)) offset += 8;
					if (flags & (1 << 13)) offset += 4;
					offset += 12;
					offset += photo_offset(unenc_response + offset);
					offset += peernotifyset_offset(unenc_response + offset);
					if (flags & (1 << 23)) offset += exchatinv_offset(unenc_response + offset);
					int botinfo_count = read_le(unenc_response + offset + 4, 4);
					offset += 8;
					for (int j = 0; j < botinfo_count; j++) offset += botinfo_offset(unenc_response + offset);
					if (flags & (1 << 4)) offset += 12;
					if (flags & (1 << 5)) offset += 4;
					if (flags & (1 << 8)) {
						int sticker_flags = read_le(unenc_response + offset + 4, 4);
						offset += 24;
						if (sticker_flags & (1 << 0)) offset += 4;
						offset += tlstr_len(unenc_response + offset, true);
						offset += tlstr_len(unenc_response + offset, true);
						if (sticker_flags & (1 << 4)) offset += photo_video_size_offset(unenc_response + offset, true, true, false) + 8;
						if (sticker_flags & (1 << 8)) offset += 8;
						offset += 8;
					}
					if (flags & (1 << 9)) offset += 4;
					if (flags & (1 << 11)) offset += 4;
					if (flags & (1 << 14)) offset += 8;
					if (flags & (1 << 15)) {
						int channelloc_cons = read_le(unenc_response + offset, 4);
						offset += 4;
						if (channelloc_cons == 0x209b82db) {
							offset += geo_offset(unenc_response + offset);
							offset += tlstr_len(unenc_response + offset, true);
						}
					}
					if (flags & (1 << 17)) offset += 4;
					if (flags & (1 << 18)) offset += 4;
					if (flags & (1 << 12)) offset += 4;
					peers[i].channel_pts = read_le(unenc_response + offset, 4);
					offset += 4;
					if (flags & (1 << 21)) offset += 20;
					if (flags & (1 << 24)) offset += 4;
					if (flags & (1 << 25)) {
						int count = read_le(unenc_response + offset + 4, 4);
						offset += 8;
						for (int j = 0; j < count; j++) offset += tlstr_len(unenc_response + offset, true);
					}
					if (flags & (1 << 26)) offset += 12;
					if (flags & (1 << 27)) {
						int size = tlstr_len(unenc_response + offset, true);
						for (int j = 0; j < themes.size(); j++) {
							if (size != tlstr_len(themes[j].emoji_id, true)) continue;
							if (memcmp(themes[j].emoji_id, unenc_response + offset, size) == 0) {
								memcpy(peers[i].theme_id, themes[j].id, 8);
								break;
							}
						}
						offset += size;
					} else memset(peers[i].theme_id, 0, 8);
					if (flags & (1 << 28)) {
						int count = read_le(unenc_response + offset + 8, 4);
						offset += 12;
						for (int j = 0; j < count; j++) offset += 8;
					}
					if (flags & (1 << 29)) offset += 12;
					if (flags & (1 << 30)) offset += chatreactions_offset(unenc_response + offset, &peers[i]);
					if (current_peer == &peers[i]) SendMessage(hMain, WM_COMMAND, MAKEWPARAM(3, CBN_SELCHANGE), NULL);
					break;
				}
			}
			break;
		}
		}
		break;
	}
	case 0x3a54685e: // messages.messagesSlice
	case 0xc776ba4e: // messages.channelMessages
	case 0x8c718e87: { // messages.Messages
		int offset_msg = 12;
		if (constructor != 0x8c718e87) {
			int flags = read_le(unenc_response + 4, 4);
			offset_msg += 8;
			if (flags & (1 << 0)) offset_msg += 4;
			if (flags & (1 << 2)) offset_msg += 4;
			if (constructor == 0xc776ba4e) offset_msg += 4;
		}

		int offset2 = offset_msg + 16;
		int flags_msg = read_le(unenc_response + offset_msg + 4, 4);
		if (flags_msg & (1 << 29)) offset2 += 4;
		if (flags_msg & (1 << 8)) offset2 += 12;
		if (read_le(unenc_response + offset_msg, 4) == 0x96fdbbe9) offset2 += 4;

		int count = read_le(unenc_response + offset_msg - 4, 4);
		bool neworrep = false;
		if (count == 1) {
			int id = read_le(unenc_response + offset_msg + 12, 4);
			for (int i = 0; i < messages.size(); i++) {
				if (messages[i].reply_needed == id) {
					messages[i].reply_needed = 0;
					if (messages[i].end_char == messages[i].end_footer) break;
					neworrep = true;
					set_reply_tofront(i, unenc_response + offset_msg, NULL);
					break;
				}
			}
			for (i = 0; i < peers_count; i++) {
				if (peers[i].name == NULL) {
					BYTE* peer_bytes = find_peer(unenc_response + offset_msg, &peers[i].id[0] - 4, true, &peers[i].type);
					set_peer_info(peer_bytes, &peers[i], false);
					if (&folders[0] == current_folder) SendMessage(hComboBoxChats, CB_INSERTSTRING, folders[0].pinned_count, (LPARAM)peers[i].name);
					if (&folders[0] == current_folder) SendMessage(hComboBoxChats, CB_SETITEMDATA, folders[0].pinned_count, (LPARAM)&peers[i]);
					neworrep = true;
					break;
				}
			}
		}
		if (count == 0 && messages.size() == 0 && current_peer != NULL) {
			remove_peer(current_peer);
			break;
		}
		if (!current_peer || neworrep || memcmp(unenc_response + offset2, current_peer->id, 8) != 0) break;
		int messages_count_old = messages.size();
		int documents_count_old = documents.size();
		for (int i = 0; i < count; i++) offset_msg += message_handler(true, unenc_response + offset_msg, false, false, false);
		int reply_check = count;
		while (messages[reply_check-1].end_char == messages[reply_check-1].end_footer) reply_check++;
		if (messages_count_old != messages.size()) for (i = 0; i < reply_check; i++) {
			if (messages[i].reply_needed) {
				for (int j = 0; j <= i; j++) if (j == i || messages[j].id == messages[i].reply_needed) break;
				if (j == i) get_message(messages[i].reply_needed, current_peer);
				else set_reply_tofront(i, NULL, j);
			}
		}
		if (IMAGELOADPOLICY == 2 && documents_count_old != documents.size()) for (int i = documents.size() - documents_count_old - 1; i >= 0; i--) {
			if (!read_le(documents[i].photo_msg_id, 8)) {
				get_photo(NULL, &documents[i], dcInfo);
				break;
			}
		}
		get_unknown_custom_emojis();
		if (messages_count_old == 0 && count > 0) {
			SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
			if (current_peer->unread_msgs_count && !current_peer->mute_until && !muted_types[current_peer->type]) update_total_unread_msgs_count(0 - current_peer->unread_msgs_count);
			current_peer->unread_msgs_count = 0;
			if (memcmp(current_peer->id, notification_peer_id, 8) == 0) remove_notification();
			if (!current_peer->status_updated) {
				offset_msg += array_find(unenc_response + offset_msg, current_peer->id, 8, 1) - (current_peer->type == 1 ? 8 : 12);
				set_peer_info(unenc_response + offset_msg, current_peer, true);
			}
			status_bar_status(current_peer);
		}
		if (current_peer->type == 2 && messages_count_old == 0 && messages.size() > 0) {
			current_peer->channel_pts = read_le(unenc_response + 8, 4);
			get_channel_difference(current_peer);
			SetTimer(hMain, 3, 30000, NULL);
		}
		SCROLLINFO si = {0};
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_TRACKPOS | SIF_PAGE | SIF_POS;
		GetScrollInfo(chat, SB_VERT, &si);
		UpdateWindow(chat);
		if (messages.size() - messages_count_old < 10) no_more_msgs = true;
		else if (SendMessage(chat, EM_GETFIRSTVISIBLELINE, 0, 0) == 0) get_history();
		break;
	}
	case 0xb45c69d1: // messages.affectedHistory
		break;
	case 0x629f1980: { // auth.loginToken
		int expires_in = (read_le(unenc_response + 4, 4) - current_time()) * 1000;
		SetTimer(current_dialog, 1, expires_in + 1500, NULL);
		char token[256] = "tg://login?token=";
		long token_size = tlstr_len(unenc_response + 8, false);
		DWORD size_out = 200;
		base64url_encode(unenc_response + (unenc_response[8] == 254 ? 12 : 9), token_size, (BYTE*)(token + strlen(token)), &size_out);

		BYTE qrcode[qrcodegen_BUFFER_LEN_MAX];
		BYTE temp[qrcodegen_BUFFER_LEN_MAX];
		qrcodegen_encodeText(token, temp, qrcode, qrcodegen_Ecc_LOW, qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
		int size = qrcodegen_getSize(qrcode);
		int scale = 180 / size;
		int imgSize = size * scale;
		BITMAPINFO bmi = {0};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = imgSize;
		bmi.bmiHeader.biHeight = -imgSize;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 32;
		bmi.bmiHeader.biCompression = BI_RGB;

		unsigned int* pixels = (unsigned int*)malloc(imgSize * imgSize * 4);
		for (int y = 0; y < imgSize; y++) {
			for (int x = 0; x < imgSize; x++) {
				int moduleX = x / scale;
				int moduleY = y / scale;
				bool black = qrcodegen_getModule(qrcode, moduleX, moduleY);
				pixels[y * imgSize + x] = black ? 0xFF000000 : 0xFFFFFFFF;
			}
		}

		HDC hdcRef = GetDC(NULL);
		HBITMAP hBmp = CreateCompatibleBitmap(hdcRef, imgSize, imgSize);
		SetDIBits(hdcRef, hBmp, 0, imgSize, pixels, &bmi, DIB_RGB_COLORS);
		ReleaseDC(NULL, hdcRef);
		free(pixels);

		HBITMAP oldBmp = (HBITMAP)SendMessage(hQRCode, STM_GETIMAGE, IMAGE_BITMAP, 0);
		if (oldBmp) DeleteObject(oldBmp);
		SendMessage(hQRCode, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);

		FILE* f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"rb");
		if (f) fclose(f);
		else get_config();
		break;
	}
	case 0x96a18d5: { // upload.file
		if (dcInfo != &dcInfoMain) SetTimer(hMain, 20 + dcInfo->dc, 60000, NULL);
		bool found = false;
		for (int i = 0; i < downloading_docs.size(); i++) {
			if (memcmp(&downloading_docs[i], last_rpcresult_msgid, 8) == 0) {
				found = true;
				int size = tlstr_len(unenc_response + 12, false);
				int start = size >= 254 ? 4 : 1;
				FILE* f = _wfopen(downloading_docs[i].filename, L"ab");
				if (f) {
					fwrite(unenc_response + 12 + start, 1, size, f);
					__int64 file_size = _ftelli64(f);
					fclose(f);
					int percentage = (int)((double)file_size / (double)downloading_docs[i].size * 100.0);
					if (size < 1048576) {
						wchar_t status_str[100];
						swprintf(status_str, L"Downloading %s... done!", downloading_docs[i].filename);
						SendMessage(hStatus, SB_SETTEXTA, 1 | SBT_OWNERDRAW, (LPARAM)status_str);
						free(downloading_docs[i].filename);
						free(downloading_docs[i].file_reference);
						downloading_docs.erase(downloading_docs.begin() + i);
						if (uploadEvents.size() == 0 && downloading_docs.size() == 0) SetTimer(hMain, 2, 3000, NULL);
					}
				} else size = -1;
				if (size >= 1048576 || size == -1) download_file(dcInfo, &downloading_docs[i]);
				break;
			}
		}
		if (found) break;
		if (memcmp(pfp_msgid, last_rpcresult_msgid, 8) == 0) {
			HBITMAP hBmp = jpg_to_bmp(unenc_response + 16, tlstr_len(unenc_response + 12, false));
			SendMessage(dlgPic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
			break;
		}
		for (i = documents.size() - 1; i >= 0; i--) {
			if (found) {
				if (!read_le(documents[i].photo_msg_id, 8)) {
					get_photo(NULL, &documents[i], &dcInfoMain);
					break;
				}
			} else if (memcmp(&documents[i].photo_msg_id, last_rpcresult_msgid, 8) == 0) {
				HBITMAP hClone;
				if (documents[i].photo_size == 1) {
					int width, height;
					BYTE* rgba = WebPDecodeRGBA(unenc_response + 16, tlstr_len(unenc_response + 12, false), &width, &height);
					hClone = rgb_to_bmp(rgba, true, width, height);
				} else hClone = jpg_to_bmp(unenc_response + 16, tlstr_len(unenc_response + 12, false));
				CHARRANGE cr;
				cr.cpMin = documents[i].min;
				cr.cpMax = documents[i].max;
				replace_in_chat(NULL, &cr, NULL, hClone, NULL, NULL, NULL);
				DeleteObject(hClone);
				found = true;
			}
		}
		if (found) break;
		if (rces.size() && memcmp(rces[0].msg_id, last_rpcresult_msgid, 8) == 0) {
			wchar_t path[MAX_PATH];
			swprintf(path, L"%s\\%016I64X.ico", get_path(appdata_path, L"custom_emojis"), rces[0].id);
			WebPDecoderConfig config;
			WebPInitDecoderConfig(&config);
			int len = tlstr_len(unenc_response + 12, false);
			WebPGetFeatures(unenc_response + (len >= 254 ? 16 : 13), len, &config.input);
			config.options.use_scaling = 1;
			config.options.scaled_width = 15;
			config.options.scaled_height = 15;
			config.output.colorspace = MODE_RGBA;
			WebPDecode(unenc_response + (len >= 254 ? 16 : 13), len, &config);
			BYTE* rgba = config.output.u.RGBA.rgba;
			if (!rgba) {
				rces.erase(rces.begin());
				if (rces.size()) get_photo(&rces[0], NULL, &dcInfoMain);
				break;
			}

			int bmpStride = ((15 * 3 + 3) & ~3);
			int bmpSize = bmpStride * 15;
			int maskStride = ((15 + 31) / 32) * 4;
			int maskSize = maskStride * 15;

			BYTE* bmpBits = (BYTE*)calloc(bmpSize, 1);
			BYTE* maskBits = (BYTE*)calloc(maskSize, 1);
			int x, y;
			for (y = 0; y < 15; y++) {
				BYTE* bmpRow = bmpBits + (15 - 1 - y) * bmpStride;
				BYTE* maskRow = maskBits + (15 - 1 - y) * maskStride;
				BYTE* rgbaRow = rgba + y * 15 * 4;
				for (x = 0; x < 15; x++) {
					if (rgbaRow[x*4 + 3] < 96) {
						maskRow[x / 8] |= (0x80 >> (x % 8));
						bmpRow[x*3 + 0] = 0;
						bmpRow[x*3 + 1] = 0;
						bmpRow[x*3 + 2] = 0;
					} else {
						bmpRow[x*3 + 0] = rgbaRow[x*4 + 2];
						bmpRow[x*3 + 1] = rgbaRow[x*4 + 1];
						bmpRow[x*3 + 2] = rgbaRow[x*4 + 0];
					}
				}
			}

			ICONDIR dir;
			dir.idReserved = 0;
			dir.idType = 1;
			dir.idCount = 1;

			ICONDIRENTRY entry;
			entry.bWidth = 15;
			entry.bHeight = 15;
			entry.bColorCount = 0;
			entry.bReserved = 0;
			entry.wPlanes = 1;
			entry.wBitCount = 24;
			entry.dwBytesInRes = sizeof(BITMAPINFOHEADER) + bmpSize + maskSize;
			entry.dwImageOffset = sizeof(ICONDIR) + sizeof(ICONDIRENTRY);

			BITMAPINFOHEADER bih = {0};
			bih.biSize = sizeof(BITMAPINFOHEADER);
			bih.biWidth = 15;
			bih.biHeight = 30;
			bih.biPlanes = 1;
			bih.biBitCount = 24;
			bih.biCompression = BI_RGB;

			FILE* f = _wfopen(path, L"wb");
			fwrite(&dir, sizeof(ICONDIR), 1, f);
			fwrite(&entry, sizeof(ICONDIRENTRY), 1, f);
			fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, f);
			fwrite(bmpBits, 1, bmpSize, f);
			fwrite(maskBits, 1, maskSize, f);
			fclose(f);

			free(bmpBits);
			free(maskBits);
			
			WebPFreeDecBuffer(&config.output);
			
			if (!nt3 && rces[0].reaction) {
				HWND child = GetWindow(reactionStatic, GW_CHILD);
				while (child != NULL) {
					wchar_t* code = (wchar_t*)GetWindowLongPtr(child, GWLP_USERDATA);
					if (code[0] == 1 && wcsncmp(wcsrchr(path, L'\\') + 1, code + 1, wcslen(code + 1)) == 0) {
						HICON hIcon = (HICON)LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
						SendMessage(child, BM_SETIMAGE, (WPARAM)IMAGE_ICON, (LPARAM)hIcon);
						break;
					}
					child = GetWindow(child, GW_HWNDNEXT);
				}
			}
			while (rces[0].ceps.size()) {
				if (memcmp(current_peer->id, rces[0].ceps[0].peer_id, 8) == 0) {
					CHARRANGE cr = {0};
					for (int j = messages.size() - 1; j >= 0; j--) {
						if (messages[j].id == rces[0].ceps[0].msg_id) {
							cr.cpMin = (rces[0].ceps[0].size == 15 ? messages[j].end_header : messages[j].start_reactions) + rces[0].ceps[0].pos;
							cr.cpMax = cr.cpMin + 1;
							break;
						}
					}
					if (cr.cpMin) {
						rces[0].ceps[0].path = path;
						replace_in_chat(NULL, &cr, NULL, NULL, NULL, &rces[0].ceps[0], NULL);
					}
				}
				rces[0].ceps.erase(rces[0].ceps.begin());
			}
			rces.erase(rces.begin());
			if (rces.size()) get_photo(&rces[0], NULL, &dcInfoMain);
		}
		break;
	}
	case 0x695150d7: // messageMediaPhoto
	case 0xdd570bd5: // messageMediaDocument
		if (sendMultiMedia != NULL) {
			bool photo = (constructor == 0x695150d7) ? true : false;
			int peer_len = (read_le(sendMultiMedia + 40, 4) == 0x35a95cb9) ? 12 : 20;
			int count = read_le(sendMultiMedia + 44 + peer_len, 4);
			int offset = 48 + peer_len;
			bool finished = true;
			for (int i = 0; i < count; i++) {
				if (memcmp(sendMultiMedia + offset + 8, last_rpcresult_msgid, 8) == 0) {
					if (photo && !SENDMEDIAASFILES) write_le(sendMultiMedia + offset + 8, 0xb3ba0635, 4);
					else write_le(sendMultiMedia + offset + 8, 0x33473058, 4);
					memset(sendMultiMedia + offset + 12, 0, 4);
					if (photo && !SENDMEDIAASFILES) write_le(sendMultiMedia + offset + 16, 0x3bb3b94a, 4);
					else write_le(sendMultiMedia + offset + 16, 0x1abfb575, 4);
					memcpy(sendMultiMedia + offset + 20, unenc_response + 16, 40);
				}
				if (read_le(sendMultiMedia + offset + 8, 4) != 0x33473058 && read_le(sendMultiMedia + offset + 8, 4) != 0xb3ba0635) finished = false;
				offset += 68;
				offset += tlstr_len(sendMultiMedia + offset, true);
				if (read_le(sendMultiMedia + offset, 4) == 0x1cb5c415) {
					int count = read_le(sendMultiMedia + offset + 4, 4);
					offset += 8;
					for (int i = 0; i < count; i++) offset += msgent_offset(sendMultiMedia + offset, NULL);
				}
			}
			if (finished) {
				internal_header(sendMultiMedia, true);
				int size = 32 + read_le(sendMultiMedia + 28, 4);
				size += get_padding(size);
				BYTE* enc_query = (BYTE*)malloc(size + 24);
				convert_message(sendMultiMedia, enc_query, size, 0);
				free(sendMultiMedia);
				sendMultiMedia = NULL;
				send_query(enc_query, size + 24);
				free(enc_query);
			}
		}
		break;
	case 0x768e3aad: { // messages.availableReactions
		for (int i = 0; i < reaction_list.size(); i++) free(reaction_list[i]);
		reaction_list.clear();
		reaction_hash = read_le(unenc_response + 4, 4);
		int count = read_le(unenc_response + 12, 4);
		int offset = 16;
		for (i = 0; i < count; i++) {
			int flags = read_le(unenc_response + offset + 4, 4);
			offset += 8;
			wchar_t str[10];
			read_string(unenc_response + offset, str);
			wchar_t file_name[50];
			file_name[0] = 0;
			for (int j = 0; j < wcslen(str); j++) {
				int cr;
				if (str[j] >= 0xD800 && str[j] <= 0xDBFF) {
					cr = ((str[j] - 0xD800) << 10) + (str[j+1] - 0xDC00) + 0x10000;
					j++;
				} else cr = str[j];
				if (file_name[0] == 0) swprintf(file_name, L"%x", cr);
				else swprintf(file_name, L"%s-%x", file_name, cr);
			}
			wchar_t* filename = _wcsdup(file_name);
			reaction_list.push_back(filename);
			offset += tlstr_len(unenc_response + offset, true);
			offset += tlstr_len(unenc_response + offset, true);
			for (j = 0; j < 5; j++) offset += doc_offset(unenc_response + offset);
			if (flags & (1 << 1)) for (j = 0; j < 2; j++) offset += doc_offset(unenc_response + offset);
		}
		break;			 
	}
	case 0x9a3d8c6d: { // account.themes
		for (int i = 0; i < themes.size(); i++) free(themes[i].emoji_id);
		themes.clear();
		theme_hash = read_le(unenc_response + 4, 8);
		int count = read_le(unenc_response + 16, 4);
		int offset = 20;
		for (i = 0; i < count; i++) {
			Theme theme;
			int flags = read_le(unenc_response + offset + 4, 8);
			offset += 8;
			memcpy(theme.id, unenc_response + offset, 8);
			offset += 16;
			offset += tlstr_len(unenc_response + offset, true);
			offset += tlstr_len(unenc_response + offset, true);
			if (flags & (1 << 2)) offset += doc_offset(unenc_response + offset);
			if (flags & (1 << 3)) {
				int count2 = read_le(unenc_response + offset + 4, 4);
				offset += 8;
				for (int j = 0; j < count2; j++) {
					int flags2 = read_le(unenc_response + offset + 4, 4);
					offset += 12;
					if (read_le(unenc_response + offset - 4, 4) == 0xc3a12462 || read_le(unenc_response + offset - 4, 4) == 0xfbd81688) {
						int argb = read_le(unenc_response + offset, 4);
						theme.color = RGB((argb >> 16) & 0xFF, (argb >> 8) & 0xFF, (argb) & 0xFF);
					}
					offset += 4;
					if (flags2 & (1 << 3)) offset += 4;
					if (flags2 & (1 << 0)) {
						int count3 = read_le(unenc_response + offset + 4, 4);
						offset += 8;
						for (int k = 0; k < count3; k++) offset += 4;
					}
					if (flags2 & (1 << 1)) offset += wallpaper_offset(unenc_response + offset);
				}
			}
			if (flags & (1 << 6)) {
				if (flags & (1 << 3)) {
					theme.emoji_id = (BYTE*)malloc(tlstr_len(unenc_response + offset, true));
					memcpy(theme.emoji_id, unenc_response + offset, tlstr_len(unenc_response + offset, true));
				}
				offset += tlstr_len(unenc_response + offset, true);
			}
			if (flags & (1 << 4)) offset += 4;
			if ((flags & (1 << 3)) && (flags & (1 << 6))) themes.push_back(theme);
		}
		register_themes();
		break;
	}
	case 0x9f071957: // messages.availableReactionsNotModified
		break;
	case 0xf41eb622: // account.themesNotModified
		break;
	case 0x20212ca8: // photos.photo
		break;
	case 0x276d3ec6: // msg_detailed_info
	case 0x62d6b459: // msgs_ack
		acknowledgement = false;	
		break;
#ifdef _DEBUG
	default:
		wchar_t text[15];
		wsprintf(text, L"%x", constructor);
		MessageBox(NULL, text, L"Unknown", MB_OK | MB_ICONINFORMATION);
#endif
	}

	if (acknowledgement) {
		// send msgs_ack
		BYTE unenc_query[64];
		BYTE enc_query[88];
		write_le(unenc_query + 28, 20, 4);
		write_le(unenc_query + 32, 0x62d6b459, 4);
		write_le(unenc_query + 36, 0x1cb5c415, 4);
		write_le(unenc_query + 40, 1, 4);
		internal_header(dcInfo, unenc_query, false);
		memcpy(unenc_query + 44, dcInfo->last_msg_id, 8);
		fortuna_read(unenc_query + 52, 12, &prng);
		convert_message(dcInfo, unenc_query, enc_query, 64, 0);
		send_query(dcInfo, enc_query, 88);
		if (closing) DestroyWindow(hMain);
	}
}

class COleCallback : public IRichEditOleCallback {
public:
	ULONG refCount;
	HWND hWnd;
	COleCallback(HWND hWnd) {
		refCount = 1;
		this->hWnd = hWnd;
	}
    STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) {
        if (riid == IID_IUnknown || riid == IID_IRichEditOleCallback) {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }
    STDMETHODIMP_(ULONG) AddRef() { return ++refCount; }
    STDMETHODIMP_(ULONG) Release() {
        ULONG c = --refCount;
        if (!c) delete this;
        return c;
    }
    STDMETHODIMP GetNewStorage(LPSTORAGE* lpstg) {
        return StgCreateDocfile(NULL, STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE, 0, lpstg);
    }
    STDMETHODIMP GetInPlaceContext(LPOLEINPLACEFRAME*, LPOLEINPLACEUIWINDOW*, LPOLEINPLACEFRAMEINFO) { return S_FALSE; }
    STDMETHODIMP ShowContainerUI(BOOL) { return S_OK; }
    STDMETHODIMP QueryInsertObject(LPCLSID clsid, LPSTORAGE lpstg, LONG cr) {
		if (*clsid == CLSID_StaticMetafile || (hWnd != msgInput && *clsid == CLSID_StaticDib)) {
			BYTE* p = (BYTE*)clsid;
			p[40] = REO_BELOWBASELINE;
			return S_OK;
		} else return E_FAIL;
	}
    STDMETHODIMP DeleteObject(LPOLEOBJECT pOleObj) { return S_OK; }
    STDMETHODIMP QueryAcceptData(LPDATAOBJECT pDataObj, CLIPFORMAT*, DWORD, BOOL, HGLOBAL) { return S_OK; }
    STDMETHODIMP ContextSensitiveHelp(BOOL) { return E_NOTIMPL; }
    STDMETHODIMP GetClipboardData(CHARRANGE*, DWORD, LPDATAOBJECT*) { return E_NOTIMPL; }
    STDMETHODIMP GetDragDropEffect(BOOL, DWORD, DWORD*) { return S_OK; }
    STDMETHODIMP GetContextMenu(WORD, LPOLEOBJECT, CHARRANGE* cr, HMENU*) {
		if (hWnd == chat) {
			for (int i = messages.size() - 1; i >= 0; i--) {
				if (cr->cpMin >= messages[i].start_char && cr->cpMin <= messages[i].end_footer) {
					sel_msg_id = messages[i].id;
					if (cr->cpMin > messages[i].end_char) {
						POINT pt;
						GetCursorPos(&pt);
						int width = (current_peer->reaction_list->size() < 12) ? 15 + current_peer->reaction_list->size() * 20 : 255;
						int height = 15 + 20 * ((current_peer->reaction_list->size() + 12) / 12);
						SetWindowPos(reactionStatic, NULL, pt.x, pt.y, NULL, NULL, SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSIZE);
					} else { // not using the function's HMENU* because that way the caret is shown
						HideCaret(hWnd);
						POINT pt;
						GetCursorPos(&pt);
						HMENU hMenu = CreatePopupMenu();
						AppendMenu(hMenu, editing_msg_id ? (MF_STRING | MF_GRAYED) : MF_STRING, 23,  L"Reply");
						if (messages[i].outgoing || memcmp(current_peer->id, myself.id, 8) == 0) AppendMenu(hMenu, (replying_msg_id || forwarding_msg_id) ? (MF_STRING | MF_GRAYED) : MF_STRING, 20,  L"Edit");
						AppendMenu(hMenu, editing_msg_id ? (MF_STRING | MF_GRAYED) : MF_STRING, 24,  L"Forward");
						AppendMenu(hMenu, MF_STRING, 21,  L"Delete for everyone");
						AppendMenu(hMenu, MF_STRING, 22,  L"Delete for myself");
						TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_LEFTALIGN, pt.x, pt.y, 0, hMain, NULL);
					}
					break;
				}
			}
			HideCaret(chat);
		}
		return S_OK;
	}
};

unsigned __stdcall SocketWorker(void* param) {
	DCInfo* dcInfo = (DCInfo*)param;
	if (dcInfo != &dcInfoMain) {
		init_connection(dcInfo, false);
		if (!dcInfo->authorized) {
			SendMessage(hStatus, SB_SETTEXTA, 1 | SBT_OWNERDRAW, (LPARAM)L"Logging in to the needed file's datacenter...");
			create_auth_key(dcInfo);
			send_ping(dcInfo);
			BYTE unenc_query[64];
			BYTE enc_query[88];
			internal_header(unenc_query, true);
			memcpy(dcInfo->future_salt, unenc_query + 16, 8);
			write_le(unenc_query + 28, 8, 4);
			write_le(unenc_query + 32, 0xe5bfffcd, 4);
			write_le(unenc_query + 36, dcInfo->dc, 4);
			fortuna_read(unenc_query + 40, 24, &prng);
			convert_message(unenc_query, enc_query, 64, 0);
			send_query(enc_query, 88);
		} else {
			get_future_salt(dcInfo);
		}
		SetTimer(hMain, 20 + dcInfo->dc, 90000, NULL);
	}
	while (true) {
		int res_len = 0;
		int recv_res = recv(dcInfo->sock, (char*)&res_len, 1, 0);
		if (recv_res == SOCKET_ERROR) {
			int err = WSAGetLastError();
			if (err == WSAECONNABORTED) break;
			else Sleep(5000);
		} else if (!recv_res) {
			closesocket(dcInfo->sock);
			WSACleanup();
			if (dcInfo == &dcInfoMain) init_connection(dcInfo, true);
			else break;
		}
		if (res_len == 0) continue;
		if (res_len == 1) {
			recv(dcInfo->sock, (char*)&res_len, 4, 0);
			wchar_t error_message[11];
			wsprintf(error_message, L"Error %d", res_len);
			MessageBox(NULL, error_message, L"Error", MB_OK | MB_ICONERROR);
			continue;
		}
		if (res_len == 127) recv(dcInfo->sock, (char*)&res_len, 3, 0);
		res_len *= 4;
		BYTE* enc_response = (BYTE*)malloc(res_len);
		int received = 0;
		while (received != res_len) {
			if (received != res_len) Sleep(50);
			int received2 = recv(dcInfo->sock, (char*)(enc_response + received), res_len - received, 0);
			if (received2 != -1) received += received2;
		}
		BYTE* unenc_response = (BYTE*)malloc(res_len-24);
		if (!convert_message(dcInfo, unenc_response, enc_response, res_len-24, 8)) {
			free(unenc_response);
			DestroyWindow(hMain);
			continue;
		}
		free(enc_response);
		memcpy(unenc_response, &dcInfo, sizeof(INT_PTR));
		PostMessage(hMain, WM_APP + 1, (WPARAM)unenc_response, 0);
	}
	if (dcInfo != &dcInfoMain) {
		for (std::list<DCInfo>::iterator it = active_dcs.begin(); it != active_dcs.end(); it++) {
			DCInfo* active_dc = &(*it);
			if (dcInfo == active_dc) {
				active_dcs.erase(it);
				break;
			}
		}
	}
    return 0;
}

unsigned __stdcall FileSenderWorker(void* param) {
	KillTimer(hMain, 2);
	BYTE* enc_query2 = (BYTE*)param;
	Document* docstemp = (Document*)read_le(enc_query2, 4);
	std::vector<wchar_t*> files(::files);
	::files.clear();
	SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(10, 0));
	InvalidateRect(hToolbar, NULL, TRUE);
	BYTE* unenc_query = (BYTE*)malloc(524368);
	BYTE* enc_query = (BYTE*)malloc(524392);
	uploadThreadEvent ute;
	ute.event = CreateEvent(NULL, TRUE, TRUE, NULL);
	uploadEvents.push_back(&ute);
	for (int i = 0; i < files.size(); i++) {
		SetTimer(msgInput, docstemp[i].min, 0, NULL);
		if (docstemp[i].size > 10485760) write_le(unenc_query + 32, 0xde7b673d, 4);
		else write_le(unenc_query + 32, 0xb304a621, 4);
		memcpy(unenc_query + 36, docstemp[i].id, 8);
		int parts = ceil(docstemp[i].size / 524288.0);
		if (docstemp[i].size > 10485760) write_le(unenc_query + 48, parts, 4);
		FILE* f = _wfopen(files_i(files[i]), L"rb");
		wchar_t status_msg[256];
		for (int j = 0; j < parts; j++) {
			if (j > 0) WaitForSingleObject(ute.event, INFINITE);
			ResetEvent(ute.event);
			swprintf(status_msg, L"Uploading %s... %d%%", docstemp[i].filename, 100 * _ftelli64(f) / docstemp[i].size);
			SendMessage(hStatus, SB_SETTEXTA, 1 | SBT_OWNERDRAW, (LPARAM)status_msg);
			write_le(unenc_query + 44, j, 4);
			int part_size = (j == parts - 1) ? (docstemp[i].size - 524288 * j) : 524288;
			int offset = (docstemp[i].size > 10485760) ? 52 : 48;
			if (part_size >= 254) {
				unenc_query[offset] = 254;
				write_le(unenc_query + offset + 1, part_size, 3);
				offset += 4;
			} else {
				unenc_query[offset] = part_size;
				offset += 1;
			}
			fread(unenc_query + offset, 1, part_size, f);
			offset += part_size;
			if (offset % 4 != 0) {
				memset(unenc_query + offset, 0, 4 - offset % 4);
				offset += 4 - offset % 4;
			}
			write_le(unenc_query + 28, offset - 32, 4);
			int padding_len = get_padding(offset);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			offset += padding_len;
			internal_header(unenc_query, true);
			memcpy(ute.lastId, unenc_query + 16, 8);
			convert_message(unenc_query, enc_query, offset, 0);
			send_query(enc_query, offset + 24);
		}
		WaitForSingleObject(ute.event, INFINITE);
		swprintf(status_msg, L"Uploading %s... done!", docstemp[i].filename);
		SendMessage(hStatus, SB_SETTEXTA, 1 | SBT_OWNERDRAW, (LPARAM)status_msg);
		fclose(f);
		free(files[i]);
		KillTimer(msgInput, docstemp[i].min);
	}
	free(docstemp);
	free(unenc_query);
	free(enc_query);
	CloseHandle(ute.event);
	uploadEvents.erase(std::remove(uploadEvents.begin(), uploadEvents.end(), &ute), uploadEvents.end());
	send_query(enc_query2 + 8, read_le(enc_query2 + 4, 4));
	free(param);
	if (uploadEvents.size() == 0 && downloading_docs.size() == 0) SetTimer(hMain, 2, 3000, NULL);
	return 0;
}

const int BUFFER_COUNT = 4;
HWAVEIN hWaveIn = NULL;
WAVEHDR hdr_buf[BUFFER_COUNT];
std::vector<BYTE> recorded;

CTextHost* textHost = NULL;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CREATE: {
		LoadLibrary(L"RICHED20.DLL");
		CoInitialize(NULL);
		DragAcceptFiles(hWnd, TRUE);

		DWORD minor, major;
		get_dll_version(L"comctl32.dll", &minor, &major);
		if (major >= 5 || (major == 4 && minor >= 70)) {
			HINSTANCE hComctlLib = LoadLibraryA("comctl32.dll");
			if (hComctlLib) {
				typedef BOOL (WINAPI *MyInitCommonControlsEx)(LPINITCOMMONCONTROLSEX);
				MyInitCommonControlsEx myInitCommonControlsEx = (MyInitCommonControlsEx)GetProcAddress(hComctlLib, "InitCommonControlsEx");
				INITCOMMONCONTROLSEX icex;
				icex.dwSize = sizeof(icex);
				icex.dwICC = ICC_WIN95_CLASSES | ICC_DATE_CLASSES;
				myInitCommonControlsEx(&icex);
				FreeLibrary(hComctlLib);
			}
		} else {
			ie3 = false;
			InitCommonControls();
		}
		if (major <= 3 || (major == 4 && minor <= 70)) ie4 = false;

		hComboBoxFolders = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS, 10, 10, 200, 300, hWnd, (HMENU)2, NULL, NULL);
		hComboBoxChats = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_OWNERDRAWFIXED | CBS_HASSTRINGS, 220, 10, width / 2.5, 300, hWnd, (HMENU)3, NULL, NULL);
		msgInput = CreateWindow(L"RichEdit20W", NULL, WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_BORDER | ES_LEFT | ES_AUTOVSCROLL | ES_MULTILINE | WS_CLIPSIBLINGS,
			10, height - 120, width - 30, 65, hWnd, NULL, NULL, NULL);
		writing_str_from = msgInput;
		chat = CreateWindow(L"RichEdit20W", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY | WS_CLIPSIBLINGS,
			10, 40, width - 30, height - 190, hWnd, NULL, NULL, NULL);
		splitter = CreateWindow(L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_NOTIFY, 10, height - 149, width - 30, 3, hWnd, NULL, NULL, NULL);
		CHARFORMAT2 cf;
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_FACE | CFM_BOLD;
		cf.dwEffects = 0;
		wcscpy(cf.szFaceName, L"Arial");
		SendMessage(msgInput, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
		SendMessage(chat, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
		COleCallback* coc_chat = new COleCallback(chat);
		SendMessage(chat, EM_SETOLECALLBACK, 0, (LPARAM)coc_chat);
		coc_chat->Release();
		COleCallback* coc_msgInput = new COleCallback(msgInput);
		SendMessage(msgInput, EM_SETOLECALLBACK, 0, (LPARAM)coc_msgInput);
		coc_msgInput->Release();
		SendMessage(chat, EM_SETEVENTMASK, 0, ENM_LINK);
		CoCreateInstance(CLSID_CActiveIMM, NULL, CLSCTX_INPROC_SERVER, IID_IActiveIMMApp, (void**)&g_pAIMM);
		if (g_pAIMM) {
			g_pAIMM->Activate(TRUE);
			ATOM atom = (ATOM)GetClassLong(msgInput, GCW_ATOM);
			g_pAIMM->FilterClientWindows(&atom, 1);
		}

		textHost = new CTextHost();
		LRESULT res;
		textHost->textServices->TxSendMessage(EM_SETSEL, 0, 0, &res);

		hMenuBar = CreateMenu();
		HMENU hMenuProfile = CreatePopupMenu();
		HMENU hMenuMute = CreatePopupMenu();
		HMENU hMenuChat = CreatePopupMenu();
		HMENU hMenuTheme = CreatePopupMenu();
		HMENU hMenuTools = CreatePopupMenu();
		HMENU hMenuHelp = CreatePopupMenu();
		AppendMenu(hMenuProfile, MF_STRING, 30,  TEXT("&View profile"));
		AppendMenu(hMenuProfile, MF_POPUP, (UINT_PTR)hMenuMute, TEXT("&Mute/unmute..."));
		AppendMenu(hMenuMute, MF_STRING, 38,  TEXT("&Mute all users"));
		AppendMenu(hMenuMute, MF_STRING, 39,  TEXT("&Mute all groups"));
		AppendMenu(hMenuMute, MF_STRING, 40,  TEXT("&Mute all channels"));
		AppendMenu(hMenuProfile, MF_STRING, 37,  TEXT("&Exit"));
		AppendMenu(hMenuProfile, MF_STRING, 36,  TEXT("&Log out and exit"));
		AppendMenu(hMenuChat, MF_STRING, 31,  TEXT("&View profile"));
		AppendMenu(hMenuChat, MF_STRING, 41,  TEXT("&Mute this chat"));
		AppendMenu(hMenuTheme, MF_STRING | MF_CHECKED, 600,  TEXT("&None"));
		AppendMenu(hMenuChat, MF_POPUP, (UINT_PTR)hMenuTheme,  TEXT("&Set theme..."));
		AppendMenu(hMenuTools, MF_STRING, 32,  TEXT("&Send media as files"));
		if (!ie4) AppendMenu(hMenuTools, MF_STRING, 42,  TEXT("S&how the uploading list"));
		AppendMenu(hMenuTools, MF_STRING, 33,  TEXT("&Options..."));
		AppendMenu(hMenuHelp, MF_STRING, 34, TEXT("&User Guide"));
		AppendMenu(hMenuHelp, MF_STRING, 35,  TEXT("&About"));
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuProfile, TEXT("&Profile"));
		AppendMenu(hMenuBar, MF_POPUP | MF_GRAYED, (UINT_PTR)hMenuChat, TEXT("&Chat"));
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuTools, TEXT("&Tools"));
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuHelp, TEXT("&Help"));
		SetMenu(hWnd, hMenuBar);
		DrawMenuBar(hWnd);

		hToolbar = CreateWindow(TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | (ie3 ? TBSTYLE_FLAT : 0) | TBSTYLE_TOOLTIPS | CCS_NORESIZE | CCS_NOPARENTALIGN | WS_CLIPSIBLINGS,
			10, height - 145, width - 30, 25, hWnd, NULL, NULL, NULL);
		SendMessage(hToolbar, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
		SendMessage(hToolbar, TB_SETMAXTEXTROWS, 0, 0);
		SendMessage(hToolbar, TB_SETBITMAPSIZE, 0, MAKELONG(16, 16));
		COLORMAP cmap = { RGB(128, 0, 128), GetSysColor(COLOR_BTNFACE) };
		HBITMAP hBmpTb = CreateMappedBitmap(GetModuleHandle(NULL), IDB_ICONS, 0, &cmap, 1);
		TBADDBITMAP tbab;
		tbab.hInst = NULL;
		tbab.nID = (UINT_PTR)hBmpTb;
		SendMessage(hToolbar, TB_ADDBITMAP, 14, (LPARAM)&tbab);
		TBBUTTON tbb[15] = {0};
		tbb[0].iBitmap = 0;
		tbb[0].idCommand = 10;
		tbb[0].fsState = TBSTATE_ENABLED;
		tbb[0].iString = (INT_PTR)"Bold";
		tbb[1].iBitmap = 1;
		tbb[1].idCommand = 11;
		tbb[1].fsState = TBSTATE_ENABLED;
		tbb[1].iString = (INT_PTR)"Italic";
		tbb[2].iBitmap = 2;
		tbb[2].idCommand = 12;
		tbb[2].fsState = TBSTATE_ENABLED;
		tbb[2].iString = (INT_PTR)"Underline";
		tbb[3].iBitmap = 3;
		tbb[3].idCommand = 13;
		tbb[3].fsState = TBSTATE_ENABLED;
		tbb[3].iString = (INT_PTR)"Strikethrough";
		tbb[4].iBitmap = 4;
		tbb[4].idCommand = 14;
		tbb[4].fsState = TBSTATE_ENABLED;
		tbb[4].iString = (INT_PTR)"Quote";
		tbb[5].iBitmap = 5;
		tbb[5].idCommand = 15;
		tbb[5].fsState = TBSTATE_ENABLED;
		tbb[5].iString = (INT_PTR)"Monospace";
		tbb[6].iBitmap = 6;
		tbb[6].idCommand = 16;
		tbb[6].fsState = TBSTATE_ENABLED;
		tbb[6].iString = (INT_PTR)"Spoiler";
		tbb[7].iBitmap = width - 296;
		tbb[7].idCommand = 9;
		tbb[7].fsStyle = TBSTYLE_SEP;
		tbb[8].iBitmap = 7;
		tbb[8].idCommand = 7;
		tbb[8].fsState = TBSTATE_HIDDEN;
		tbb[8].iString = (INT_PTR)"You're replying to a message. Click to cancel.";
		tbb[9].iBitmap = 8;
		tbb[9].idCommand = 8;
		tbb[9].fsState = TBSTATE_HIDDEN;
		tbb[9].iString = (INT_PTR)"You're editing a message. Click to cancel.";
		tbb[10].iBitmap = 9;
		tbb[10].idCommand = 19;
		tbb[10].fsState = TBSTATE_HIDDEN;
		tbb[10].iString = (INT_PTR)"You're forwarding a message. Click to cancel.";
		tbb[11].iBitmap = 10;
		tbb[11].idCommand = 4;
		tbb[11].fsState = TBSTATE_ENABLED;
		if (ie4) tbb[11].fsStyle = TBSTYLE_DROPDOWN;
		tbb[11].iString = (INT_PTR)"Attach files";
		tbb[12].iBitmap = 11;
		tbb[12].idCommand = 6;
		tbb[12].fsState = TBSTATE_ENABLED;
		tbb[12].fsStyle = TBSTYLE_CHECK;
		tbb[12].iString = (INT_PTR)"Record a voice message";
		tbb[13].iBitmap = 12;
		tbb[13].idCommand = 5;
		tbb[13].fsState = EMOJIS ? TBSTATE_ENABLED : 0;
		tbb[13].fsStyle = TBSTYLE_CHECK;
		tbb[13].iString = (INT_PTR)"Emojis";
		tbb[14].iBitmap = 13;
		tbb[14].idCommand = 1;
		tbb[14].fsState = TBSTATE_ENABLED;
		tbb[14].iString = (INT_PTR)"Send message";
		SendMessage(hToolbar, TB_ADDBUTTONSA, 15, (LPARAM)&tbb);
		SendMessage(hToolbar, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_DRAWDDARROWS);
		tbSeparatorHider = CreateWindowEx(0, L"Static", NULL, WS_CHILD | (ie3 ? WS_VISIBLE : 0) | WS_CLIPSIBLINGS, width / 2, height - 143, 25, 21, hWnd, NULL, NULL, NULL);
		
		hTabs = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | TCS_FIXEDWIDTH | (nt3 ? TCS_OWNERDRAWFIXED : 0), width - 234, height - 359, 214, 214, hWnd, (HMENU)4, NULL, NULL);
		hOverlayTabs = CreateWindow(L"STATIC", NULL, WS_CHILD | WS_CLIPSIBLINGS | SS_NOTIFY, width - 234, height - 359, 214, 214, hWnd, NULL, NULL, NULL);
		emojiStatic = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 5, 27, 204, 182, hTabs, NULL, NULL, NULL);
		reactionStatic = CreateWindowEx(WS_EX_DLGMODALFRAME | WS_EX_TOOLWINDOW, L"STATIC", NULL, WS_POPUP | SS_NOTIFY, 0, 0, 0, 0, hWnd, NULL, NULL, NULL);
		emojiScroll = CreateWindow(L"SCROLLBAR", NULL, WS_CHILD | WS_VISIBLE | SBS_VERT, 193, 27, 16, 182, hTabs, NULL, NULL, NULL);
		SetWindowPos(chat, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(tbSeparatorHider, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		SetWindowPos(emojiScroll, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		HIMAGELIST hImgListTabs = ImageList_Create(15, 15, ILC_COLOR24 | ILC_MASK, 10, 0);
		wchar_t* icons[] = {L"1f553", L"1f600", L"1f44b", L"1f436", L"1f347", L"1f30d", L"1f383", L"1f576", L"1f3e7", L"1f3c1"};
		get_path(exe_path, L"emojis");
		wchar_t file_name[MAX_PATH];
		for (int i = 0; i < 10; i++) {
			swprintf(file_name, L"%s\\%s.ico", exe_path, icons[i]);
			HICON hIcon = (HICON)LoadImage(NULL, file_name, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
			ImageList_AddIcon(hImgListTabs, hIcon);
			DestroyIcon(hIcon);
		}
		SendMessage(hTabs, TCM_SETIMAGELIST, 0, (LPARAM)hImgListTabs);
		SendMessage(hTabs, TCM_SETITEMSIZE, 0, MAKELPARAM(21, 20));
		SendMessage(hTabs, TCM_SETPADDING, 0, MAKELPARAM(2, 3));

		TCITEMA tie;
		tie.mask = TCIF_IMAGE;
		tie.iImage = 0;
		SendMessage(hTabs, TCM_INSERTITEMA, 0, (LPARAM)&tie);
		tie.iImage = 1;
		SendMessage(hTabs, TCM_INSERTITEMA, 1, (LPARAM)&tie);
		tie.iImage = 2;
		SendMessage(hTabs, TCM_INSERTITEMA, 2, (LPARAM)&tie);
		tie.iImage = 3;
		SendMessage(hTabs, TCM_INSERTITEMA, 3, (LPARAM)&tie);
		tie.iImage = 4;
		SendMessage(hTabs, TCM_INSERTITEMA, 4, (LPARAM)&tie);
		tie.iImage = 5;
		SendMessage(hTabs, TCM_INSERTITEMA, 5, (LPARAM)&tie);
		tie.iImage = 6;
		SendMessage(hTabs, TCM_INSERTITEMA, 6, (LPARAM)&tie);
		tie.iImage = 7;
		SendMessage(hTabs, TCM_INSERTITEMA, 7, (LPARAM)&tie);
		tie.iImage = 8;
		SendMessage(hTabs, TCM_INSERTITEMA, 8, (LPARAM)&tie);
		tie.iImage = 9;
		SendMessage(hTabs, TCM_INSERTITEMA, 9, (LPARAM)&tie);

		FILE* f = _wfopen(get_path(appdata_path, L"fav_emojis.dat"), L"rb");
		if (f) {
			int count;
			fread(&count, 4, 1, f);
			for (int i = 0; i < count; i++) {
				int count2;
				fread(&count2, 4, 1, f);
				wchar_t* code = (wchar_t*)malloc(count2 * 2);
				fread(code, 2, count2, f);
				fav_emojis.push_back(code);
			}
		}
		if (fav_emojis.size() == 0) TabCtrl_SetCurSel(hTabs, 1);
		NMHDR hdr;
		hdr.hwndFrom = hTabs;
		hdr.code = TCN_SELCHANGE;
		SendMessage(hWnd, WM_NOTIFY, NULL, (LPARAM)&hdr);

		oldMsgInputProc = (WNDPROC)SetWindowLong(msgInput, GWL_WNDPROC, (LONG)WndProcMsgInput);
		oldChatProc = (WNDPROC)SetWindowLong(chat, GWL_WNDPROC, (LONG)WndProcChat);
		oldEmojiScrollProc = (WNDPROC)SetWindowLong(hTabs, GWL_WNDPROC, (LONG)WndProcEmojiScroll);
		oldEmojiScrollFallbackProc = (WNDPROC)SetWindowLong(emojiScroll, GWL_WNDPROC, (LONG)WndProcEmojiScrollFallback);
		oldEmojiStaticProc = (WNDPROC)SetWindowLong(emojiStatic, GWL_WNDPROC, (LONG)WndProcEmojiStatic);
		oldReactionStaticProc = (WNDPROC)SetWindowLong(reactionStatic, GWL_WNDPROC, (LONG)WndProcReactionStatic);
		oldSplitterProc = (WNDPROC)SetWindowLong(splitter, GWL_WNDPROC, (LONG)WndProcSplitter);
		
		hStatus = CreateWindow(STATUSCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 0, 0, hWnd, NULL, NULL, NULL);
		int status_parts[2] = {width / 2, -1};
		SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)status_parts);
		if (nt3) SendMessage(hStatus, SB_SETMINHEIGHT, 18, 0);
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 1: {
			if (!current_peer) break;
			bool file_not_found = false;
			int files_count = current_peer->perm.cansendmed ? files.size() : 0;
			for (int i = 0; i < files_count; i++) {
				FILE* f = _wfopen(files_i(files[i]), L"rb");
				if (!f) {
					if (!file_not_found)
						MessageBox(NULL, L"The message will not be sent because at least one of the files selected no longer exists. Removing them from the list...", L"Error", MB_OK | MB_ICONERROR);
					files.erase(files.begin() + i);
					if (!files.size()) {
						SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(10, 0));
						InvalidateRect(hToolbar, NULL, TRUE);
					}
					i--;
					files_count--;
					file_not_found = true;
				} else fclose(f);
			}
			if (file_not_found) break;

			GETTEXTLENGTHEX gtl;
			gtl.flags = GTL_DEFAULT;
			gtl.codepage = 1200;
			int length = SendMessage(msgInput, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
			if (length == 0 && files_count == 0 && !forwarding_msg_id) break;
			SetTimer(msgInput, 1, 0, NULL);

			int msgcontent_len = 0;
			int peer_len = place_peer(NULL, current_peer, true);
			Document* docstemp = (files_count > 0) ? (Document*)malloc(files_count * sizeof(Document)) : NULL;
			bool only_video = (files_count > 0) ? true : false;
			bool banned_type = false;
			for (i = 0; i < files_count; i++) {
				if (files_count > 1) msgcontent_len += 24 + peer_len; // inputSingleMedia
				msgcontent_len += 36; // inputmedia base
				HKEY hKey;
				unsigned long mime_type_size = 256;
				wchar_t mime_type[256];
				if (RegOpenKeyEx(HKEY_CLASSES_ROOT, wcsrchr(files[i], L'.'), 0, KEY_READ, &hKey) != ERROR_SUCCESS
					|| RegQueryValueEx(hKey, L"Content Type", NULL, NULL, (BYTE*)mime_type, &mime_type_size) != ERROR_SUCCESS)
					wcscpy(mime_type, L"application/octet-stream");
				if (hKey) RegCloseKey(hKey);
				bool photo = (wcsncmp(mime_type, L"image", 5) == 0 && !SENDMEDIAASFILES) ? true : false;
				bool video = (wcsncmp(mime_type, L"video", 5) == 0 && !SENDMEDIAASFILES) ? true : false;
				bool audio = (wcsncmp(mime_type, L"audio", 5) == 0 && !SENDMEDIAASFILES) ? true : false;
				bool voice = (wcscmp(files[i], L"voice.wav") == 0 && !SENDMEDIAASFILES) ? true : false;

				if (current_peer->type != 0) {
					if (!current_peer->perm.cansendphoto && photo) {
						MessageBox(NULL, L"Photos aren't allowed to be sent in this group.", L"Error", MB_OK | MB_ICONERROR);
						banned_type = true;
					} else if (!current_peer->perm.cansendvideo && video) {
						MessageBox(NULL, L"Videos aren't allowed to be sent in this group.", L"Error", MB_OK | MB_ICONERROR);
						banned_type = true;
					} else if (!current_peer->perm.cansenddocs && !photo && !video && !audio && !voice) {
						MessageBox(NULL, L"Documents aren't allowed to be sent in this group.", L"Error", MB_OK | MB_ICONERROR);
						banned_type = true;
					} else if (!current_peer->perm.cansendvoice && voice) {
						MessageBox(NULL, L"Voice messages aren't allowed to be sent in this group.", L"Error", MB_OK | MB_ICONERROR);
						banned_type = true;	
					} else if (!current_peer->perm.cansendaudio && audio) {
						MessageBox(NULL, L"Audios aren't allowed to be sent in this group.", L"Error", MB_OK | MB_ICONERROR);
						banned_type = true;	
					}
					if (banned_type) {
						free(docstemp);
						break;
					}
				}
				
				if (!photo) msgcontent_len += str_to_tlstr_len(mime_type);
				FILE* f = _wfopen(files_i(files[i]), L"rb");
				_fseeki64(f, 0, SEEK_END);
				docstemp[i].size = _ftelli64(f);
				fclose(f);
				fortuna_read(docstemp[i].id, 8, &prng);
				wchar_t* str_ptr = wcsrchr(files[i], L'\\');
				docstemp[i].filename = (str_ptr == NULL) ? files[i] : str_ptr + 1;

				if (docstemp[i].size <= 10485760) msgcontent_len += 36;
				msgcontent_len += str_to_tlstr_len(docstemp[i].filename);
				if (!photo) msgcontent_len += str_to_tlstr_len(docstemp[i].filename);

				if (audio) msgcontent_len += 12;
				if (photo) {
					msgcontent_len -= 12;
					docstemp[i].min = 6;
				}
				else if (video) {
					msgcontent_len += 24;
					docstemp[i].min = 5;
				} else {
					only_video = false;
					if (voice) docstemp[i].min = 3;
					else docstemp[i].min = 4;
				} 
			}

			wchar_t* msg = (wchar_t*)malloc((length+1)*2);
			GETTEXTEX gte = {0};
			gte.cb = (length + 1) * 2;
			gte.flags = GT_DEFAULT;
			gte.codepage = 1200;
			SendMessage(msgInput, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)(msg));
			SendMessage(msgInput, EM_HIDESELECTION, TRUE, 0);

			if (banned_type) break;
			if (only_video && length > 0) length = 0;
			else only_video = false;
			
			CHARFORMAT2 cf;
			cf.cbSize = sizeof(cf);
			bool formats_active[7] = {false};
			int format_values[4] = {CFM_BOLD, CFM_ITALIC, CFM_UNDERLINE, CFM_STRIKEOUT};
			std::vector<int> format_vecs[9];
			int format_len = 0, format_count = 0;
			for (i = 0; i < length; i++) {
				SendMessage(msgInput, EM_SETSEL, i, i+1);
				SendMessage(msgInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
				for (int j = 0; j < 7; j++) {
					if ((j < 4 && (cf.dwMask & format_values[j]) && (cf.dwEffects & format_values[j]))
						|| (j == 4 && cf.crTextColor == RGB(169, 169, 169))
						|| (j == 5 && wcscmp(cf.szFaceName, L"Courier New") == 0)
						|| (j == 6 && !(cf.dwEffects & CFE_AUTOBACKCOLOR) && cf.crBackColor == RGB(0, 0, 0))) {
						if (!formats_active[j]) {
							bool closelast = (i == length - 1) ? true : false;
							format_len += 12;
							if (j == 4) format_len += 4;
							format_count++;
							format_vecs[j].push_back(i);
							formats_active[j] = true;
							if (closelast) j--;
						} else if (i == length - 1) format_vecs[j].push_back(i+1);
					} else if (formats_active[j]) {
						format_vecs[j].push_back(i);
						formats_active[j] = false;
					}
				}
				if (msg[i] == 11 || msg[i] == 13) msg[i] = 10;
			}
			if (format_count > 0) format_len += 8;

			
			int tl_len = str_to_tlstr_len(msg);
			
			msgcontent_len += ((forwarding_msg_id || files_count > 1) && length == 0) ? 0 : 16 + peer_len + tl_len + format_len; // msg1 without media
			bool container = false;
			if (files_count > 1 || (forwarding_msg_id && length > 0)) {
				container = true;
				msgcontent_len += 8;
				if (length > 0) msgcontent_len += 16; // message container header
				if (forwarding_msg_id) msgcontent_len += 16; // message container header
			}
			if (forwarding_msg_id) {
				for (int k = 0; k < peers_count; k++) if (memcmp(forwarding_peer_id, peers[k].id, 8) == 0) break;
				msgcontent_len += 36 + place_peer(NULL, &peers[k], true) + peer_len;
			}
			if (editing_msg_id) msgcontent_len -= 4;
			if (replying_msg_id) msgcontent_len += 12;

			int padding_len = get_padding(32 + msgcontent_len);
			int unenc_query_len = 32 + msgcontent_len + padding_len;

			bool message_added = false;
			if (editing_msg_id == 0 && files_count != 1 && length > 0 && current_peer->online != -1 && (current_peer->type == 0 || (current_peer->type == 1 && current_peer->chat_users->size() > 1))) {
				StreamData sd = {0};
				EDITSTREAM es = {0};
				es.dwCookie = (DWORD_PTR)&sd;
				es.pfnCallback = StreamOutCallback;
				SendMessage(msgInput, EM_SETSEL, 0, length);
				SendMessage(msgInput, EM_STREAMOUT, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es);
				es.pfnCallback = StreamInCallback;
				message_adder(false, false, 2, NULL, NULL, NULL, &es, NULL, NULL, NULL, NULL, NULL, NULL, false, true, false, current_time());
				free(sd.buf);
				update_chats_order(current_peer->id, NULL, current_peer->type);
				message_added = true;
			}

			BYTE* unenc_query = (BYTE*)malloc(unenc_query_len);
			BYTE* enc_query = (BYTE*)malloc(32+unenc_query_len);

			if (!container) internal_header(unenc_query, true);
			write_le(unenc_query + 28, msgcontent_len, 4);
			int offset = 32;
			if (container) {
				write_le(unenc_query + 32, 0x73f1f8dc, 4);
				write_le(unenc_query + 36, files_count + ((length > 0) ? 1 : 0) + (forwarding_msg_id ? 1 : 0), 4);
				offset += 8;
			}
			if (files_count == 1 || length > 0) {
				if (container) {
					create_msg_id(unenc_query + 40);
					create_seq_no(unenc_query + 48, true);
					offset += 16;
				}
				int start = offset;
				if (!editing_msg_id) {
					if (files_count != 1) {
						if (message_added) messages[messages.size()-1].id = read_le(unenc_query + offset - 16, 4);
						write_le(unenc_query + offset, 0x983f9745, 4); // sendMessage
					} else write_le(unenc_query + offset, 0x7852834e, 4); // sendMedia
				} else write_le(unenc_query + offset, 0xdfd14005, 4); // editMessage
				int flags = 0;
				if (format_len) flags += 8;
				if (editing_msg_id) flags += 2048;
				if (editing_msg_id && files_count == 1) flags += 16384;
				if (replying_msg_id) flags += 1;
				write_le(unenc_query + offset + 4, flags, 4);
				offset += place_peer(unenc_query + offset + 8, current_peer, true) + 8;
				if (replying_msg_id) {
					write_le(unenc_query + offset, 0x22c0f6d5, 4);
					write_le(unenc_query + offset + 4, 0, 4);
					write_le(unenc_query + offset + 8, replying_msg_id, 4);
					offset += 12;
				}
				if (editing_msg_id) {
					write_le(unenc_query + offset, editing_msg_id, 4);
					offset += 4;
				}
				if (files_count == 1 && !editing_msg_id) offset += place_inputmedia(unenc_query + offset, docstemp, 0);
				write_string(unenc_query + offset, msg);
				offset += tl_len;
				if (files_count == 1 && editing_msg_id) offset += place_inputmedia(unenc_query + offset, docstemp, 0);
				if (!editing_msg_id) {
					fortuna_read(unenc_query + offset, 8, &prng);
					offset += 8;
				}
				if (format_len > 0) offset += insert_format(unenc_query + offset, format_count, &format_vecs[0]);
				if (container) write_le(unenc_query + start - 4, offset - start, 4);
			}

			if (forwarding_msg_id) {
				if (container) {
					create_msg_id(unenc_query + offset);
					create_seq_no(unenc_query + offset + 8, true);
					offset += 16;
				}
				int start = offset;
				write_le(unenc_query + offset, 0xd5039208, 4);
				write_le(unenc_query + offset + 4, 0, 4);
				for (int k = 0; k < peers_count; k++) if (memcmp(forwarding_peer_id, peers[k].id, 8) == 0) break;
				offset += 8 + place_peer(unenc_query + offset + 8, &peers[k], true);
				write_le(unenc_query + offset, 0x1cb5c415, 4);
				write_le(unenc_query + offset + 4, 1, 4);
				write_le(unenc_query + offset + 8, forwarding_msg_id, 4);
				write_le(unenc_query + offset + 12, 0x1cb5c415, 4);
				write_le(unenc_query + offset + 16, 1, 4);
				fortuna_read(unenc_query + offset + 20, 8, &prng);
				offset += 28 + place_peer(unenc_query + offset + 28, current_peer, true);
				if (container) write_le(unenc_query + start - 4, offset - start, 4);
			}

			if (files_count > 1) {
				for (i = 0; i < files_count; i++) {
					create_msg_id(unenc_query + offset);
					memcpy(docstemp[i].access_hash, unenc_query + offset, 8); // this is, in fact, not the access hash
					create_seq_no(unenc_query + offset + 8, true);
					offset += 16;
					int start = offset;
					write_le(unenc_query + offset, 0x14967978, 4);
					memset(unenc_query + offset + 4, 0, 4);
					offset += place_peer(unenc_query + offset + 8, current_peer, true) + 8;
					offset += place_inputmedia(unenc_query + offset, docstemp, i);
					write_le(unenc_query + start - 4, offset - start, 4);
				}

				int smmcontent_len = 16 + peer_len + 72 * files_count;
				if (only_video) smmcontent_len += tl_len + format_len - 4;
				int padding_len = get_padding(32 + smmcontent_len);
				sendMultiMedia = (BYTE*)malloc(32 + smmcontent_len + padding_len);
				write_le(sendMultiMedia + 28, smmcontent_len, 4);
				write_le(sendMultiMedia + 32, 0x37b74355, 4);
				memset(sendMultiMedia + 36, 0, 4);
				place_peer(sendMultiMedia + 40, current_peer, true);
				write_le(sendMultiMedia + 40 + peer_len, 0x1cb5c415, 4);
				write_le(sendMultiMedia + 44 + peer_len, files_count, 4);
				int offset = 48 + peer_len;
				for (i = 0; i < files_count; i++) {
					write_le(sendMultiMedia + offset, 0x1cc6e91f, 4);
					if (i == 0 && format_len > 0 && only_video) write_le(sendMultiMedia + offset + 4, 1, 4);
					else memset(sendMultiMedia + offset + 4, 0, 4);
					memcpy(sendMultiMedia + offset + 8, docstemp[i].access_hash, 8);
					fortuna_read(sendMultiMedia + offset + 60, 8, &prng);
					offset += 68;
					if (i == 0 && only_video) {
						write_string(sendMultiMedia + offset, msg);
						offset += tl_len;
						if (format_len > 0) offset += insert_format(sendMultiMedia + offset, format_count, &format_vecs[0]);
					} else {
						memset(sendMultiMedia + offset, 0, 4);
						offset += 4;
					}
				}
				fortuna_read(sendMultiMedia + 32 + smmcontent_len, padding_len, &prng);
			}

			free(msg);
			SetWindowText(msgInput, L"");
			SendMessage(msgInput, EM_HIDESELECTION, FALSE, 0);
			if (container) internal_header(unenc_query, false);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			convert_message(unenc_query, enc_query + 8, unenc_query_len, 0);
			free(unenc_query);
			if (files_count == 0) {
				send_query(enc_query + 8, unenc_query_len+24);
				free(enc_query);
			}
			else {
				write_le(enc_query, (int)docstemp, 4);
				write_le(enc_query + 4, unenc_query_len+24, 4);
				unsigned threadID;
				_beginthreadex(NULL, 0, FileSenderWorker, &enc_query[0], 0, &threadID);
			}
			if (replying_msg_id) SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(7, 0), 0);
			if (editing_msg_id) SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(8, 0), 0);
			if (forwarding_msg_id) SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(19, 0), 0);
			break;
		}
		case 2: {
			if (HIWORD(wParam) != CBN_SELCHANGE) break;
			SetFocus(hWnd);
			int selIndex = SendMessage(hComboBoxFolders, CB_GETCURSEL, 0, 0);
			if (selIndex == -1) break;
			if (theme_brush != NULL) apply_theme(0);
			KillTimer(hWnd, 3);
			SendMessage(chat, WM_SETTEXT, 0, (LPARAM)L"");
			SendMessage(hStatus, SB_SETTEXTA, 0, (LPARAM)"");
			current_peer = NULL;
			EnableMenuItem(hMenuBar, 1, MF_BYPOSITION | MF_GRAYED);
			DrawMenuBar(hMain);
			SendMessage(hComboBoxChats, CB_RESETCONTENT, 0, 0);
			current_folder = (ChatsFolder*)SendMessage(hComboBoxFolders, CB_GETITEMDATA, selIndex, 0);
			for (int i = 0; i < current_folder->count; i++) {
				SendMessage(hComboBoxChats, CB_ADDSTRING, 0, (LPARAM)peers[current_folder->peers[i]].name);
				SendMessage(hComboBoxChats, CB_SETITEMDATA, i, (LPARAM)&peers[current_folder->peers[i]]);
			}
			break;
		}
		case 3: {
			if (HIWORD(wParam) != CBN_SELCHANGE) break;
			int selIndex = SendMessage(hComboBoxChats, CB_GETCURSEL, 0, 0);
			SetFocus(msgInput);
			if (selIndex == - 1 || (messages.size() > 0 && current_peer == &peers[current_folder->peers[selIndex]])) break;
			KillTimer(hWnd, 3);
			SendMessage(chat, WM_SETTEXT, 0, (LPARAM)L"");
			SendMessage(hStatus, SB_SETTEXTA, 0, (LPARAM)"");
			old_peer = current_peer;
			current_peer = (Peer*)SendMessage(hComboBoxChats, CB_GETITEMDATA, selIndex, 0);
			if (old_peer == current_peer) old_peer = NULL;

			if (current_peer->full) {
				no_more_msgs = false;
				if (current_peer->perm.cansendmsg) {
					ShowWindow(msgInput, SW_SHOW);
					ShowWindow(hToolbar, SW_SHOW);
					if (ie3) ShowWindow(tbSeparatorHider, SW_SHOW);
					ShowWindow(splitter, SW_SHOW);
					SetWindowPos(chat, NULL, NULL, NULL, width - 20, height - 165 + edits_border_offset, SWP_NOMOVE | SWP_NOZORDER);
					if (current_peer->perm.cansendmed) SendMessage(hToolbar, TB_ENABLEBUTTON, 4, MAKELPARAM(TRUE, 0));
					else SendMessage(hToolbar, TB_ENABLEBUTTON, 4, FALSE);
					if (current_peer->perm.cansendvoice) SendMessage(hToolbar, TB_ENABLEBUTTON, 6, MAKELPARAM(TRUE, 0));
					else SendMessage(hToolbar, TB_ENABLEBUTTON, 6, FALSE);
				} else {
					ShowWindow(msgInput, SW_HIDE);
					ShowWindow(hToolbar, SW_HIDE);
					if (ie3) ShowWindow(tbSeparatorHider, SW_HIDE);
					ShowWindow(hTabs, SW_HIDE);
					ShowWindow(hOverlayTabs, SW_HIDE);
					ShowWindow(splitter, SW_HIDE);
					SetWindowPos(chat, NULL, NULL, NULL, width - 20, height - 70, SWP_NOMOVE | SWP_NOZORDER);
					SetFocus(hWnd);
				}

				if (!old_peer || old_peer->reaction_list != current_peer->reaction_list) {
					HWND child = GetWindow(reactionStatic, GW_CHILD);
					while (child != NULL) {
						if (!nt3) {
							HICON hOldIcon = (HICON)SendMessage(child, BM_GETIMAGE, IMAGE_ICON, NULL);
							if (hOldIcon) DestroyIcon(hOldIcon);
						}
						DestroyWindow(child);
						child = GetWindow(reactionStatic, GW_CHILD);
					}

					InvalidateRect(reactionStatic, NULL, TRUE);
					UpdateWindow(reactionStatic);
					int emoji_x = -15, emoji_y = 5;
					if (current_peer->reaction_list != NULL) for (int i = 0; i < current_peer->reaction_list->size(); i++) {
						bool custom_emoji = current_peer->reaction_list->at(i)[0] == 1;
						wchar_t path[MAX_PATH];
						if (custom_emoji) {
							swprintf(path, L"%s\\%s.ico", get_path(appdata_path, L"custom_emojis"), current_peer->reaction_list->at(i) + 1);
						} else swprintf(path, L"%s\\%s.ico", get_path(exe_path, L"emojis"), current_peer->reaction_list->at(i));
						HICON hIcon = (HICON)LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
						if (nt3) hIcon = (HICON)(GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
						if (!hIcon) {
							if (custom_emoji) {
								__int64 custom_emoji_id;
								swscanf(current_peer->reaction_list->at(i) + 1, L"%I64X", &custom_emoji_id);
								unknown_custom_emoji_solver(NULL, NULL, NULL, custom_emoji_id, true);
							} else {
								try_to_add_fe0f(path);
								hIcon = (HICON)LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
								if (nt3) hIcon = (HICON)(GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
							}
						}
						if (hIcon || custom_emoji) {
							if (emoji_x == 225) {
								emoji_x = 5;
								emoji_y += 20;
							} else emoji_x += 20;
							HWND hBtn = CreateWindow(L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? BS_OWNERDRAW : BS_ICON), emoji_x, emoji_y, 20, 20, reactionStatic, (HMENU)1, GetModuleHandle(NULL), NULL);
							if (!nt3 && hIcon) SendMessage(hBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
							SetWindowLongPtr(hBtn, GWLP_USERDATA, (LONG_PTR)current_peer->reaction_list->at(i));
							WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hBtn, GWLP_WNDPROC, (LONG_PTR)WndProcReactionButton);
							SetProp(hBtn, L"oldproc", (HANDLE)oldProc);
						}
					}
					get_unknown_custom_emojis();
					int width = (emoji_y == 5) ? (emoji_x + 30) : 255;
					int height = emoji_y + 30;
					SetWindowPos(reactionStatic, NULL, NULL, NULL, width, height, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
				}

				if (read_le(current_peer->theme_id, 8) != 0) {
					for (int i = 0; i < themes.size(); i++) {
						if (memcmp(current_peer->theme_id, themes[i].id, 8) == 0) {
							apply_theme(i + 1);
							break;
						}
					}
				} else if (theme_brush != NULL) apply_theme(0);
				EnableMenuItem(hMenuBar, 1, MF_BYPOSITION | MF_ENABLED);
				DrawMenuBar(hMain);
				HMENU hMenuChat = GetSubMenu(hMenuBar, 1);
				if (memcmp(current_peer->id, myself.id, 8) == 0) {
					EnableMenuItem(hMenuChat, 0, MF_BYPOSITION | MF_GRAYED);
					EnableMenuItem(hMenuChat, 1, MF_BYPOSITION | MF_GRAYED);
				}
				else {
					EnableMenuItem(hMenuChat, 0, MF_BYPOSITION | MF_ENABLED);
					EnableMenuItem(hMenuChat, 1, MF_BYPOSITION | MF_ENABLED);
				}
				EnableMenuItem(hMenuChat, 2, MF_BYPOSITION | (current_peer->type == 0 ? MF_ENABLED : MF_GRAYED));
				ModifyMenu(hMenuChat, 1, MF_BYPOSITION | MF_STRING, 41, current_peer->mute_until ? L"Unmute this chat" : L"Mute this chat");
				read_react_ment(true);
				read_react_ment(false);

				// messages.getHistory
				get_history();
			} else get_full_peer(current_peer);
			break;
		}
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16: {
			CHARFORMAT2 cf;
			cf.cbSize = sizeof(cf);
			SendMessage(msgInput, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			switch (LOWORD(wParam)) {
			case 10:
				cf.dwMask = CFM_BOLD;
				cf.dwEffects ^= CFE_BOLD;
				break;
			case 11:
				cf.dwMask = CFM_ITALIC;
				cf.dwEffects ^= CFE_ITALIC;
				break;
			case 12:
				cf.dwMask = CFM_UNDERLINE;
				cf.dwEffects ^= CFE_UNDERLINE;
				break;
			case 13:
				cf.dwMask = CFM_STRIKEOUT;
				cf.dwEffects ^= CFE_STRIKEOUT;
				break;
			case 14:
				cf.dwMask = CFM_COLOR | CFM_PROTECTED;
				if (cf.crTextColor == RGB(169, 169, 169)) cf.crTextColor = 0;
				else cf.crTextColor = RGB(169, 169, 169);
				cf.dwEffects = CFE_PROTECTED;
				break;
			case 15:
				cf.dwMask = CFM_FACE;
				if (wcscmp(cf.szFaceName, L"Courier New") == 0) wcscpy(cf.szFaceName, L"Arial");
				else wcscpy(cf.szFaceName, L"Courier New");
				cf.dwEffects = 0;
				break;
			case 16:
				cf.dwMask = CFM_BACKCOLOR;
				cf.dwEffects ^= CFE_AUTOBACKCOLOR;
				break;
			}
			SendMessage(msgInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			break;
		} 
		case 5: {
			if (SendMessage(hToolbar, TB_GETSTATE, 5, 0) & TBSTATE_CHECKED) {
				ShowWindow(hTabs, SW_SHOW);
				ShowWindow(hOverlayTabs, SW_SHOW);
			} else {
				ShowWindow(hTabs, SW_HIDE);
				ShowWindow(hOverlayTabs, SW_HIDE);
			}
			break;
		} 
		case 6: {
			WAVEFORMATEX wf;
			wf.wFormatTag = WAVE_FORMAT_PCM;
			wf.nChannels = CHANNELS;
			wf.nSamplesPerSec = SAMPLERATE;
			wf.wBitsPerSample = BITSPERSAMPLE;
			wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
			wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
			wf.cbSize = 0;
			int buffer_size = wf.nAvgBytesPerSec / 10;
			if (SendMessage(hToolbar, TB_GETSTATE, 6, 0) & TBSTATE_CHECKED) {
				MMRESULT res = waveInOpen(&hWaveIn, WAVE_MAPPER, &wf, (DWORD_PTR)hWnd, 0, CALLBACK_WINDOW);
				if (res != MMSYSERR_NOERROR) {
					MessageBox(hWnd, L"Failed to open audio input device.", L"Error", MB_OK | MB_ICONERROR);
					SendMessage(hToolbar, TB_SETSTATE, 6, 0);
					break;
				}

				for (int i = 0; i < BUFFER_COUNT; i++) {
					ZeroMemory(&hdr_buf[i], sizeof(WAVEHDR));
					hdr_buf[i].lpData = (LPSTR)CoTaskMemAlloc(buffer_size);
					hdr_buf[i].dwBufferLength = buffer_size;
					hdr_buf[i].dwFlags = 0;
					waveInPrepareHeader(hWaveIn, &hdr_buf[i], sizeof(WAVEHDR));
					waveInAddBuffer(hWaveIn, &hdr_buf[i], sizeof(WAVEHDR));
				}
				waveInStart(hWaveIn);
				SetTimer(msgInput, 2, 0, NULL);
			} else {
				waveInStop(hWaveIn);
				waveInReset(hWaveIn);
				for (int i = 0; i < BUFFER_COUNT; ++i) {
					waveInUnprepareHeader(hWaveIn, &hdr_buf[i], sizeof(WAVEHDR));
					if (hdr_buf[i].lpData) {
						CoTaskMemFree(hdr_buf[i].lpData);
						hdr_buf[i].lpData = NULL;
					}
				}
				waveInClose(hWaveIn);
				hWaveIn = NULL;
				HANDLE hf = CreateFile(get_path(appdata_path, L"voice.wav"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
				BYTE riff[44];
				riff[0] = 'R'; riff[1] = 'I'; riff[2] = 'F'; riff[3] = 'F';
				write_le(riff + 4, 36 + recorded.size(), 4);
				riff[8] = 'W'; riff[9] = 'A'; riff[10] = 'V'; riff[11] = 'E';
				riff[12] = 'f'; riff[13] = 'm'; riff[14] = 't'; riff[15] = ' ';
				write_le(riff + 16, 16, 4);
				write_le(riff + 20, 1, 2);
				write_le(riff + 22, wf.nChannels, 2);
				write_le(riff + 24, wf.nSamplesPerSec, 4);
				write_le(riff + 28, wf.nAvgBytesPerSec, 4);
				write_le(riff + 32, wf.nBlockAlign, 2);
				write_le(riff + 34, wf.wBitsPerSample, 2);
				riff[36] = 'd'; riff[37] = 'a'; riff[38] = 't'; riff[39] = 'a';
				write_le(riff + 40, recorded.size(), 4);
				DWORD written;
				WriteFile(hf, riff, 44, &written, NULL);
				WriteFile(hf, &recorded[0], recorded.size(), &written, NULL);
				CloseHandle(hf);
				recorded.clear();
				wchar_t* filename = (wchar_t*)malloc(20);
				wcscpy(filename, L"voice.wav");
				files.push_back(filename);
				KillTimer(msgInput, 2);
				set_typing(0xfd5ec8f5, 0);
				open_files_list();
			}
			break;
		}
		case 4: {
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof(OPENFILENAME);
			wchar_t file_names[512] = {0};
			file_names[0] = 0;
			ofn.lpstrFile = file_names;
			ofn.nMaxFile = sizeof(file_names);
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
			if (!editing_msg_id) ofn.Flags |= OFN_ALLOWMULTISELECT;
			if (editing_msg_id && files.size() == 1) {
				free(files[0]);
				files.clear();
			}
			if (GetOpenFileName(&ofn)) {
				int len = wcslen(file_names);
				if (file_names[len + 1] != 0) {
					int start_pos = len + 1;
					while (true) {
						int file_len = wcslen(file_names + start_pos);
						wchar_t* file = (wchar_t*)malloc(2*(len + file_len + 2));
						swprintf(file, L"%s\\%s", file_names, file_names + start_pos);
						files.push_back(file);
						start_pos += file_len + 1;
						if (file_names[start_pos] == 0) break;
					}
				} else {
					wchar_t* file = (wchar_t*)malloc(2*(len + 1));
					wcscpy(file, file_names);
					files.push_back(file);
				}
				SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(14, 0));
				open_files_list();
			}
			break;
		} 
		case 7: {
			replying_msg_id = 0;
			set_sep_width(forwarding_msg_id ? width - 309 : width - 286);
			SendMessage(hToolbar, TB_SETSTATE, 7, MAKELONG(TBSTATE_HIDDEN, 0));
			break;
		} 
		case 8: {
			editing_msg_id = 0;
			set_sep_width(width - 286);
			SendMessage(hToolbar, TB_SETSTATE, 8, MAKELONG(TBSTATE_HIDDEN, 0));
			SetWindowText(msgInput, L"");
			break;
		} 
		case 19: {
			forwarding_msg_id = 0;
			set_sep_width(replying_msg_id ? width - 309 : width - 286);
			SendMessage(hToolbar, TB_SETSTATE, 19, MAKELONG(TBSTATE_HIDDEN, 0));
			break;
		} 
		case 20: {
			int sel_msg = -1;
			for (int i = messages.size() - 1; i >= 0; i--) if (messages[i].id == sel_msg_id) {
				sel_msg = i;
				break;
			}
			if (sel_msg == -1) break;
			editing_msg_id = sel_msg_id;
			StreamData sd = {0};
			EDITSTREAM es = {0};
			es.dwCookie = (DWORD_PTR)&sd;
			es.pfnCallback = StreamOutCallback;
			int end_msg_txt = messages[sel_msg].end_char - 1;
			for (i = documents.size() - 1; i >= 0; i--) {
				if (documents[i].min >= messages[sel_msg].start_char && documents[i].max <= messages[sel_msg].end_char) {
					end_msg_txt = documents[i].min;
					if (end_msg_txt != messages[sel_msg].end_header) end_msg_txt--;
					break;
				}
				if (documents[i].max < messages[sel_msg].start_char) break;
			}
			SendMessage(chat, EM_SETSEL, messages[sel_msg].end_header, end_msg_txt);
			SendMessage(chat, EM_STREAMOUT, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es);
			SendMessage(chat, EM_SETSEL, -1, 0);
			es.pfnCallback = StreamInCallback;
			SetFocus(msgInput);
			SendMessage(msgInput, EM_STREAMIN, SF_RTF | SF_UNICODE, (LPARAM)&es);
			free(sd.buf);
			SendMessage(msgInput, EM_SETSEL, -1, -1);
			set_sep_width(width - 309);
			SendMessage(hToolbar, TB_SETSTATE, 8, MAKELONG(TBSTATE_ENABLED, 0));
			for (i = 0; i < files.size(); i++) free(files[i]);
			files.clear();
			SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(10, 0));
			InvalidateRect(hToolbar, NULL, TRUE);
			break;
		} 
		case 21:
		case 22: {
			int sel_msg = -1;
			for (int i = messages.size() - 1; i >= 0; i--) if (messages[i].id == sel_msg_id) {
				sel_msg = i;
				break;
			}
			if (sel_msg == -1) break;
			BYTE unenc_query[64];
			BYTE enc_query[88];
			internal_header(unenc_query, true);
			write_le(unenc_query + 28, 20, 4);
			write_le(unenc_query + 32, 0xe58e95d2, 4);
			write_le(unenc_query + 36, LOWORD(wParam) == 21 ? 1 : 0, 4);
			write_le(unenc_query + 40, 0x1cb5c415, 4);
			write_le(unenc_query + 44, 1, 4);
			write_le(unenc_query + 48, messages[sel_msg].id, 4);
			fortuna_read(unenc_query + 52, 12, &prng);
			convert_message(unenc_query, enc_query, 64, 0);
			send_query(enc_query, 88);
			delete_message(sel_msg, true);
			break;
		} 
		case 23: {
			replying_msg_id = sel_msg_id;
			set_sep_width(forwarding_msg_id ? width - 332 : width - 309);
			SendMessage(hToolbar, TB_SETSTATE, 7, MAKELONG(TBSTATE_ENABLED, 0));
			SetFocus(msgInput);
			break;
		} 
		case 24: {
			forwarding_msg_id = sel_msg_id;
			memcpy(forwarding_peer_id, current_peer->id, 8);
			set_sep_width(replying_msg_id ? width - 332 : width - 309);
			SendMessage(hToolbar, TB_SETSTATE, 19, MAKELONG(TBSTATE_ENABLED, 0));
			SetFocus(msgInput);
			break;
		} 
		case 30:
		case 31: {
			BYTE buffer[24] = {0};
			LONG dlgUnits = GetDialogBaseUnits();
			DLGTEMPLATE *dlg = (DLGTEMPLATE*)buffer;
			dlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME;
			dlg->cx = MulDiv(490, 4, LOWORD(dlgUnits));
			dlg->cy = MulDiv(180, 8, HIWORD(dlgUnits));
			if (current_dialog) DestroyWindow(current_dialog);
			current_dialog = CreateDialogIndirectParam(GetModuleHandle(NULL), dlg, hWnd, DlgProc, LOWORD(wParam) == 30 ? (LPARAM)&myself : (LPARAM)current_peer);
			break;
		} 
		case 32: {
			HMENU hMenuItem = GetSubMenu(hMenuBar, 2);
			if (SENDMEDIAASFILES) {
				SENDMEDIAASFILES = false;
				CheckMenuItem(hMenuItem, 0, MF_BYPOSITION | MF_UNCHECKED);
			} else {
				SENDMEDIAASFILES = true;
				CheckMenuItem(hMenuItem, 0, MF_BYPOSITION | MF_CHECKED);
			}
			break;
		} 
		case 33: {
			BYTE buffer[24] = {0};
			LONG dlgUnits = GetDialogBaseUnits();
			DLGTEMPLATE *dlg = (DLGTEMPLATE*)buffer;
			dlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME | DS_CONTEXTHELP;
			dlg->cx = MulDiv(400, 4, LOWORD(dlgUnits));
			dlg->cy = MulDiv(225, 8, HIWORD(dlgUnits));
			if (current_dialog) DestroyWindow(current_dialog);
			current_dialog = CreateDialogIndirect(GetModuleHandle(NULL), dlg, hWnd, DlgProcOptions);
			break;
		}
		case 34:
			if (GetFileAttributes(get_path(exe_path, L"help.hlp")) == INVALID_FILE_ATTRIBUTES || !WinHelp(hMain, exe_path, HELP_FINDER, 0))
				HtmlHelp(hMain, get_path(exe_path, L"help.chm"), HH_DISPLAY_TOPIC, 0);
			break;
		case 35: {
			BYTE buffer[24] = {0};
			LONG dlgUnits = GetDialogBaseUnits();
			DLGTEMPLATE *dlg = (DLGTEMPLATE*)buffer;
			dlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME;
			dlg->cx = MulDiv(384, 4, LOWORD(dlgUnits));
			dlg->cy = MulDiv(143, 8, HIWORD(dlgUnits));
			if (current_about) DestroyWindow(current_about);
			current_about = CreateDialogIndirect(GetModuleHandle(NULL), dlg, hWnd, DlgProcInfo);
			break;
		} 
		case 36: {
			BYTE unenc_query[48];
			BYTE enc_query[72];
			internal_header(unenc_query, true);
			write_le(unenc_query + 28, 4, 4);
			write_le(unenc_query + 32, 0x3e72ba19, 4);
			fortuna_read(unenc_query + 36, 12, &prng);
			convert_message(unenc_query, enc_query, 48, 0);
			send_query(enc_query, 72);
			break;
		} 
		case 37: {
			exit_telegacy();
			break;
		} 
		case 38:
		case 39:
		case 40:
		case 41: {
			int type = LOWORD(wParam) - 38;
			BYTE unenc_query[96];
			BYTE enc_query[120];
			internal_header(unenc_query, true);
			write_le(unenc_query + 32, 0x84be5b93, 4);
			int offset = 40;
			if (LOWORD(wParam) == 38) write_le(unenc_query + 36, 0x193b4417, 4);
			else if (LOWORD(wParam) == 39) write_le(unenc_query + 36, 0x4a95e84e, 4);
			else if (LOWORD(wParam) == 40) write_le(unenc_query + 36, 0xb1db7c7e, 4);
			else {
				write_le(unenc_query + 36, 0xb8bc5b0c, 4);
				offset += place_peer(unenc_query + 40, current_peer, true);
			}
			write_le(unenc_query + offset, 0xcacb6ae2, 4);
			offset += 4;
			if ((LOWORD(wParam) == 41 && current_peer->mute_until) || (LOWORD(wParam) != 41 && muted_types[type])) {
				memset(unenc_query + offset, 0, 4);
				offset += 4;
			} else {
				write_le(unenc_query + offset, 4, 4);
				write_le(unenc_query + offset + 4, 2147483647, 4);
				offset += 8;
			}
			if (LOWORD(wParam) == 41) {
				int mute_until_old = current_peer->mute_until;
				memcpy(&current_peer->mute_until, unenc_query + offset - 4, 4);
				HMENU hMenuChat = GetSubMenu(hMenuBar, 1);
				ModifyMenu(hMenuChat, 1, MF_BYPOSITION | MF_STRING, 41, current_peer->mute_until ? L"Unmute this chat" : L"Mute this chat");
				if (current_peer->unread_msgs_count && !muted_types[current_peer->type])
					update_total_unread_msgs_count(current_peer->mute_until ? (0 - current_peer->unread_msgs_count) : current_peer->unread_msgs_count);
			}
			else {
				memcpy(&muted_types[type], unenc_query + offset - 4, 4);
				change_mute_all(type, muted_types[type] ? false : true);
				for (int i = 0; i < peers_count; i++) {
					if (peers[i].type == type && peers[i].unread_msgs_count && !peers[i].mute_until)
						update_total_unread_msgs_count(muted_types[type] ? (0 - peers[i].unread_msgs_count) : peers[i].unread_msgs_count);
				}
			}

			write_le(unenc_query + 28, offset - 32, 4);
			int padding_len = get_padding(offset);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			offset += padding_len;
			convert_message(unenc_query, enc_query, offset, 0);
			send_query(enc_query, offset + 24);
			break;
		}
		case 42:
			files_show_dropdown();
			break;
		case 999: {
			for (int i = 0; i < files.size(); i++) free(files[i]);
			files.clear();
			SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(10, 0));
			InvalidateRect(hToolbar, NULL, TRUE);
			break;
		}
		}
		if (LOWORD(wParam) >= 600 && LOWORD(wParam) < 601 + themes.size()) {
			BYTE unenc_query[80];
			BYTE enc_query[104];
			internal_header(unenc_query, true);
			write_le(unenc_query + 32, 0xe63be13f, 4);
			int offset = 36 + place_peer(unenc_query + 36, current_peer, true);
			if (LOWORD(wParam) == 600) {
				memset(unenc_query + offset, 0, 4);
				offset += 4;
			} else {
				memcpy(unenc_query + offset, themes[LOWORD(wParam) - 601].emoji_id, tlstr_len(themes[LOWORD(wParam) - 601].emoji_id, true));
				offset += tlstr_len(themes[LOWORD(wParam) - 601].emoji_id, true);
			}
			write_le(unenc_query + 28, offset - 32, 4);
			int padding_len = get_padding(offset);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			offset += padding_len;
			convert_message(unenc_query, enc_query, offset, 0);
			send_query(enc_query, offset + 24);
			memcpy(last_rpcresult_msgid, unenc_query + 16, 8);
		} else if (LOWORD(wParam) >= 1000 && LOWORD(wParam) < 1000 + files.size()) {
			free(files[LOWORD(wParam) - 1000]);
			files.erase(files.begin() + LOWORD(wParam) - 1000);
			if (!files.size()) {
				SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(10, 0));
				InvalidateRect(hToolbar, NULL, TRUE);
			}
		}
		break;
	case WM_DRAWITEM: {
		LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT)lParam;
		if (lpdis->CtlType == ODT_COMBOBOX) {
			wchar_t* name;
			Peer* peer = NULL;
			if (lpdis->hwndItem == hComboBoxChats) {
				if (lpdis->itemID == -1 && !current_peer) break;
				peer = lpdis->itemID == -1 ? current_peer : &peers[current_folder->peers[lpdis->itemID]];
				name = peer->name;
			} else {
				if (lpdis->itemID == -1 && !current_folder) break;
				name = lpdis->itemID == -1 ? current_folder->name : folders[lpdis->itemID].name;
			}

			LRESULT res;
			textHost->textServices->TxSendMessage(WM_SETFONT, (WPARAM)GetStockObject(SYSTEM_FONT), TRUE, &res);
			textHost->textServices->TxSendMessage(EM_REPLACESEL, 0, (LPARAM)name, &res);

			CHARFORMAT2 cf;
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_COLOR;
			cf.dwEffects = 0;
			cf.crTextColor = GetSysColor(lpdis->itemState & ODS_SELECTED && lpdis->rcItem.top != 3 ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT);
			textHost->textServices->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &res);

			if (peer && peer != current_peer && peer->unread_msgs_count) {
				if (peer->mute_until || muted_types[peer->type]) {
					cf.crTextColor = RGB(128, 128, 128);
					textHost->textServices->TxSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf, &res);
				}
				wchar_t unread_count[10];
				swprintf(unread_count, L" (%d)", peer->unread_msgs_count);
				textHost->textServices->TxSendMessage(EM_REPLACESEL, 0, (LPARAM)unread_count, &res);
			}

			int deleted_wchars = 0;
			for (int i = 0; i < wcslen(name); i++) i = emoji_adder(i, name, 0, 15, NULL, &deleted_wchars);

			RECTL myrect = {0, 0, lpdis->rcItem.right - lpdis->rcItem.left, lpdis->rcItem.bottom - lpdis->rcItem.top};
			HDC hRefDC = GetDC(NULL);
			HDC memDC = CreateCompatibleDC(hRefDC);
			HBITMAP hbm = CreateCompatibleBitmap(lpdis->hDC, myrect.right, myrect.bottom);
			HBITMAP hbmOld = (HBITMAP)SelectObject(memDC, hbm);

			FillRect(memDC, (RECT*)&myrect, GetSysColorBrush(lpdis->itemState & ODS_SELECTED && lpdis->rcItem.top != 3 ? COLOR_HIGHLIGHT : COLOR_WINDOW));
			myrect.left++;
			
			textHost->textServices->TxDraw(DVASPECT_CONTENT, NULL, NULL, NULL, memDC, NULL, &myrect, NULL, NULL, NULL, NULL, NULL);
			BitBlt(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, myrect.right, myrect.bottom, memDC, 0, 0, SRCCOPY);
			SelectObject(memDC, hbmOld);
			DeleteObject(hbm);
			DeleteDC(memDC);
			ReleaseDC(NULL, hRefDC);

			textHost->textServices->TxSendMessage(EM_SETSEL, 0, -1, &res);
			textHost->textServices->TxSendMessage(EM_REPLACESEL, 0, (LPARAM)L"", &res);
		} else if (lpdis->hwndItem == hStatus) {
			LRESULT res;
			if (!nt3) textHost->textServices->TxSendMessage(WM_SETFONT, (WPARAM)hDefaultFont, TRUE, &res);
			textHost->textServices->TxSendMessage(EM_REPLACESEL, 0, lpdis->itemID == 0 ? (LPARAM)status_str : (LPARAM)lpdis->itemData, &res);
			int deleted_wchars = 0;
			if (lpdis->itemID == 0 && current_peer && current_peer->type == 0) for (int i = 0; i < lpdis->itemData; i++) i = emoji_adder(i, status_str, 0, 15, NULL, &deleted_wchars);

			CHARFORMAT2 cf;
			cf.cbSize = sizeof(cf);
			cf.dwMask = CFM_COLOR;
			cf.crTextColor = COLOR_WINDOWTEXT;
			cf.dwEffects = 0;
			if (nt3) {
				cf.dwMask |= CFM_FACE | CFM_SIZE | CFM_BOLD;
				cf.yHeight = 200;
				wcscpy(cf.szFaceName, L"MS Sans Serif");
			}
			textHost->textServices->TxSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf, &res);

			lpdis->rcItem.left += nt3 ? 2 : 1;
			if (!deleted_wchars && !nt3) lpdis->rcItem.top += 1;
			textHost->textServices->TxDraw(DVASPECT_CONTENT, NULL, NULL, NULL, lpdis->hDC, NULL, (RECTL*)&lpdis->rcItem, NULL, NULL, NULL, NULL, NULL);
			
			textHost->textServices->TxSendMessage(EM_SETSEL, 0, -1, &res);
			textHost->textServices->TxSendMessage(EM_REPLACESEL, 0, (LPARAM)L"", &res);
		} else if (lpdis->hwndItem == hTabs) {
			wchar_t* icons[] = {L"1f553", L"1f600", L"1f44b", L"1f436", L"1f347", L"1f30d", L"1f383", L"1f576", L"1f3e7", L"1f3c1"};
			wchar_t path[MAX_PATH];
			swprintf(path, L"%s\\%s.ico", get_path(exe_path, L"emojis"), icons[lpdis->itemID]);
			if (lpdis->itemState & ODS_SELECTED) lpdis->rcItem.left += 4;
			lpdis->rcItem.top += (lpdis->itemState & ODS_SELECTED) ? 3 : 1;
			paint_emoji_bitmap(lpdis->hDC, path, &lpdis->rcItem);
		}
		break;
	}
	case WM_HELP:
		if (GetFileAttributes(get_path(exe_path, L"help.hlp")) == INVALID_FILE_ATTRIBUTES || !WinHelp(hMain, exe_path, HELP_FINDER, 0))
				HtmlHelp(hMain, get_path(exe_path, L"help.chm"), HH_DISPLAY_TOPIC, 0);
		break;
	case WM_DROPFILES: {
		HDROP hDrop = (HDROP)wParam;
		int count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
		if (editing_msg_id && files.size() == 1) {
			free(files[0]);
			files.clear();
		}
		for (int i = 0; i < count; i++) {
			wchar_t path[256];
			DragQueryFile(hDrop, i, path, 256);
			wchar_t* file = (wchar_t*)malloc(2*(wcslen(path)+1));
			wcscpy(file, path);
			files.push_back(file);
			if (editing_msg_id) break;
		}
		DragFinish(hDrop);
		SetForegroundWindow(hWnd);
		SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(14, 0));
		open_files_list();
		return 0;
	}
	case MM_WIM_DATA: {
		if (SendMessage(hToolbar, TB_GETSTATE, 6, 0) & TBSTATE_CHECKED) {
			WAVEHDR* ph = (WAVEHDR*)lParam;
			if (ph->dwBytesRecorded > 0) {
				recorded.insert(recorded.end(), (BYTE*)ph->lpData, (BYTE*)ph->lpData + ph->dwBytesRecorded);
			}
			waveInAddBuffer(hWaveIn, ph, sizeof(WAVEHDR));
		}
		break;
    }
	case WM_APP + 1:
		response_handler((DCInfo*)read_le((BYTE*)wParam, 4), (BYTE*)(wParam + 32), true, read_le((BYTE*)(wParam + 28), 4));
		free((BYTE*)wParam);
		break;
	case WM_TRAYICON:
		if (lParam == WM_LBUTTONDOWN) {
			bring_me_to_life();
			break;
		} else if (lParam == WM_RBUTTONUP) {
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hWnd);
			TrackPopupMenu(GetSubMenu(hMenuBar, 0), TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
			PostMessage(hWnd, WM_NULL, 0, 0);
			break;
		} else if (lParam == WM_USER + 5) click_on_notification();
		break;
	case WM_TIMER:
		if (wParam == 0) {
			status_bar_status(current_peer);
			KillTimer(hWnd, 0);
		} else if (wParam == 1) send_ping(&dcInfoMain);
		else if (wParam == 2) {
			SendMessage(hStatus, SB_SETTEXTA, 1, (LPARAM)"");
			KillTimer(hWnd, 2);
		} else if (wParam == 3) {
			get_channel_difference(current_peer);
		} else if (wParam >= 10 && wParam < 20) {
			DCInfo* dcInfo = &dcInfoMain;
			for (std::list<DCInfo>::iterator it = active_dcs.begin(); it != active_dcs.end(); it++) {
				DCInfo* active_dc = &(*it);
				if (wParam - 10 == active_dc->dc) {
					dcInfo = active_dc;
					break;
				}
			}
			memcpy(dcInfo->server_salt, dcInfo->future_salt, 8);
			get_future_salt(dcInfo);
			KillTimer(hWnd, wParam);
		} else if (wParam >= 20 && wParam < 30) {
			for (std::list<DCInfo>::iterator it = active_dcs.begin(); it != active_dcs.end(); it++) {
				DCInfo* active_dc = &(*it);
				if (wParam - 20 == active_dc->dc) {
					closesocket(active_dc->sock);
					break;
				}
			}
			KillTimer(hWnd, wParam);
			KillTimer(hWnd, wParam - 10);
		} else if (wParam >= 50 && wParam < 100) {
			for (int i = 0; i < unmuteTimers.size(); i++) {
				if (wParam == unmuteTimers[i].timer_id) {
					if (unmuteTimers[i].peer_id <= 2 && unmuteTimers[i].peer_id >= 0) {
						if (muted_types[unmuteTimers[i].peer_id]) {
							int new_unread_msgs_count = 0;
							for (int j = 0; j < peers_count; j++) {
								if (peers[j].type == unmuteTimers[i].peer_id && peers[j].unread_msgs_count && !peers[j].mute_until)
									new_unread_msgs_count += peers[i].unread_msgs_count;
							}
							if (new_unread_msgs_count) update_total_unread_msgs_count(new_unread_msgs_count);
							change_mute_all(unmuteTimers[i].peer_id, true);
						}
					} else {
						for (int j = 0; j < peers_count; j++) {
							if (memcmp((BYTE*)&unmuteTimers[i].peer_id, peers[j].id, 8) == 0) {
								if (peers[j].mute_until) {
									if (peers[j].unread_msgs_count && !muted_types[peers[j].type]) update_total_unread_msgs_count(peers[j].unread_msgs_count);
									if (&peers[j] == current_peer) {
										HMENU hMenuChat = GetSubMenu(hMenuBar, 1);
										ModifyMenu(hMenuChat, 1, MF_BYPOSITION | MF_STRING, 41, L"Mute this chat");
									}
								}
								break;
							}
						}
					}
					unmuteTimers.erase(unmuteTimers.begin() + i);
					break;
				}
			}
			KillTimer(hWnd, wParam);
		}
		break;
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED) {
			minimized = true;
			update_own_status(false);
		}
		else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			int sep_width = 0;
			if (replying_msg_id && forwarding_msg_id) sep_width = width - 332;
			else if (editing_msg_id || replying_msg_id || forwarding_msg_id) sep_width = width - 309;
			else sep_width = width - 286;
			set_sep_width(sep_width);
			
			int status_parts[2] = {width / 2, -1};
			SendMessage(hStatus, SB_SETPARTS, 2, (LPARAM)status_parts);
			bool cantwrite = current_peer && !current_peer->perm.cansendmsg;
			HDWP hdwp = BeginDeferWindowPos(9);
			hdwp = DeferWindowPos(hdwp, chat, NULL, 10, 40, width - 20, cantwrite ? height - 70 : (height - 165 + edits_border_offset), SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, msgInput, NULL, 10, height - 95 + edits_border_offset, width - 20, 65 - edits_border_offset, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, tbSeparatorHider, NULL, width / 2, height - 118 + edits_border_offset, 35, 21, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hTabs, NULL, width - 224, height - 334 + edits_border_offset, 214, 214, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hOverlayTabs, NULL, width - 224, height - 334 + edits_border_offset, 214, 214, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hToolbar, NULL, 10, height - (ie3 ? 120 : 124) + edits_border_offset, width - 20, 25, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, splitter, NULL, 10, height - 124 + edits_border_offset, width - 20, 3, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hStatus, NULL, 0, 0, 0, 0, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hComboBoxChats, NULL, NULL, NULL, width / 2.5, 300, SWP_NOZORDER | SWP_NOMOVE);
			EndDeferWindowPos(hdwp);
			SCROLLINFO si = {0};
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
			GetScrollInfo(chat, SB_VERT, &si);
			if (si.nPos + si.nPage - si.nMax < 5) SendMessage(chat, EM_SCROLL, SB_LINEDOWN, 0);
			if (minimized && read_le(dcInfoMain.future_salt, 8) != 0) update_own_status(true);
			minimized = false;
			if (current_peer) SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
			return 0;
		}
		break;
	case WM_CTLCOLORSTATIC:
		if (HWND(lParam) == tbSeparatorHider || HWND(lParam) == splitter) return (long)(theme_brush == NULL ? GetStockObject(WHITE_BRUSH) : theme_brush);
		break;
	case WM_SETCURSOR:
		if ((HWND)wParam == splitter) {
			SetCursor(LoadCursor(NULL, IDC_SIZENS));
			return TRUE;
		}
		break;
	case WM_NOTIFY: {
		NMHDR* pNMHDR = (NMHDR*)lParam;
		ENLINK* pENLink = (ENLINK*)lParam;
		if (pNMHDR->hwndFrom == chat && pNMHDR->code == EN_LINK && (pENLink->msg == WM_LBUTTONDOWN)) {
			bool found = false;
			POINT pt;
			GetCursorPos(&pt);
			ScreenToClient(chat, &pt);
			unsigned int sel_char = SendMessage(chat, EM_CHARFROMPOS, 0, (LPARAM)&pt) - 1;
			for (int i = documents.size() - 1; i >= 0; i--) {
				if (documents[i].min <= sel_char && documents[i].max >= sel_char) {
					found = true;
					FILE* f = _wfopen(documents[i].filename, L"rb");
					int index = - 1;
					for (int j = 0; j < downloading_docs.size(); j++) {
						if (memcmp(documents[i].id, downloading_docs[j].id, 8) == 0) {
							index = j;
							break;
						}
					}
					if (f) {
						_fseeki64(f, 0, SEEK_END);
						__int64 file_size = _ftelli64(f);
						fclose(f);
						if (index != -1) {
							DeleteFile(documents[i].filename);
							SendMessage(hStatus, SB_SETTEXTA, 1, (LPARAM)"");
							free(downloading_docs[index].filename);
							free(downloading_docs[index].file_reference);
							downloading_docs.erase(downloading_docs.begin() + index);
						} else if (file_size != documents[i].size) {
							if (index == -1) {
								DeleteFile(documents[i].filename);
								Document downloading_doc;
								downloading_doc = documents[i];
								downloading_doc.filename = _wcsdup(documents[i].filename);
								downloading_doc.file_reference = (BYTE*)malloc(tlstr_len(documents[i].file_reference, true));
								memcpy(downloading_doc.file_reference, documents[i].file_reference, tlstr_len(documents[i].file_reference, true));
								downloading_docs.push_back(downloading_doc);
								download_file(&dcInfoMain, &downloading_docs[downloading_docs.size()-1]);
							}
						} else {
							if ((INT_PTR)ShellExecute(NULL, L"open", documents[i].filename, NULL, NULL, SW_SHOWNORMAL) <= 32) {
								wchar_t cmd[MAX_PATH * 2];
								swprintf(cmd, L"shell32.dll,OpenAs_RunDLL %s", documents[i].filename);
								ShellExecute(NULL, L"open", L"rundll32.exe", cmd, NULL, SW_SHOWNORMAL);
							}
						}
					} else if (index == -1) {
						Document downloading_doc;
						downloading_doc = documents[i];
						downloading_doc.filename = _wcsdup(documents[i].filename);
						downloading_doc.file_reference = (BYTE*)malloc(tlstr_len(documents[i].file_reference, true));
						memcpy(downloading_doc.file_reference, documents[i].file_reference, tlstr_len(documents[i].file_reference, true));
						downloading_docs.push_back(downloading_doc);
						download_file(&dcInfoMain, &downloading_docs[downloading_docs.size()-1]);
					} else {
						SendMessage(hStatus, SB_SETTEXTA, 1, (LPARAM)"");
						free(downloading_docs[index].filename);
						free(downloading_docs[index].file_reference);
						downloading_docs.erase(downloading_docs.begin() + index);
					}
					break;
				}
			}
			if (!found) {
				for (i = 0; i < links.size(); i++) {
					if (links[i].chrg.cpMin == pENLink->chrg.cpMin && links[i].chrg.cpMax == pENLink->chrg.cpMax) {
						if ((INT_PTR)ShellExecute(NULL, L"open", links[i].lpstrText, NULL, NULL, SW_SHOW) <= 32)
							MessageBox(NULL, L"Couldn't open the link!", L"Error", MB_OK | MB_ICONERROR);
						found = true;
						break;
					}
				}
			}
			if (!found) {
				int size = pENLink->chrg.cpMax - pENLink->chrg.cpMin + 1;
				wchar_t* url = (wchar_t*)malloc(2*size);
				TEXTRANGE tr;
				tr.chrg = pENLink->chrg;
				tr.lpstrText = url;
				SendMessage(chat, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
				ShellExecute(NULL, L"open", url, NULL, NULL, SW_SHOW);
				free(url);
			}
		} else if (pNMHDR->hwndFrom == hTabs && pNMHDR->code == TCN_SELCHANGE) {
			HWND child = GetWindow(emojiStatic, GW_CHILD);
			while (child != NULL) {
				HWND next = GetWindow(child, GW_HWNDNEXT);
				if (!nt3) {
					HICON hOldIcon = (HICON)SendMessage(child, BM_GETIMAGE, IMAGE_ICON, NULL);
					DestroyIcon(hOldIcon);
				}
				wchar_t* code = (wchar_t*)GetWindowLongPtr(child, GWLP_USERDATA);
				free(code);
				DestroyWindow(child);
				child = next;
			}
			if (!EMOJIS) break;
			InvalidateRect(emojiStatic, NULL, TRUE);
			UpdateWindow(emojiStatic);
			int category_num = TabCtrl_GetCurSel(hTabs) - 1, emoji_x = -7, emoji_y = 13;
			if (category_num == -1) {
				for (int i = 0; i < fav_emojis.size(); i++) {
					if (emoji_x == 153) {
						emoji_x = 13;
						emoji_y += 20;
					} else emoji_x += 20;
					wchar_t path[MAX_PATH];
					swprintf(path, L"%s\\%s.ico", get_path(exe_path, L"emojis"), fav_emojis[i]);
					HICON hIcon = (HICON)LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
					if (nt3) hIcon = (HICON)(GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES);
					if (hIcon) {
						HWND hBtn = CreateWindow(L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? BS_OWNERDRAW : BS_ICON), emoji_x, emoji_y, 20, 20, emojiStatic, (HMENU)1, GetModuleHandle(NULL), NULL);
						if (!nt3) SendMessage(hBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
						wchar_t* code = _wcsdup(fav_emojis[i]);
						SetWindowLongPtr(hBtn, GWLP_USERDATA, (LONG_PTR)code);
						//WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hBtn, GWLP_WNDPROC, (LONG_PTR)WndProcReactionButton);
						//SetProp(hBtn, L"oldproc", (HANDLE)oldProc);
					}
				}
			} else {
				FILE* f = _wfopen(get_path(exe_path, L"emoji_categories.dat"), L"rb");
				if (f) {
					int count = 0;
					for (int i = 0; i < category_num; i++) {
						fread(&count, 4, 1, f);
						fseek(f, count * 4, SEEK_CUR);
					}
					fread(&count, 4, 1, f);
					for (i = 0; i < count; i++) {
						int emoji = 0;
						fread(&emoji, 4, 1, f);
						wchar_t searchPath[MAX_PATH];
						swprintf(searchPath, L"%s\\%x*", get_path(exe_path, L"emojis"), emoji);
						WIN32_FIND_DATA ffd;
						HANDLE hFind = FindFirstFile(searchPath, &ffd);
						if (hFind == INVALID_HANDLE_VALUE) continue;
						do {
							if (wcsnicmp(ffd.cFileName, wcsrchr(searchPath, L'\\') + 1, 4) != 0) continue;
							if (emoji_x == 153) {
								emoji_x = 13;
								emoji_y += 20;
							} else emoji_x += 20;
							wchar_t path[MAX_PATH];
							swprintf(path, L"%s\\%s", get_path(exe_path, L"emojis"), ffd.cFileName);
							HICON hIcon = (HICON)LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
							HWND hBtn = CreateWindow(L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? BS_OWNERDRAW : BS_ICON), emoji_x, emoji_y, 20, 20, emojiStatic, (HMENU)1, GetModuleHandle(NULL), NULL);
							if (!nt3) SendMessage(hBtn, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
							*(wcsrchr(ffd.cFileName, L'.')) = 0;
							wchar_t* code = _wcsdup(ffd.cFileName);
							SetWindowLongPtr(hBtn, GWLP_USERDATA, (LONG_PTR)code);
							//WNDPROC oldProc = (WNDPROC)SetWindowLongPtr(hBtn, GWLP_WNDPROC, (LONG_PTR)WndProcReactionButton);
							//SetProp(hBtn, L"oldproc", (HANDLE)oldProc);
						}
						while (FindNextFile(hFind, &ffd));
						FindClose(hFind);
					}
					fclose(f);
				}
			}
			SCROLLINFO si = { sizeof(si) };
			si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
			si.nMin = 0;
			si.nMax = emoji_y + 33;
			if (si.nMax <= 182) {
				si.nMax = 1000;
				si.nPage = 1000;
			} else si.nPage = 182;
			si.nPos = 0;
			SetScrollInfo(emojiScroll, SB_CTL, &si, TRUE);
			SetFocus(msgInput);
		} else if (pNMHDR->hwndFrom == hToolbar && pNMHDR->code == TBN_DROPDOWN) files_show_dropdown();
		break;			
	}
	case WM_MOUSEACTIVATE: {
		POINT pt;
		GetCursorPos(&pt);
		RECT rc;
		GetWindowRect(reactionStatic, &rc);
		if (pt.x < rc.left || pt.x > rc.right || pt.y < rc.top || pt.y > rc.bottom) ShowWindow(reactionStatic, SW_HIDE);
		break;
	}
	case WM_NCACTIVATE:
		if (dontlosefocus) {
			dontlosefocus = false;
			return FALSE;
		}
		break;
	case WM_CLOSE:
		if (CLOSETOTRAY) {
			ShowWindow(hWnd, SW_HIDE);
			update_own_status(false);
		} else exit_telegacy();
		return 0;
	case WM_DESTROY:
		CoUninitialize();
		if (g_pAIMM) {
			g_pAIMM->FilterClientWindows(NULL, 0);
			g_pAIMM->Deactivate();
			g_pAIMM->Release();
			g_pAIMM = NULL;
		}
		NOTIFYICONDATA nid = {0};
		nid.cbSize = sizeof(nid);
		nid.hWnd = hMain;
		nid.uID = 1;
		Shell_NotifyIcon(NIM_DELETE, &nid);

		if (dcInfoMain.authorized) {
			FILE* f = _wfopen(get_path(appdata_path, L"session.dat"), L"rb+");
			fseek(f, 264, SEEK_SET);
			fwrite(dcInfoMain.server_salt, 1, 8, f);
			fseek(f, 284, SEEK_SET);
			fwrite(&pts, 4, 1, f);
			fwrite(&qts, 4, 1, f);
			fwrite(&date, 4, 1, f);
			fclose(f);

			if (peers_count > 0 || read_le(myself.id, 8) != 0) {
				f = _wfopen(get_path(appdata_path, L"database.dat"), L"wb");
				fwrite(&database_version, 4, 1, f);
				fwrite(&muted_types, 4, 3, f);
				fwrite(&peers_count, 4, 1, f);
				for (int i = -1; i < peers_count; i++) {
					Peer* peer = (i == -1) ? &myself : &peers[i];
					fwrite(peer->id, 1, 8, f);
					fwrite(peer->access_hash, 1, 8, f);
					int name_len = wcslen(peer->name)+1;
					fwrite(&name_len, 4, 1, f);
					fwrite(peer->name, 2, name_len, f);
					int handle_len = peer->handle ? wcslen(peer->handle) + 1 : 0;
					fwrite(&handle_len, 4, 1, f);
					if (handle_len) fwrite(peer->handle, 2, handle_len, f);
					fwrite(&peer->online, 4, 1, f);
					fwrite(peer->photo, 1, 8, f);
					fwrite(&peer->photo_dc, 4, 1, f);
					fwrite(&peer->last_read, 4, 1, f);
					fwrite(&peer->unread_msgs_count, 4, 1, f);
					fwrite(&peer->mute_until, 4, 1, f);
					fwrite(&peer->perm, sizeof(Permissions), 1, f);
					fwrite(&peer->amadmin, 1, 1, f);
					fwrite(&peer->full, 1, 1, f);
					fwrite(&peer->type, 1, 1, f);
					if (peer->type != 0) {
						fwrite(&peer->name_set_time, 4, 1, f);
						fwrite(&peer->pfp_set_time, 4, 1, f);
					}
					if (peer->full) {
						int about_len = peer->about ? wcslen(peer->about)+1 : 0;
						fwrite(&about_len, 4, 1, f);
						if (about_len) fwrite(peer->about, 2, about_len, f);
						if (peer->type == 0) fwrite(peer->birthday, 1, 4, f);
						fwrite(&peer->theme_id, 1, 8, f);
						fwrite(&peer->theme_set_time, 4, 1, f);
						if (peer->type == 1) {
							int chat_users_count = peer->chat_users->size();
							fwrite(&chat_users_count, 4, 1, f);
							for (int j = 0; j < chat_users_count; j++) {
								fwrite(peer->chat_users->at(j).id, 1, 8, f);
								fwrite(peer->chat_users->at(j).access_hash, 1, 8, f);
								int name_len = wcslen(peer->chat_users->at(j).name)+1;
								fwrite(&name_len, 4, 1, f);
								fwrite(peer->chat_users->at(j).name, 2, name_len, f);
							}
						}
						if (peer->type != 0) {
							int reaction_count, reaction_len;
							if (peer->reaction_list == &reaction_list) {
								reaction_count = 0xFFFFFFFF;
								fwrite(&reaction_count, 4, 1, f);
							} else if (peer->reaction_list == NULL) {
								reaction_count = 0;
								fwrite(&reaction_count, 4, 1, f);
							} else {
								reaction_count = peer->reaction_list->size();
								fwrite(&reaction_count, 4, 1, f);
								for (int j = 0; j < reaction_count; j++) {
									reaction_len = wcslen(peer->reaction_list->at(j)) + 1;
									fwrite(&reaction_len, 4, 1, f);
									fwrite(peer->reaction_list->at(j), 2, reaction_len, f);
								}
							}
						}
						if (peer->type == 2) fwrite(&peer->chat_users, 4, 1, f);
					}
					if (peer->type == 2) fwrite(&peer->channel_pts, 4, 1, f);
				}
				fwrite(&folders_count, 4, 1, f);
				for (i = 0; i < folders_count; i++) {
					fwrite(folders[i].id, 1, 4, f);
					fwrite(&folders[i].count, 4, 1, f);
					fwrite(&folders[i].pinned_count, 4, 1, f);
					fwrite(folders[i].peers, 4, folders[i].count, f);
					int name_len = wcslen(folders[i].name)+1;
					fwrite(&name_len, 4, 1, f);
					fwrite(folders[i].name, 2, name_len, f);
				}
				fwrite(&reaction_hash, 4, 1, f);
				int reactions_count = reaction_list.size();
				int reaction_len;
				fwrite(&reactions_count, 4, 1, f);
				for (i = 0; i < reactions_count; i++) {
					reaction_len = wcslen(reaction_list[i]) + 1;
					fwrite(&reaction_len, 4, 1, f);
					fwrite(reaction_list[i], 2, reaction_len, f);
				}
				fwrite(&theme_hash, 8, 1, f);
				int themes_count = themes.size();
				int theme_len;
				fwrite(&themes_count, 4, 1, f);
				for (i = 0; i < themes_count; i++) {
					fwrite(themes[i].id, 1, 8, f);
					fwrite(&themes[i].color, 4, 1, f);
					theme_len = tlstr_len(themes[i].emoji_id, true);
					fwrite(&theme_len, 4, 1, f);
					fwrite(themes[i].emoji_id, 1, theme_len, f);
				}
				fclose(f);
			}

			f = _wfopen(get_path(appdata_path, L"fav_emojis.dat"), L"wb");
			int count = fav_emojis.size();
			fwrite(&count, 4, 1, f);
			for (int i = 0; i < count; i++) {
				int count2 = wcslen(fav_emojis[i]) + 1;
				fwrite(&count2, 4, 1, f);
				fwrite(fav_emojis[i], 2, count2, f);
			}
		}

		WINDOWPLACEMENT wp;
		wp.length = sizeof(wp);
		GetWindowPlacement(hWnd, &wp);
		RECT rc = wp.rcNormalPosition;
		wchar_t value[10];
		swprintf(value, L"%d", rc.right - rc.left);
		WritePrivateProfileString(L"General", L"width", value, get_path(appdata_path, L"options.ini"));
		swprintf(value, L"%d", rc.bottom - rc.top);
		WritePrivateProfileString(L"General", L"height", value, appdata_path);
		swprintf(value, L"%d", IsZoomed(hWnd));
		WritePrivateProfileString(L"General", L"maximized", value, appdata_path);
		swprintf(value, L"%d", edits_border_offset);
		WritePrivateProfileString(L"General", L"edits_border_offset", value, appdata_path);
		wchar_t download_path[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, download_path);
		WritePrivateProfileString(L"General", L"download_path", download_path, appdata_path);
		swprintf(value, L"%d", CLOSETOTRAY);
		WritePrivateProfileString(L"General", L"close_to_tray", value, appdata_path);
		swprintf(value, L"%d", balloon_notifications);
		WritePrivateProfileString(L"General", L"balloon_notifications", value, appdata_path);

		swprintf(value, L"%d", IMAGELOADPOLICY);
		WritePrivateProfileString(L"Content", L"image_load_policy", value, appdata_path);
		swprintf(value, L"%d", EMOJIS);
		WritePrivateProfileString(L"Content", L"emojis", value, appdata_path);
		swprintf(value, L"%d", SPOILERS);
		WritePrivateProfileString(L"Content", L"spoilers", value, appdata_path);

		WritePrivateProfileString(L"Sounds", L"notification_sound", sound_paths[0], appdata_path);
		WritePrivateProfileString(L"Sounds", L"incoming_sound", sound_paths[1], appdata_path);
		WritePrivateProfileString(L"Sounds", L"outgoing_sound", sound_paths[2], appdata_path);

		swprintf(value, L"%d", SAMPLERATE);
		WritePrivateProfileString(L"Voice", L"sample_rate", value, appdata_path);
		swprintf(value, L"%d", BITSPERSAMPLE);
		WritePrivateProfileString(L"Voice", L"bits_per_sample", value, appdata_path);
		swprintf(value, L"%d", CHANNELS);
		WritePrivateProfileString(L"Voice", L"channels", value, appdata_path);

		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

WNDPROC oldNameProc, oldHandleProc, oldAboutProc, oldBirthdayProc, oldOptionsTabsProc;
HWND name, handle, about, birthday, hOptionsTabs, downloadEdit = NULL;
HWND sound_edits[3] = {0};
LRESULT CALLBACK WndProcDisabled(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_LBUTTONDBLCLK:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
	case WM_PAINT:
		if (hWnd == birthday && ie3 && msg != WM_PAINT) return 0;
		else HideCaret(hWnd);
		break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
		case VK_HOME: case VK_END: case VK_PRIOR: case VK_NEXT:
			return 0;
		}
		break;
	}
	WNDPROC oldProc;
	if (hWnd == name) oldProc = oldNameProc;
	else if (hWnd == handle) oldProc = oldHandleProc;
	else if (hWnd == about) oldProc = oldAboutProc;
	else if (hWnd == birthday) oldProc = oldBirthdayProc;
	return CallWindowProc(oldProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProcOptionsTabs(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case 10: {
			wchar_t path[MAX_PATH];
			BROWSEINFO bi = {0};
			bi.hwndOwner = hWnd;
			bi.lpszTitle = L"Select a folder";
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
			if (pidl) {
				SHGetPathFromIDList(pidl, path);
				CoTaskMemFree(pidl);
				SendMessage(downloadEdit, EM_SETSEL, 0, -1);
				SendMessage(downloadEdit, EM_REPLACESEL, FALSE, (LPARAM)path);
				SendMessage(downloadEdit, EM_SETSEL, 0, 0);
			}
			break;
		}
		case 11:
			if (HIWORD(wParam) == BN_CLICKED) CLOSETOTRAY = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) ? true : false;
			break;
		case 12:
			if (HIWORD(wParam) == BN_CLICKED) balloon_notifications = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) ? true : false;
			break;
		case 13:
			if (HIWORD(wParam) == BN_CLICKED && SendMessage((HWND)lParam, BM_GETCHECK, 0, 0)) IMAGELOADPOLICY = 0;
			break;
		case 14:
			if (HIWORD(wParam) == BN_CLICKED && SendMessage((HWND)lParam, BM_GETCHECK, 0, 0)) IMAGELOADPOLICY = 1;
			break;
		case 15:
			if (HIWORD(wParam) == BN_CLICKED && SendMessage((HWND)lParam, BM_GETCHECK, 0, 0)) IMAGELOADPOLICY = 2;
			break;
		case 16:
			if (HIWORD(wParam) == BN_CLICKED) {
				if (SendMessage((HWND)lParam, BM_GETCHECK, 0, 0)) {
					SendMessage(hToolbar, TB_ENABLEBUTTON, 5, MAKELPARAM(TRUE, 0));
					EMOJIS = true;
				} else {
					SendMessage(hToolbar, TB_ENABLEBUTTON, 5, MAKELPARAM(FALSE, 0));
					EMOJIS = false;
				}
				NMHDR hdr;
				hdr.hwndFrom = hTabs;
				hdr.code = TCN_SELCHANGE;
				SendMessage(hMain, WM_NOTIFY, NULL, (LPARAM)&hdr);
				SetForegroundWindow(current_dialog);
			}
			break;
		case 17:
			if (HIWORD(wParam) == BN_CLICKED) SPOILERS = SendMessage((HWND)lParam, BM_GETCHECK, 0, 0) ? true : false;
			break;
		case 18:
		case 19:
		case 20: {
			int i = LOWORD(wParam) - 18;
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof(ofn);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = L"WAV Files (*.wav)\0*.wav\0";
			ofn.lpstrFile = sound_paths[i];
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
			if (GetOpenFileName(&ofn)) {
				SendMessage(sound_edits[i], EM_SETSEL, 0, -1);
				SendMessage(sound_edits[i], EM_REPLACESEL, FALSE, (LPARAM)sound_paths[i]);
				SendMessage(sound_edits[i], EM_SETSEL, 0, 0);
			}
			break;
		}
		case 21:
		case 22:
		case 23: {
			int i = LOWORD(wParam) - 21;
			sound_paths[i][0] = 0;
			SendMessage(sound_edits[i], EM_SETSEL, 0, -1);
			SendMessage(sound_edits[i], EM_REPLACESEL, FALSE, (LPARAM)L"None");
			break;
		}
		case 24:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int selIndex = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if (selIndex == 0) SAMPLERATE = 8000;
				else if (selIndex == 1) SAMPLERATE = 16000;
				else if (selIndex == 2) SAMPLERATE = 44100;
				else SAMPLERATE = 48000;
				SetFocus(hOptionsTabs);
			}
			break;
		case 25:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int selIndex = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if (selIndex == 0) BITSPERSAMPLE = 8;
				else if (selIndex == 1) BITSPERSAMPLE = 16;
				else BITSPERSAMPLE = 24;
				SetFocus(hOptionsTabs);
			}
			break;
		case 26:
			if (HIWORD(wParam) == CBN_SELCHANGE) {
				int selIndex = SendMessage((HWND)lParam, CB_GETCURSEL, 0, 0);
				if (selIndex == 0) CHANNELS = 1;
				else CHANNELS = 2;
				SetFocus(hOptionsTabs);
			}
			break;
		}
		break;
	case WM_ERASEBKGND:
		if (nt3) {
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
			return TRUE;
		}
	}
	return CallWindowProc(oldOptionsTabsProc, hWnd, msg, wParam, lParam);
}

void apply_edits() {
	if (downloadEdit) {
		wchar_t path[MAX_PATH];
		GetWindowText(downloadEdit, path, MAX_PATH);
		SetCurrentDirectory(path);
		downloadEdit = NULL;
	}
	for (int i = 0; i < 3; i++) {
		if (sound_edits[i]) {
			wchar_t path[MAX_PATH];
			GetWindowText(sound_edits[i], path, MAX_PATH);
			sound_edits[i] = NULL;
			if (wcscmp(path, sound_paths[i]) == 0 || wcscmp(path, L"None") == 0) continue;
			DWORD attrs = GetFileAttributes(path);
			if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY) && _wcsicmp(wcsrchr(path, L'.'), L".wav") == 0) {
				wcscpy(sound_paths[i], path);
			}
		}
	}
}

INT_PTR CALLBACK DlgProcOptions(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_INITDIALOG: {
		hOptionsTabs = CreateWindow(WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE | (nt3 ? TCS_OWNERDRAWFIXED : 0), 5, 5, 390, 190, hDlg, (HMENU)5, NULL, NULL);
		oldOptionsTabsProc = (WNDPROC)SetWindowLongPtr(hOptionsTabs, GWLP_WNDPROC, (LONG_PTR)WndProcOptionsTabs);

		TCITEMA tie = {0};
		tie.mask = TCIF_TEXT;

		tie.pszText = "General";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 0, (LPARAM)&tie);
		tie.pszText = "Content";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 1, (LPARAM)&tie);
		tie.pszText = "Sounds";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 2, (LPARAM)&tie);
		tie.pszText = "Voice";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 3, (LPARAM)&tie);

		SendMessage(hOptionsTabs, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);

		TabCtrl_SetCurSel(hOptionsTabs, 0);
		NMHDR hdr;
		hdr.hwndFrom = hOptionsTabs;
		hdr.code = TCN_SELCHANGE;
		SendMessage(hDlg, WM_NOTIFY, NULL, (LPARAM)&hdr);

		HWND buttonOK = CreateWindowEx(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 260, 200, 65, 21, hDlg, (HMENU)IDOK, 0, NULL);
		HWND buttonCancel = CreateWindowEx(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 330, 200, 65, 21, hDlg, (HMENU)IDCANCEL, 0, NULL);
		SendMessage(buttonOK, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SendMessage(buttonCancel, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		
		SetWindowText(hDlg, L"Options");
		place_dialog_center(hDlg, true);
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDOK) DestroyWindow(hDlg);
		break;
	case WM_HELP: {
		HELPINFO* hi = (HELPINFO*)lParam;
		if (hi->iContextType == HELPINFO_WINDOW) {
			int id = GetDlgCtrlID((HWND)hi->hItemHandle);
			if (id >= 6 && id <= 26) {
				if (GetFileAttributes(get_path(exe_path, L"help.hlp")) == INVALID_FILE_ATTRIBUTES ||
				!WinHelp(hDlg, exe_path, HELP_CONTEXTPOPUP, id)) {
					HH_POPUP hhp;
					hhp.cbStruct = sizeof(hhp);
					hhp.hinst = NULL;
					if (id == 19 || id == 20) hhp.idString = 18;
					else if (id == 22 || id == 23) hhp.idString = 21;
					else hhp.idString = id;
					wchar_t path[MAX_PATH];
					swprintf(path, L"%s::/popups.txt", get_path(exe_path, L"help.chm"));
					hhp.pszText = NULL;
					hhp.pt = hi->MousePos;
					hhp.pt.y += 18;
					hhp.clrForeground = (COLORREF)-1;
					hhp.clrBackground = (COLORREF)-1;
					hhp.rcMargins.left = 8;
					hhp.rcMargins.top = 8;
					hhp.rcMargins.right = 8;
					hhp.rcMargins.bottom = 8;
					hhp.pszFont = L"MS Sans Serif, 8";
					HtmlHelp(hDlg, path, HH_DISPLAY_TEXT_POPUP, (DWORD_PTR)&hhp);
				}
			}
		}
		return TRUE;
	}
	case WM_NOTIFY: {
		NMHDR* pNMHDR = (NMHDR*)lParam;
		if (pNMHDR->hwndFrom == hOptionsTabs && pNMHDR->code == TCN_SELCHANGE) {
			apply_edits();
			HWND child = GetWindow(hOptionsTabs, GW_CHILD);
			while (child != NULL) {
				HWND next = GetWindow(child, GW_HWNDNEXT); 
				DestroyWindow(child);
				child = next;
			}
			switch (TabCtrl_GetCurSel(hOptionsTabs)) {
			case 0: {
				HWND downloadInfo = CreateWindow(L"STATIC", L"Current download directory:", WS_CHILD | WS_VISIBLE, 10, 30, 370, 15, hOptionsTabs, (HMENU)6, NULL, NULL);
				downloadEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? WS_BORDER : 0) | WS_TABSTOP | ES_AUTOHSCROLL, 10, 45, 345, 20, hOptionsTabs, (HMENU)6, NULL, NULL);
				HWND browse = CreateWindowEx(0, L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 45, 20, 20, hOptionsTabs, (HMENU)10, 0, NULL);
				HWND trayCheckbox = CreateWindow(L"BUTTON", L"Minimize to tray when closing", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 10, 70, 370, 20, hOptionsTabs, (HMENU)11, NULL, NULL);
				HWND balloonCheckbox = CreateWindow(L"BUTTON", L"Use balloon notifications", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 10, 95, 370, 20, hOptionsTabs, (HMENU)12, NULL, NULL);
				SendMessage(downloadInfo, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(downloadEdit, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(browse, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(trayCheckbox, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(balloonCheckbox, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);

				wchar_t path[MAX_PATH];
				DWORD len = GetCurrentDirectory(MAX_PATH, path);
				SendMessage(downloadEdit, EM_REPLACESEL, FALSE, (LPARAM)path);
				SendMessage(downloadEdit, EM_SETSEL, 0, 0);
				
				if (CLOSETOTRAY) SendMessage(trayCheckbox, BM_SETCHECK, BST_CHECKED, 0);
				if (balloon_notifications) SendMessage(balloonCheckbox, BM_SETCHECK, BST_CHECKED, 0);
				if (!balloon_notifications_available) EnableWindow(balloonCheckbox, FALSE);
				if (nt3) {
					EnableWindow(trayCheckbox, FALSE);
					EnableWindow(browse, FALSE);
				}
				break;
			}
			case 1: {
				HWND grp = CreateWindow(L"BUTTON", L"How to load images in chats?", WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 10, 30, 370, 100, hOptionsTabs, NULL, NULL, NULL);
				HWND imagehwnds[3];
				imagehwnds[0] = CreateWindow(L"BUTTON", L"Don't load images at all", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP | WS_TABSTOP, 15, 50, 360, 20, hOptionsTabs, (HMENU)13, NULL, NULL);
				imagehwnds[1] = CreateWindow(L"BUTTON", L"Load just the minimal image thumbnails", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 15, 75, 360, 20, hOptionsTabs, (HMENU)14, NULL, NULL);
				imagehwnds[2] = CreateWindow(L"BUTTON", L"Autodownload and load the bigger sized image thumbnails", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_TABSTOP, 15, 100, 360, 20, hOptionsTabs, (HMENU)15, NULL, NULL);
				HWND emojiCheckbox = CreateWindow(L"BUTTON", L"Enable emojis", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 10, 135, 370, 20, hOptionsTabs, (HMENU)16, NULL, NULL);
				HWND spoilerCheckbox = CreateWindow(L"BUTTON", L"Apply appropriate formatting to spoilers in chats", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP, 10, 160, 370, 20, hOptionsTabs, (HMENU)17, NULL, NULL);
				SendMessage(grp, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(imagehwnds[0], WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(imagehwnds[1], WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(imagehwnds[2], WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(emojiCheckbox, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(spoilerCheckbox, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(imagehwnds[IMAGELOADPOLICY], BM_SETCHECK, BST_CHECKED, 0);
				if (EMOJIS) SendMessage(emojiCheckbox, BM_SETCHECK, BST_CHECKED, 0);
				if (SPOILERS) SendMessage(spoilerCheckbox, BM_SETCHECK, BST_CHECKED, 0);
				break;
			}
			case 2: {
				HWND sound1_label = CreateWindow(L"STATIC", L"New message notification sound:", WS_CHILD | WS_VISIBLE, 10, 30, 370, 15, hOptionsTabs, (HMENU)7, NULL, NULL);
				sound_edits[0] = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? WS_BORDER : 0) | WS_TABSTOP | ES_AUTOHSCROLL, 10, 45, 320, 20, hOptionsTabs, (HMENU)7, NULL, NULL);
				HWND sound1_button = CreateWindowEx(0, L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 335, 45, 20, 20, hOptionsTabs, (HMENU)18, 0, NULL);
				HWND sound1_buttonX = CreateWindowEx(0, L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 45, 20, 20, hOptionsTabs, (HMENU)21, 0, NULL);

				HWND sound2_label = CreateWindow(L"STATIC", L"Incoming message in the current chat sound:", WS_CHILD | WS_VISIBLE, 10, 70, 370, 15, hOptionsTabs, (HMENU)8, NULL, NULL);
				sound_edits[1] = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? WS_BORDER : 0) | WS_TABSTOP | ES_AUTOHSCROLL, 10, 85, 320, 20, hOptionsTabs, (HMENU)8, NULL, NULL);
				HWND sound2_button = CreateWindowEx(0, L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 335, 85, 20, 20, hOptionsTabs, (HMENU)19, 0, NULL);
				HWND sound2_buttonX = CreateWindowEx(0, L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 85, 20, 20, hOptionsTabs, (HMENU)22, 0, NULL);

				HWND sound3_label = CreateWindow(L"STATIC", L"Outgoing message sound:", WS_CHILD | WS_VISIBLE, 10, 110, 370, 15, hOptionsTabs, (HMENU)9, NULL, NULL);
				sound_edits[2] = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? WS_BORDER : 0) | WS_TABSTOP | ES_AUTOHSCROLL, 10, 125, 320, 20, hOptionsTabs, (HMENU)9, NULL, NULL);
				HWND sound3_button = CreateWindowEx(0, L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 335, 125, 20, 20, hOptionsTabs, (HMENU)20, 0, NULL);
				HWND sound3_buttonX = CreateWindowEx(0, L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 125, 20, 20, hOptionsTabs, (HMENU)23, 0, NULL);

				SendMessage(sound1_label, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound_edits[0], WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound1_button, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound1_buttonX, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound2_label, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound_edits[1], WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound2_button, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound2_buttonX, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound3_label, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound_edits[2], WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound3_button, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(sound3_buttonX, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);

				for (int i = 0; i < 3; i++) {
					if (sound_paths[i][0] == 0) SendMessage(sound_edits[i], EM_REPLACESEL, FALSE, (LPARAM)L"None");
					else SendMessage(sound_edits[i], EM_REPLACESEL, FALSE, (LPARAM)sound_paths[i]);
					SendMessage(sound_edits[i], EM_SETSEL, 0, 0);
				}
				break;
			}
			case 3: {
				HWND voice1_label = CreateWindow(L"STATIC", L"Sample rate:", WS_CHILD | WS_VISIBLE, 10, 30, 370, 15, hOptionsTabs, (HMENU)24, NULL, NULL);
				HWND comboSample = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP, 10, 45, 370, 100, hOptionsTabs, (HMENU)24, NULL, NULL);
				SendMessage(comboSample, CB_ADDSTRING, 0, (LPARAM)L"8000 Hz");
				SendMessage(comboSample, CB_ADDSTRING, 0, (LPARAM)L"16000 Hz");
				SendMessage(comboSample, CB_ADDSTRING, 0, (LPARAM)L"44100 Hz");
				SendMessage(comboSample, CB_ADDSTRING, 0, (LPARAM)L"48000 Hz");
				if (SAMPLERATE <= 8000) SendMessage(comboSample, CB_SETCURSEL, 0, 0);
				else if (SAMPLERATE <= 16000) SendMessage(comboSample, CB_SETCURSEL, 1, 0);
				else if (SAMPLERATE <= 44100) SendMessage(comboSample, CB_SETCURSEL, 2, 0);
				else SendMessage(comboSample, CB_SETCURSEL, 3, 0);

				HWND voice2_label = CreateWindow(L"STATIC", L"Bits per sample:", WS_CHILD | WS_VISIBLE, 10, 70, 370, 15, hOptionsTabs, (HMENU)25, NULL, NULL);
				HWND comboBits = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP, 10, 85, 370, 100, hOptionsTabs, (HMENU)25, NULL, NULL);
				SendMessage(comboBits, CB_ADDSTRING, 0, (LPARAM)L"8-bit");
				SendMessage(comboBits, CB_ADDSTRING, 0, (LPARAM)L"16-bit");
				SendMessage(comboBits, CB_ADDSTRING, 0, (LPARAM)L"24-bit");
				if (BITSPERSAMPLE <= 8) SendMessage(comboBits, CB_SETCURSEL, 0, 0);
				else if (BITSPERSAMPLE <= 16) SendMessage(comboBits, CB_SETCURSEL, 1, 0);
				else SendMessage(comboBits, CB_SETCURSEL, 2, 0);

				HWND voice3_label = CreateWindow(L"STATIC", L"Channels:", WS_CHILD | WS_VISIBLE, 10, 110, 370, 15, hOptionsTabs, (HMENU)26, NULL, NULL);
				HWND comboChannels = CreateWindow(L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP, 10, 125, 370, 100, hOptionsTabs, (HMENU)26, NULL, NULL);
				SendMessage(comboChannels, CB_ADDSTRING, 0, (LPARAM)L"Mono");
				SendMessage(comboChannels, CB_ADDSTRING, 0, (LPARAM)L"Stereo");
				if (CHANNELS <= 1) SendMessage(comboChannels, CB_SETCURSEL, 0, 0);
				else SendMessage(comboChannels, CB_SETCURSEL, 1, 0);

				SendMessage(voice1_label, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(comboSample, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(voice2_label, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(comboBits, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(voice3_label, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SendMessage(comboChannels, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
				SetFocus(hOptionsTabs);
				break;
			}
			}
		}
		break;
	}
	case WM_DRAWITEM: {
		DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
		TCITEM tci = {0};
		tci.mask = TCIF_TEXT;
		wchar_t text[8];
		tci.pszText = text;
		tci.cchTextMax = 8;
		SendMessage(hOptionsTabs, TCM_GETITEM, dis->itemID, (LPARAM)&tci);
		FillRect(dis->hDC, &dis->rcItem, (HBRUSH)GetStockObject(WHITE_BRUSH));
		SetBkColor(dis->hDC, RGB(255, 255, 255));
		TextOut(dis->hDC, dis->rcItem.left + ((dis->itemState & ODS_SELECTED) ? 8 : 4), dis->rcItem.top + ((dis->itemState & ODS_SELECTED) ? 3 : 1), text, wcslen(text));
		break;
	}
	case WM_CTLCOLORDLG:
		if (nt3) return (long)GetStockObject(WHITE_BRUSH);
		break;
	case WM_DESTROY:
		apply_edits();
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcInfo(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_INITDIALOG: {
		bool about = IsWindowVisible(hMain) ? true : false;
		infoLabel = CreateWindow(L"STATIC", about ? L"v1.0.0  |  Copyright © 2026 N3xtery  |  GNU GPL v3 License" : L"Connecting to the server...",
			WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 117, 384, 75, hDlg, NULL, NULL, NULL);
		SendMessage(infoLabel, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);

		infoBmp = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_LOGO));
		BITMAP bm;
		GetObject(infoBmp, sizeof(bm), &bm);  
		infoBmpMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);
		HDC hdcSrc = CreateCompatibleDC(NULL);
		HDC hdcDst = CreateCompatibleDC(NULL);
		HBITMAP hbmSrcT = SelectBitmap(hdcSrc, infoBmp);
		HBITMAP hbmDstT = SelectBitmap(hdcDst, infoBmpMask);
		COLORREF clrTopLeft = GetPixel(hdcSrc, 0, 0);
		COLORREF clrSaveBk = SetBkColor(hdcSrc, clrTopLeft);
		BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);
		COLORREF clrSaveDstText = SetTextColor(hdcSrc, RGB(255, 255, 255));
		SetBkColor(hdcSrc, RGB(0, 0, 0));
		BitBlt(hdcSrc, 0, 0, bm.bmWidth, bm.bmHeight, hdcDst, 0, 0, SRCAND);
		SetTextColor(hdcDst, clrSaveDstText);
		SetBkColor(hdcSrc, clrSaveBk);
		SelectBitmap(hdcSrc, hbmSrcT);
		SelectBitmap(hdcDst, hbmDstT);
		DeleteDC(hdcSrc);
		DeleteDC(hdcDst);

		place_dialog_center(hDlg, about);
		if (about) SetWindowText(hDlg, L"About Telegacy");
		else UpdateWindow(hDlg);
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) DestroyWindow(hDlg);
		break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hDlg,&ps);
		HDC hdcMem = CreateCompatibleDC(NULL);
		HBITMAP hbmOld = SelectBitmap(hdcMem, infoBmpMask);
		BITMAP bm;
		GetObject(infoBmpMask, sizeof(bm), &bm);
		BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCAND);
		SelectBitmap(hdcMem, infoBmp);
		BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCPAINT);
		SelectBitmap(hdcMem, hbmOld);
		DeleteDC(hdcMem);
		EndPaint(hDlg, &ps);
		break;
	}
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		if (nt3) return (long)GetStockObject(WHITE_BRUSH);
		break;
	case WM_DESTROY: {
		if (infoBmpMask) DeleteObject(infoBmpMask);
		infoBmp = NULL;
		infoBmpMask = NULL;
		if (current_info) current_info = NULL;
		else current_about = NULL;
	}
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_INITDIALOG: {
		Peer* peer = (Peer*)lParam;
		dlg_peer = *peer;
		SetWindowText(hDlg, peer == current_peer ? L"Profile" : L"My profile");
		name = CreateWindow(L"RichEdit20W", NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | WS_TABSTOP, 180, 10, 300, 25, hDlg, NULL, NULL, NULL);
		SendMessage(name, EM_LIMITTEXT, 64, 0);
		handle = CreateWindow(L"RichEdit20W", NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL | WS_TABSTOP, 180, 55, 120, 25, hDlg, NULL, NULL, NULL);
		SendMessage(handle, EM_LIMITTEXT, 32, 0);
		about = CreateWindow(L"RichEdit20W", NULL, WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | ES_MULTILINE | WS_VSCROLL | WS_TABSTOP, 310, 55, 170, 120, hDlg, NULL, NULL, NULL);
		SendMessage(about, EM_LIMITTEXT, peer->type == 0 ? 70 : 255, 0);
		COleCallback* coc_name = new COleCallback(name);
		SendMessage(name, EM_SETOLECALLBACK, 0, (LPARAM)coc_name);
		coc_name->Release();
		COleCallback* coc_about = new COleCallback(about);
		SendMessage(about, EM_SETOLECALLBACK, 0, (LPARAM)coc_about);
		coc_about->Release();
		CHARFORMAT2 cf;
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_FACE | CFM_BOLD;
		cf.dwEffects = 0;
		wcscpy(cf.szFaceName, L"Arial");
		SendMessage(about, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
		PARAFORMAT2 pf;
		pf.cbSize = sizeof(pf);
		pf.dwMask = PFM_ALIGNMENT;
		pf.wAlignment = PFA_CENTER;
		SendMessage(name, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
		SendMessage(handle, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
		HWND hLabelHandle = CreateWindow(L"STATIC", L"Handle", WS_CHILD | WS_VISIBLE | SS_CENTER, 180, 40, 120, 15, hDlg, NULL, NULL, NULL);
		HWND hLabelAbout = CreateWindow(L"STATIC", L"About", WS_CHILD | WS_VISIBLE | SS_CENTER, 310, 40, 170, 15, hDlg, NULL, NULL, NULL);
		HWND hLabelBirthday = CreateWindow(L"STATIC", L"Birthday", WS_CHILD | WS_VISIBLE | SS_CENTER, 180, 85, 120, 15, hDlg, NULL, NULL, NULL);
		SendMessage(hLabelHandle, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SendMessage(hLabelAbout, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SendMessage(hLabelBirthday, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		if (peer->type == 0 && (peer == &myself || read_le(peer->birthday, 4) != 0)) {
			SYSTEMTIME stRange[2] = {0};
			GetLocalTime(&stRange[0]);
			stRange[1].wYear = stRange[0].wYear;
			stRange[1].wMonth = 12;
			stRange[1].wDay = 31;
			stRange[0].wYear -= 150;
			if (ie3) {
				birthday = CreateWindow(DATETIMEPICK_CLASS, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | DTS_SHORTDATEFORMAT | DTS_SHOWNONE, 180, 100, 120, 25, hDlg, NULL, NULL, NULL);
				SendMessage(birthday, DTM_SETRANGE, GDTR_MIN | GDTR_MAX, (LPARAM)&stRange);
				SendMessage(birthday, DTM_SETMCFONT, (WPARAM)hDefaultFont, 0);
			} else birthday = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE | (nt3 ? WS_BORDER : 0), 180, 100, 120, 25, hDlg, NULL, NULL, NULL);
			HFONT hSystemFont = (HFONT)GetStockObject(SYSTEM_FONT);
			SendMessage(birthday, WM_SETFONT, (WPARAM)hSystemFont, 0);
			if (read_le(peer->birthday, 4) != 0) {
				SYSTEMTIME st = {0};
				st.wDay = peer->birthday[0];
				st.wMonth = peer->birthday[1];
				int year = read_le(peer->birthday + 2, 2);
				if (!year) SendMessage(birthday, DTM_SETFORMATA, 0, (LPARAM)"MMM dd");
				st.wYear = year ? year : stRange[1].wYear;
				if (ie3) SendMessage(birthday, DTM_SETSYSTEMTIME, GDT_VALID, (LPARAM)&st);
				else {
					wchar_t date[50];
					GetDateFormat(LOCALE_USER_DEFAULT, year ? DATE_SHORTDATE : NULL, &st, year ? NULL : L"MMM dd", date, 50);
					SendMessage(birthday, EM_REPLACESEL, FALSE, (LPARAM)date);
				}
			} else if (ie3) {
				SendMessage(birthday, DTM_SETFORMATA, 0, (LPARAM)"MMM dd");
				SendMessage(birthday, DTM_SETSYSTEMTIME, GDT_NONE, 0);
			}
		} else birthday = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE, 180, 100, 120, 25, hDlg, NULL, NULL, NULL);

		HWND hButtonOk = CreateWindow(L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 180, 145, 55, 25, hDlg, peer->perm.canchangedesc ? (HMENU)IDOK : (HMENU)IDCANCEL, NULL, NULL);
		HWND hButtonCancel = CreateWindow(L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 245, 145, 55, 25, hDlg, (HMENU)IDCANCEL, NULL, NULL);
		SendMessage(hButtonOk, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SendMessage(hButtonCancel, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		dlgPic = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, 10, 10, 160, 160, hDlg, NULL, NULL, NULL);
		SETTEXTEX st;
		st.flags = ST_DEFAULT;
		st.codepage = 1200;
		SendMessage(name, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)peer->name);
		int name_len = wcslen(peer->name);
		int deleted_wchars = 0;
		for (int i = 0; i < name_len; i++) i = emoji_adder(i, peer->name, 0, 15, name, &deleted_wchars);
		if (peer->handle) SendMessage(handle, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)peer->handle);
		if (peer->full && peer->about) {
			SendMessage(about, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)peer->about);
			int about_len = wcslen(peer->about);
			int deleted_wchars = 0;
			for (int i = 0; i < about_len; i++) i = emoji_adder(i, peer->about, 0, 15, about, &deleted_wchars);
		}
		if (read_le(peer->photo, 8) != 0) get_pfp(&dcInfoMain, peer);
		else {
			HDC hRefDC = GetDC(NULL);
			HDC memDC = CreateCompatibleDC(hRefDC);
			HBITMAP hBmp = CreateCompatibleBitmap(hRefDC, 160, 160);
			HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBmp);

			RECT rc = { 0, 0, 160, 160 };
			FillRect(memDC, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
			
			DrawText(memDC, L"(no profile picture)", -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

			SelectObject(memDC, oldBmp);
			DeleteDC(memDC);
			ReleaseDC(NULL, hRefDC);
			SendMessage(dlgPic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
		}
		SetFocus(birthday);
		SetFocus(dlgPic);
		if (!peer->perm.canchangedesc) {
			SendMessage(name, EM_SETREADONLY, TRUE, 0);
			SendMessage(handle, EM_SETREADONLY, TRUE, 0);
			SendMessage(about, EM_SETREADONLY, TRUE, 0);
			oldNameProc = (WNDPROC)SetWindowLongPtr(name, GWLP_WNDPROC, (LONG_PTR)WndProcDisabled);
			oldHandleProc = (WNDPROC)SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)WndProcDisabled);
			oldAboutProc = (WNDPROC)SetWindowLongPtr(about, GWLP_WNDPROC, (LONG_PTR)WndProcDisabled);
			oldBirthdayProc = (WNDPROC)SetWindowLongPtr(birthday, GWLP_WNDPROC, (LONG_PTR)WndProcDisabled);
			EnableWindow(dlgPic, FALSE);
		} else if (peer != &myself) {
			oldBirthdayProc = (WNDPROC)SetWindowLongPtr(birthday, GWLP_WNDPROC, (LONG_PTR)WndProcDisabled);
			if (peer->type == 1) {
				SendMessage(handle, EM_SETREADONLY, TRUE, 0);
				oldHandleProc = (WNDPROC)SetWindowLongPtr(handle, GWLP_WNDPROC, (LONG_PTR)WndProcDisabled);
			}
		} else if (!ie3) oldBirthdayProc = (WNDPROC)SetWindowLongPtr(birthday, GWLP_WNDPROC, (LONG_PTR)WndProcDisabled);

		place_dialog_center(hDlg, true);
		break;
	}
	case WM_NOTIFY: {
		NMHDR *hdr = (NMHDR*)lParam;
		if (hdr->code == DTN_DATETIMECHANGE) {
			LPNMDATETIMECHANGE p = (LPNMDATETIMECHANGE)lParam;
			if (p->dwFlags == GDT_VALID) {
				SYSTEMTIME st;
				GetLocalTime(&st);
				if (st.wYear == p->st.wYear) SendMessage(birthday, DTM_SETFORMAT, 0, (LPARAM)L"MMM dd");
				else {
					wchar_t default_format[16];
					GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SSHORTDATE, default_format, 16);
					SendMessage(birthday, DTM_SETFORMAT, 0, (LPARAM)default_format);
				}
			}
		}
		break;
	}
	case WM_COMMAND: {
		if (LOWORD(wParam) == IDOK) {
			int name_len = GetWindowTextLength(name);
			GETTEXTLENGTHEX gtl;
			gtl.flags = GTL_DEFAULT;
			gtl.codepage = 1200;
			int about_len = SendMessage(about, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
			int handle_len = GetWindowTextLength(handle);
			wchar_t name_str[256];
			wchar_t about_str[1024];
			wchar_t handle_str[33];
			GETTEXTEX gte = {0};
			gte.flags = GT_DEFAULT;
			gte.codepage = 1200;
			gte.cb = (name_len + 1) * 2;
			SendMessage(name, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)name_str);
			gte.cb = (about_len + 1) * 2;
			SendMessage(about, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)about_str);
			gte.cb = (handle_len + 1) * 2;
			SendMessage(handle, EM_GETTEXTEX, (WPARAM)&gte, (LPARAM)handle_str);
			BYTE new_birthday[4];
			if (ie3 && dlg_peer.type == 0) {
				SYSTEMTIME st;
				if (SendMessage(birthday, DTM_GETSYSTEMTIME, 0, (LPARAM)&st) == GDT_VALID) {
					new_birthday[0] = st.wDay;
					new_birthday[1] = st.wMonth;
					SYSTEMTIME st_current;
					GetLocalTime(&st_current);
					if (st_current.wYear != st.wYear) memcpy(new_birthday + 2, &st.wYear, 2);
					else memset(new_birthday + 2, 0, 4);
				} else memset(new_birthday, 0, 4);
			}
			writing_str_from = name;
			wide_to_utf8(name_str, (BYTE*)-1); // converts 0xFFFCs to actual emojis
			writing_str_from = about;
			wide_to_utf8(about_str, (BYTE*)-1);
			writing_str_from = msgInput;
			bool changed_name = wcscmp(name_str, dlg_peer.name) != 0;
			bool changed_about = dlg_peer.about ? wcscmp(about_str, dlg_peer.about) != 0 : about_str[0] != 0;
			bool changed_handle = dlg_peer.handle ? wcscmp(handle_str, dlg_peer.handle) != 0 : handle_str[0] != 0;
			bool changed_birthday = ie3 && dlg_peer.type == 0 && memcmp(dlg_peer.birthday, new_birthday, 4) != 0;
			if (changed_name || changed_about || changed_handle || changed_birthday) {
				BYTE unenc_query[512];
				BYTE enc_query[536];
				write_le(unenc_query + 32, 0x73f1f8dc, 4);
				write_le(unenc_query + 36, (dlg_peer.type == 0 ? (changed_name || changed_about) : (changed_name + changed_about)) + changed_handle + changed_birthday, 4);
				int offset = 40;
				if (dlg_peer.type == 0) {
					if (changed_name || changed_about) {
						create_msg_id(unenc_query + offset);
						create_seq_no(unenc_query + offset + 8, true);
						int offset_init = offset + 16;
						offset += 24;
						int flags = 0;
						if (changed_name) {
							write_string(unenc_query + offset, name_str);
							offset += tlstr_len(unenc_query + offset, true);
							memset(unenc_query + offset, 0, 4);
							offset += 4;
							flags += 3;
						}
						if (changed_about) {
							write_string(unenc_query + offset, about_str);
							offset += tlstr_len(unenc_query + offset, true);
							flags += 4;
							if (myself.about) free(myself.about);
							myself.about = _wcsdup(about_str);
						}
						write_le(unenc_query + offset_init, 0x78515775, 4);
						write_le(unenc_query + offset_init + 4, flags, 4);
						write_le(unenc_query + offset_init - 4, offset - offset_init, 4);
					}
					if (changed_handle) {
						create_msg_id(unenc_query + offset);
						create_seq_no(unenc_query + offset + 8, true);
						int offset_init = offset + 16;
						offset += 20;
						write_string(unenc_query + offset, handle_str);
						offset += tlstr_len(unenc_query + offset, true);
						write_le(unenc_query + offset_init, 0x3e0bdd7c, 4);
						write_le(unenc_query + offset_init - 4, offset - offset_init, 4);
					}
					if (changed_birthday) {
						create_msg_id(unenc_query + offset);
						create_seq_no(unenc_query + offset + 8, true);
						int offset_init = offset + 16;
						offset += 24;
						if (read_le(new_birthday, 4) != 0) {
							int year = read_le(new_birthday + 2, 2);
							bool year_set = year ? true : false;
							write_le(unenc_query + offset - 4, 1, 4);
							write_le(unenc_query + offset, 0x6c8e1e06, 4);
							write_le(unenc_query + offset + 4, year_set ? 1 : 0, 4);
							write_le(unenc_query + offset + 8, new_birthday[0], 4);
							write_le(unenc_query + offset + 12, new_birthday[1], 4);
							offset += 16;
							if (year_set) {
								write_le(unenc_query + offset + 16, year, 4);
								offset += 4;
							}
						} else memset(unenc_query + offset - 4, 0, 4);
						write_le(unenc_query + offset_init, 0xcc6e0c11, 4);
						write_le(unenc_query + offset_init - 4, offset - offset_init, 4);
						memcpy(myself.birthday, new_birthday, 4);
					}
				} else {
					if (changed_name) {
						writing_str_from = name;
						create_msg_id(unenc_query + offset);
						create_seq_no(unenc_query + offset + 8, true);
						offset += 16;
						int offset_init = offset;
						if (dlg_peer.type == 1) {
							write_le(unenc_query + offset, 0x73783ffd, 4);
							memcpy(unenc_query + offset + 4, dlg_peer.id, 8);
							offset += 12;
						} else {
							write_le(unenc_query + offset, 0x566decd0, 4);
							offset += 4 + place_peer(unenc_query + offset + 4, &dlg_peer, true);
						}
						write_string(unenc_query + offset, name_str);
						offset += tlstr_len(unenc_query + offset, true);
						write_le(unenc_query + offset_init - 4, offset - offset_init, 4);
					}
					if (changed_about) {
						writing_str_from = about;
						create_msg_id(unenc_query + offset);
						create_seq_no(unenc_query + offset + 8, true);
						offset += 16;
						int offset_init = offset;
						write_le(unenc_query + offset, 0xdef60797, 4);
						offset += 4 + place_peer(unenc_query + offset + 4, &dlg_peer, true);
						write_string(unenc_query + offset, about_str);
						for (int i = 0; i < peers_count; i++) {
							if (memcmp(peers[i].id, dlg_peer.id, 8) == 0) {
								if (peers[i].about) free(peers[i].about);
								peers[i].about = _wcsdup(about_str);
								break;
							}
						}
						offset += tlstr_len(unenc_query + offset, true);
						write_le(unenc_query + offset_init - 4, offset - offset_init, 4);
					}
					if (changed_handle) {
						create_msg_id(unenc_query + offset);
						memcpy(handle_msgid, unenc_query + offset, 8);
						dlg_peer.handle = _wcsdup(handle_str);
						create_seq_no(unenc_query + offset + 8, true);
						offset += 16;
						int offset_init = offset;
						write_le(unenc_query + offset, 0x3514b3de, 4);
						offset += 4 + place_peer(unenc_query + offset + 4, &dlg_peer, true);
						write_string(unenc_query + offset, handle_str);
						offset += tlstr_len(unenc_query + offset, true);
						write_le(unenc_query + offset_init - 4, offset - offset_init, 4);
					}
				}
				internal_header(unenc_query, false);
				write_le(unenc_query + 28, offset - 32, 4);
				int padding_len = get_padding(offset);
				fortuna_read(unenc_query + offset, padding_len, &prng);
				offset += padding_len;
				convert_message(unenc_query, enc_query, offset, 0);
				send_query(enc_query, offset + 24);
			}
			DestroyWindow(hDlg);
		} else if (LOWORD(wParam) == IDCANCEL) DestroyWindow(hDlg);
		else if (HIWORD(wParam) == STN_CLICKED) {
			OPENFILENAME ofn = {0};
			ofn.lStructSize = sizeof(OPENFILENAME);
			wchar_t file_name[MAX_PATH] = {0};
			file_name[0] = 0;
			ofn.lpstrTitle = L"Update profile picture";
			ofn.lpstrFile = file_name;
			ofn.nMaxFile = sizeof(file_name);
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR;
			ofn.lpstrFilter = L"Image Files (*.bmp;*.jpg;*.jpeg;*.png)\0*.bmp;*.jpg;*.jpeg;*.png\0";
			if (GetOpenFileName(&ofn)) {
				// adjusting for filesenderworker
				for (int i = 0; i < files.size(); i++) free(files[i]);
				files.clear();
				SendMessage(hToolbar, TB_CHANGEBITMAP, 4, MAKELPARAM(10, 0));
				InvalidateRect(hToolbar, NULL, TRUE);
				int len = wcslen(file_name);
				wchar_t* str = _wcsdup(file_name);
				files.push_back(str);
				Document* docstemp = (Document*)malloc(sizeof(Document));
				docstemp->min = 123;
				fortuna_read(docstemp->id, 8, &prng);
				docstemp->filename = files[0];

				BYTE unenc_query[256];
				internal_header(unenc_query, true);
				
				int offset = 32;
				if (dlg_peer.type == 0) {
					write_le(unenc_query + offset, 0x388a3b5, 4);
					offset += 4;
				} else {
					if (dlg_peer.type == 1) {
						write_le(unenc_query + offset, 0x35ddd674, 4);
						memcpy(unenc_query + offset + 4, dlg_peer.id, 8);
						offset += 12;
					} else {
						write_le(unenc_query + offset, 0xf12e57c9, 4);
						offset += 4 + place_peer(unenc_query + offset + 4, &dlg_peer, true);
					}
					write_le(unenc_query + offset, 0xbdcdaec0, 4);
					offset += 4;
				}
				write_le(unenc_query + offset, 1, 4);
				write_le(unenc_query + offset + 4, 0xf52ff27f, 4);
				memcpy(unenc_query + offset + 8, docstemp->id, 8);
				int parts_offset = offset + 16;
				write_string(unenc_query + offset + 20, docstemp->filename);
				offset += 20 + tlstr_len(unenc_query + offset + 20, true);
				FILE* f = _wfopen(str, L"rb");
				write_md5(unenc_query + offset, f);
				offset += 36;
				docstemp->size = _ftelli64(f);
				fclose(f);
				write_le(unenc_query + parts_offset, ceil(docstemp->size / 524288.0), 4);
				write_le(unenc_query + 28, offset - 32, 4);
				int padding_len = get_padding(offset);
				fortuna_read(unenc_query + offset, padding_len, &prng);
				offset += padding_len;
				BYTE* enc_query = (BYTE*)malloc(offset + 32);
				convert_message(unenc_query, enc_query + 8, offset, 0);
				write_le(enc_query + 4, offset + 24, 4);
				write_le(enc_query, (int)docstemp, 4);
				unsigned threadID;
				_beginthreadex(NULL, 0, FileSenderWorker, enc_query, 0, &threadID);
				DestroyWindow(hDlg);
			}
		}

        return TRUE;
	}
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		if (nt3) return (long)GetStockObject(WHITE_BRUSH);
		break;
	case WM_DESTROY:
		if (dlgPic) {
			HBITMAP hBmp = (HBITMAP)SendMessage(dlgPic, STM_GETIMAGE, IMAGE_BITMAP, 0);
			DeleteObject(hBmp);
		}
		current_dialog = NULL;
		return TRUE;
    }
    return FALSE;
}

LRESULT CALLBACK WndProcReactionButton(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	WNDPROC oldProc = (WNDPROC)GetProp(hWnd, L"oldproc");
    switch (msg) {
	case WM_LBUTTONDOWN:
		if (GetParent(hWnd) == reactionStatic) dontlosefocus = true;
		break;
	case WM_SETFOCUS:
		return 0;
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_NCDESTROY:
		RemoveProp(hWnd, L"oldproc");
	}
	return CallWindowProc(oldProc, hWnd, msg, wParam, lParam);
}

char get_current_lang() {
	LANGID langId = LOWORD(GetKeyboardLayout(GetWindowThreadProcessId(GetForegroundWindow(), NULL)));
	if (langId == 0x0404) return 0; // traditional chinese
	else if (langId == 0x0804) return 1; // simplified chinese
	else if (langId == 0x0411) return 2; // japanese
	else if (langId == 0x0412) return 3; // korean
	else return -1;
}

LRESULT CALLBACK WndProcMsgInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static int ime_str_start, ime_str_len = 0;
	static bool ime_composition = false;
	static char ime_lang;
	static bool typing = false;
	switch (msg) {
	case WM_CHAR:
	case EM_SETTEXTEX: 
		if (!current_peer || editing_msg_id || wParam == 13 || (current_peer->type == 0 && current_peer->online == -1)) break;
		if (!typing) SetTimer(hWnd, 0, 0, NULL);
		SetTimer(hWnd, 1, 2000, NULL);
		break;
	case WM_TIMER:
		if (wParam == 0) {
			// messages.setTyping
			typing = true;
			set_typing(0x16bf744e, 0);
			SetTimer(hWnd, 0, 5000, NULL);
		} else if (wParam == 1) {
			typing = false;
			KillTimer(hWnd, 0);
			KillTimer(hWnd, 1);
		} else if (wParam == 2) { // recording voice
			set_typing(0xd52f73f7, 0);
			SetTimer(hWnd, 2, 5000, NULL);
		} else if (wParam == 3) { // voice
			set_typing(0xf351d7ab, 4);
			SetTimer(hWnd, 3, 5000, NULL);
		} else if (wParam == 4) { // file
			set_typing(0xaa0cd9e4, 4);
			SetTimer(hWnd, 4, 5000, NULL);
		} else if (wParam == 5) { // video
			set_typing(0xe9763aec, 4);
			SetTimer(hWnd, 5, 5000, NULL);
		} else if (wParam == 6) { // photo
			set_typing(0xd1d34a26, 4);
			SetTimer(hWnd, 6, 5000, NULL);
		}
		break;
	case WM_KEYDOWN:
		if (wParam == VK_RETURN && !(GetKeyState(VK_SHIFT) & 0x8000)) {
			SendMessage(hMain, WM_COMMAND, MAKEWPARAM(1, 0), NULL);
			return 0;
		}
		break;
	case WM_IME_NOTIFY: {
		if (wParam == IMN_OPENCANDIDATE && (ime_lang == 0 || ime_lang == 2)) {
			HIMC hIMC = NULL;
			g_pAIMM->GetContext(hWnd, &hIMC);
			POINT pt;
			GetCaretPos(&pt);
			CANDIDATEFORM cand;
			cand.dwIndex = 0;
			if (ime_lang == 2) {
				cand.dwStyle = CFS_EXCLUDE;
				cand.ptCurrentPos.x = pt.x;
				cand.ptCurrentPos.y = pt.y;
				cand.rcArea.top = pt.y - 10;
				cand.rcArea.bottom = pt.y+25;
				cand.rcArea.left = pt.x-100;
				cand.rcArea.right = pt.x+100;
			} else cand.dwStyle = CFS_DEFAULT;
			g_pAIMM->SetCandidateWindow(hIMC, &cand);
			g_pAIMM->ReleaseContext(hWnd, hIMC);
		}
		break;
	}
	case WM_IME_COMPOSITION: {
		HIMC hIMC;
		g_pAIMM->GetContext(hWnd, &hIMC);
		if (lParam & GCS_RESULTSTR) {
			ime_lang = get_current_lang();
			if (ime_lang == 0 || (!ime_composition && ime_lang == 2) || ime_lang == 3) {
				CHARRANGE cr;
				SendMessage(msgInput, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
				if (cr.cpMax - cr.cpMin != 1 && ime_lang == 3) return 0;
				long size = 0;
				g_pAIMM->GetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, &size, 0);
				wchar_t buf[3];
                g_pAIMM->GetCompositionStringW(hIMC, GCS_RESULTSTR, size, &size, &buf);
				size /= 2;
                buf[size] = 0;
				SETTEXTEX st;
				st.flags = ST_SELECTION;
				st.codepage = 1200;
				SendMessage(msgInput, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)buf);
				ime_str_start++;
				ime_composition = false;
			}
		} else if (ime_lang != 0) {
            long size = 0;
            g_pAIMM->GetCompositionStringW(hIMC, GCS_COMPSTR, NULL, &size, 0);
            wchar_t* buf = (wchar_t*)malloc(size+2);
            g_pAIMM->GetCompositionStringW(hIMC, GCS_COMPSTR, size, &size, buf);
			long pos = 0;
			g_pAIMM->GetCompositionStringW(hIMC, GCS_CURSORPOS, 0, &pos, NULL);
			long clausesLen = 0;
			g_pAIMM->GetCompositionStringW(hIMC, GCS_COMPCLAUSE, 0, &clausesLen, NULL);			
			size /= 2;

			char offset = 0;
			if (ime_lang == 1) {
				if (buf[0] == 12288) {
					size--;
					offset++;
					if (size == 0) return 0;
				}
				if (buf[size-1+offset] == 12288) size--;
			}
            buf[size] = 0;
			
			if (ime_lang != 3) {
				SendMessage(msgInput, EM_HIDESELECTION, TRUE, 0);
				SendMessage(msgInput, EM_SETSEL, ime_str_start, ime_str_start+ime_str_len);
			}

			SETTEXTEX st;
			st.flags = ST_SELECTION;
			st.codepage = 1200;
			SendMessage(msgInput, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)(buf+offset));
			free(buf);
			ime_str_len = size;

			int end = pos;
			if (size != 0 && ime_lang == 3) pos++;

			if (ime_lang != 3) {
				CHARFORMAT2 cf;
				cf.cbSize = sizeof(cf);
				cf.dwMask = CFM_UNDERLINETYPE;
				cf.bUnderlineType = CFU_UNDERLINEDOTTED;
				SendMessage(msgInput, EM_SETSEL, ime_str_start, ime_str_start+ime_str_len);
				SendMessage(msgInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

				if (clausesLen > 0) {
					int* clauses = (int*)malloc(clausesLen);
					g_pAIMM->GetCompositionStringW(hIMC, GCS_COMPCLAUSE, clausesLen, &clausesLen, clauses);
					clausesLen /= 4;
					for (int i = 0; i < clausesLen; i++) {
						if (clauses[i] > pos && clauses[i] <= size) {
							end = clauses[i];
							break;
						}
					}
				}
			}
			
			SendMessage(msgInput, EM_SETSEL, ime_str_start+pos, ime_str_start+end);
			if (ime_lang != 3) SendMessage(msgInput, EM_HIDESELECTION, FALSE, 0);
		} else {
			POINT pt;
			GetCaretPos(&pt);
			COMPOSITIONFORM cf;
			cf.dwStyle = CFS_POINT;
			cf.ptCurrentPos.x = pt.x;
			cf.ptCurrentPos.y = pt.y;
			g_pAIMM->SetCompositionWindow(hIMC, &cf);
			g_pAIMM->ReleaseContext(hWnd, hIMC);
			break;
		}
		g_pAIMM->ReleaseContext(hWnd, hIMC);
		return 0;
	}
	case WM_IME_STARTCOMPOSITION: {
		ime_composition = true;
		CHARRANGE cr;
		SendMessage(msgInput, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
		ime_str_start = cr.cpMin;
		ime_lang = get_current_lang();
		if (ime_lang == 0) break;
		return 0;
	}
	case WM_IME_ENDCOMPOSITION: {
		if (ime_lang == 0 || ime_lang == 3) break;
		CHARRANGE cr;
		SendMessage(msgInput, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
		SendMessage(msgInput, EM_HIDESELECTION, TRUE, 0);
		SendMessage(msgInput, EM_SETSEL, ime_str_start, ime_str_start+ime_str_len);
		CHARFORMAT cf;
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_UNDERLINE;
		cf.dwEffects = 0;
		SendMessage(msgInput, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		SendMessage(msgInput, EM_HIDESELECTION, FALSE, 0);
		SendMessage(msgInput, EM_SETSEL, cr.cpMax, cr.cpMax);
		ime_str_len = 0;
		ime_composition = false;
		return 0;
	}
	case WM_CAPTURECHANGED: {
		if (ime_composition) {
			HIMC hIMC;
			g_pAIMM->GetContext(hWnd, &hIMC);
			g_pAIMM->NotifyIME(hIMC, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
			g_pAIMM->ReleaseContext(hWnd, hIMC);
		}
		break;
	}
	}
	LRESULT lResult;
	if (g_pAIMM == NULL || g_pAIMM->OnDefWindowProc(hWnd, msg, wParam, lParam, &lResult) != S_OK)
		lResult = CallWindowProc(oldMsgInputProc, hWnd, msg, wParam, lParam);
	return lResult;
}

LRESULT CALLBACK WndProcChat(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_SETTEXT: {
		messages.clear();
		for (int i = 0; i < links.size(); i++) free(links[i].lpstrText);
		links.clear();

		for (i = 0; i < documents.size(); i++) {
			free(documents[i].filename);
			free(documents[i].file_reference);
		}
		documents.clear();

		memset(group_id_tofront, 0, 8);

		if (replying_msg_id) SendMessage(hMain, WM_COMMAND, MAKEWPARAM(7, 0), 0);
		if (editing_msg_id) SendMessage(hMain, WM_COMMAND, MAKEWPARAM(8, 0), 0);
		break;
	}
	case WM_RBUTTONDOWN: {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(hWnd, &pt);
		unsigned int last_visible_char = SendMessage(hWnd, EM_CHARFROMPOS, 0, (LPARAM)&pt);
		SendMessage(chat, EM_SETSEL, last_visible_char, last_visible_char);
		HideCaret(hWnd);
		break;
	}
	case WM_PAINT:
	case WM_LBUTTONUP:
		HideCaret(hWnd);
		break;
	case WM_KEYDOWN:
		switch (wParam) {
		case VK_LEFT: case VK_RIGHT: case VK_UP: case VK_DOWN:
		case VK_HOME: case VK_END: case VK_PRIOR: case VK_NEXT:
			return 0;
		}
		break;
	case WM_MOUSEWHEEL:
	case WM_VSCROLL: {
		if (msg == WM_VSCROLL) {
			WORD submsg = LOWORD(wParam);
			if (submsg == SB_THUMBTRACK) ischatscrolling = true;
			else if (submsg == SB_THUMBPOSITION) ischatscrolling = false;
			if (submsg != SB_ENDSCROLL) break;
		}
		RECT rect;
		SendMessage(hWnd, EM_GETRECT, 0, (LPARAM)&rect);
		POINTL point;
		point.x = rect.left;
		point.y = rect.bottom;
		unsigned int last_visible_char = SendMessage(hWnd, EM_CHARFROMPOS, 0, (LPARAM)&point);
		bool sent_seen = false;
		int unread_msgs_count_old = current_peer->unread_msgs_count;
		for (int i = messages.size() - 1; i >= 0; i--) {
			if (!sent_seen && !messages[i].outgoing && !messages[i].seen && messages[i].end_char <= last_visible_char) {
				make_seen(&messages[i]);
				sent_seen = true;
				current_peer->unread_msgs_count--;
			} else if (sent_seen && !messages[i].outgoing && !messages[i].seen) {
				messages[i].seen = true;
				current_peer->unread_msgs_count--;
			} else if (!messages[i].outgoing && messages[i].seen) break;
		}
		if (unread_msgs_count_old != current_peer->unread_msgs_count && !current_peer->mute_until && !muted_types[current_peer->type]) {
			update_total_unread_msgs_count(current_peer->unread_msgs_count - unread_msgs_count_old);
			if (!current_peer->unread_msgs_count) remove_notification();
		}
		if (!no_more_msgs) {
			if (msg == WM_MOUSEWHEEL) {
				SCROLLINFO si = {0};
				si.cbSize = sizeof(si);
				si.fMask = SIF_POS | SIF_PAGE;
				GetScrollInfo(chat, SB_VERT, &si);
				int direction = GET_WHEEL_DELTA_WPARAM(wParam);
				if (direction > 0) si.nPos -= 35;
				else si.nPos += 35;
				if (si.nPos <= 0) get_history();
			} else if (SendMessage(chat, EM_GETFIRSTVISIBLELINE, 0, 0) == 0) get_history();
		}
		break;
	}
	}
	return CallWindowProc(oldChatProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProcEmojiScrollFallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_VSCROLL: {
		SendMessage(hTabs, WM_VSCROLL, wParam, lParam);
		break;
	}
	}
	return CallWindowProc(oldEmojiScrollFallbackProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProcEmojiScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_VSCROLL: {
		HWND hWnd = (HWND)lParam;
		SCROLLINFO si = {0};
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_CTL, &si);

		int oldPos = si.nPos;
		switch (LOWORD(wParam)) {
			case SB_TOP:		si.nPos = si.nMin; break;
			case SB_BOTTOM:		si.nPos = si.nMax; break;
			case SB_LINEUP:     si.nPos -= 20; break;
			case SB_LINEDOWN:   si.nPos += 20; break;
			case SB_PAGEUP:     si.nPos -= si.nPage; break;
			case SB_PAGEDOWN:   si.nPos += si.nPage; break;
			case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
		}
		si.nPos = max(si.nMin, min(si.nMax - si.nPage + 1, si.nPos));
		si.fMask = SIF_POS;
		SetScrollInfo (hWnd, SB_CTL, &si, TRUE);
		GetScrollInfo(hWnd, SB_CTL, &si);
		
		if (si.nPos != oldPos) ScrollWindowEx(emojiStatic, 0, oldPos - si.nPos, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
		break;
	}
	}
	return CallWindowProc(oldEmojiScrollProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProcEmojiStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_COMMAND:
		if (LOWORD(wParam) == 1) {
			wchar_t* code = (wchar_t*)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
			wchar_t file_name[MAX_PATH];
			swprintf(file_name, L"%s\\%s.ico", get_path(exe_path, L"emojis"), code);
			insert_emoji(file_name, 15, msgInput);
			wchar_t* code_new;
			bool first = false, present = false;
			for (int i = 0; i < fav_emojis.size(); i++) {
				if (wcscmp(fav_emojis[i], code) == 0) {
					if (i == 0) first = true;
					else {
						code_new = fav_emojis[i];
						fav_emojis.erase(fav_emojis.begin() + i);
						present = true;
					}
					break;
				}
			}
			if (!first) {
				if (!present) {
					code_new = _wcsdup(code);
				}
				fav_emojis.push_front(code_new);
				if (fav_emojis.size() > 40) {
					free(fav_emojis[40]);
					fav_emojis.pop_back();
				}
			}
			SetFocus(msgInput);
		}
		break;
	case WM_DRAWITEM:
		paint_emoji_button((DRAWITEMSTRUCT*)lParam);
		return TRUE;
		break;
	}
	return CallWindowProc(oldEmojiStaticProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProcReactionStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_MOUSEACTIVATE:
		return MA_NOACTIVATE;
	case WM_COMMAND:
		if (LOWORD(wParam) == 1) {
			SetForegroundWindow(hMain);
			int sel_msg = -1;
			for (int i = messages.size() - 1; i >= 0; i--) if (messages[i].id == sel_msg_id) {
				sel_msg = i;
				break;
			}
			if (sel_msg == -1) break;
			wchar_t* code_init = (wchar_t*)GetWindowLongPtr((HWND)lParam, GWLP_USERDATA);
			BYTE unenc_query[192];
			internal_header(unenc_query, true);
			write_le(unenc_query + 32, 0xd30d78d4, 4);
			write_le(unenc_query + 36, 1, 4);
			int offset = 40 + place_peer(unenc_query + 40, current_peer, true);
			write_le(unenc_query + offset, messages[sel_msg].id, 4);
			write_le(unenc_query + offset + 4, 0x1cb5c415, 4);
			int count_offset = offset + 8;
			offset += 12;
			bool unreact = false;
			int written_count = 0;
			int condition = messages[sel_msg].reacted.size() + 1;
			for (i = 0; i < condition; i++) {
				if (i != condition - 1 && messages[sel_msg].reacted[i] == code_init) {
					unreact = true;
					messages[sel_msg].reacted.erase(messages[sel_msg].reacted.begin() + i);
					condition--;
					i--;
					continue;
				}
				if (i == messages[sel_msg].reacted.size() && unreact) continue;
				if (i != condition - 1) continue;
				wchar_t* code = (i == messages[sel_msg].reacted.size()) ? code_init : messages[sel_msg].reacted[i];
				if (code[0] == 1) {
					write_le(unenc_query + offset, 0x8935fc73, 4);
					offset += 4;
					__int64 id;
					swscanf(code + 1, L"%I64X", &id);
					memcpy(unenc_query + offset, &id, 8);
					offset += 8;
				} else if (wcscmp(code, L"2b50") == 0) {
					write_le(unenc_query + offset, 0x523da4eb, 4);
					offset += 4;
				} else {
					write_le(unenc_query + offset, 0x1b2286b8, 4);
					offset += 4;
					int str_pos = 0;
					while (true) {
						wchar_t* end_ptr = wcschr(code, L'-');
						int number = (int)wcstol(code, &end_ptr, 16);
						if (number != 0xFE0F) wide_to_utf8_one(number, &str_pos, unenc_query + offset + 1);
						if (end_ptr[0] != L'\0') code = end_ptr + 1;
						else break;
					}
					unenc_query[offset] = str_pos;
					int padding_str = (4 - (str_pos + 1) % 4) % 4;
					memset(unenc_query + offset + 1 + str_pos, 0, padding_str);
					offset += 1 + str_pos + padding_str;
				}
				written_count++;
			}
			if (written_count > 0) write_le(unenc_query + count_offset, written_count, 4);
			else {
				offset -= 8;
				unenc_query[36] = 0;
			}
			write_le(unenc_query + 28, offset - 32, 4);
			int padding_len = get_padding(offset);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			BYTE enc_query[216];
			convert_message(unenc_query, enc_query, offset + padding_len, 0);
			send_query(enc_query, 24 + offset + padding_len);
			ShowWindow(reactionStatic, SW_HIDE);
			SetFocus(msgInput);
		}
		break;
	case WM_DRAWITEM:
		paint_emoji_button((DRAWITEMSTRUCT*)lParam);
		return TRUE;
		break;
	}
	return CallWindowProc(oldEmojiStaticProc, hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK WndProcSplitter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_LBUTTONDOWN:
		dragging = true;
		SetCapture(hWnd);
		break;
	case WM_MOUSEMOVE:
		if (dragging) {
			int y = GET_Y_LPARAM(lParam);
			POINT pt = {0, y};
			ClientToScreen(hWnd, &pt);
			ScreenToClient(hMain, &pt);
			if (pt.y < 85 || pt.y > height - 85) break;
			edits_border_offset = pt.y - (height - 123);
			bool cantwrite = current_peer && !current_peer->perm.cansendmsg;
			HDWP hdwp = BeginDeferWindowPos(7);
			hdwp = DeferWindowPos(hdwp, chat, NULL, 10, 40, width - 20, cantwrite ? height - 70 : (height - 165 + edits_border_offset), SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, msgInput, NULL, 10, height - 95 + edits_border_offset, width - 20, 65 - edits_border_offset, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, tbSeparatorHider, NULL, width / 2, height - 118 + edits_border_offset, 35, 21, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hTabs, NULL, width - 224, height - 334 + edits_border_offset, 214, 214, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hOverlayTabs, NULL, width - 224, height - 334 + edits_border_offset, 214, 214, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, hToolbar, NULL, 10, height - (ie3 ? 120 : 124) + edits_border_offset, width - 20, 25, SWP_NOZORDER);
			hdwp = DeferWindowPos(hdwp, splitter, NULL, 10, height - 124 + edits_border_offset, width - 20, 3, SWP_NOZORDER);
			EndDeferWindowPos(hdwp);
			SCROLLINFO si = {0};
			si.cbSize = sizeof(si);
			si.fMask = SIF_POS | SIF_PAGE | SIF_RANGE;
			GetScrollInfo(chat, SB_VERT, &si);
			if (si.nPos + si.nPage - si.nMax < 5) SendMessage(chat, EM_SCROLL, SB_LINEDOWN, 0);
			if (current_peer) SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
		}
		break;
	case WM_LBUTTONUP:
		if (dragging) {
			dragging = false;
			ReleaseCapture();
		}
		break;
	}
	return CallWindowProc(oldSplitterProc, hWnd, msg, wParam, lParam);
}

bool pass_pushed = false;
INT_PTR CALLBACK DlgProc2FA(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_INITDIALOG: {
		HWND h2FALbl = CreateWindow(L"STATIC", L"Enter your password:", WS_CHILD | WS_VISIBLE, 10, 10, 155, 15, hDlg, NULL, NULL, NULL);
		h2FA = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_PASSWORD | WS_TABSTOP | (nt3 ? WS_BORDER : 0), 10, 25, 125, 25, hDlg, NULL, NULL, NULL);
		SendMessage(h2FA, EM_LIMITTEXT, 63, 0);
		hPass = CreateWindow(L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | (nt3 ? BS_OWNERDRAW : BS_ICON), 140, 25, 25, 25, hDlg, (HMENU)5, NULL, NULL);
		if (!nt3) {
			HIMAGELIST hImg = ImageList_LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ICONS), 16, 0, RGB(128, 0, 128));
			HICON hIcon = ImageList_GetIcon(hImg, 6, ILD_NORMAL);
			ImageList_Destroy(hImg);
			SendMessage(hPass, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		}

		h2FAHint = CreateWindow(L"BUTTON", L"Hint?", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 28, 60, 55, 25, hDlg, (HMENU)3, NULL, NULL);
		HWND h2FABtn = CreateWindow(L"BUTTON", L"Log in", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 93, 60, 55, 25, hDlg, (HMENU)4, NULL, NULL);
		SendMessage(h2FALbl, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SendMessage(h2FA, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SendMessage(h2FABtn, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SendMessage(h2FAHint, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
		SetWindowText(hDlg, L"2FA authentication");
		place_dialog_center(hDlg, false);
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL) DestroyWindow(hDlg);
		else if (LOWORD(wParam) == 3 || LOWORD(wParam) == 4) {
			if (LOWORD(wParam) == 3 && hint) MessageBox(hDlg, hint, L"Hint", MB_OK | MB_ICONINFORMATION);
			else {
				if (LOWORD(wParam) == 3) hint_needed = true;
				BYTE unenc_query[48];
				BYTE enc_query[72];
				internal_header(unenc_query, true);
				write_le(unenc_query + 28, 4, 4);
				write_le(unenc_query + 32, 0x548a30f5, 4);
				fortuna_read(unenc_query + 36, 12, &prng);
				convert_message(unenc_query, enc_query, 48, 0);
				send_query(enc_query, 72);
			}
		} else if (LOWORD(wParam) == 5) {
			if (pass_pushed) SendMessage(h2FA, EM_SETPASSWORDCHAR, (WPARAM)'*', NULL);
			else SendMessage(h2FA, EM_SETPASSWORDCHAR, NULL, NULL);
			InvalidateRect(h2FA, NULL, TRUE);
			pass_pushed = !pass_pushed;
		}
		break;
	case WM_DRAWITEM: {
		DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
		if (dis->itemAction == ODA_DRAWENTIRE) {
			HBITMAP hIcons = LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ICONS));

			HDC hdcRef = GetDC(NULL);
			HDC hdc = CreateCompatibleDC(hdcRef);
			HDC hdcMask = CreateCompatibleDC(hdcRef);

			HBITMAP hBmp = CreateCompatibleBitmap(hdcRef, 16, 16);
			HBITMAP hBmpMask = CreateBitmap(16, 16, 1, 1, NULL);

			HBITMAP hBmpOld = SelectBitmap(hdc, hBmp);
			HBITMAP hBmpMaskOld = SelectBitmap(hdcMask, hIcons);
			BitBlt(hdc, 0, 0, 16, 16, hdcMask, 96, 0, SRCCOPY);
			SelectBitmap(hdcMask, hBmpMask);

			COLORREF clrSaveBk = SetBkColor(hdc, RGB(128, 0, 128));
			BitBlt(hdcMask, 0, 0, 16, 16, hdc, 0, 0, SRCCOPY);

			COLORREF clrSaveDstText = SetTextColor(hdc, RGB(255, 255, 255));
			SetBkColor(hdc, RGB(0, 0, 0));
			BitBlt(hdc, 0, 0, 16, 16, hdcMask, 0, 0, SRCAND);
			SetTextColor(hdcMask, clrSaveDstText);
			SetBkColor(hdc, clrSaveBk);

			BitBlt(dis->hDC, 4, 4, 16, 16, hdcMask, 0, 0, SRCAND);
			BitBlt(dis->hDC, 4, 4, 16, 16, hdc, 0, 0, SRCPAINT);

			SelectBitmap(hdc, hBmpOld);
			SelectBitmap(hdcMask, hBmpMaskOld);
			DeleteDC(hdc);
			DeleteDC(hdcMask);
			DeleteObject(hBmp);
			DeleteObject(hBmpMask);
			ReleaseDC(NULL, hdcRef);
		}
		break;
	}
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		if (nt3) return (long)GetStockObject(WHITE_BRUSH);
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		DestroyWindow(hMain);
		return TRUE;
	case WM_DESTROY: {
		if (!nt3) {
			HICON hIcon = (HICON)SendMessage(hPass, BM_GETIMAGE, IMAGE_ICON, NULL);
			if (hIcon) DestroyIcon(hIcon);
		}
		if (hint) free(hint);
	}
	}
	return FALSE;
}

INT_PTR CALLBACK DlgProcLogin(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {
		if (!hNumber) {
			SetWindowText(hDlg, L"Log in");
			HWND hLabelPhone = CreateWindow(L"STATIC", L"Your phone number:", WS_CHILD | WS_VISIBLE, 10, 10, 120, 15, hDlg, NULL, NULL, NULL);
			HWND hLabelCode = CreateWindow(L"STATIC", L"Code:", WS_CHILD | WS_VISIBLE, 10, 55, 120, 15, hDlg, NULL, NULL, NULL);
			HWND hLabelQr = CreateWindow(L"STATIC", L"Or, scan this QR code to log in:", WS_CHILD | WS_VISIBLE, 10, 100, 190, 15, hDlg, NULL, NULL, NULL);
			hNumber = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | (nt3 ? WS_BORDER : 0), 10, 25, 110, 25, hDlg, NULL, NULL, NULL);
			hNumberBtn = CreateWindow(L"BUTTON", L"Get code", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 125, 25, 54, 25, hDlg, (HMENU)3, NULL, NULL);
			hCode = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_TABSTOP | (nt3 ? WS_BORDER : 0), 10, 70, 110, 25, hDlg, NULL, NULL, NULL);
			hCodeBtn = CreateWindow(L"BUTTON", L"Log in", WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_TABSTOP, 125, 70, 54, 25, hDlg, (HMENU)4, NULL, NULL);
			hQRCode = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, 10, 115, 165, 165, hDlg, NULL, NULL, NULL);
			SendMessage(hLabelPhone, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
			SendMessage(hLabelCode, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
			SendMessage(hLabelQr, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
			SendMessage(hNumber, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
			SendMessage(hCode, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
			SendMessage(hNumberBtn, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);
			SendMessage(hCodeBtn, WM_SETFONT, (WPARAM)hDefaultFont, TRUE);

			SendMessage(hNumber, EM_LIMITTEXT, 19, 0);
			SendMessage(hCode, EM_LIMITTEXT, 5, 0);
			HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		}
		needtosetuplogin = true;
		send_ping(&dcInfoMain);
		return TRUE;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == 3) {
			int num_len = GetWindowTextLength(hNumber);
			if (num_len == 0) {
				MessageBox(hDlg, L"Enter your phone number!", L"Error", MB_OK | MB_ICONERROR);
				return TRUE;
			}
			wchar_t number[20];
			GetWindowText(hNumber, number, num_len+1);

			int num_tlstr_len = str_to_tlstr_len(number);

			BYTE unenc_query[128];
			BYTE enc_query[152];
			internal_header(unenc_query, true);
			write_le(unenc_query + 32, 0xa677244f, 4);

			write_string(unenc_query + 36, number);
			if (phone_number_bytes != NULL) free(phone_number_bytes);
			phone_number_bytes = (BYTE*)malloc(num_tlstr_len + 1);
			memcpy(phone_number_bytes, unenc_query + 36, num_tlstr_len);

			int offset = 36 + num_tlstr_len;
			write_le(unenc_query + offset, 27752131, 4);
			write_string(unenc_query + offset + 4, L"f60c7955c55ad59d438d007f1fd59c0d");
			write_le(unenc_query + offset + 40, 0xad253d78, 4);
			memset(unenc_query + offset + 44, 0, 4);

			offset += 48;
			write_le(unenc_query + 28, offset - 32, 4);
			int padding_len = get_padding(offset);
			fortuna_read(unenc_query + offset, padding_len, &prng);
			offset += padding_len;
			convert_message(unenc_query, enc_query, offset, 0);
			send_query(enc_query, offset + 24);
			return TRUE;
		} else if (LOWORD(wParam) == 4) {
			// auth.signin
			int code_len = GetWindowTextLength(hCode);
			wchar_t code[6];
			GetWindowText(hCode, code, code_len+1);
			int code_tlstr_len = str_to_tlstr_len(code);
			int num_tlstr_len = tlstr_len(phone_number_bytes, true);
			int unenc_query_len = 60 + num_tlstr_len + code_tlstr_len;
			int padding_len = get_padding(unenc_query_len);
			unenc_query_len += padding_len;

			BYTE unenc_query[112];
			BYTE enc_query[136];
			internal_header(unenc_query, true);
			write_le(unenc_query + 28, 36 + num_tlstr_len, 4);
			write_le(unenc_query + 32, 0x8d52a951, 4);
			write_le(unenc_query + 36, 1, 4);
			memcpy(unenc_query + 40, phone_number_bytes, num_tlstr_len);
			memcpy(unenc_query + 40 + num_tlstr_len, phone_code_hash, 20);
			write_string(unenc_query + 60 + num_tlstr_len, code);
			fortuna_read(unenc_query + 60 + num_tlstr_len + code_tlstr_len, padding_len, &prng);
			convert_message(unenc_query, enc_query, unenc_query_len, 0);
			send_query(enc_query, unenc_query_len + 24);
		}
		break;
	case WM_TIMER:
		if (wParam == 1) {
			BYTE unenc_query[96];
			BYTE enc_query[120];
			internal_header(unenc_query, true);
			write_le(unenc_query + 28, 52, 4);
			write_le(unenc_query + 32, 0xb7e085fe, 4);
			write_le(unenc_query + 36, 27752131, 4);
			write_string(unenc_query + 40, L"f60c7955c55ad59d438d007f1fd59c0d");
			write_le(unenc_query + 76, 0x1cb5c415, 4);
			memset(unenc_query + 80, 0, 4);
			fortuna_read(unenc_query + 84, 12, &prng);
			convert_message(unenc_query, enc_query, 96, 0);
			send_query(enc_query, 120);
			KillTimer(hDlg, 1);
		}
		break;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		if (nt3) return (long)GetStockObject(WHITE_BRUSH);
		break;
	case WM_CLOSE:
		DestroyWindow(hDlg);
		DestroyWindow(hMain);
		return TRUE;
	case WM_DESTROY: {
		KillTimer(hDlg, 1);
		HBITMAP oldBmp = (HBITMAP)SendMessage(hQRCode, STM_GETIMAGE, IMAGE_BITMAP, 0);
		if (oldBmp) DeleteObject(oldBmp);
		break;
	}
	}
	return FALSE;
}

int init_connection(DCInfo* dcInfo, bool socketworkerreconnect) {
	if (!current_info && dcInfo == &dcInfoMain && !socketworkerreconnect) {
		BYTE buffer[24] = {0};
		LONG dlgUnits = GetDialogBaseUnits();
		DLGTEMPLATE *dlg = (DLGTEMPLATE*)buffer;
		dlg->style = WS_POPUP | DS_MODALFRAME;
		dlg->cx = MulDiv(384, 4, LOWORD(dlgUnits));
		dlg->cy = MulDiv(143, 8, HIWORD(dlgUnits));
		current_info = CreateDialogIndirect(GetModuleHandle(NULL), dlg, hMain, DlgProcInfo);
	}

	// connect to telegram data center
	WSADATA wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);
	dcInfo->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	sockaddr_in server;
	server.sin_family = AF_INET;

	FILE* f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"rb");
	if (f) {
		if (!dcInfo->dc) fread(&dcInfo->dc, 4, 1, f);
		char ip[16];
		fseek(f, 8 + (dcInfo->dc - 1) * 20, SEEK_SET);
		fread(ip, 1, 16, f);
		int port;
		fread(&port, 4, 1, f);
		fclose(f);
		server.sin_port = htons(port);
		server.sin_addr.S_un.S_addr = inet_addr(ip);
	} else {
		dcInfo->dc = 2;
		server.sin_port = htons(443);
		server.sin_addr.S_un.S_addr = inet_addr(test_server ? "149.154.167.40" : "149.154.167.50");
	}

	if (connect(dcInfo->sock, (sockaddr*)&server, sizeof(server))) {
		MessageBox(NULL, L"Couldn't connect to a Telegram DC!", L"Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	// sending the transport header
	BYTE byte = 0xef;
	send(dcInfo->sock, (char*)&byte, 1, 0);
	return 1;
}

void reconnect(DCInfo* dcInfo) {
	closesocket(dcInfo->sock);
	WSACleanup();
	if (init_connection(dcInfo, false)) create_auth_key(dcInfo);
}

void create_auth_key(DCInfo* dcInfo) {
	if (current_info) SetWindowText(infoLabel, L"Creating an authorization key...");

	// query
	BYTE req_pq_multi[40];

	// auth
	memset(req_pq_multi, 0, 8);

	// msg_id
	create_msg_id(dcInfo, req_pq_multi + 8);

	// msg_len
	write_le(req_pq_multi + 16, 20, 4);

	// constructor
	write_le(req_pq_multi + 20, 0xbe7e8ef1, 4);

	// nonce
	BYTE nonce[16];
	fortuna_read(nonce, 16, &prng);
	memcpy(req_pq_multi + 24, nonce, 16);

	// sending data
	send_query(dcInfo, req_pq_multi, 40);
	
	// receiving data
	BYTE resPQ[100];
	recv(dcInfo->sock, (char*)resPQ, 1, 0);
	recv(dcInfo->sock, (char*)resPQ, 100, 0);

	// time synchronization
	time_diff = read_le(resPQ + 12, 4) - time(NULL);
	
	// checking nonce
	if (memcmp(nonce, resPQ + 24, 16) != 0) {
		if (current_info) SetWindowText(infoLabel, L"Nonce mismatch 1! Reconnecting...");
		reconnect(dcInfo);
		return;
	}

	// reading server nonce
	BYTE server_nonce[16];
	memcpy(server_nonce, resPQ + 40, 16);
	
	// reading fingerprint
	BYTE fingerprint[8];
	memcpy(fingerprint, resPQ + 76, 8);
	
	// decomposing pq into p and q
	unsigned __int64 pq = read_be(resPQ + 57, 8);
	unsigned __int64 p = pq_factorize(pq);
	unsigned __int64 q = pq / p;
	
	BYTE pq_inner_data[192];
	
	// constructor
	write_le(pq_inner_data, 0xa9f55f95, 4);

	// pq
	memcpy(pq_inner_data + 4, resPQ + 56, 12);

	// p
	pq_inner_data[16] = 4;
	write_be(pq_inner_data + 17, p, 4);
	memset(pq_inner_data + 21, 0, 3);

	// q
	pq_inner_data[24] = 4;
	write_be(pq_inner_data + 25, q, 4);
	memset(pq_inner_data + 29, 0, 3);

	// nonce
	memcpy(pq_inner_data + 32, nonce, 16);

	// server nonce
	memcpy(pq_inner_data + 48, server_nonce, 16);

	// new nonce
	BYTE new_nonce[32];
	fortuna_read(new_nonce, 32, &prng);
	memcpy(pq_inner_data + 64, new_nonce, 32);

	// dc
	write_le(pq_inner_data + 96, (test_server ? 10000 : 0) + dcInfo->dc, 4);
	
	// random padding
	fortuna_read(pq_inner_data + 100, 92, &prng);
	
	// reverse order
	BYTE pq_inner_data_rev[224];
	for (int i = 0; i < 192; i++) {
		pq_inner_data_rev[i] = pq_inner_data[191-i];
	}
	
	BYTE hash_input[224];

	// generating random temp key
	BYTE temp_key[32];
	fortuna_read(temp_key, 32, &prng);
	memcpy(hash_input, temp_key, 32);
	memcpy(hash_input + 32, pq_inner_data, 192);

	sha256_init(&md);
	sha256_process(&md, hash_input, 224);
	sha256_done(&md, pq_inner_data_rev + 192);

	BYTE zero_iv[32] = {0};
	BYTE aes_encrypted[224];
	aes_ige(pq_inner_data_rev, aes_encrypted, 224, temp_key, zero_iv, 1);

	BYTE aes_hash[32];
	sha256_init(&md);
	sha256_process(&md, aes_encrypted, 224);
	sha256_done(&md, aes_hash);

	for (i = 0; i < 32; i++) {
		temp_key[i] = temp_key[i] ^ aes_hash[i];
	}
	
	BYTE key_aes_encrypted[256];
	memcpy(key_aes_encrypted, temp_key, 32);
	memcpy(key_aes_encrypted + 32, aes_encrypted, 224);

	init_LTM();
	rsa_key pubkey;
	rsa_import(test_server ? pubkey_der_test : pubkey_der, pubkey_der_len, &pubkey);
	BYTE req_dh_params[340];
	unsigned long rsa_len = 340;
	rsa_exptmod(key_aes_encrypted, 256, req_dh_params+84, &rsa_len, PK_PUBLIC, &pubkey);

	memset(req_dh_params, 0, 8);

	create_msg_id(dcInfo, req_dh_params + 8);

	write_le(req_dh_params + 16, 320, 4);

	write_le(req_dh_params + 20, 0xd712e4be, 4);

	// nonce
	memcpy(req_dh_params + 24, nonce, 16);

	// server nonce
	memcpy(req_dh_params + 40, server_nonce, 16);

	// p
	req_dh_params[56] = 4;
	write_be(req_dh_params + 57, p, 4);
	memset(req_dh_params + 61, 0, 3);

	// q
	req_dh_params[64] = 4;
	write_be(req_dh_params + 65, q, 4);
	memset(req_dh_params + 69, 0, 3);

	memcpy(req_dh_params + 72, fingerprint, 8);

	write_be(req_dh_params + 80, 0xfe000100, 4);

	// sending data
	send_query(dcInfo, req_dh_params, 340);
	
	// receiving length
	BYTE server_dh_params[652];
	recv(dcInfo->sock, (char*)server_dh_params, 4, 0);

	// receiving server_dh_params
	recv(dcInfo->sock, (char*)server_dh_params, 652, 0);
	
	// checking nonce
	if (memcmp(nonce, server_dh_params + 24, 16) != 0 || memcmp(server_nonce, server_dh_params + 40, 16) != 0) {
		if (current_info) SetWindowText(infoLabel, L"Nonce mismatch 2! Reconnecting...");
		reconnect(dcInfo);
		return;
	}

	BYTE hash_input1[48];
	BYTE tmp1[20];
	memcpy(hash_input1, new_nonce, 32);
	memcpy(hash_input1 + 32, server_nonce, 16);
	sha1_init(&md);
	sha1_process(&md, hash_input1, 48);
	sha1_done(&md, tmp1);

	BYTE hash_input2[48];
	BYTE tmp2[20];
	memcpy(hash_input2, server_nonce, 16);
	memcpy(hash_input2 + 16, new_nonce, 32);
	sha1_init(&md);
	sha1_process(&md, hash_input2, 48);
	sha1_done(&md, tmp2);

	BYTE hash_input3[64];
	BYTE tmp3[20];
	memcpy(hash_input3, new_nonce, 32);
	memcpy(hash_input3 + 32, new_nonce, 32);
	sha1_init(&md);
	sha1_process(&md, hash_input3, 64);
	sha1_done(&md, tmp3);

	BYTE tmp_aes_key[32];
	memcpy(tmp_aes_key, tmp1, 20);
	memcpy(tmp_aes_key + 20, tmp2, 12);

	BYTE tmp_aes_iv[32];
	memcpy(tmp_aes_iv, tmp2 + 12, 8);
	memcpy(tmp_aes_iv + 8, tmp3, 20);
	memcpy(tmp_aes_iv + 28, new_nonce, 4);
	
	BYTE aes_decrypted[592];
	aes_ige(server_dh_params + 60, aes_decrypted, 592, tmp_aes_key, tmp_aes_iv, 0);

	// checking nonce
	if (memcmp(nonce, aes_decrypted + 24, 16) != 0 || memcmp(server_nonce, aes_decrypted + 40, 16) != 0) {
		reconnect(dcInfo);
		return;
	}
	
	// checking sha1
	BYTE tmp4[20];
	sha1_init(&md);
	sha1_process(&md, aes_decrypted + 20, 564);
	sha1_done(&md, tmp4);
	if (memcmp(tmp4, aes_decrypted, 20) != 0) {
		if (current_info) SetWindowText(infoLabel, L"SHA1 mismatch! Reconnecting...");
		reconnect(dcInfo);
		return;
	}

	BYTE b_bytes[256];
	fortuna_read(b_bytes, 256, &prng);

	mp_int g, dh_prime, b, g_b, g_a, ak, temp, temp2;
	mp_init_multi(&g, &dh_prime, &b, &g_b, &g_a, &ak, &temp, &temp2, NULL);
	mp_read_unsigned_bin(&dh_prime, aes_decrypted + 64, 256);
	mp_set_int(&g, 3);
	mp_read_unsigned_bin(&g_a, aes_decrypted + 324, 256);
	mp_read_unsigned_bin(&b, b_bytes, 256);
	mp_exptmod(&g, &b, &dh_prime, &g_b);
	
	// dh_prime checks
	unsigned char dh_prime_default[] = { 0xC7, 0x1C, 0xAE, 0xB9, 0xC6, 0xB1, 0xC9, 0x04, 0x8E, 0x6C, 0x52, 0x2F, 0x70, 0xF1, 0x3F, 0x73, 0x98, 0x0D, 0x40, 0x23, 0x8E, 0x3E, 0x21, 0xC1, 0x49, 0x34, 0xD0, 0x37, 0x56, 0x3D, 0x93, 0x0F, 0x48, 0x19, 0x8A, 0x0A, 0xA7, 0xC1, 0x40, 0x58, 0x22, 0x94, 0x93, 0xD2, 0x25, 0x30, 0xF4, 0xDB, 0xFA, 0x33, 0x6F, 0x6E, 0x0A, 0xC9, 0x25, 0x13, 0x95, 0x43, 0xAE, 0xD4, 0x4C, 0xCE, 0x7C, 0x37, 0x20, 0xFD, 0x51, 0xF6, 0x94, 0x58, 0x70, 0x5A, 0xC6, 0x8C, 0xD4, 0xFE, 0x6B, 0x6B, 0x13, 0xAB, 0xDC, 0x97, 0x46, 0x51, 0x29, 0x69, 0x32, 0x84, 0x54, 0xF1, 0x8F, 0xAF, 0x8C, 0x59, 0x5F, 0x64, 0x24, 0x77, 0xFE, 0x96, 0xBB, 0x2A, 0x94, 0x1D, 0x5B, 0xCD, 0x1D, 0x4A, 0xC8, 0xCC, 0x49, 0x88, 0x07, 0x08, 0xFA, 0x9B, 0x37, 0x8E, 0x3C, 0x4F, 0x3A, 0x90, 0x60, 0xBE, 0xE6, 0x7C, 0xF9, 0xA4, 0xA4, 0xA6, 0x95, 0x81, 0x10, 0x51, 0x90, 0x7E, 0x16, 0x27, 0x53, 0xB5, 0x6B, 0x0F, 0x6B, 0x41, 0x0D, 0xBA, 0x74, 0xD8, 0xA8, 0x4B, 0x2A, 0x14, 0xB3, 0x14, 0x4E, 0x0E, 0xF1, 0x28, 0x47, 0x54, 0xFD, 0x17, 0xED, 0x95, 0x0D, 0x59, 0x65, 0xB4, 0xB9, 0xDD, 0x46, 0x58, 0x2D, 0xB1, 0x17, 0x8D, 0x16, 0x9C, 0x6B, 0xC4, 0x65, 0xB0, 0xD6, 0xFF, 0x9C, 0xA3, 0x92, 0x8F, 0xEF, 0x5B, 0x9A, 0xE4, 0xE4, 0x18, 0xFC, 0x15, 0xE8, 0x3E, 0xBE, 0xA0, 0xF8, 0x7F, 0xA9, 0xFF, 0x5E, 0xED, 0x70, 0x05, 0x0D, 0xED, 0x28, 0x49, 0xF4, 0x7B, 0xF9, 0x59, 0xD9, 0x56, 0x85, 0x0C, 0xE9, 0x29, 0x85, 0x1F, 0x0D, 0x81, 0x15, 0xF6, 0x35, 0xB1, 0x05, 0xEE, 0x2E, 0x4E, 0x15, 0xD0, 0x4B, 0x24, 0x54, 0xBF, 0x6F, 0x4F, 0xAD, 0xF0, 0x34, 0xB1, 0x04, 0x03, 0x11, 0x9C, 0xD8, 0xE3, 0xB9, 0x2F, 0xCC, 0x5B };
	if (memcmp(dh_prime_default, aes_decrypted + 64, 256) != 0) {
		if (current_info) SetWindowText(infoLabel, L"dh_prime mismatch, so doing checks now :(");
		int result = 0;
		unsigned int result2 = 0;
		if (mp_count_bits(&dh_prime) != 2048) goto dh_prime_fail;
		mp_sub_d(&dh_prime, 1, &temp);
		mp_div_2(&temp, &temp);
		mp_prime_is_prime(&dh_prime, 10, &result);
		if (!result) goto dh_prime_fail;
		mp_prime_is_prime(&temp, 10, &result);
		if (!result) goto dh_prime_fail;
		mp_mod_d(&dh_prime, 3, &result2);
		if (result2 != 2) goto dh_prime_fail;
		if (0) {
dh_prime_fail:
			if (current_info) SetWindowText(infoLabel, L"dh_prime didn't pass the checks! Reconnecting...");
			mp_clear_multi(&g, &dh_prime, &b, &g_b, &g_a, &ak, &temp, &temp2, NULL);
			reconnect(dcInfo);
			return;
		}
		
		// g, g_a, g_b checks
		mp_mul_2(&temp, &temp);
		if (mp_cmp_d(&g, 1) != MP_GT || mp_cmp_d(&g_a, 1) != MP_GT || mp_cmp_d(&g_b, 1) != MP_GT ||
			mp_cmp(&g, &temp) != MP_LT || mp_cmp(&g_a, &temp) != MP_LT ||  mp_cmp(&g_b, &temp) != MP_LT) {
			if (current_info) SetWindowText(infoLabel, L"g/g_a/g_b checks failed! Reconnecting...");
			mp_clear_multi(&g, &dh_prime, &b, &g_b, &g_a, &ak, &temp, &temp2, NULL);
			reconnect(dcInfo);
			return;
		}

		// g_a, g_b additional checks
		mp_2expt(&temp, 2048-64);
		mp_sub(&dh_prime, &temp, &temp2);
		if (mp_cmp(&g_a, &temp) != MP_GT || mp_cmp(&g_b, &temp) != MP_GT ||
			mp_cmp(&g_a, &temp2) != MP_LT || mp_cmp(&g_b, &temp2) != MP_LT) {
			if (current_info) SetWindowText(infoLabel, L"g_a/g_b additional checks failed! Reconnecting...");
			mp_clear_multi(&g, &dh_prime, &b, &g_b, &g_a, &ak, &temp, &temp2, NULL);
			reconnect(dcInfo);
			return;
		}
	}

	BYTE client_dh_inner_data[304];
	write_le(client_dh_inner_data, 0x6643b654, 4);
	memcpy(client_dh_inner_data + 4, nonce, 16);
	memcpy(client_dh_inner_data + 20, server_nonce, 16);
	memset(client_dh_inner_data + 36, 0, 8);
	write_be(client_dh_inner_data + 44, 0xfe000100, 4);
	mp_to_unsigned_bin(&g_b, client_dh_inner_data + 48);

	BYTE data_with_hash[336];
	sha1_init(&md);
	sha1_process(&md, client_dh_inner_data, 304);
	sha1_done(&md, data_with_hash);

	memcpy(data_with_hash + 20, client_dh_inner_data, 304);
	fortuna_read(data_with_hash + 324, 12, &prng);

	BYTE set_client_dh_params[396];
	aes_ige(data_with_hash, set_client_dh_params + 60, 336, tmp_aes_key, tmp_aes_iv, 1);

	memset(set_client_dh_params, 0, 8);
	
	create_msg_id(dcInfo, set_client_dh_params + 8);

	write_le(set_client_dh_params + 16, 376, 4);
	write_le(set_client_dh_params + 20, 0xf5045f1f, 4);

	memcpy(set_client_dh_params + 24, nonce, 16);
	memcpy(set_client_dh_params + 40, server_nonce, 16);
	write_be(set_client_dh_params + 56, 0xfe500100, 4);

	send_query(dcInfo, set_client_dh_params, 396);
	
	// receiving data
	BYTE dh_gen_ok[72];
	recv(dcInfo->sock, (char*)dh_gen_ok, 1, 0);
	recv(dcInfo->sock, (char*)dh_gen_ok, 72, 0);

	// checking nonce
	if (memcmp(nonce, dh_gen_ok + 24, 16) != 0 || memcmp(server_nonce, dh_gen_ok + 40, 16) != 0) {
		if (current_info) SetWindowText(infoLabel, L"Nonce mismatch 4! Reconnecting...");
		reconnect(dcInfo);
		return;
	}

	mp_exptmod(&g_a, &b, &dh_prime, &ak);
	mp_to_unsigned_bin(&ak, dcInfo->auth_key);
	mp_clear_multi(&g, &dh_prime, &b, &g_b, &g_a, &ak, &temp, &temp2, NULL);

	BYTE tmp5[20];
	sha1_init(&md);
	sha1_process(&md, dcInfo->auth_key, 256);
	sha1_done(&md, tmp5);
	memcpy(dcInfo->auth_key_id, tmp5 + 12, 8);

	for (i = 0; i < 8; i++) {
		dcInfo->server_salt[i] = new_nonce[i] ^ server_nonce[i];
	}

	fortuna_read(dcInfo->session_id, 8, &prng);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	if (GetEnvironmentVariable(L"APPDATA", appdata_path, MAX_PATH) == 0) {
		GetWindowsDirectory(appdata_path, MAX_PATH);
		wcscat(appdata_path, L"\\Application Data");
		if (GetFileAttributes(appdata_path) == INVALID_FILE_ATTRIBUTES) CreateDirectory(appdata_path, NULL);
	}
	wcscat(appdata_path, L"\\Telegacy\0");
	GetModuleFileName(NULL, exe_path, MAX_PATH);

	if (wcsstr((wchar_t*)lpCmdLine, L"/uninstall")) {
		int result = MessageBox(NULL, L"Delete user data?", L"Confirm", MB_YESNO | MB_ICONQUESTION);
		if (result == IDYES) {
			SHFILEOPSTRUCT file_op = {NULL, FO_DELETE, appdata_path, NULL, FOF_NOCONFIRMATION | FOF_NOERRORUI, false, 0, L""};
			SHFileOperation(&file_op);
		}
		return 0;
	} else if (wcsstr((wchar_t*)lpCmdLine, L"/progman")) {
		DWORD idInst = 0;
		if (DdeInitialize(&idInst, NULL, APPCMD_CLIENTONLY, 0) != DMLERR_NO_ERROR) return 1;
		HSZ service_topic = DdeCreateStringHandle(idInst, L"Progman", CP_WINUNICODE);
		if (!service_topic) return 1;
		HCONV hConv = DdeConnect(idInst, service_topic, service_topic, NULL);
		if (!hConv) return 1;
		
		wchar_t* cmdStr = L"[DeleteGroup(\"Telegacy\")]";
		DdeClientTransaction((LPBYTE)cmdStr, (wcslen(cmdStr) + 1) * 2, hConv, 0, 0, XTYP_EXECUTE, 1000, NULL);

		if (wcsstr((wchar_t*)lpCmdLine, L"_install")) {
			wchar_t* cmdStr = L"[CreateGroup(\"Telegacy\")]";
			DdeClientTransaction((LPBYTE)cmdStr, (wcslen(cmdStr) + 1) * 2, hConv, 0, 0, XTYP_EXECUTE, 1000, NULL);
			wchar_t cmdStr2[MAX_PATH];
			swprintf(cmdStr2, L"[AddItem(\"%s\",\"Telegacy\")]", exe_path);
			DdeClientTransaction((LPBYTE)cmdStr2, (wcslen(cmdStr2) + 1) * 2, hConv, 0, 0, XTYP_EXECUTE, 1000, NULL);
			swprintf(cmdStr2, L"[AddItem(\"%s\",\"Uninstall Telegacy\")]", get_path(exe_path, L"uninstall.exe"));
			DdeClientTransaction((LPBYTE)cmdStr2, (wcslen(cmdStr2) + 1) * 2, hConv, 0, 0, XTYP_EXECUTE, 1000, NULL);
		}

		DdeDisconnect(hConv);
		DdeFreeStringHandle(idInst, service_topic);
		DdeUninitialize(idInst);
		return 0;
	}

	HWND existing_instance = FindWindow(L"Telegacy", NULL);
	if (existing_instance) {
		SetForegroundWindow(existing_instance);
		return 0;
	}

	InitializeCriticalSection(&csSock);
	InitializeCriticalSection(&csCM);
	if (GetFileAttributes(appdata_path) == INVALID_FILE_ATTRIBUTES) {
		CreateDirectory(appdata_path, NULL);
		wcscat(appdata_path, L"\\custom_emojis");
		CreateDirectory(appdata_path, NULL);
		*(wcsrchr(appdata_path, L'\\') + 1) = 0;
	} else wcscat(appdata_path, L"\\");

	width = GetPrivateProfileInt(L"General", L"width", 500, get_path(appdata_path, L"options.ini"));
	height = GetPrivateProfileInt(L"General", L"height", 450, appdata_path);
	if (GetPrivateProfileInt(L"General", L"maximized", 0, appdata_path)) maximized = true;
	edits_border_offset = GetPrivateProfileInt(L"General", L"edits_border_offset", 0, appdata_path);
	wchar_t download_path[MAX_PATH];
	GetPrivateProfileString(L"General", L"download_path", L"", download_path, MAX_PATH, appdata_path);
	if (!download_path[0]) {
		HKEY hKey;
		if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			DWORD type = REG_SZ;
			DWORD size = MAX_PATH;
			if (RegQueryValueEx(hKey, L"Desktop", NULL, &type, (LPBYTE)download_path, &size) != ERROR_SUCCESS) wcscpy(download_path, L"C:\\");
			RegCloseKey(hKey);
		}
	}
	SetCurrentDirectory(download_path);
	if (!GetPrivateProfileInt(L"General", L"close_to_tray", 1, appdata_path)) CLOSETOTRAY = false;
	if (!GetPrivateProfileInt(L"General", L"balloon_notifications", 1, appdata_path)) balloon_notifications = false;

	IMAGELOADPOLICY = GetPrivateProfileInt(L"Content", L"image_load_policy", 2, appdata_path);
	if (!GetPrivateProfileInt(L"Content", L"emojis", 1, appdata_path)) EMOJIS = false;
	if (!GetPrivateProfileInt(L"Content", L"spoilers", 1, appdata_path)) SPOILERS = false;

	GetPrivateProfileString(L"Sounds", L"notification_sound", L"1", sound_paths[0], MAX_PATH, appdata_path);
	GetPrivateProfileString(L"Sounds", L"incoming_sound", L"", sound_paths[1], MAX_PATH, appdata_path);
	GetPrivateProfileString(L"Sounds", L"outgoing_sound", L"", sound_paths[2], MAX_PATH, appdata_path);
	if (sound_paths[0][0] == L'1') {
		HKEY hKey;
		if (RegOpenKeyEx(HKEY_CURRENT_USER, L"AppEvents\\Schemes\\Apps\\.Default\\MailBeep\\.Current", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
			unsigned long size = MAX_PATH;
			RegQueryValueEx(hKey, NULL, NULL, NULL, (BYTE*)sound_paths[0], &size);
			ExpandEnvironmentStrings(sound_paths[0], sound_paths[0], MAX_PATH);
			RegCloseKey(hKey);
		} else sound_paths[0][0] = 0;
	}

	SAMPLERATE = GetPrivateProfileInt(L"Voice", L"sample_rate", 16000, appdata_path);
	BITSPERSAMPLE = GetPrivateProfileInt(L"Voice", L"bits_per_sample", 16, appdata_path);
	CHANNELS = GetPrivateProfileInt(L"Voice", L"channels", 1, appdata_path);

	// random number generator
	HCRYPTPROV hProv;
	BYTE entropy[36];
	bool cryptapi = true;
	HINSTANCE hCryptLib = LoadLibraryA("advapi32.dll");
	if (hCryptLib) {
		typedef BOOL (WINAPI *MyCryptAcquireContextA)(HCRYPTPROV*, LPCSTR, LPCSTR, DWORD, DWORD);
		typedef BOOL (WINAPI *MyCryptGenRandom)(HCRYPTPROV, DWORD, BYTE*);
		typedef BOOL (WINAPI *MyCryptReleaseContext)(HCRYPTPROV, ULONG_PTR);
		MyCryptAcquireContextA myCryptAcquireContextA = (MyCryptAcquireContextA)GetProcAddress(hCryptLib, "CryptAcquireContextA");
		MyCryptGenRandom myCryptGenRandom = (MyCryptGenRandom)GetProcAddress(hCryptLib, "CryptGenRandom");
		MyCryptReleaseContext myCryptReleaseContext = (MyCryptReleaseContext)GetProcAddress(hCryptLib, "CryptReleaseContext");
		if (!myCryptAcquireContextA || !myCryptGenRandom || !myCryptReleaseContext) cryptapi = false;
		if (cryptapi && !myCryptAcquireContextA(&hProv, NULL, MS_DEF_PROV_A, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
			int res = GetLastError();
			if (res != NTE_BAD_KEYSET || !myCryptAcquireContextA(&hProv, NULL, MS_DEF_PROV_A, PROV_RSA_FULL, CRYPT_NEWKEYSET)) {
				cryptapi = false;
			}
		}
		if (cryptapi) {
			myCryptGenRandom(hProv, 36, entropy);
			myCryptReleaseContext(hProv, 0);
		}
		FreeLibrary(hCryptLib);
	} else cryptapi = false;

	if (!cryptapi) {
		LARGE_INTEGER li;
		QueryPerformanceCounter(&li);
		memcpy(entropy, (BYTE*)&li.QuadPart, 8);
		write_le(entropy + 8, GetCurrentProcessId(), 4);
		write_le(entropy + 12, GetCurrentThreadId(), 4);
		GetCursorPos((POINT*)&entropy[16]);
		write_le(entropy + 24, (int)&li, 4);
		write_le(entropy + 28, time(NULL), 4);
		write_le(entropy + 32, GetTickCount(), 4);
	}
	
	register_prng(&fortuna_desc);
	fortuna_start(&prng);
	fortuna_add_entropy(entropy, 36, &prng);
	fortuna_ready(&prng);

	if (LOBYTE(LOWORD(GetVersion())) == 3) {
		nt3 = true;
		CLOSETOTRAY = false;
	}
	if (nt3) {
		LOGFONT lf = {0};
		HDC hdcRef = GetDC(NULL);
		lf.lfHeight = -MulDiv(8, GetDeviceCaps(hdcRef, LOGPIXELSY), 72);
		ReleaseDC(NULL, hdcRef);
		lf.lfWeight = 700;
		wcscpy(lf.lfFaceName, L"MS Sans Serif");
		hDefaultFont = CreateFontIndirect(&lf);
	} else hDefaultFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

	// register main class
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	WNDCLASS wcMain = {0};
	wcMain.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcMain.lpfnWndProc = WndProc;
	wcMain.hInstance = hInstance;
	wcMain.lpszClassName = L"Telegacy";
	wcMain.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcMain.hIcon = hIcon;
	RegisterClass(&wcMain);
	hMain = CreateWindow(L"Telegacy", L"Telegacy", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

	if (!init_connection(&dcInfoMain, false)) return 0;

	dcInfoMain.ready = false;
	FILE* f = _wfopen(get_path(appdata_path, L"session.dat"), L"rb");
	if (f) {
		BYTE buf[100];
		memset(buf, 0, 8);
		create_msg_id(buf + 8);
		write_le(buf + 16, 20, 4);
		write_le(buf + 20, 0xbe7e8ef1, 4);
		fortuna_read(buf + 24, 16, &prng);
		send_query(buf, 40);
		recv(dcInfoMain.sock, (char*)buf, 1, 0);
		recv(dcInfoMain.sock, (char*)buf, 100, 0);
		int time_new = read_le(buf + 12, 4);
		time_diff = time_new - time(NULL);

		dcInfoMain.authorized = true;
		fread(dcInfoMain.auth_key, 1, 256, f);
		fread(dcInfoMain.auth_key_id, 1, 8, f);
		fread(dcInfoMain.server_salt, 1, 8, f);
		fread(dcInfoMain.session_id, 1, 8, f);
		fread(&dcInfoMain.current_seq_no, 4, 1, f);
		fread(&pts, 4, 1, f);
		fread(&qts, 4, 1, f);
		fread(&date, 4, 1, f);
		fclose(f);
	} else create_auth_key(&dcInfoMain);

	SetTimer(hMain, 1, 45000, NULL);
	if (!dcInfoMain.authorized) {
		BYTE buffer[24] = {0};
		LONG dlgUnits = GetDialogBaseUnits();
		DLGTEMPLATE *dlg = (DLGTEMPLATE*)buffer;
		dlg->style = WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME;
		dlg->cx = MulDiv(190, 4, LOWORD(dlgUnits));
		dlg->cy = MulDiv(295, 8, HIWORD(dlgUnits));
		if (current_dialog) DestroyWindow(current_dialog);
		current_dialog = CreateDialogIndirect(GetModuleHandle(NULL), dlg, NULL, DlgProcLogin);
	} else {
		SetWindowText(infoLabel, L"Updating data...");
		get_future_salt(&dcInfoMain);
	}
	unsigned threadID;
	_beginthreadex(NULL, 0, SocketWorker, (void*)&dcInfoMain, 0, &threadID);

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!IsDialogMessage(current_dialog, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	closesocket(dcInfoMain.sock);
	WSACleanup();
	return 0;
}
