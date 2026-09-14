/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

void telegacy_log(const char* format, ...) {
	static bool cs_inited = false;
	char buf[2048];
	va_list args;
	SYSTEMTIME st;
	char timed_buf[2200];
	wchar_t log_path[MAX_PATH];
	FILE* f;

	if (!cs_inited) {
		InitializeCriticalSection(&csLog);
		cs_inited = true;
	}
	EnterCriticalSection(&csLog);

	va_start(args, format);
	_vsnprintf(buf, sizeof(buf) - 1, format, args);
	buf[sizeof(buf) - 1] = 0;
	va_end(args);

	GetLocalTime(&st);
	_snprintf(timed_buf, sizeof(timed_buf) - 1, "[%02d:%02d:%02d.%03d] %s\n",
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, buf);
	timed_buf[sizeof(timed_buf) - 1] = 0;

	fprintf(stderr, "%s", timed_buf);
	fflush(stderr);

	OutputDebugStringA(timed_buf);

	if (appdata_path[0] != 0) {
		wcscpy(log_path, appdata_path);
		get_path(log_path, L"telegacy_debug.log");
		f = _wfopen(log_path, L"a");
		if (f) {
			fputs(timed_buf, f);
			fclose(f);
		}
	}
	LeaveCriticalSection(&csLog);
}

int current_time() {
	return time(NULL) + time_diff;
}

char get_padding(int len) {
	char padding_len = 16 - len % 16;
	if (padding_len < 12) padding_len += 16;
	return padding_len;
}

int send_query(DCInfo* dcInfo, BYTE* enc_query, int length) {
	int dc_num = dcInfo ? dcInfo->dc : 0;
	telegacy_log("[SEND] DC %d, length=%d", dc_num, length);
	if (dcInfo == &dcInfoMain) EnterCriticalSection(&csSock);
	BYTE len_b[4];
	if (length/4 >= 127) {
		len_b[0] = 127;
		write_le(len_b + 1, length/4, 3);
		send(dcInfo->sock, (char*)(len_b), 4, 0);
	} else {
		len_b[0] = length/4;
		send(dcInfo->sock, (char*)(len_b), 1, 0);
	}
	int sent = 0;
	while (sent < length) {
		int res = send(dcInfo->sock, (char*)(enc_query + sent), length - sent, 0);
		if (res == SOCKET_ERROR) break;
		else sent += res;
	}
	if (dcInfo == &dcInfoMain) LeaveCriticalSection(&csSock);
	return sent;
}

void create_msg_id(DCInfo* dcInfo, BYTE* buf) {
	int unix_time = current_time();
	int last_time = read_le(dcInfo->last_msg_id_sent + 4, 4);
	if (unix_time < last_time) unix_time = last_time;
	write_le(buf + 4, unix_time, 4);
	unsigned int second;
	fortuna_read((BYTE*)&second, 4, &prng);
	if (second > 4292967295) second -= 2000000;
	if (memcmp(buf + 4, dcInfo->last_msg_id_sent + 4, 4) == 0) {
		unsigned int first = read_le(dcInfo->last_msg_id_sent, 4);
		if (second <= first) {
			unsigned short add;
			fortuna_read((BYTE*)&add, 2, &prng);
			if (add < 32) add += 32;
			second = first + add;
		}
	}
	memcpy(buf, &second, 4);
	buf[0] -= buf[0] % 4;
	memcpy(dcInfo->last_msg_id_sent, buf, 8);
}

void create_msg_key(DCInfo* dcInfo, BYTE* unenc_query, int length, int x, BYTE* msg_key) {
	BYTE* keyplusdata = (BYTE*)malloc(32 + length);
	memcpy(keyplusdata, dcInfo->auth_key + 88 + x, 32);
	memcpy(keyplusdata + 32, unenc_query, length);
	BYTE msg_key_large[32];
	sha256_init(&md);
	sha256_process(&md, keyplusdata, 32 + length);
	sha256_done(&md, msg_key_large);
	free(keyplusdata);
	memcpy(msg_key, msg_key_large + 8, 16);
}

bool convert_message(DCInfo* dcInfo, BYTE* unenc, BYTE* enc, int length, int x) {
	EnterCriticalSection(&csCM);
	BYTE msg_key[16];
	if (x == 0) {
		create_msg_key(dcInfo, unenc, length, x, msg_key);
	} else {
		memcpy(msg_key, enc + 8, 16);
	}

	BYTE a[32];
	BYTE a_input[52];
	memcpy(a_input, msg_key, 16);
	memcpy(a_input + 16, dcInfo->auth_key + x, 36);
	sha256_init(&md);
	sha256_process(&md, a_input, 52);
	sha256_done(&md, a);

	BYTE b[32];
	BYTE b_input[52];
	memcpy(b_input, dcInfo->auth_key + 40 + x, 36);
	memcpy(b_input + 36, msg_key, 16);
	sha256_init(&md);
	sha256_process(&md, b_input, 52);
	sha256_done(&md, b);

	BYTE aes_key[32];
	memcpy(aes_key, a, 8);
	memcpy(aes_key + 8, b + 8, 16);
	memcpy(aes_key + 24, a + 24, 8);

	BYTE aes_iv[32];
	memcpy(aes_iv, b, 8);
	memcpy(aes_iv + 8, a + 8, 16);
	memcpy(aes_iv + 24, b + 24, 8);
	if (x == 0) {
		memcpy(enc, dcInfo->auth_key_id, 8);
		memcpy(enc + 8, msg_key, 16);
		aes_ige(unenc, enc + 24, length, aes_key, aes_iv, 1);
	} else {
		aes_ige(enc + 24, unenc, length, aes_key, aes_iv, 0);

		// security checks
		BYTE msg_key2[16];
		create_msg_key(dcInfo, unenc, length, x, msg_key2);
		if (memcmp(msg_key, msg_key2, 16) != 0) {
			MessageBox(hMain, L"msg_key mismatch!", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
		int diff = length - read_le(unenc + 28, 4) + 32;
		if (diff < 12 || diff > 1024) {
			MessageBox(hMain, L"length mismatch!", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
		if (memcmp(unenc + 8, dcInfo->session_id, 8) != 0) {
			MessageBox(hMain, L"session_id mismatch!", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
		int msg_id_new = read_le(unenc + 20, 4);
#ifdef NDEBUG
		int curr_time = current_time();
		if (msg_id_new - curr_time > 30 || curr_time - msg_id_new > 300) {
			MessageBox(hMain, L"msg_id too old or new!", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
#endif
		int msg_id_old = read_le(dcInfo->last_msg_id + 4, 4);
		if (unenc[16] % 2 != 1) {
			MessageBox(hMain, L"msg_id mismatch!", L"Error", MB_OK | MB_ICONERROR);
			return false;
		}
		memcpy(dcInfo->last_msg_id, unenc + 16, 8);
	}
	LeaveCriticalSection(&csCM);
	return true;
}

void create_seq_no(DCInfo* dcInfo, BYTE* buf, bool content_related) {
	int seq_no = dcInfo->current_seq_no * 2;
	if (content_related) {
		seq_no++;
		dcInfo->current_seq_no++;
		if (dcInfo->authorized) {
			FILE* f = _wfopen(get_path(appdata_path, L"session.dat"), L"rb+");
			if (f) {
				if (dcInfo == &dcInfoMain) fseek(f, 280, SEEK_SET);
				else fseek(f, dcInfo->dc_file_index + 284, SEEK_SET);
				int current_seq_no_plus = dcInfo->current_seq_no + 10;
				fwrite(&current_seq_no_plus, 4, 1, f);
				fclose(f);
			}
		}
	}
	write_le(buf, seq_no, 4);
}

void internal_header(DCInfo* dcInfo, BYTE* unenc_query, bool content_related) {
	memcpy(unenc_query, dcInfo->server_salt, 8);
	memcpy(unenc_query + 8, dcInfo->session_id, 8);
	create_msg_id(dcInfo, unenc_query + 16);
	create_seq_no(dcInfo, unenc_query + 24, content_related);
}

void internal_header(BYTE* unenc_query, bool content_related) {
	internal_header(&dcInfoMain, unenc_query, content_related);
}
void create_seq_no(BYTE* buf, bool content_related) {
	create_seq_no(&dcInfoMain, buf, content_related);
}
bool convert_message(BYTE* unenc, BYTE* enc, int length, int x) {
	return convert_message(&dcInfoMain, unenc, enc, length, x);
}
void create_msg_key(BYTE* unenc_query, int length, int x, BYTE* msg_key) {
	create_msg_key(&dcInfoMain, unenc_query, length, x, msg_key);
}
void create_msg_id(BYTE* buf) {
	create_msg_id(&dcInfoMain, buf);
}
int send_query(BYTE* enc_query, int length) {
	return send_query(&dcInfoMain, enc_query, length);
}

void init_peer_defaults(Peer* peer, const BYTE* id, char type, bool is_forum) {
	if (!peer) return;
	memset(peer, 0, sizeof(Peer));
	if (id) memcpy(peer->id, id, 8);
	peer->type = type;
	peer->online = -1;
	peer->reaction_list = &reaction_list;
	memset(&peer->perm, 1, sizeof(Permissions));
	peer->is_forum = is_forum;
	if (type == 2) {
		peer->is_broadcast = !is_forum;
		if (is_forum) {
			peer->topics = new std::vector<ForumTopic>();
		}
	} else {
		peer->is_broadcast = false;
	}
}

bool is_valid_peer_constructor(int cons, int type) {
	if (type == 0) {
		return (cons == TL_USER || cons == TL_USER_EMPTY);
	} else if (type == 1) {
		return (cons == TL_CHAT || cons == TL_CHAT_FORBIDDEN || cons == TL_CHAT_EMPTY);
	} else if (type == 2) {
		return (cons == TL_CHANNEL || cons == TL_CHANNEL_FORBIDDEN);
	}
	return false;
}

int set_peer_info(BYTE* unenc_response, Peer* peer, bool just_update) {
	if (!unenc_response || !peer) return 0;
	int cons = read_le(unenc_response, 4);
	if (!is_valid_peer_constructor(cons, peer->type)) return 0;
	if (!just_update) {
		if (peer->reaction_list && peer->reaction_list != &reaction_list) {
			if (is_valid_reaction_list(peer->reaction_list)) {
				for (int i = 0; i < peer->reaction_list->size(); i++) free(peer->reaction_list->at(i));
				peer->reaction_list->clear();
				delete peer->reaction_list;
			}
		}
		if (peer->topics) {
			if (is_valid_topics_vector(peer->topics)) {
				for (size_t t = 0; t < peer->topics->size(); t++) {
					if (peer->topics->at(t).title) free(peer->topics->at(t).title);
				}
				delete peer->topics;
			}
		}
		peer->reaction_list = &reaction_list;
		peer->chat_users = NULL;
		peer->full = false;
		peer->last_recv = 0;
		peer->is_forum = false;
		peer->topics = NULL;
		peer->active_topic_id = 0;
		peer->requested_topic_id = 0;
		peer->is_broadcast = false;
		peer->is_bot = false;
		peer->name = NULL;
		peer->handle = NULL;
	}
	peer->name_set_time = current_time();
	peer->pfp_set_time = current_time();
	peer->status_updated = true;
	int flags = read_le(unenc_response + 4, 4);
	int offset = 8;
	if (peer->type == 0) {
		bool min = (flags & (1 << 20)) ? true : false;
		peer->amadmin = false;
		peer->is_bot = ((flags & (1 << 18)) || (flags & (1 << 14))) ? true : false;
		peer->is_broadcast = false;
		offset += 4;
		memcpy(peer->id, unenc_response + offset, 8);
		offset += 8;
		bool hash_is_zero = true;
		for (int h = 0; h < 8; h++) {
			if (peer->access_hash[h] != 0) {
				hash_is_zero = false;
				break;
			}
		}
		if (flags & (1 << 0)) {
			if (!min || hash_is_zero) memcpy(peer->access_hash, unenc_response + offset, 8);
			offset += 8;
		}
		wchar_t* new_name = NULL;
		if ((flags & (1 << 10)) && peer->online != -2 && !min) {
			new_name = _wcsdup(L"Saved Messages");
			if (flags & (1 << 2)) offset += set_name(unenc_response + offset, NULL);
			else offset += tlstr_len(unenc_response + offset, true);
		}
		else if (flags & (1 << 2)) offset += set_name(unenc_response + offset, (min && peer->name) ? NULL : &new_name);
		else if (flags & (1 << 13)) new_name = _wcsdup(L"Deleted User");
		else if (!min || !peer->name) {
			new_name = read_string(unenc_response + offset, NULL);
			offset += tlstr_len(unenc_response + offset, true);
		}
		if (new_name) {
			if (peer->name) free(peer->name);
			peer->name = new_name;
		}
		if (!min) {
			if (flags & (1 << 3)) {
				wchar_t* new_handle = read_string(unenc_response + offset, NULL);
				if (peer->handle) free(peer->handle);
				peer->handle = new_handle;
				offset += tlstr_len(unenc_response + offset, true);
			} else {
				if (peer->handle) free(peer->handle);
				peer->handle = NULL;
			}
		} else if (flags & (1 << 3)) {
			offset += tlstr_len(unenc_response + offset, true);
		}
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
		} else if (!min) memset(peer->photo, 0, 8);
		memset(&peer->perm, 1, sizeof(Permissions));
		if (!(flags & (1 << 10))) peer->perm.canchangedesc = false;
		if (!min && (flags & (1 << 6)) && !(flags & (1 << 10))) user_status_updated(unenc_response + offset, peer);
		else if (!min) peer->online = -1;
	} else if (peer->type == 1) {
		if (flags & (1 << 0)) peer->amadmin = true;
		else peer->amadmin = false;
		peer->is_broadcast = false;
		peer->is_bot = false;
		if (peer->handle) { free(peer->handle); peer->handle = NULL; }
		memcpy(peer->id, unenc_response + offset, 8);
		offset += 8;
		wchar_t* new_name = read_string(unenc_response + offset, NULL);
		if (new_name) {
			if (peer->name) free(peer->name);
			peer->name = new_name;
		}
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
	} else if (cons == TL_CHANNEL_FORBIDDEN) {
		memset(peer->channel_msg_id, 0, 8);
		peer->amadmin = false;
		peer->is_bot = false;
		peer->is_forum = (flags & (1 << 10)) ? true : false;
		if (peer->is_forum) {
			peer->is_broadcast = false;
			if (!peer->topics) peer->topics = new std::vector<ForumTopic>();
		} else {
			peer->is_broadcast = (flags & (1 << 5)) ? true : false;
		}
		memcpy(peer->id, unenc_response + offset, 8);
		offset += 8;
		memcpy(peer->access_hash, unenc_response + offset, 8);
		offset += 8;
		wchar_t* new_name = read_string(unenc_response + offset, NULL);
		if (new_name) {
			if (peer->name) free(peer->name);
			peer->name = new_name;
		}
		offset += tlstr_len(unenc_response + offset, true);
		memset(peer->photo, 0, 8);
		memset(&peer->perm, 0, sizeof(Permissions));
		return offset;
	} else {
		memset(peer->channel_msg_id, 0, 8);
		bool min = (flags & (1 << 12)) ? true : false;
		if (flags & (1 << 0)) peer->amadmin = true;
		else peer->amadmin = false;
		peer->is_bot = false;
		int flags2 = read_le(unenc_response + 8, 4);
		peer->is_forum = ((flags & (1 << 30)) || (flags2 & (1 << 17))) ? true : false;
		if (peer->is_forum) {
			peer->is_broadcast = false;
			if (!peer->topics) peer->topics = new std::vector<ForumTopic>();
		} else {
			peer->is_broadcast = (flags & (1 << 5)) ? true : false;
		}
		offset += 4;
		memcpy(peer->id, unenc_response + offset, 8);
		offset += 8;
		bool hash_is_zero = true;
		for (int h = 0; h < 8; h++) {
			if (peer->access_hash[h] != 0) {
				hash_is_zero = false;
				break;
			}
		}
		if (flags & (1 << 13)) {
			if (!min || hash_is_zero) memcpy(peer->access_hash, unenc_response + offset, 8);
			offset += 8;
		}
		wchar_t* new_name = read_string(unenc_response + offset, NULL);
		if (new_name) {
			if (!min || !peer->name) {
				if (peer->name) free(peer->name);
				peer->name = new_name;
			} else {
				free(new_name);
			}
		}
		offset += tlstr_len(unenc_response + offset, true);
		if (flags & (1 << 6)) {
			if (!min || !peer->handle) {
				wchar_t* new_handle = read_string(unenc_response + offset, NULL);
				if (peer->handle) free(peer->handle);
				peer->handle = new_handle;
			}
			offset += tlstr_len(unenc_response + offset, true);
		} else if (!min) {
			if (peer->handle) free(peer->handle);
			peer->handle = NULL;
		}
		if (read_le(unenc_response + offset, 4) == 0x1c6e1c11) {
			if (!min || (read_le(peer->photo, 8) == 0)) memcpy(peer->photo, unenc_response + offset + 8, 8);
			offset += chatphoto_offset(unenc_response + offset);
			if (!min || (read_le(peer->photo, 8) == 0)) peer->photo_dc = read_le(unenc_response + offset - 4, 4);
		} else {
			if (!min) memset(peer->photo, 0, 8);
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
	if (memcmp(peer->id, notification_peer_id, 8) == 0 && notif_newpeer_msg) {
		new_msg_notification(peer, notif_newpeer_msg == (BYTE*)-1 ? NULL : notif_newpeer_msg, false);
		if (notif_newpeer_msg != (BYTE*)-1) free(notif_newpeer_msg);
		notif_newpeer_msg = NULL;
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
				init_peer_defaults(&peers[peers_count-1], id, type, false);
				folders[0].peers = (int*)realloc(folders[0].peers, peers_count * sizeof(int));
				folders[0].count++;
				folders[0].peers[folders[0].count-1] = folders[0].count-1;
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
							if (&folders[j] == current_folder) SendMessage(hComboBoxChats, CB_INSERTSTRING, folders[j].pinned_count, NULL);
							if (&folders[j] == current_folder) SendMessage(hComboBoxChats, CB_SETITEMDATA, folders[j].pinned_count, (LPARAM)&peers[0]);
							if (&folders[j] == current_folder && current_peer == &peers[0]) SendMessage(hComboBoxChats, CB_SETCURSEL, folders[j].pinned_count, 0);
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
	get_lang_string("s_down", lang_str, NULL);
	swprintf(status_str, lang_str, document->filename, percentage);
	SendMessage(hStatus, SB_SETTEXTA, 1 | SBT_OWNERDRAW, (LPARAM)status_str);
	KillTimer(hMain, 2);
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
	for (int j = 0; j < msgent_count; j++) offset += msgent_offset(unenc_response + offset, NULL);
	if (!update) SendMessage(hComboBoxFolders, CB_ADDSTRING, 0, (LPARAM)folder->name);
	if (!update) SendMessage(hComboBoxFolders, CB_SETITEMDATA, i, (LPARAM)folder);
	if (flags & (1 << 25)) offset += tlstr_len(unenc_response + offset, true);
	if (flags & (1 << 27)) offset += 4;

	// pinned_peers
	if (read_le(unenc_response + offset, 4) == 0x1cb5c415) offset += 4;
	int pinned_count = read_le(unenc_response + offset, 4);
	offset += 4;
	folder->pinned_count = 0;
	std::vector<int> peers_temp;
	for (j = 0; j < pinned_count; j++) {
		int inputpeer = read_le(unenc_response + offset, 4);
		offset += 4;
		for (int k = 0; k < peers_count; k++) {
			if (memcmp(unenc_response + offset, peers[k].id, 8) == 0) {
				peers_temp.push_back(k);
				folder->pinned_count++;
				break;
			}
		}
		offset += (inputpeer == 0x35a95cb9) ? 8 : 16;
	}

	// include_peers
	if (read_le(unenc_response + offset, 4) == 0x1cb5c415) offset += 4;
	int include_count = read_le(unenc_response + offset, 4);
	offset += 4;
	for (j = 0; j < include_count; j++) {
		int inputpeer = read_le(unenc_response + offset, 4);
		offset += 4;
		for (int k = 0; k < peers_count; k++) {
			if (memcmp(unenc_response + offset, peers[k].id, 8) == 0) {
				bool already = false;
				for (size_t p = 0; p < peers_temp.size(); p++) {
					if (peers_temp[p] == k) { already = true; break; }
				}
				if (!already) peers_temp.push_back(k);
				break;
			}
		}
		offset += (inputpeer == 0x35a95cb9) ? 8 : 16;
	}

	// exclude_peers
	std::vector<int> excluded_peer_indices;
	if (read_le(unenc_response + offset, 4) == 0x1cb5c415) offset += 4;
	int exclude_count = read_le(unenc_response + offset, 4);
	offset += 4;
	for (int ex = 0; ex < exclude_count; ex++) {
		int inputpeer = read_le(unenc_response + offset, 4);
		offset += 4;
		for (int k = 0; k < peers_count; k++) {
			if (memcmp(unenc_response + offset, peers[k].id, 8) == 0) {
				excluded_peer_indices.push_back(k);
				break;
			}
		}
		offset += (inputpeer == 0x35a95cb9) ? 8 : 16;
	}

	if (FILTER_CONTACTS(flags) || FILTER_NON_CONTACTS(flags) || FILTER_GROUPS(flags) || FILTER_BROADCASTS(flags) || FILTER_BOTS(flags)) {
		for (int k = 0; k < peers_count; k++) {
			bool already_in = false;
			for (size_t pt = 0; pt < peers_temp.size(); pt++) {
				if (peers_temp[pt] == k) {
					already_in = true;
					break;
				}
			}
			if (already_in) continue;

			bool is_excluded = false;
			for (size_t ex = 0; ex < excluded_peer_indices.size(); ex++) {
				if (excluded_peer_indices[ex] == k) {
					is_excluded = true;
					break;
				}
			}
			if (is_excluded) continue;
			if (EXCLUDE_MUTED(flags) && is_peer_muted(&peers[k])) continue;
			if (EXCLUDE_READ(flags) && peers[k].unread_msgs_count == 0) continue;

			bool match = false;
			if (FILTER_GROUPS(flags) && (peers[k].type == 1 || (peers[k].type == 2 && !peers[k].is_broadcast))) {
				match = true;
			}
			if (FILTER_BROADCASTS(flags) && (peers[k].type == 2 && peers[k].is_broadcast)) {
				match = true;
			}
			if ((FILTER_CONTACTS(flags) || FILTER_NON_CONTACTS(flags)) && (peers[k].type == 0 && !peers[k].is_bot)) {
				match = true;
			}
			if (FILTER_BOTS(flags) && (peers[k].type == 0 && peers[k].is_bot)) {
				match = true;
			}

			if (match) {
				peers_temp.push_back(k);
			}
		}
	}

	if (folder->pinned_count < (int)peers_temp.size()) {
		std::sort(peers_temp.begin() + folder->pinned_count, peers_temp.end());
	}
	folder->count = peers_temp.size();

	if (update) {
		free(folder->peers);
		if (folder == current_folder) SendMessage(hComboBoxChats, CB_RESETCONTENT, 0, 0);
	}
	folder->peers = (int*)malloc(sizeof(int) * folder->count);
	for (j = 0; j < folder->count; j++) {
		folder->peers[j] = peers_temp[j];
		if (update && folder == current_folder) {
			wchar_t* pname = peers[folder->peers[j]].name;
			SendMessage(hComboBoxChats, CB_ADDSTRING, 0, (LPARAM)(pname ? pname : L""));
			SendMessage(hComboBoxChats, CB_SETITEMDATA, j, (LPARAM)&peers[folder->peers[j]]);
			if (&peers[folder->peers[j]] == current_peer) SendMessage(hComboBoxChats, CB_SETCURSEL, j, 0);
		}
	}
	return offset;
}

DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb) {
	StreamData* data = (StreamData*)dwCookie;
	if (data->buf == NULL) data->buf = (BYTE*)malloc(cb);
	else data->buf = (BYTE*)realloc(data->buf, data->length + cb);
	memcpy(data->buf + data->length, pbBuff, cb);
	data->length += cb;
	*pcb = cb;
	return 0;
}

DWORD CALLBACK StreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb) {
	StreamData* data = (StreamData*)dwCookie;
	if (data->length - data->written < cb) cb = data->length - data->written;
	memcpy(pbBuff, data->buf + data->written, cb);
	data->written += cb;
	*pcb = cb;
	return 0;
}

int insert_format(BYTE* unenc_query, int format_count, std::vector<int>* format_vecs) {
	int offset = 0;
	write_le(unenc_query + offset, 0x1cb5c415, 4);
	write_le(unenc_query + offset + 4, format_count, 4);
	offset += 8;
	for (int i = 0; i < 7; i++) {
		for (int j = 0; j < format_vecs[i].size(); j+=2) {
			if (i == 0) write_le(unenc_query + offset, 0xbd610bc9, 4);
			else if (i == 1) write_le(unenc_query + offset, 0x826f8b60, 4);
			else if (i == 2) write_le(unenc_query + offset, 0x9c4e7e8b, 4);
			else if (i == 3) write_le(unenc_query + offset, 0xbf0693d4, 4);
			else if (i == 4) {
				write_le(unenc_query + offset, 0xf1ccaaac, 4);
				memset(unenc_query + offset + 4, 0, 4);
				offset += 4;
			}
			else if (i == 5) write_le(unenc_query + offset, 0x28a20571, 4);
			else if (i == 6) write_le(unenc_query + offset, 0x32ca960f, 4);
			write_le(unenc_query + offset + 4, format_vecs[i][j], 4);
			write_le(unenc_query + offset + 8, format_vecs[i][j+1]-format_vecs[i][j], 4);
			offset += 12;
		}
	}
	return offset;
}

void write_md5(BYTE* unenc_query, FILE* f) {
	hash_state md;
	BYTE buffer[4096];
	int checked;
	md5_init(&md);
	while ((checked = fread(buffer, 1, sizeof(buffer), f)) > 0) md5_process(&md, buffer, (unsigned long)checked);
	
	char md5_checksum[16];
	md5_done(&md, (BYTE*)md5_checksum);
	const char* digits = "0123456789abcdef";
	unenc_query[0] = 32;
	for (int i = 0; i < 16; ++i) {
		unenc_query[1 + i * 2] = digits[(md5_checksum[i] >> 4) & 0xF];
		unenc_query[2 + i * 2] = digits[md5_checksum[i] & 0xF];
	}
	memset(unenc_query + 33, 0, 3);
}

wchar_t* files_i(wchar_t* file_name) {
	return wcscmp(file_name, L"voice.wav") == 0 ? get_path(appdata_path, L"voice.wav") : file_name;
}

int place_inputmedia(BYTE* unenc_query, Document* docstemp, int index) {
	if (docstemp->min == 6) write_le(unenc_query, 0x7d8375da, 4);
	else write_le(unenc_query, 0x037c9330, 4);
	write_le(unenc_query + 4, 0, 4);
	memcpy(unenc_query + 12, docstemp[index].id, 8);
	write_le(unenc_query + 20, ceil(docstemp[index].size / 524288.0), 4);
	write_string(unenc_query + 24, docstemp[index].filename);
	int offset = tlstr_len(unenc_query + 24, true) + 24;
	if (docstemp[index].size <= 10485760) {
		write_le(unenc_query + 8, 0xf52ff27f, 4);
		FILE* f = _wfopen(files_i(files[index]), L"rb");
		write_md5(unenc_query + offset, f);
		fclose(f);
		offset += 36;
	} else write_le(unenc_query + 8, 0xfa4f0bb5, 4);
	if (docstemp->min != 6) {
		HKEY hKey;
		unsigned long mime_type_size = 256;
		wchar_t mime_type[256];
		if (RegOpenKeyEx(HKEY_CLASSES_ROOT, wcsrchr(files[index], L'.'), 0, KEY_READ, &hKey) != ERROR_SUCCESS
			|| RegQueryValueEx(hKey, L"Content Type", NULL, NULL, (BYTE*)mime_type, &mime_type_size) != ERROR_SUCCESS)
			wcscpy(mime_type, L"application/octet-stream");
		if (hKey) RegCloseKey(hKey);
		write_string(unenc_query + offset, mime_type);
		offset += str_to_tlstr_len(mime_type);
		write_le(unenc_query + offset, 0x1cb5c415, 4);
		bool audio = (wcsncmp(mime_type, L"audio", 5) == 0 && !SENDMEDIAASFILES) ? true : false;
		bool video = (wcsncmp(mime_type, L"video", 5) == 0 && !SENDMEDIAASFILES) ? true : false;
		write_le(unenc_query + offset + 4, (audio || video) ? 2 : 1, 4);
		write_le(unenc_query + offset + 8, 0x15590068, 4);
		write_string(unenc_query + offset + 12, docstemp[index].filename);
		offset += tlstr_len(unenc_query + offset + 12, true) + 12;
		if (audio) {
			write_le(unenc_query + offset, 0x9852f9c6, 4);
			write_le(unenc_query + offset + 4, 0, 4);
			wchar_t command[512];
			wchar_t buffer[32];
			swprintf(command, L"open \"%s\" alias myaudio", files_i(files[index]));
			mciSendString(command, NULL, 0, NULL);
			mciSendString(L"status myaudio length", buffer, 32, NULL);
			mciSendString(L"close myaudio", NULL, 0, NULL);
			write_le(unenc_query + offset + 8, _wtoi(buffer) / 1000, 4);
			offset += 12;
		} else if (video) {
			write_le(unenc_query + offset, 0x43c57c48, 4);
			memset(unenc_query + offset + 4, 0, 20);
			offset += 24;
		}
	}
	return offset;
}

void get_future_salt(DCInfo* dcInfo) {
	BYTE unenc_query[64];
	BYTE enc_query[88];
	internal_header(dcInfo, unenc_query, true);
	write_le(unenc_query + 28, 8, 4);
	write_le(unenc_query + 32, 0xb921bd04, 4);
	write_le(unenc_query + 36, 2, 4);
	fortuna_read(unenc_query + 40, 24, &prng);
	convert_message(dcInfo, unenc_query, enc_query, 64, 0);
	send_query(dcInfo, enc_query, 88);
}

int update_own_status(bool status) {
	BYTE unenc_query[64];
	BYTE enc_query[88];
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, 8, 4);
	write_le(unenc_query + 32, 0x6628562c, 4);
	if (status) write_le(unenc_query + 36, TL_BOOL_FALSE, 4);
	else write_le(unenc_query + 36, TL_BOOL_TRUE, 4);
	fortuna_read(unenc_query + 40, 24, &prng);
	convert_message(unenc_query, enc_query, 64, 0);
	return send_query(enc_query, 88);
}

void get_history() {
	if (getting_history || no_more_msgs) return;
	getting_history = true;
	if (current_peer && current_peer->is_forum && current_peer->active_topic_id > 0) {
		BYTE unenc_query[128];
		BYTE enc_query[152];
		internal_header(unenc_query, true);
		write_le(unenc_query + 32, 0x22ddd30c, 4);
		char offset = place_peer(unenc_query + 36, current_peer, true);
		write_le(unenc_query + 36 + offset, current_peer->active_topic_id, 4);
		int offset_id = 0;
		if (messages.size() > 0) {
			size_t m;
			for (m = 0; m < messages.size(); m++) {
				if (messages[m].id != 0 && (current_peer->active_topic_id == 1 || messages[m].id != current_peer->active_topic_id)) {
					if (offset_id == 0 || messages[m].id < offset_id) {
						offset_id = messages[m].id;
					}
				}
			}
		}
		write_le(unenc_query + 40 + offset, offset_id, 4);
		write_le(unenc_query + 44 + offset, 0, 4);
		write_le(unenc_query + 48 + offset, 0, 4);
		write_le(unenc_query + 52 + offset, MSGSFETCHCOUNT, 4);
		memset(unenc_query + 56 + offset, 0, 16);
		char padding_len = get_padding(72 + offset);
		write_le(unenc_query + 28, 40 + offset, 4);
		fortuna_read(unenc_query + 72 + offset, padding_len, &prng);
		char len = 72 + offset + padding_len;
		convert_message(unenc_query, enc_query, len, 0);
		send_query(enc_query, len + 24);
		return;
	}
	BYTE unenc_query[112];
	BYTE enc_query[136];
	internal_header(unenc_query, true);
	write_le(unenc_query + 32, 0x4423e6c5, 4);
	char offset = place_peer(unenc_query + 36, current_peer, true);
	memset(unenc_query + 36 + offset, 0, 8);
	write_le(unenc_query + 44 + offset, messages.size(), 4);
	write_le(unenc_query + 48 + offset, MSGSFETCHCOUNT, 4);
	write_le(unenc_query + 52 + offset, -1, 4);
	write_le(unenc_query + 56 + offset, -1, 4);
	memset(unenc_query + 60 + offset, 0, 8);
	char padding_len = get_padding(68 + offset);
	write_le(unenc_query + 28, 36 + offset, 4);
	fortuna_read(unenc_query + 68 + offset, padding_len, &prng);
	char len = 68 + offset + padding_len;
	convert_message(unenc_query, enc_query, len, 0);
	send_query(enc_query, len+24);
}

void get_forum_topics(Peer* peer) {
	if (!peer || !peer->is_forum) return;
	BYTE unenc_query[128];
	BYTE enc_query[152];
	internal_header(unenc_query, true);
	write_le(unenc_query + 32, 0x3ba47bff, 4); // messages.getForumTopics
	memset(unenc_query + 36, 0, 4);
	char offset = place_peer(unenc_query + 40, peer, true);
	memset(unenc_query + 40 + offset, 0, 12); // both offset_date, offset_id, offset_topic should be 0
	write_le(unenc_query + 52 + offset, 100, 4);
	char padding_len = get_padding(56 + offset);
	write_le(unenc_query + 28, 24 + offset, 4);
	fortuna_read(unenc_query + 56 + offset, padding_len, &prng);
	char len = 56 + offset + padding_len;
	convert_message(unenc_query, enc_query, len, 0);
	send_query(enc_query, len + 24);
}

void set_typing(int cons, int add) {
	if (!current_peer || memcmp(myself.id, current_peer->id, 8) == 0) return;
	BYTE unenc_query[80];
	BYTE enc_query[104];
	internal_header(unenc_query, true);
	write_le(unenc_query + 32, 0x58943ee2, 4);
	bool has_topic = current_peer->is_forum && current_peer->active_topic_id > 0;
	write_le(unenc_query + 36, has_topic ? 1 : 0, 4);
	char offset = place_peer(unenc_query + 40, current_peer, true) + 40;
	if (has_topic) {
		write_le(unenc_query + offset, current_peer->active_topic_id, 4);
		offset += 4;
	}
	write_le(unenc_query + offset, cons, 4);
	if (add == 4) memset(unenc_query + offset + 4, 0, 4);
	offset += add + 4;
	write_le(unenc_query + 28, offset - 32, 4);
	char padding_len = get_padding(offset);
	fortuna_read(unenc_query + offset, padding_len, &prng);
	offset += padding_len;
	convert_message(unenc_query, enc_query, offset, 0);
	send_query(enc_query, offset + 24);
}

void make_seen(Message* message) {
	if (!message || !current_peer) return;
	BYTE unenc_query[96];
	BYTE enc_query[120];
	bool is_forum_topic = (current_peer->is_forum && current_peer->active_topic_id > 0);
	char peer_len = place_peer(unenc_query + 36, current_peer, is_forum_topic ? true : ((current_peer->type == 2) ? false : true));
	char len_data = 36 + peer_len;
	if (is_forum_topic) {
		write_le(unenc_query + 32, 0xf731a9f4, 4); // messages.readDiscussion
		write_le(unenc_query + len_data, current_peer->active_topic_id, 4); // topic root id
		len_data += 4;
		write_le(unenc_query + len_data, message->id, 4); // read_max_id
		len_data += 4;
	} else {
		write_le(unenc_query + 32, (current_peer->type == 2) ? 0xcc104937 : 0xe306d3a, 4);
		write_le(unenc_query + len_data, message->id, 4);
		len_data += 4;
	}
	char padding_len = get_padding(len_data);
	char len = len_data + padding_len;
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, len - 32 - padding_len, 4);
	fortuna_read(unenc_query + len_data, padding_len, &prng);
	convert_message(unenc_query, enc_query, len, 0);
	send_query(enc_query, len + 24);
	message->seen = true;
}

void mark_message_seen(Message* message, bool emit_network) {
	if (!message || message->outgoing || message->seen) return;
	if (emit_network) {
		make_seen(message);
	} else {
		message->seen = true;
	}

	if (current_peer) {
		if (current_peer->is_forum && current_peer->topics) {
			for (size_t t = 0; t < current_peer->topics->size(); t++) {
				if (current_peer->topics->at(t).id == current_peer->active_topic_id) {
					if (message->id > current_peer->topics->at(t).read_inbox_max_id) {
						current_peer->topics->at(t).read_inbox_max_id = message->id;
					}
					if (current_peer->topics->at(t).unread_count > 0) {
						current_peer->topics->at(t).unread_count--;
					}
					break;
				}
			}
			InvalidateRect(hComboBoxTopics, NULL, TRUE);
		}
		if (current_peer->unread_msgs_count > 0) {
			current_peer->unread_msgs_count--;
			if (!is_peer_muted(current_peer) && total_unread_msgs_count > 0) {
				update_total_unread_msgs_count(-1);
			}
			InvalidateRect(hComboBoxChats, NULL, TRUE);
		}
	}
}

void mark_active_chat_seen(int specific_msg_id) {
	if (!current_peer) return;

	int max_unread_id = specific_msg_id;
	for (int i = (int)messages.size() - 1; i >= 0; i--) {
		if (!messages[i].outgoing) {
			if (!messages[i].seen) {
				if (messages[i].id > max_unread_id) max_unread_id = messages[i].id;
				messages[i].seen = true;
			}
		}
	}

	if (max_unread_id > 0) {
		Message dummy;
		memset(&dummy, 0, sizeof(dummy));
		dummy.id = max_unread_id;
		dummy.seen = true;
		make_seen(&dummy);
	}

	if (current_peer->is_forum && current_peer->topics && current_peer->active_topic_id > 0) {
		for (size_t t = 0; t < current_peer->topics->size(); t++) {
			if (current_peer->topics->at(t).id == current_peer->active_topic_id) {
				if (max_unread_id > current_peer->topics->at(t).read_inbox_max_id) {
					current_peer->topics->at(t).read_inbox_max_id = max_unread_id;
				}
				int old_topic_unread = current_peer->topics->at(t).unread_count;
				current_peer->topics->at(t).unread_count = 0;
				current_peer->topics->at(t).unread_mentions_count = 0;
				if (current_peer->unread_msgs_count >= old_topic_unread) {
					current_peer->unread_msgs_count -= old_topic_unread;
					if (!is_peer_muted(current_peer) && total_unread_msgs_count > 0) {
						int to_dec = (old_topic_unread > total_unread_msgs_count) ? total_unread_msgs_count : old_topic_unread;
						update_total_unread_msgs_count(-to_dec);
					}
				} else {
					current_peer->unread_msgs_count = 0;
				}
				break;
			}
		}
		InvalidateRect(hComboBoxTopics, NULL, TRUE);
		InvalidateRect(hComboBoxChats, NULL, TRUE);
	} else if (current_peer->unread_msgs_count > 0) {
		int to_dec = current_peer->unread_msgs_count;
		if (to_dec > total_unread_msgs_count) to_dec = total_unread_msgs_count;
		if (to_dec > 0) update_total_unread_msgs_count(0 - to_dec);
		current_peer->unread_msgs_count = 0;
		InvalidateRect(hComboBoxChats, NULL, TRUE);
	}

	remove_notification();
}

void update_positions(int diff, int pos, int new_links) {
	for (int k = messages.size() - 1; k >= 0; k--) {
		if (pos >= messages[k].start_char) break;
		messages[k].start_char += diff;
		messages[k].end_header += diff;
		messages[k].end_char += diff;
		messages[k].start_reactions += diff;
		messages[k].end_footer += diff;
	}
	for (k = documents.size() - 1; k >= 0; k--) {
		if (pos >= documents[k].min) break;
		documents[k].min += diff;
		documents[k].max += diff;
	}
	for (k = links.size() - 1 - new_links; k >= 0; k--) {
		if (pos >= links[k].chrg.cpMax) continue;
		links[k].chrg.cpMin += diff;
		links[k].chrg.cpMax += diff;
	}
}

int replace_in_chat(FINDTEXTEX* ft, CHARRANGE* cr, wchar_t* replacement, HBITMAP hBitmap, BYTE* reactions, CustomEmojiPlacement* cep, ReplyFront* rf) {
	int min, max;
	if (ft) {
		if (rf) {
			min = ft->chrg.cpMin;
			max = ft->chrg.cpMax;
		} else {
			min = SendMessage(chat, EM_FINDTEXTEX, FR_DOWN, (LPARAM)ft);
			max = min + wcslen(ft->lpstrText);
		}
	} else {
		min = cr->cpMin;
		max = cr->cpMax;
	}
	if (min != -1) {
		SCROLLINFO si = {0};
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS;
		GetScrollInfo(chat, SB_VERT, &si);

		RECT rect;
		SendMessage(chat, EM_GETRECT, 0, (LPARAM)&rect);
		POINTL point;
		point.x = rect.left;
		point.y = rect.top;
		int pos = SendMessage(chat, EM_CHARFROMPOS, 0, (LPARAM)&point);
		bool ismsgup = pos > max;
		bool ismsgmid = pos >= min && pos <= max;
		
		CHARRANGE cr;
		SendMessage(chat, EM_EXGETSEL, 0, (LPARAM)&cr);
		if (drawchat) SendMessage(chat, WM_SETREDRAW, FALSE, 0);

		int diff = 0 - (max - min);
		SendMessage(chat, EM_SETSEL, min, max);
		if (replacement) {
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)replacement);
			diff += wcslen(replacement);
		} else if (hBitmap) {
			CHARFORMAT2 cf;
			cf.cbSize = sizeof(CHARFORMAT2);
			cf.dwMask = CFM_LINK;
			SendMessage(chat, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			insert_image(chat, NULL, hBitmap);
			SendMessage(chat, EM_SETSEL, min, max);
			SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			diff = 0;
		} else if (reactions) {
			std::vector<int> format_bold;
			int msg_id = read_le(reactions - 8, 4);
			diff += set_reactions(reactions, NULL, &format_bold, 0, msg_id);
			for (int j = 0; j < format_bold.size(); j+=2) {
				SendMessage(chat, EM_SETSEL, min + format_bold[j], min + format_bold[j+1]);
				CHARFORMAT cf = { sizeof(CHARFORMAT) };
				cf.dwMask = CFM_BOLD;
				cf.dwEffects = CFE_BOLD;
				SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
			}
		} else if (cep) {
			insert_emoji(cep->path, cep->size, chat);
			diff = 0;
		} else if (rf) {
			drawchat = false;
			bool addmsg = (rf->message) ? true : false;
			get_lang_string("c_rep", lang_str, NULL);
			int replying_len = wcslen(lang_str);
			if (addmsg) {
				message_handler(true, rf->message, false, false, true);
				if (si.nPos >= (int)(si.nMax - si.nPage) - 15) SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
				if (rf->i + 1 < (int)messages.size()) {
					ft->chrg.cpMin = messages[rf->i+1].end_char + replying_len + 1;
					ft->chrg.cpMax = messages[rf->i+1].end_char + replying_len + 2;
				}
			}
			if (rf->i + addmsg < (int)messages.size()) {
				diff = set_reply(addmsg ? 0 : rf->j, messages[rf->i+addmsg].end_char + replying_len + 1, SendMessage(chat, EM_FINDTEXTEX, FR_DOWN, (LPARAM)ft) == -1 ? (BYTE*)-1 : NULL, true);
			}
			if (addmsg) delete_message(0, false);
			if (rf->i < (int)messages.size()) {
				messages[rf->i].end_footer += diff;
				messages[rf->i].start_reactions += diff;
				messages[rf->i].reply_needed = 0;
			}
			drawchat = true;
		}

		if ((cr.cpMin >= min && cr.cpMin <= max) || (cr.cpMax >= min && cr.cpMax <= max)) {
			cr.cpMin = min;
			cr.cpMax = min;
		} else {
			if (cr.cpMin > max) cr.cpMin += diff;
			if (cr.cpMax > max) cr.cpMax += diff;
		}

		if (cr.cpMin != cr.cpMax) SendMessage(chat, EM_EXSETSEL, 0, (LPARAM)&cr);
		else SendMessage(chat, EM_SETSEL, INT_MAX - 1, INT_MAX - 1);

		if (ischatscrolling) {
			si.nPos = si.nTrackPos;
			ischatscrolling = false;
		}
		if (drawchat) {
			if (si.nPos >= (int)(si.nMax - si.nPage) - 25) SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
			else {
				if (ismsgup) {
					SCROLLINFO si_new = {0};
					si_new.cbSize = sizeof(si_new);
					si_new.fMask = SIF_RANGE;
					GetScrollInfo(chat, SB_VERT, &si_new);
					SendMessage(chat, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, si_new.nMax - si.nMax + si.nPos), 0);
				} else SendMessage(chat, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, si.nPos), 0);
			}
			SendMessage(chat, WM_SETREDRAW, TRUE, 0);
			InvalidateRect(chat, NULL, TRUE);
		}

		if (diff) update_positions(diff, min, 0);
		return diff;
	} else return 0;
}

int array_find(BYTE* buf, BYTE* find, int find_len, int find_count, int max_len) {
	if (!buf || !find || find_len <= 0 || max_len <= 0) return -1;
	BYTE* valid_start = NULL;
	BYTE* valid_end = NULL;

	for (int i = 0; i + find_len <= max_len; i += 4) {
		BYTE* cur = buf + i;
		if (cur < valid_start || cur + find_len > valid_end) {
			if (IsBadReadPtr(cur, find_len)) break;
			valid_start = cur;
			DWORD cur_addr = (DWORD)cur;
			DWORD end_addr = cur_addr + find_len;
			DWORD page_end = (end_addr + 4095) & ~0xFFF;
			int remaining = max_len - i;
			int candidate_len = (int)(page_end - cur_addr);
			if (candidate_len > remaining) candidate_len = remaining;
			if (candidate_len > find_len && !IsBadReadPtr(cur, candidate_len)) {
				valid_end = cur + candidate_len;
			} else {
				valid_end = cur + find_len;
			}
		}

		for (int j = 0; j < find_count; j++) {
			if (memcmp(cur, find + j * find_len, find_len) == 0) return i;
		}
	}
	return -1;
}

int parse_chat_participants(BYTE* buf, int offset, int chat_users_count, std::vector<Peer>* chat_users, int max_len) {
	if (!buf || !chat_users || chat_users_count <= 0 || max_len <= offset) return offset;
	BYTE user_cons[4];
	write_le(user_cons, TL_USER, 4);      // user (Layer 225)

	int user_pos = offset;
	for (int j = 0; j < chat_users_count; j++) {
		if (user_pos + 4 >= max_len) break;
		int found = array_find(buf + user_pos + 4, user_cons, 4, 1, max_len - (user_pos + 4));
		if (found < 0) break;
		user_pos += found + 4;
		if (user_pos + 32 > max_len) break;
		Peer peer;
		memset(&peer, 0, sizeof(Peer));
		peer.type = 0;
		set_peer_info(buf + user_pos, &peer, false);
		chat_users->push_back(peer);
		if (offset + 4 <= max_len) {
			offset += (read_le(buf + offset, 4) == 0xe46bcee4) ? 12 : 24;
		} else {
			break;
		}
	}
	return offset;
}

Peer* get_peer_by_id(const BYTE* id) {
	if (!id) return NULL;
	if (read_le(myself.id, 8) != 0 && memcmp(myself.id, id, 8) == 0) return &myself;
	if (current_peer && memcmp(current_peer->id, id, 8) == 0) return current_peer;
	for (int i = 0; i < peers_count; i++) {
		if (memcmp(peers[i].id, id, 8) == 0) return &peers[i];
	}
	return NULL;
}

int place_peer(BYTE* unenc_query, Peer* peer, bool peer_name) {
	switch (peer->type) {
	case 0:
		if (unenc_query == NULL) return 20;
		if (peer_name) write_le(unenc_query, 0xdde8a54c, 4);
		else write_le(unenc_query, 0xf21158c6, 4);
		memcpy(unenc_query + 4, peer->id, 8);
		memcpy(unenc_query + 12, peer->access_hash, 8);
		return 20;
	case 1: {
		char offset = peer_name ? 4 : 0;
		if (unenc_query == NULL) return 8 + offset;
		if (peer_name) write_le(unenc_query, 0x35a95cb9, 4);
		memcpy(unenc_query + offset, peer->id, 8);
		return 8 + offset;
	}
	case 2:
		if (unenc_query == NULL) return 20;
		if (peer_name) write_le(unenc_query, 0x27bcbbfc, 4);
		else write_le(unenc_query, 0xf35aec28, 4);
		memcpy(unenc_query + 4, peer->id, 8);
		memcpy(unenc_query + 12, peer->access_hash, 8);
		return 20;
	}
	return 0;
}

void set_reply_tofront(int i, BYTE* message, int j) {
	ReplyFront rf;
	rf.i = i;
	rf.j = j;
	rf.message = message;
	FINDTEXTEX ft;
	get_lang_string("c_rep", lang_str, NULL);
	int replying_len = wcslen(lang_str);
	ft.chrg.cpMin = messages[i].end_char + replying_len + 1;
	ft.chrg.cpMax = messages[i].end_char + replying_len + 2;
	ft.lpstrText = L"\r";
	replace_in_chat(&ft, NULL, NULL, NULL, NULL, NULL, &rf);
}

void get_date(wchar_t* buf, int date_init, bool preposition) {
	time_t timestamp = date_init;
	struct tm t = *localtime(&timestamp);
	time_t now = current_time();
	struct tm t_now = *localtime(&now);
	SYSTEMTIME st;
	st.wYear = t.tm_year + 1900;
	st.wMonth = t.tm_mon + 1;
	st.wDay = t.tm_mday;
	st.wHour = t.tm_hour;
	st.wMinute = t.tm_min;
	st.wSecond = t.tm_sec;
	st.wMilliseconds = 0;
	if (t.tm_year == t_now.tm_year && t.tm_yday == t_now.tm_yday) {
		if (preposition) {
			get_lang_string("s_ptime", lang_str, NULL);
			if (lang_str[0]) swprintf(buf, L"%s ", lang_str);
		}
	} else if (t.tm_year == t_now.tm_year && t.tm_yday == t_now.tm_yday-1) {
		preposition = true;
		get_lang_string("s_pyest", lang_str, NULL);
		swprintf(buf, L"%s ", lang_str);
	} else {
		if (preposition) {
			get_lang_string("s_pdate", lang_str, NULL);
			if (lang_str[0]) swprintf(buf, L"%s ", lang_str);
		}
		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, buf + (preposition ? wcslen(lang_str) + 1 : 0), 100);
		wcscat(buf, L" ");
	}
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, buf + wcslen(buf), 100);
	if (preposition) {
		get_lang_string("s_ptime2", lang_str, NULL);
		if (lang_str[0] == L'-') lang_str[0] = L' ';
		wcscat(buf, lang_str);
	}
}

int set_name(BYTE* unenc_response, wchar_t** name) {
	int name1_len = tlstr_to_str_len(unenc_response);
	int name1_tllen = tlstr_len(unenc_response, true);
	int name2_len = tlstr_to_str_len(unenc_response + name1_tllen);
	int name2_tllen = tlstr_len(unenc_response + name1_tllen, true);
	if (name2_len == 0) name2_len--; 
	if (name) {
		*name = (wchar_t*)malloc((name1_len + name2_len + 2)*2);
		read_string(unenc_response, *name);
	}
	if (name2_len != -1 && name) {
		(*name)[name1_len] = ' ';
		read_string(unenc_response + name1_tllen, *name + name1_len + 1);
	}
	return name1_tllen + name2_tllen;
}

int compound_emoji_checker(wchar_t* msg, int chars_left) {
	int compound_chars = 0;
	if (chars_left >= 3 && msg[1] == 0x200D) {
		if (chars_left >= 4 && msg[2] >= 0xD800 && msg[2] <= 0xDBFF && msg[3] >= 0xDC00 && msg[3] <= 0xDFFF)
			compound_chars++;
		if (chars_left >= 4 && msg[3 + compound_chars] == 0xFE0F) compound_chars += 3;
		else compound_chars += 2;
		if (chars_left >= 3 + compound_chars) compound_chars += compound_emoji_checker(msg + compound_chars, chars_left - compound_chars); 
	} else if (chars_left >= 2) {
		int cr = ((msg[1] - 0xD800) << 10) + (msg[2] - 0xDC00) + 0x10000;
		if (cr == 0x1F3FB || cr == 0x1F3FC || cr == 0x1F3FD || cr == 0x1F3FE || cr == 0x1F3FF) {
			compound_chars += 2;
			compound_chars += compound_emoji_checker(msg + compound_chars, chars_left - compound_chars);
		} 
	}
	return compound_chars;
}

bool insert_emoji(wchar_t* path, int size, HWND richedit) {
	if (EMOJIS) {
		HICON hIcon = (HICON)LoadImage(NULL, path, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
		if (hIcon || (nt3 && GetFileAttributes(path) != -1)) {
			HDC hdcMeta = CreateMetaFile(NULL);
			SetWindowExtEx(hdcMeta, 15, 15, NULL);
			if (nt3) {
				RECT rc = {0};
				paint_emoji_bitmap(hdcMeta, path, &rc);
			} else {
				DrawIconEx(hdcMeta, 0, 0, hIcon, 0, 0, 0, NULL, DI_NORMAL);
				DestroyIcon(hIcon);
			}

			path[wcslen(path)-4] = L'\0';
			wchar_t* emoji_str = wcsrchr(path, L'\\') + 1;
			Escape(hdcMeta, MFCOMMENT, (wcslen(emoji_str) + 1) * 2, (char*)emoji_str, NULL);
			path[wcslen(path)] = '.';
			HMETAFILE hWmf = CloseMetaFile(hdcMeta);
			HMETAFILEPICT* pmp = (HMETAFILEPICT*)GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
			METAFILEPICT* pmfp = (METAFILEPICT*)GlobalLock(pmp);
			pmfp->mm = MM_ANISOTROPIC;
			pmfp->xExt = MulDiv(size, 2540, dpi);
			pmfp->yExt = MulDiv(size, 2540, dpi);
			pmfp->hMF = hWmf;
			GlobalUnlock(pmp);
			insert_image(richedit, pmp, NULL);
			return true;
		} else return false;
	} else {
		wchar_t placeholder[] = {0xFE0F, 0};
		riched_write(richedit, placeholder);
		return true;
	}
}

int emoji_adder(int i, wchar_t* msg, int pos, int size, HWND chat, int* deleted_wchars) {
	if (msg[i] < 0x23E9) return i;
	int specials = 0, cr = 0;
	int msg_len = wcslen(msg);
	bool is_emoji = false, surr_pair = false, is_flag = false;
	if (i > 0 && msg[i] == 0xFE0F) {
		specials = (i < msg_len - 1 && msg[i+1] == 0x20E3) ? 2 : 1;
		i--;
		cr = msg[i];
		is_emoji = true;
	} else if (msg[i] >= 0xD800 && msg[i] <= 0xDBFF && i < msg_len - 1 && msg[i+1] >= 0xDC00 && msg[i+1] <= 0xDFFF) {
		cr = ((msg[i] - 0xD800) << 10) + (msg[i+1] - 0xDC00) + 0x10000;
		surr_pair = true;
		if (cr == 0x1F0CF ||  cr == 0x1F18E || (cr >= 0x1F191 && cr <= 0x1F19A) || cr == 0x1F201 || (cr >= 0x1F232 && cr != 0x1F237 && cr <= 0x1F23A) || cr == 0x1F250 || cr == 0x1F251
			|| (cr >= 0x1F300 && cr <= 0x1F64F) || (cr >= 0x1F680 && cr <= 0x1F6FF) || (cr >= 0x1F7E0 && cr <= 0x1F7FF) || (cr >= 0x1F900 && cr <= 0x1F9FF) || (cr >= 0x1FA70 && cr <= 0x1FAFF))
			is_emoji = true;
		else if (i < msg_len - 2 && msg[i+2] == 0xFE0F)
			is_emoji = true, specials = 1;
		else if (cr >= 0x1F1E6 && cr <= 0x1F1FF && i < msg_len - 3 && msg[i+2] >= 0xD800 && msg[i+2] <= 0xDBFF && msg[i+3] >= 0xDC00 && msg[i+3] <= 0xDFFF)
			is_emoji = true, surr_pair = true, is_flag = true;
		else i++;
	} else if ((msg[i] >= 0x23E9 && msg[i] <= 0x23EC) || msg[i] == 0x23F0 || msg[i] == 0x23F3 || msg[i] == 0x26C8 || msg[i] == 0x26CE || msg[i] == 0x26D1
		|| msg[i] == 0x26E9 || msg[i] == 0x26F0 || msg[i] == 0x26F1 || msg[i] == 0x26F4 || msg[i] == 0x26F7 || msg[i] == 0x26F8 || msg[i] == 0x2705 || msg[i] == 0x270A
		|| msg[i] == 0x270B || msg[i] == 0x2728 || msg[i] == 0x274C || msg[i] == 0x274E || (msg[i] >= 0x2753 && msg[i] <= 0x2755) || (msg[i] >= 0x2795 && msg[i] <= 0x2797)) {
		cr = msg[i];
		is_emoji = true;
	}
	if (is_emoji) {
		int compound_chars = 0;
		wchar_t file_name[MAX_PATH];
		swprintf(file_name, L"%s\\", get_path(exe_path, L"emojis"));
		if (is_flag) {
			int cr2 = ((msg[i+2] - 0xD800) << 10) + (msg[i+3] - 0xDC00) + 0x10000;
			swprintf(file_name, L"%s%x-%x.ico", file_name, cr, cr2);
			compound_chars = 2;
		} else {
			swprintf(file_name, L"%s%04x", file_name, cr);
			compound_chars = compound_emoji_checker(msg + i + surr_pair + specials, msg_len - i - surr_pair - specials);
			int compound_chars_loop = compound_chars + specials;
			if (compound_chars > 0 || specials > 0) for (int j = i + surr_pair + 1; compound_chars_loop > 0; j++) {
				if (msg[j] >= 0xD800 && msg[j] <= 0xDBFF && j < msg_len -1 && msg[j+1] >= 0xDC00 && msg[j+1] <= 0xDFFF) {
					int cr = ((msg[j] - 0xD800) << 10) + (msg[j+1] - 0xDC00) + 0x10000;
					if (cr != 0x1F3FB && cr != 0x1F3FC && cr != 0x1F3FD && cr != 0x1F3FE && cr != 0x1F3FF) swprintf(file_name, L"%s-%04x", file_name, cr);
					j++;
					compound_chars_loop--;
				} else swprintf(file_name, L"%s-%04x", file_name, msg[j]);
				compound_chars_loop--;
			}	
			wcscat(file_name, L".ico");
		}
		LRESULT res;
		if (chat) SendMessage(chat, EM_SETSEL, pos + i - *deleted_wchars, pos + i - *deleted_wchars + 1 + compound_chars + surr_pair + specials);
		else textHost->textServices->TxSendMessage(EM_SETSEL, pos + i - *deleted_wchars, pos + i - *deleted_wchars + 1 + compound_chars + surr_pair + specials, &res);
		
		if (!insert_emoji(file_name, size, chat)) {
			wchar_t placeholder[] = {0xFE0F, 0};
			riched_write(chat, placeholder);
		}
		*deleted_wchars += compound_chars + surr_pair + specials;
		i += compound_chars + surr_pair + specials;
	}
	return i;
}

int set_reply(int i, int start_footer, BYTE* quote_text, bool setformat) {
	PARAFORMAT pf = {0};
	pf.cbSize = sizeof(pf);
	SendMessage(chat, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
	int written_info = 0;
	StreamData sd = {0};
	EDITSTREAM es = {0};
	es.dwCookie = (DWORD_PTR)&sd;
	es.pfnCallback = StreamOutCallback;
	bool streamadded = false;
	bool toobig = (i != -1 && messages[i].end_char - 1 - messages[i].end_header > 75) ? true : false;
	if (i != -1) {
		SendMessage(chat, EM_SETSEL, messages[i].start_char, quote_text ? messages[i].end_header : (toobig ? messages[i].end_header + 75 : messages[i].end_char - 1));
		SendMessage(chat, EM_STREAMOUT, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es);
		es.pfnCallback = StreamInCallback;
		SendMessage(chat, EM_SETSEL, start_footer + written_info, start_footer + written_info);
		written_info += SendMessage(chat, EM_STREAMIN, SF_RTF | SF_UNICODE | SFF_SELECTION, (LPARAM)&es);
		free(sd.buf);
	}
	int pos_init = start_footer + written_info;
	if (quote_text != (BYTE*)-1) {
		if (quote_text) {
			wchar_t* quote = read_string(quote_text, NULL);
			if (wcslen(quote) > 78) wcscpy(quote + 75, L"...");
			written_info += riched_write(chat, quote);
			int deleted_wchars = 0;
			for (int j = 0; j < wcslen(quote); j++) j = emoji_adder(j, quote, pos_init, 13, chat, &deleted_wchars);
			written_info -= deleted_wchars;
			free(quote);
		} else if (toobig) written_info += riched_write(chat, L"...");
	}

	FINDTEXTEX ft = {0};
	ft.chrg.cpMin = start_footer;
	ft.chrg.cpMax = start_footer + written_info;
	ft.lpstrText = L" \r";
	for (int j = 0; j < 2; j++) {
		while (SendMessage(chat, EM_FINDTEXTEXW, FR_DOWN, (LPARAM)&ft) != -1) {
			CHARRANGE cr = ft.chrgText;
			SendMessage(chat, EM_EXSETSEL, 0, (LPARAM)&cr);
			SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L" ");
			if (j == 0) {
				written_info--;
				ft.chrg.cpMax--;
			}
		}
		ft.lpstrText = L"\r";
	}

	if (setformat) {
		SendMessage(chat, EM_SETSEL, start_footer, start_footer + written_info);
		CHARFORMAT2 cf;
		cf.cbSize = sizeof(cf);
		cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_SIZE | CFM_LINK | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_FACE | CFM_BACKCOLOR | CFM_WEIGHT;
		LOGFONT lf = {0};
		GetObject(hFonts[0], sizeof(lf), &lf);
		wcscpy(cf.szFaceName, lf.lfFaceName);
		cf.dwEffects = CFE_ITALIC;
		cf.crTextColor = colors[3];
		cf.crBackColor = colors[1];
		cf.yHeight = 160;
		cf.wWeight = lf.lfWeight;
		SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
		SendMessage(chat, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
	}

	// scale down emojis from 15x15 to 13x13
	wchar_t obj_char[] = {0xFFFC, 0};
	if (quote_text) ft.chrg.cpMax = pos_init;
	ft.lpstrText = obj_char;
	IRichEditOle* ole = NULL;
	while (SendMessage(chat, EM_FINDTEXTEXW, FR_DOWN, (LPARAM)&ft) != -1) {
		if (ole == NULL) SendMessage(chat, EM_GETOLEINTERFACE, 0, (LPARAM)&ole);
		REOBJECT reo = { sizeof(REOBJECT) };
		reo.cp = ft.chrgText.cpMin;
		if (SUCCEEDED(ole->GetObject(REO_IOB_USE_CP, &reo, REO_GETOBJ_POLEOBJ))) {
			IDataObject* data;
			if (SUCCEEDED(reo.poleobj->QueryInterface(IID_IDataObject, (void**)&data))) {
				FORMATETC fmt = { CF_METAFILEPICT, NULL, DVASPECT_CONTENT, -1, TYMED_MFPICT };
				STGMEDIUM stg = {0};
				if (SUCCEEDED(data->GetData(&fmt, &stg))) {
					METAFILEPICT* mp = (METAFILEPICT*)GlobalLock(stg.hMetaFilePict);
					HDC hdcMeta = CreateMetaFile(NULL);
					SetWindowExtEx(hdcMeta, 15, 15, NULL);
					SetViewportExtEx(hdcMeta, 13, 13, NULL);
					PlayMetaFile(hdcMeta, mp->hMF);
					DeleteMetaFile(mp->hMF);
					GlobalUnlock(stg.hMetaFilePict);

					HMETAFILEPICT* pmp = (HMETAFILEPICT*)GlobalAlloc(GMEM_MOVEABLE, sizeof(METAFILEPICT));
					METAFILEPICT* pmfp = (METAFILEPICT*)GlobalLock(pmp);
					pmfp->hMF = CloseMetaFile(hdcMeta);
					pmfp->mm = MM_ANISOTROPIC;
					pmfp->xExt = MulDiv(13, 2540, dpi);
					pmfp->yExt = MulDiv(13, 2540, dpi);
					GlobalUnlock(pmp);
					
					SendMessage(chat, EM_SETSEL, ft.chrgText.cpMin, ft.chrgText.cpMax);
					insert_image(chat, pmp, NULL);
				}
				data->Release();
			}
			reo.poleobj->Release();
		}
		ft.chrg.cpMin = ft.chrgText.cpMax;
	}
	if (ole != NULL) ole->Release();
	SendMessage(chat, EM_SETSEL, start_footer + written_info, start_footer + written_info);
	return written_info;
}

const wchar_t* get_topic_title(ForumTopic* topic) {
	if (!topic) return L"";
	if (topic->title && wcslen(topic->title) > 0 && !topic->title_missing) {
		return topic->title;
	}
	if (topic->id == 1 || topic->hidden || topic->title_missing) {
		return L"General";
	}
	return (topic->title && wcslen(topic->title) > 0) ? topic->title : L"General";
}

void status_bar_status(Peer* peer) {
	if (!peer) {
		SendMessage(hStatus, SB_SETTEXTA, 0, (LPARAM)"");
		return;
	}
	if (peer->type == 0) {
		swprintf(status_str, L"%s ", peer->name ? peer->name : L"");
		switch (peer->online) {
		case -1:
			SendMessage(hStatus, SB_SETTEXTA, 0, (LPARAM)"");
			return;
		case 0:
			get_lang_string("s_on", lang_str, NULL);
			wcscat(status_str, lang_str);
			break;
		case 1:
			get_lang_string("s_rec", lang_str, NULL);
			wcscat(status_str, lang_str);
			break;
		case 2:
			get_lang_string("s_week", lang_str, NULL);
			wcscat(status_str, lang_str);
			break;
		case 3:
			get_lang_string("s_mon", lang_str, NULL);
			wcscat(status_str, lang_str);
			break;
		default:
			get_lang_string("s_was", lang_str, NULL);
			wcscat(status_str, lang_str);
			wcscat(status_str, L" ");
			get_date(&status_str[wcslen(status_str)], peer->online, true);
			break;
		}
	} else if (peer->type == 1) {
		int count = peer->chat_users ? peer->chat_users->size() : 0;
		get_lang_string("s_par", lang_str, NULL);
		swprintf(status_str, lang_str, count);
	} else {
		int count = (int)peer->chat_users;
		if (peer->perm.cansendmsg) {
			get_lang_string("s_par", lang_str, NULL);
			swprintf(status_str, lang_str, count);
		} else {
			get_lang_string("s_sub", lang_str, NULL);
			swprintf(status_str, lang_str, count);
		}
	}
	if (peer->is_forum && peer->topics && peer->active_topic_id > 0) {
		ForumTopic* at = NULL;
		for (size_t i = 0; i < peer->topics->size(); i++) {
			if (peer->topics->at(i).id == peer->active_topic_id) {
				at = &peer->topics->at(i);
				break;
			}
		}
		if (at) {
			const wchar_t* ttitle = get_topic_title(at);
			wchar_t topic_info[256];
			if (at->closed) {
				swprintf(topic_info, L" | Topic: %s [Closed]", ttitle);
			} else if (at->pinned) {
				swprintf(topic_info, L" | Topic: %s [Pinned]", ttitle);
			} else {
				swprintf(topic_info, L" | Topic: %s", ttitle);
			}
			wcscat(status_str, topic_info);
		}
	}
	SendMessage(hStatus, SB_SETTEXTA, 0 | SBT_OWNERDRAW, peer->name ? wcslen(peer->name) : 0);
}

void user_status_updated(BYTE* userStatus, Peer* peer) {
	if (peer->online == -1) return;
	switch (read_le(userStatus, 4)) {
	case 0x9d05049:
		peer->online = -1;
		break;
	case 0xedb93949:
		peer->online = 0;
		break;
	case 0x8c703f:
		peer->online = read_le(userStatus + 4, 4);
		break;
	case 0x7b197dc8:
		peer->online = 1;
		break;
	case 0x541a1d1a:
		peer->online = 2;
		break;
	case 0x65899777:
		peer->online = 3;
		break;
	}
	if (current_peer == peer) status_bar_status(peer);
}

void delete_message(int j, bool scroll) {
	for (int k = 0; k < documents.size(); k++) {
		if (documents[k].min >= messages[j].start_char && documents[k].max <= messages[j].end_char) {
			free(documents[k].filename);
			documents.erase(documents.begin() + k);
		} else if (documents[k].min > messages[j].end_footer) break;
	}
	for (k = 0; k < links.size(); k++) if (links[k].chrg.cpMin >= messages[j].start_char && links[k].chrg.cpMax <= messages[j].end_char) {
		free(links[k].lpstrText);
		links.erase(links.begin() + k);
	}
	CHARRANGE cr;
	cr.cpMin = messages[j].start_char;
	cr.cpMax = messages[j].end_footer;
	replace_in_chat(NULL, &cr, L"", NULL, NULL, NULL, NULL);
	messages.erase(messages.begin() + j);
	if (scroll) SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
}

void apply_theme(int i) {
	HMENU hMenuTheme = GetSubMenu(GetSubMenu(hMenuBar, 1), 2);
	for (int j = 0; j < themes.size() + 1; j++) CheckMenuItem(hMenuTheme, j, MF_BYPOSITION | MF_UNCHECKED);
	CheckMenuItem(hMenuTheme, i, MF_BYPOSITION | MF_CHECKED);
	if (theme_brush) DeleteObject(theme_brush);
	if (CHATTHEMES || theme_brush) {
		if (i == 0 || !CHATTHEMES) {
			theme_brush = NULL;
			SetClassLongPtr(hMain, GCLP_HBRBACKGROUND, (LONG_PTR)hBrushes[0]);
		} else {
			theme_brush = CreateSolidBrush(themes[i - 1].color);
			SetClassLongPtr(hMain, GCLP_HBRBACKGROUND, (LONG_PTR)theme_brush);
		}
		update_toolbar();
		InvalidateRect(hMain, NULL, TRUE);
	}
}

void register_themes() {
	HMENU hMenuChat = GetSubMenu(hMenuBar, 1);
	HMENU hMenuTheme = GetSubMenu(hMenuChat, 2);
	int count = GetMenuItemCount(hMenuTheme);
	MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
    mii.fMask = MIIM_BITMAP;
	for (int i = 1; i < count; i++) {
		GetMenuItemInfo(hMenuTheme, 1, TRUE, &mii);
		DeleteObject(mii.hbmpItem);
		RemoveMenu(hMenuTheme, 1, MF_BYPOSITION);
	}
	for (i = 0; i < themes.size(); i++) {
		wchar_t* emoji_str = read_string(themes[i].emoji_id, NULL);
		wchar_t file_name[MAX_PATH];
		swprintf(file_name, L"%s\\", get_path(exe_path, L"emojis"));
		wemoji_to_path(emoji_str, file_name, true);
		free(emoji_str);
		HICON hIcon = (HICON)LoadImage(NULL, file_name, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
		if (nt3) hIcon = (HICON)(GetFileAttributes(file_name) != -1);
		if (!hIcon) {
			wcscpy(file_name + wcslen(file_name) - 4, L"-fe0f.ico");
			hIcon = (HICON)LoadImage(NULL, file_name, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
			if (nt3) hIcon = (HICON)(GetFileAttributes(file_name) != -1);
		}
		HDC hdcScreen = GetDC(NULL);
		HDC hdcMem = CreateCompatibleDC(hdcScreen);
    
		HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, 45, 15);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
    
		HBRUSH hSolidBrush = CreateSolidBrush(themes[i].color);
		RECT rcSolid = {0, 0, 45, 15};
		FillRect(hdcMem, &rcSolid, hSolidBrush);
		DeleteObject(hSolidBrush);

		if (hIcon) {
			if (nt3) {
				RECT rc = {15, 0, 0, 0};
				paint_emoji_bitmap(hdcMem, file_name, &rc);
			} else {
				DrawIconEx(hdcMem, 15, 0, hIcon, 15, 15, 0, NULL, DI_NORMAL);
				DestroyIcon(hIcon);
			}
		}
    
		SelectObject(hdcMem, hOldBitmap);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdcScreen);
		AppendMenu(hMenuTheme, MF_BITMAP, 601 + i, (LPCTSTR)hBitmap);
	}
}

void set_permissions(BYTE* unenc_response, Peer* peer) {
	int perm_flags = read_le(unenc_response + 4, 4);
	peer->perm.cansendmsg = (perm_flags & (1 << 1)) ? false : true;
	peer->perm.cansendmed = (perm_flags & (1 << 2)) ? false : true;
	peer->perm.cansendvoice = (perm_flags & (1 << 23)) ? false : true;
	peer->perm.cansendphoto = (perm_flags & (1 << 19)) ? false : true;
	peer->perm.cansendvideo = (perm_flags & (1 << 20)) ? false : true;
	peer->perm.cansendaudio = (perm_flags & (1 << 22)) ? false : true;
	peer->perm.canchangedesc = (perm_flags & (1 << 10)) ? false : true;
	peer->perm.cansenddocs = (perm_flags & (1 << 24)) ? false : true;
}

void get_channel_difference(Peer* peer) {
	BYTE unenc_query[96];
	BYTE enc_query[120];
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, 40, 4);
	write_le(unenc_query + 32, 0x3173d78, 4);
	memset(unenc_query + 36, 0, 4);
	write_le(unenc_query + 40, 0xf35aec28, 4);
	memcpy(unenc_query + 44, peer->id, 8);
	memcpy(unenc_query + 52, peer->access_hash, 8);
	write_le(unenc_query + 60, 0x94d42ee7, 4);
	write_le(unenc_query + 64, peer->channel_pts, 4);
	write_le(unenc_query + 68, 100, 4);
	fortuna_read(unenc_query + 72, 24, &prng);
	convert_message(unenc_query, enc_query, 96, 0);
	send_query(enc_query, 120);
	memcpy(peer->channel_msg_id, unenc_query + 16, 8);
}

void get_peerid_from_msg(BYTE* unenc_response, BYTE** id, BYTE** msg_id) {
	int offset = 0;
	int cons = read_le(unenc_response + offset, 4);
	bool service = is_message_service_constructor(cons);
	if (service) message_handler(true, unenc_response + offset, false, false, false); // for themes
	int flags_msg = read_le(unenc_response + offset + 4, 4);
	int flags_msg2 = 0;
	if (!service) flags_msg2 = read_le(unenc_response + offset + 8, 4);
	if (service) offset += 12;
	else offset += 16;
	if (msg_id) *msg_id = &unenc_response[offset-4];
	if (flags_msg & (1 << 8)) offset += 12;
	if (flags_msg & (1 << 29)) offset += 4;
	if (!service && (flags_msg2 & (1 << 12))) offset += tlstr_len(unenc_response + offset, true);
	offset += 4;
	*id = &unenc_response[offset];
}

BYTE* get_message_peer_id(BYTE* message) {
	int msg_cons = read_le(message, 4);
	if (msg_cons == TL_MESSAGE_EMPTY) {
		int flags = read_le(message + 4, 4);
		if (flags & (1 << 0)) return message + 16;
		return NULL;
	}
	bool service = is_message_service_constructor(msg_cons);
	int flags_msg = read_le(message + 4, 4);
	int flags_msg2 = 0;
	if (!service) flags_msg2 = read_le(message + 8, 4);
	int offset = service ? 12 : 16;
	if (flags_msg & (1 << 8)) offset += 12;
	if (flags_msg & (1 << 29)) offset += 4;
	if (!service && (flags_msg2 & (1 << 12))) offset += tlstr_len(message + offset, true);
	offset += 4; // skip peer constructor (e.g. peerChannel, peerChat, peerUser)
	return &message[offset];
}

int get_msg_id_from_raw(BYTE* message) {
	if (!message) return 0;
	int cons = read_le(message, 4);
	if (cons == TL_MESSAGE_EMPTY || is_message_service_constructor(cons)) return read_le(message + 8, 4);
	return read_le(message + 12, 4);
}

HBITMAP jpg_to_bmp(BYTE* myjpg, int myjpg_size) {
	jpeg_decompress_struct cinfo;
	jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, myjpg, myjpg_size);
	jpeg_read_header(&cinfo, TRUE);
	jpeg_start_decompress(&cinfo);

	int width = cinfo.output_width;
	int height = cinfo.output_height;
	BYTE* rgb = (BYTE*)malloc(width * height * 3);
	while (cinfo.output_scanline < cinfo.output_height) {
		BYTE* dest = &rgb[cinfo.output_scanline * width * 3];
		jpeg_read_scanlines(&cinfo, &dest, 1);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return rgb_to_bmp(rgb, false, width, height);
}

void get_full_peer(Peer* peer) {
	if (peer->full) {
		if (peer->about) {
			free(peer->about);
			peer->about = NULL;
		}
		if (peer->reaction_list && peer->reaction_list != &reaction_list) {
			if (is_valid_reaction_list(peer->reaction_list)) {
				for (int i = 0; i < peer->reaction_list->size(); i++) free(peer->reaction_list->at(i));
				peer->reaction_list->clear();
				delete peer->reaction_list;
			}
			peer->reaction_list = &reaction_list;
		}
		if (peer->type == 1 && peer->chat_users) {
			for (int j = 0; j < peer->chat_users->size(); j++) {
				free(peer->chat_users->at(j).name);
				if (peer->chat_users->at(j).handle) free(peer->chat_users->at(j).handle);
			}
			peer->chat_users->clear();
		}
	}
	if (peer->type == 0) {
		// users.getFullUser
		BYTE unenc_query[80];
		BYTE enc_query[104];
		internal_header(unenc_query, true);
		write_le(unenc_query + 28, 24, 4);
		write_le(unenc_query + 32, 0xb60f5918, 4);
		write_le(unenc_query + 36, 0xf21158c6, 4);
		memcpy(unenc_query + 40, peer->id, 8);
		memcpy(unenc_query + 48, peer->access_hash, 8);
		fortuna_read(unenc_query + 56, 24, &prng);
		convert_message(unenc_query, enc_query, 80, 0);
		send_query(enc_query, 104);
	} else if (peer->type == 1) {
		// messages.getFullChat
		BYTE unenc_query[64];
		BYTE enc_query[88];
		internal_header(unenc_query, true);
		write_le(unenc_query + 28, 12, 4);
		write_le(unenc_query + 32, 0xaeb00b34, 4);
		memcpy(unenc_query + 36, peer->id, 8);
		fortuna_read(unenc_query + 44, 20, &prng);
		convert_message(unenc_query, enc_query, 64, 0);
		send_query(enc_query, 88);
	} else if (peer->type == 2) {
		// channels.getFullChannel
		BYTE unenc_query[80];
		BYTE enc_query[104];
		internal_header(unenc_query, true);
		write_le(unenc_query + 28, 24, 4);
		write_le(unenc_query + 32, 0x8736a09, 4);
		write_le(unenc_query + 36, 0xf35aec28, 4);
		memcpy(unenc_query + 40, peer->id, 8);
		memcpy(unenc_query + 48, peer->access_hash, 8);
		fortuna_read(unenc_query + 56, 24, &prng);
		convert_message(unenc_query, enc_query, 80, 0);
		send_query(enc_query, 104);
	}
}

void update_name_in_list(int i) {
	if (current_peer == &peers[i]) {
		InvalidateRect(hComboBoxChats, NULL, TRUE);
		if (peers[i].type == 0) status_bar_status(&peers[i]);
	}
}

void get_state(bool problem) {
	BYTE unenc_query[48];
	BYTE enc_query[72];
	internal_header(unenc_query, true);
	if (problem) memcpy(difference_msg_id, unenc_query + 16, 8);
	write_le(unenc_query + 28, 4, 4);
	write_le(unenc_query + 32, 0xedd4882a, 4);
	fortuna_read(unenc_query + 36, 12, &prng);
	convert_message(unenc_query, enc_query, 48, 0);
	send_query(enc_query, 72);
}

void get_message(int id, Peer* peer) {
	if (peer->type != 2) {
		BYTE unenc_query[64];
		BYTE enc_query[88];
		internal_header(unenc_query, true);
		write_le(unenc_query + 28, 20, 4);
		write_le(unenc_query + 32, 0x63c66506, 4);
		write_le(unenc_query + 36, 0x1cb5c415, 4);
		write_le(unenc_query + 40, 1, 4);
		write_le(unenc_query + 44, 0xa676a322, 4);
		write_le(unenc_query + 48, id, 4);
		fortuna_read(unenc_query + 52, 12, &prng);
		convert_message(unenc_query, enc_query, 64, 0);
		send_query(enc_query, 88);
	} else {
		BYTE unenc_query[96];
		BYTE enc_query[120];
		internal_header(unenc_query, true);
		write_le(unenc_query + 28, 40, 4);
		write_le(unenc_query + 32, 0xad8c9a23, 4);
		place_peer(unenc_query + 36, peer, true);
		write_le(unenc_query + 56, 0x1cb5c415, 4);
		write_le(unenc_query + 60, 1, 4);
		write_le(unenc_query + 64, 0xa676a322, 4);
		write_le(unenc_query + 68, id, 4);
		fortuna_read(unenc_query + 72, 24, &prng);
		convert_message(unenc_query, enc_query, 96, 0);
		send_query(enc_query, 120);
	}
}

void try_to_add_fe0f(wchar_t* file_name) {
	file_name = wcsrchr(file_name, L'\\') + 1;
	if (wcschr(file_name, L'-')) {
		if (wcsncmp(file_name, L"2764", 4) == 0 || wcsncmp(file_name, L"00", 2) == 0) {
			memmove(file_name + 9, file_name + 4, (wcslen(file_name + 4) + 1) * 2);
			wcsncpy(file_name + 4, L"-fe0f", 5);
		} else if (wcsncmp(file_name + 11, L"2764", 4) == 0) {
			memmove(file_name + 20, file_name + 15, (wcslen(file_name + 15) + 1) * 2);
			wcsncpy(file_name + 15, L"-fe0f", 5);
		} else if (wcsncmp(file_name, L"1f3f3", 5) == 0) {
			memmove(file_name + 10, file_name + 5, (wcslen(file_name + 5) + 1) * 2);
			wcsncpy(file_name + 5, L"-fe0f", 5);
			if (file_name[16] == L'2') wcscpy(file_name + wcslen(file_name) - 4, L"-fe0f.ico");
		} else if (wcsncmp(file_name, L"1f441", 5) == 0) wcscpy(file_name, L"1f441-fe0f-200d-1f5e8-fe0f.ico");
		else wcscpy(file_name + wcslen(file_name) - 4, L"-fe0f.ico");
	} else wcscpy(file_name + wcslen(file_name) - 4, L"-fe0f.ico");
}

void unknown_custom_emoji_solver(int msg_id, int pos, int size, __int64 custom_emoji_id, bool reaction) {
	if (!EMOJIS) return;
	CustomEmojiPlacement cep;
	if (!reaction) {
		memcpy(cep.peer_id, current_peer->id, 8);
		cep.msg_id = msg_id;
		cep.pos = pos;
		cep.size = size;
	}
	bool found = false;
	for (int j = 0; j < rces.size(); j++) {
		if (rces[j].id == custom_emoji_id) {
			if (!reaction) rces[j].ceps.push_back(cep);
			found = true;
			break;
		}
	}
	if (!found) {
		RequestedCustomEmoji rce;
		rce.id = custom_emoji_id;
		rce.access_hash = 0;
		memset(rce.msg_id, 0, 8);
		if (!reaction) rce.ceps.push_back(cep);
		rce.reaction = reaction;
		rces.push_back(rce);
	}
}

int set_reactions(BYTE* reactions, Message* message_footer, std::vector<int>* format_vecs, int modifier, int msg_id) {
	int written_info = 0;
	int count = read_le(reactions, 4);
	int offset = 4;
	bool firstemojiset = false;
	if (!message_footer) {
		int idx = read_le(reactions - 4, 4);
		if (idx >= 0 && idx < (int)messages.size()) message_footer = &messages[idx];
		else return 0;
	}
	for (int i = 0; i < count; i++) {
		bool set = false;
		if (reactions[offset + 4] == 1) {
			offset += 4;
			set = true;
		}
		offset += 8;
		int cons = read_le(reactions + offset, 4);
		if (cons == 0x79f5d419) offset += 8;
		else {
			wchar_t file_name[MAX_PATH];
			offset += 4;
			wchar_t emoji_str[10] = {0};
			__int64 custom_emoji_id = 0;
			if (cons == 0x1b2286b8) {
				read_string(reactions + offset, emoji_str);
				offset += tlstr_len(reactions + offset, true);
			}
			if (cons == 0x8935fc73) {
				custom_emoji_id = read_le(reactions + offset, 8);
				swprintf(file_name, L"%s\\%016I64X.ico", get_path(appdata_path, L"custom_emojis"), custom_emoji_id);
				offset += 8;
			} else swprintf(file_name, L"%s\\", get_path(exe_path, L"emojis"));
			int react_count = read_le(reactions + offset, 4);
			offset += 4;
			wchar_t divider[20];
			if (!firstemojiset) swprintf(divider, L" | %d ", react_count);
			else swprintf(divider, L"   %d ", react_count);
			written_info += riched_write(chat, divider);
			if (cons == 0x1b2286b8) wemoji_to_path(emoji_str, file_name, true);
			else if (cons == 0x523da4eb) wcscat(file_name, L"2b50.ico");
			wchar_t* file_name_code = wcsrchr(file_name, L'\\') + 1;
			int code_len = wcsrchr(file_name, L'.') - file_name_code;
			if (set) {
				HWND child = GetWindow(reactionStatic, GW_CHILD);
				while (child != NULL) {
					HWND next = GetWindow(child, GW_HWNDNEXT);
					wchar_t* code = (wchar_t*)GetWindowLongPtr(child, GWLP_USERDATA);
					if (cons == 0x8935fc73) {
						if (wcsncmp(code + 1, file_name_code, code_len) == 0) {
							message_footer->reacted.push_back(code);
							break;
						}
					} else if (wcsncmp(code, file_name_code, code_len) == 0) {
						message_footer->reacted.push_back(code);
						break;
					}
					child = next;
				}
				char num[10];
				sprintf(num, "%d ", react_count);
				format_vecs[0].push_back(modifier + written_info - strlen(num));
				format_vecs[0].push_back(modifier + written_info - 1);
			}
			if (!insert_emoji(file_name, 13, chat)) {
				if (cons == 0x8935fc73) {
					SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"?");
					unknown_custom_emoji_solver(msg_id, written_info, 13, custom_emoji_id, false);
				} else {
					try_to_add_fe0f(file_name);
					if (!insert_emoji(file_name, 13, chat)) SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"?");
				}
			}
			firstemojiset = true;
			written_info++;
		}
	}
	if (!firstemojiset) SendMessage(chat, EM_REPLACESEL, FALSE, (LPARAM)L"");
	return written_info;
}

void update_pts(int new_pts) {
#ifdef _DEBUG
	if (pts - new_pts > 100 || (pts != 0 && new_pts - pts > 100000)) {
		MessageBox(hMain, L"Pts error!", L":(", MB_OK | MB_ICONERROR);
	}
#endif
	pts = new_pts;
}

void peer_set_name(BYTE* unenc_response, wchar_t** name, char type) {
	if (!unenc_response) return;
	if (type == 0) {
		int flags = read_le(unenc_response + 4, 4);
		int offset = 20;
		if (flags & (1 << 0)) offset += 8;
		if (flags & (1 << 2)) offset += set_name(unenc_response + offset, name);
		else *name = read_string(unenc_response + offset, NULL);
	} else if (type == 1) *name = read_string(unenc_response + 16, NULL);
	else {
		int cons = read_le(unenc_response, 4);
		if (cons == TL_CHANNEL_FORBIDDEN) {
			*name = read_string(unenc_response + 24, NULL);
		} else {
			int flags = read_le(unenc_response + 4, 4);
			int offset = 20;
			if (flags & (1 << 13)) offset += 8;
			*name = read_string(unenc_response + offset, NULL);
		}
	}
}

void get_photo(RequestedCustomEmoji* rce, Document* document, DCInfo* dcInfo) {
	BYTE unenc_query[144];
	BYTE enc_query[168];
	internal_header(dcInfo, unenc_query, true);
	memcpy(rce ? rce->msg_id : document->photo_msg_id, unenc_query + 16, 8);
	write_le(unenc_query + 32, 0xbe5335be, 4);
	write_le(unenc_query + 36, 0, 4);
	write_le(unenc_query + 40, (rce || document->photo_size == 1) ? 0xbad07584 : 0x40181ffe, 4);
	memcpy(unenc_query + 44, rce ? (BYTE*)&rce->id : document->id, 8);
	memcpy(unenc_query + 52, rce ? (BYTE*)&rce->access_hash : document->access_hash, 8);
	int fileref_len = tlstr_len(rce ? rce->file_reference : document->file_reference, true);
	memcpy(unenc_query + 60, rce ? rce->file_reference : document->file_reference, fileref_len);
	int offset_query = 60 + fileref_len;
	memset(unenc_query + offset_query, 0, 12);
	unenc_query[offset_query] = 1;
	unenc_query[offset_query + 1] = 'm';
	offset_query += 12;
	write_le(unenc_query + offset_query, 1048576, 4);
	offset_query += 4;
	write_le(unenc_query + 28, offset_query - 32, 4);
	int padding_len = get_padding(offset_query);
	fortuna_read(unenc_query + offset_query, padding_len, &prng);
	offset_query += padding_len;
	convert_message(dcInfo, unenc_query, enc_query, offset_query, 0);
	send_query(dcInfo, enc_query, offset_query + 24);
}

void get_pfp(DCInfo* dcInfo, Peer* peer) {
	BYTE unenc_query[112];
	BYTE enc_query[136];
	internal_header(unenc_query, true);
	write_le(unenc_query + 32, 0xbe5335be, 4);
	memset(unenc_query + 36, 0, 4);
	write_le(unenc_query + 40, 0x37257e99, 4);
	memset(unenc_query + 44, 0, 4);
	int offset = 48 + place_peer(unenc_query + 48, peer, true);
	memcpy(unenc_query + offset, peer->photo, 8);
	offset += 8;
	memset(unenc_query + offset, 0, 8);
	offset += 8;
	write_le(unenc_query + offset, 1048576, 4);
	offset += 4;
	write_le(unenc_query + 28, offset - 32, 4);
	int padding_len = get_padding(offset);
	fortuna_read(unenc_query + offset, padding_len, &prng);
	offset += padding_len;
	convert_message(unenc_query, enc_query, offset, 0);
	send_query(enc_query, offset + 24);
	memcpy(pfp_msgid, unenc_query + 16, 8);
}

void send_ping(DCInfo* dcInfo) {
	BYTE unenc_query[64];
	BYTE enc_query[88];
	internal_header(dcInfo, unenc_query, false);
	write_le(unenc_query + 28, 12, 4);
	write_le(unenc_query + 32, 0x7abe77ec, 4);
	memset(unenc_query + 36, 0, 8);
	fortuna_read(unenc_query + 44, 20, &prng);
	convert_message(dcInfo, unenc_query, enc_query, 64, 0);
	send_query(dcInfo, enc_query, 88);
}

wchar_t* get_winver() {
	static OSVERSIONINFOEX osviex;
	static OSVERSIONINFO osvi;
	static wchar_t winver_buf[64] = {0};
	if (winver_buf[0] != 0) return winver_buf;

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	bool server = false;
	const wchar_t* winver = NULL;
	if (osvi.dwMajorVersion >= 5) {
		osviex.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
		GetVersionEx((OSVERSIONINFO*)&osviex);
		if (osviex.wProductType == VER_NT_SERVER) server = true;
	}
	if (osvi.dwMajorVersion == 3) {
		if (osvi.dwMinorVersion == 10) winver = L"Windows NT 3.1";
		else if (osvi.dwMinorVersion == 50) winver = L"Windows NT 3.5";
		else if (osvi.dwMinorVersion == 51) winver = L"Windows NT 3.51";
	} else if (osvi.dwMajorVersion == 4) {
		if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT) {
			if (server) winver = L"Windows NT 4.0 Server";
			else winver = L"Windows NT 4.0";
		} else if (osvi.dwMinorVersion == 0) winver = L"Windows 95";
		else if (osvi.dwMinorVersion == 10) {
			if (wcscmp(osvi.szCSDVersion, L" A ") != 0) winver = L"Windows 98 FE";
			else winver = L"Windows 98 SE";
		} else if (osvi.dwMinorVersion == 90) winver = L"Windows ME";
	} else if (osvi.dwMajorVersion == 5) {
		if (osvi.dwMinorVersion == 0) {
			if (server) winver = L"Windows 2000 Server";
			else winver = L"Windows 2000";
		} else if (osvi.dwMinorVersion == 1) winver = L"Windows XP";
		else if (osvi.dwMinorVersion == 2) winver = L"Windows Server 2003";
	} else if (osvi.dwMajorVersion == 6) {
		if (osvi.dwMinorVersion == 0) {
			if (server) winver = L"Windows Server 2008";
			else winver = L"Windows Vista";
		} else if (osvi.dwMinorVersion == 1) {
			if (server) winver = L"Windows Server 2008 R2";
			else winver = L"Windows 7";
		} else if (osvi.dwMinorVersion == 2) {
			if (server) winver = L"Windows Server 2012 or above";
			else winver = L"Windows 8 or above";
		}
	}
	if (winver == NULL) {
		if (server) winver = L"Windows Server";
		else winver = L"Windows";
	}
	wcscpy(winver_buf, winver);
	return winver_buf;
}

int write_init_connection(BYTE* buf) {
	int offset = 0;
	write_le(buf + offset, TL_INVOKE_WITH_LAYER, 4); offset += 4;
	write_le(buf + offset, MTPROTO_LAYER, 4); offset += 4;
	write_le(buf + offset, TL_INIT_CONNECTION, 4); offset += 4;
	memset(buf + offset, 0, 4); offset += 4;
	write_le(buf + offset, 27752131, 4); offset += 4;

	wchar_t pc_name[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = sizeof(pc_name);
	if (!GetComputerName(pc_name, &size)) wcscpy(pc_name, L"PC");
	write_string(buf + offset, pc_name);
	offset += tlstr_len(buf + offset, true);

	write_string(buf + offset, get_winver());
	offset += tlstr_len(buf + offset, true);

	write_string(buf + offset, version ? version : L"1.0.4");
	offset += tlstr_len(buf + offset, true);

	write_string(buf + offset, L"en");
	offset += tlstr_len(buf + offset, true);

	write_string(buf + offset, L"tdesktop");
	offset += tlstr_len(buf + offset, true);

	write_string(buf + offset, L"en");
	offset += tlstr_len(buf + offset, true);

	return offset;
}

void send_init_connection(DCInfo* dcInfo) {
	telegacy_log("[send_init_connection] DC %d, layer=%d", dcInfo ? dcInfo->dc : 0, MTPROTO_LAYER);
	BYTE unenc_query[320];
	BYTE enc_query[344];
	internal_header(dcInfo, unenc_query, true);
	int offset = 32 + write_init_connection(unenc_query + 32);
	write_le(unenc_query + offset, TL_HELP_GET_CONFIG, 4);
	offset += 4;
	write_le(unenc_query + 28, offset - 32, 4);
	int padding_len = get_padding(offset);
	fortuna_read(unenc_query + offset, padding_len, &prng);
	offset += padding_len;
	convert_message(dcInfo, unenc_query, enc_query, offset, 0);
	send_query(dcInfo, enc_query, offset + 24);
}

void get_config() {
	send_init_connection(&dcInfoMain);
}

void get_unknown_custom_emojis() {
	if (rces.size() && !rces.back().access_hash) {
		int size = 44 + rces.size() * 8;
		size += get_padding(size);
		BYTE* unenc_query = (BYTE*)malloc(size);
		BYTE* enc_query = (BYTE*)malloc(size + 24);
		internal_header(unenc_query, true);
		int offset = 44;
		for (int i = 0; i < rces.size(); i++) {
			if (!rces[i].access_hash) {
				rces[i].access_hash = 1;
				memcpy(unenc_query + offset, &rces[i].id, 8);
				offset += 8;
			}
		}
		write_le(unenc_query + 28, offset - 32, 4);
		write_le(unenc_query + 32, 0xd9ab0f54, 4);
		write_le(unenc_query + 36, 0x1cb5c415, 4);
		write_le(unenc_query + 40, (offset - 44) / 8, 4);
		fortuna_read(unenc_query + offset, size - offset, &prng);
		convert_message(unenc_query, enc_query, size, 0);
		send_query(enc_query, size + 24);
		free(unenc_query);
		free(enc_query);
	}
}

void exit_telegacy() {
	if (dcInfoMain.authorized) {
		ShowWindow(hMain, SW_HIDE);
		closing = true;
		if (!update_own_status(false)) DestroyWindow(hMain);
	}
}

int save_dcs(BYTE* unenc_response, BYTE* this_dc) {
	int dc_count = read_le(unenc_response, 4);
	FILE* f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"rb+");
	if (!f) {
		f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"wb+");
		if (f) {
			int def_this_dc = read_le(this_dc, 4);
			int def_dc_count = 5;
			fwrite(&def_this_dc, 4, 1, f);
			fwrite(&def_dc_count, 4, 1, f);
			static const char* init_dc_ips[] = {
				"149.154.175.55",  // DC 1
				"149.154.167.50",  // DC 2
				"149.154.175.100", // DC 3
				"149.154.167.92",  // DC 4
				"91.108.56.122"    // DC 5
			};
			int def_port = 443;
			for (int d = 0; d < 5; d++) {
				char ip_buf[16] = {0};
				strncpy(ip_buf, init_dc_ips[d], 15);
				fwrite(ip_buf, 1, 16, f);
				fwrite(&def_port, 4, 1, f);
			}
		}
	}
	if (f) {
		fseek(f, 0, SEEK_SET);
		fwrite(this_dc, 1, 4, f); // this dc
	}
	int offset = 4;
	for (int i = 0; i < dc_count; i++) {
		int flags = read_le(unenc_response + offset + 4, 4);
		int dc_id = read_le(unenc_response + offset + 8, 4);
		int ip_len = unenc_response[offset + 12];
		int ip_str_len = tlstr_len(unenc_response + offset + 12, true);
		int port = read_le(unenc_response + offset + 12 + ip_str_len, 4);
		if ((flags & 0x0B) == 0 && dc_id >= 1 && dc_id <= 5 && ip_len > 0 && ip_len < 16 && port > 0 && f) {
			char ip[16] = {0};
			memcpy(ip, unenc_response + offset + 13, ip_len);
			fseek(f, 8 + (dc_id - 1) * 20, SEEK_SET);
			fwrite(ip, 1, 16, f);
			fwrite(&port, 4, 1, f);
		}
		offset += 16 + ip_str_len;
		if (flags & (1 << 10)) offset += tlstr_len(unenc_response + offset, true);
	}
	if (f) fclose(f);
	return offset;
}

void apply_notifysettings(BYTE* unenc_response, int* mute_until, bool* notifications_muted, __int64 id) {
	int notify_flags = read_le(unenc_response + 4, 4);
	int offset_dlg = 8;
	if (NOTIFY_HAS_SHOW_PREVIEWS(notify_flags)) offset_dlg += 4;
	if (NOTIFY_HAS_SILENT(notify_flags)) {
		int silent_cons = read_le(unenc_response + offset_dlg, 4);
		if (notifications_muted) {
			if (silent_cons == TL_BOOL_TRUE) *notifications_muted = true;
			else if (silent_cons == TL_BOOL_FALSE) *notifications_muted = false;
		}
		offset_dlg += 4;
	}
	if (NOTIFY_HAS_MUTE_UNTIL(notify_flags)) {
		int mute_val = read_le(unenc_response + offset_dlg, 4);
		if (mute_until) {
			if (mute_val > current_time()) {
				*mute_until = mute_val;
				if (*mute_until != TL_MUTE_FOREVER) {
					int time_diff = *mute_until - current_time();
					if (time_diff > 0) {
						UnmuteTimer ut;
						ut.peer_id = id;
						ut.timer_id = 50 + unmuteTimers.size();
						unmuteTimers.push_back(ut);
						SetTimer(hMain, ut.timer_id, time_diff * 1000, NULL);
					} else *mute_until = notifications_muted ? -1 : 0;
				}
			} else {
				*mute_until = notifications_muted ? -1 : 0;
			}
		}
		offset_dlg += 4;
	} else if (mute_until) *mute_until = 0;
}

int pending_notif_msg_id = 0;
NotificationContext active_balloon_ctx = {0};

struct NotifDlgInit {
	NOTIFYICONDATAV2 nid;
	NotificationContext ctx;
};

int get_message_topic_id(BYTE* msgrpl, Peer* peer) {
	if (!msgrpl) return 1;
	if (!peer) peer = current_peer;
	int cons = read_le(msgrpl, 4);
	if (cons == 0xe5af939) return 1;
	int rflags = read_le(msgrpl + 4, 4);
	if (rflags & (1 << 1)) { // reply_to_top_id
		int roff = 8;
		if (rflags & (1 << 4)) roff += 4;
		if (rflags & (1 << 0)) roff += 12;
		if (rflags & (1 << 5)) roff += msgfwd_offset(msgrpl + roff);
		if (rflags & (1 << 8)) roff += messagemedia_offset(msgrpl + roff);
		int top_id = read_le(msgrpl + roff, 4);
		if (top_id > 0) return top_id;
	}
	if (rflags & (1 << 4)) { // reply_to_msg_id
		int rmsg_id = read_le(msgrpl + 8, 4);
		if (rflags & (1 << 3)) { // forum_topic
			return rmsg_id > 0 ? rmsg_id : 1;
		}
		if (peer && peer->is_forum && peer->topics) {
			for (size_t t = 0; t < peer->topics->size(); t++) {
				if (peer->topics->at(t).id == rmsg_id) {
					return rmsg_id;
				}
			}
		}
	}
	return 1;
}

void select_topic_by_id(int topic_id) {
	if (!current_peer || !current_peer->is_forum) return;
	if (hComboBoxTopics && IsWindow(hComboBoxTopics)) {
		int count = SendMessage(hComboBoxTopics, CB_GETCOUNT, 0, 0);
		for (int t = 0; t < count; t++) {
			int tid = (int)SendMessage(hComboBoxTopics, CB_GETITEMDATA, t, 0);
			if (tid == topic_id) {
				if (SendMessage(hComboBoxTopics, CB_GETCURSEL, 0, 0) != t) {
					SendMessage(hComboBoxTopics, CB_SETCURSEL, t, 0);
					SendMessage(hMain, WM_COMMAND, MAKEWPARAM(9, CBN_SELCHANGE), (LPARAM)hComboBoxTopics);
				}
				return;
			}
		}
	}
	current_peer->active_topic_id = topic_id;
	current_peer->requested_topic_id = topic_id;
}

void navigate_to_notification(NotificationContext* ctx) {
	if (!ctx) return;
	BYTE target_peer_id[8];
	memcpy(target_peer_id, ctx->peer_id, 8);
	int target_topic_id = ctx->topic_id;
	int target_msg_id = ctx->message_id;

	bool need_async_history = false;
	bool found = false;
	if (current_peer && memcmp(current_peer->id, target_peer_id, 8) == 0) {
		found = true;
		if (current_peer->is_forum && target_topic_id > 0 && current_peer->active_topic_id != target_topic_id) {
			need_async_history = true;
			select_topic_by_id(target_topic_id);
		} else {
			SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
			SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
		}
	}
	if (!found) for (int i = 0; i < current_folder->count; i++) {
		if (memcmp(target_peer_id, peers[current_folder->peers[i]].id, 8) == 0) {
			Peer* p = &peers[current_folder->peers[i]];
			if (p->is_forum && target_topic_id > 0) {
				p->requested_topic_id = target_topic_id;
				p->active_topic_id = target_topic_id;
			}
			need_async_history = true;
			SendMessage(hComboBoxChats, CB_SETCURSEL, i, 0);
			SendMessage(hMain, WM_COMMAND, MAKEWPARAM(3, CBN_SELCHANGE), (LPARAM)hComboBoxChats);
			found = true;
			break;
		}
	}
	if (!found && current_folder != &folders[0]) for (int i = 0; i < folders[0].count; i++) {
		if (memcmp(target_peer_id, peers[folders[0].peers[i]].id, 8) == 0) {
			Peer* p = &peers[folders[0].peers[i]];
			if (p->is_forum && target_topic_id > 0) {
				p->requested_topic_id = target_topic_id;
				p->active_topic_id = target_topic_id;
			}
			need_async_history = true;
			SendMessage(hComboBoxFolders, CB_SETCURSEL, 0, 0);
			SendMessage(hMain, WM_COMMAND, MAKEWPARAM(2, CBN_SELCHANGE), (LPARAM)hComboBoxFolders);
			SendMessage(hComboBoxChats, CB_SETCURSEL, i, 0);
			SendMessage(hMain, WM_COMMAND, MAKEWPARAM(3, CBN_SELCHANGE), (LPARAM)hComboBoxChats);
			found = true;
			break;
		}
	}
	if (found) {
		bring_me_to_life();
		if (need_async_history) {
			pending_notif_msg_id = target_msg_id;
		} else {
			int found_msg_idx = -1;
			if (target_msg_id > 0) {
				for (int m = (int)messages.size() - 1; m >= 0; m--) {
					if (messages[m].id == target_msg_id) {
						found_msg_idx = m;
						break;
					}
				}
			}
			if (found_msg_idx >= 0) {
				SendMessage(chat, EM_SETSEL, messages[found_msg_idx].start_char, messages[found_msg_idx].end_footer);
				SendMessage(chat, EM_SCROLLCARET, 0, 0);
			} else {
				SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
				SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
			}

			mark_active_chat_seen(target_msg_id);
		}
	}
}

void remove_notification() {
	if (balloon_notifications) {
		NOTIFYICONDATAV2 nid = {0};
		nid.cbSize = sizeof(nid);
		nid.hWnd = hMain;
		nid.uID = 1;
		nid.uFlags = 0x00000010;
		Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&nid);
	} else if (current_notification) {
		DestroyWindow(current_notification);
		current_notification = NULL;
	}
	memset(notification_peer_id, 0, 8);
	memset(&active_balloon_ctx, 0, sizeof(NotificationContext));
	InvalidateRect(hComboBoxChats, NULL, FALSE);
}

void click_on_notification() {
	navigate_to_notification(&active_balloon_ctx);
	remove_notification();
}

struct NotificationWindowData {
	NotificationContext ctx;
	wchar_t szTitle[64];
	wchar_t szText[256];
};

void draw_notif_text(HDC hDC, const wchar_t* str, RECT* rc, UINT format, HFONT hFont) {
	if (!str || !rc) return;
	int len = wcslen(str);
	if (len == 0) return;
	HGDIOBJ oldFont = NULL;
	if (hFont) oldFont = SelectObject(hDC, hFont);
	char ansi_buf[1024];
	char* pbuf = ansi_buf;
	int buf_size = sizeof(ansi_buf);
	if (len * 2 + 1 > buf_size) {
		buf_size = len * 2 + 10;
		pbuf = (char*)malloc(buf_size);
	}
	if (pbuf) {
		int ansi_len = WideCharToMultiByte(1251, 0, str, len, pbuf, buf_size - 1, NULL, NULL);
		if (ansi_len > 0) {
			pbuf[ansi_len] = 0;
			DrawTextA(hDC, pbuf, ansi_len, rc, format);
			if (pbuf != ansi_buf) free(pbuf);
			if (oldFont) SelectObject(hDC, oldFont);
			return;
		}
		if (pbuf != ansi_buf) free(pbuf);
	}
	DrawTextW(hDC, str, len, rc, format);
	if (oldFont) SelectObject(hDC, oldFont);
}

LRESULT CALLBACK WndProcNotification(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_CREATE: {
		CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
		NotificationWindowData* pdata = (NotificationWindowData*)pcs->lpCreateParams;
		SetWindowLong(hWnd, GWL_USERDATA, (LONG)pdata);
		SetTimer(hWnd, 1, 10000, NULL);
		return 0;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		NotificationWindowData* pdata = (NotificationWindowData*)GetWindowLong(hWnd, GWL_USERDATA);
		if (pdata) {
			RECT rcClient;
			GetClientRect(hWnd, &rcClient);
			FillRect(hdc, &rcClient, hBrushes[2]);
			DrawEdge(hdc, &rcClient, EDGE_RAISED, BF_RECT);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, colors[3]);

			LOGFONT lf = {0};
			lf.lfHeight = -MulDiv(9, dpi, 72);
			lf.lfWeight = FW_BOLD;
			lf.lfCharSet = RUSSIAN_CHARSET;
			lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
			wcscpy(lf.lfFaceName, L"Arial");
			HFONT hFontBold = CreateFontIndirect(&lf);
			if (!hFontBold) {
				wcscpy(lf.lfFaceName, L"Tahoma");
				hFontBold = CreateFontIndirect(&lf);
			}
			lf.lfWeight = FW_NORMAL;
			HFONT hFontNormal = CreateFontIndirect(&lf);
			if (!hFontNormal) {
				wcscpy(lf.lfFaceName, L"Tahoma");
				hFontNormal = CreateFontIndirect(&lf);
			}

			RECT rcName = {12, 10, rcClient.right - 12, 0};
			draw_notif_text(hdc, pdata->szTitle, &rcName, DT_WORDBREAK | DT_CALCRECT, hFontBold);
			draw_notif_text(hdc, pdata->szTitle, &rcName, DT_WORDBREAK, hFontBold);

			RECT rcMsg = {12, rcName.bottom + 6, rcClient.right - 12, rcClient.bottom - 8};
			draw_notif_text(hdc, pdata->szText, &rcMsg, DT_WORDBREAK, hFontNormal);

			DeleteObject(hFontBold);
			DeleteObject(hFontNormal);
		}
		EndPaint(hWnd, &ps);
		return 0;
	}
	case WM_ERASEBKGND:
		return 1;
	case WM_SETCURSOR:
		SetCursor(LoadCursor(NULL, IDC_HAND));
		return TRUE;
	case WM_TIMER:
		if (wParam == 1) remove_notification();
		return 0;
	case WM_LBUTTONDOWN: {
		NotificationWindowData* pdata = (NotificationWindowData*)GetWindowLong(hWnd, GWL_USERDATA);
		if (pdata) navigate_to_notification(&pdata->ctx);
		remove_notification();
		return 0;
	}
	case WM_DESTROY: {
		NotificationWindowData* pdata = (NotificationWindowData*)GetWindowLong(hWnd, GWL_USERDATA);
		if (pdata) free(pdata);
		SetWindowLong(hWnd, GWL_USERDATA, 0);
		current_notification = NULL;
		return 0;
	}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}

void new_msg_notification(Peer* peer, BYTE* msg_bytes, bool groupmed, int topic_id, int message_id, bool mentioned, bool silent) {
	if (!peer) return;
	if (memcmp(peer->id, myself.id, 8) == 0) return;
	bool is_muted = is_peer_muted(peer);
	if (!is_muted || mentioned) {
		if (!silent && !groupmed && dcInfoMain.ready) {
			memcpy(notification_peer_id, peer->id, 8);
			memcpy(active_balloon_ctx.peer_id, peer->id, 8);
			active_balloon_ctx.topic_id = topic_id;
			active_balloon_ctx.message_id = message_id;

			if (peer->name) {
				NotifDlgInit init_data;
				memset(&init_data, 0, sizeof(NotifDlgInit));
				memcpy(init_data.ctx.peer_id, peer->id, 8);
				init_data.ctx.topic_id = topic_id;
				init_data.ctx.message_id = message_id;

				NOTIFYICONDATAV2* nid = &init_data.nid;
				if (peer->is_forum && topic_id > 0) {
					const wchar_t* topic_name = NULL;
					if (peer->topics) {
						for (size_t t = 0; t < peer->topics->size(); t++) {
							if (peer->topics->at(t).id == topic_id) {
								topic_name = get_topic_title(&peer->topics->at(t));
								break;
							}
						}
					}
					if (!topic_name) {
						topic_name = (topic_id == 1) ? L"General" : L"Topic";
					}
					wchar_t clean_topic[128];
					clean_title_for_combobox(topic_name, clean_topic, 120);
					wchar_t clean_peer[128];
					clean_title_for_combobox(peer->name, clean_peer, 120);
					swprintf(nid->szInfoTitle, L"%s (#%s)", clean_peer, clean_topic);
					nid->szInfoTitle[63] = 0;
				} else {
					wchar_t clean_peer[128];
					clean_title_for_combobox(peer->name, clean_peer, 63);
					wcsncpy(nid->szInfoTitle, clean_peer, 63);
					nid->szInfoTitle[63] = 0;
				}

				if (msg_bytes) {
					int msg_len = tlstr_to_str_len(msg_bytes);
					if (msg_len == 0) {
						get_lang_string("n_att", lang_str, NULL);
						wcscpy(nid->szInfo, lang_str);
					} else if (msg_len < 256) read_string(msg_bytes, nid->szInfo);
					else {
						wchar_t* msg_temp = read_string(msg_bytes, NULL);
						wcsncpy(nid->szInfo, msg_temp, 252);
						wcscpy(nid->szInfo + 252, L"...");
						free(msg_temp);
					}
				} else {
					get_lang_string("n_ser", lang_str, NULL);
					wcscpy(nid->szInfo, lang_str);
				}
				if (balloon_notifications) {
					nid->cbSize = sizeof(NOTIFYICONDATAV2);
					nid->hWnd = hMain;
					nid->uID = 1;
					nid->uFlags = 0x00000010;
					nid->dwInfoFlags = 0x00000001 | 0x00000010;
					nid->uTimeout = 10000;
					nid->uVersion = 3;
					Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)nid);
				} else {
					NotificationWindowData* pdata = (NotificationWindowData*)malloc(sizeof(NotificationWindowData));
					if (pdata) {
						pdata->ctx = init_data.ctx;
						wcsncpy(pdata->szTitle, init_data.nid.szInfoTitle, 63);
						pdata->szTitle[63] = 0;
						wcsncpy(pdata->szText, init_data.nid.szInfo, 255);
						pdata->szText[255] = 0;

						HDC hdc = GetDC(hMain);
						LOGFONT lf = {0};
						lf.lfHeight = -MulDiv(9, dpi, 72);
						lf.lfWeight = FW_BOLD;
						lf.lfCharSet = RUSSIAN_CHARSET;
						lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
						wcscpy(lf.lfFaceName, L"Arial");
						HFONT hFontBold = CreateFontIndirect(&lf);
						if (!hFontBold) {
							wcscpy(lf.lfFaceName, L"Tahoma");
							hFontBold = CreateFontIndirect(&lf);
						}
						lf.lfWeight = FW_NORMAL;
						HFONT hFontNormal = CreateFontIndirect(&lf);
						if (!hFontNormal) {
							wcscpy(lf.lfFaceName, L"Tahoma");
							hFontNormal = CreateFontIndirect(&lf);
						}

						RECT rcName = {12, 10, 290, 0};
						draw_notif_text(hdc, pdata->szTitle, &rcName, DT_WORDBREAK | DT_CALCRECT, hFontBold);

						RECT rcMsg = {12, rcName.bottom + 6, 290, 0};
						draw_notif_text(hdc, pdata->szText, &rcMsg, DT_WORDBREAK | DT_CALCRECT, hFontNormal);

						int width = (rcMsg.right > rcName.right ? rcMsg.right : rcName.right) + 16;
						if (width < 200) width = 200;
						if (width > 340) width = 340;
						int height = rcMsg.bottom + 14;
						if (height < 60) height = 60;

						RECT work;
						SystemParametersInfo(SPI_GETWORKAREA, 0, &work, 0);
						int x = work.right - width - 12;
						int y = work.bottom - height - 12;

						ReleaseDC(hMain, hdc);
						DeleteObject(hFontBold);
						DeleteObject(hFontNormal);

						static bool notif_class_registered = false;
						if (!notif_class_registered) {
							WNDCLASS wc = {0};
							wc.lpfnWndProc = WndProcNotification;
							wc.hInstance = GetModuleHandle(NULL);
							wc.hCursor = LoadCursor(NULL, IDC_HAND);
							wc.hbrBackground = hBrushes[2];
							wc.lpszClassName = L"TelegacyNotif";
							RegisterClass(&wc);
							notif_class_registered = true;
						}

						if (current_notification) remove_notification();
						current_notification = CreateWindowEx(
							WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
							L"TelegacyNotif",
							L"",
							WS_POPUP | WS_BORDER,
							x, y, width, height,
							NULL, NULL, GetModuleHandle(NULL), (LPVOID)pdata
						);
						if (current_notification) {
							ShowWindow(current_notification, SW_SHOWNOACTIVATE);
							UpdateWindow(current_notification);
						} else {
							free(pdata);
						}
					}
				}
				if (sound_paths[0][0]) PlaySound(sound_paths[0], NULL, SND_FILENAME | SND_ASYNC);
			} else {
				if (notif_newpeer_msg && notif_newpeer_msg != (BYTE*)-1) free(notif_newpeer_msg);
				if (msg_bytes) {
					int len = tlstr_len(msg_bytes, true);
					notif_newpeer_msg = (BYTE*)malloc(len);
					memcpy(notif_newpeer_msg, msg_bytes, len);
				} else msg_bytes = (BYTE*)-1; // service msg
				return;
			}
		}
		update_total_unread_msgs_count(1);
	}
	peer->unread_msgs_count++;
}

void update_total_unread_msgs_count(int new_total_unread_msgs_count) {
	int old_total_unread_msgs_count = total_unread_msgs_count;
	total_unread_msgs_count += new_total_unread_msgs_count;
	if (total_unread_msgs_count < 0) total_unread_msgs_count = 0;
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(nid);
	nid.hWnd = hMain;
	nid.uID = 1;
	nid.uFlags = NIF_TIP;
	if (!total_unread_msgs_count) {
		wcscpy(nid.szTip, L"Telegacy");
		SetWindowText(hMain, nid.szTip);
	} else {
		swprintf(nid.szTip, L"Telegacy (%d)", total_unread_msgs_count);
		SetWindowText(hMain, nid.szTip);
		if (total_unread_msgs_count == 1) swprintf(nid.szTip, L"Telegacy | %d unread message", total_unread_msgs_count);
		else swprintf(nid.szTip, L"Telegacy | %d unread messages", total_unread_msgs_count);
	}
	if ((total_unread_msgs_count > 0 && old_total_unread_msgs_count == 0) || (total_unread_msgs_count == 0 && old_total_unread_msgs_count > 0)) {
		HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(total_unread_msgs_count > 0 ? IDI_ICON_UNREAD : IDI_ICON));
		SendMessage(hMain, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		SendMessage(hMain, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		nid.uFlags |= NIF_ICON;
		nid.hIcon = hIcon;
	}
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void get_dll_version(wchar_t* dll, DWORD* minor, DWORD* major) {
	wchar_t path[MAX_PATH];
	HMODULE hShell = GetModuleHandle(dll);
	GetModuleFileName(hShell, path, MAX_PATH);
	DWORD handle;
	DWORD size = GetFileVersionInfoSize(path, &handle);
	BYTE* data = (BYTE*)malloc(size);
	GetFileVersionInfo(path, handle, size, data);
	VS_FIXEDFILEINFO* info;
	UINT len;
	VerQueryValue(data, TEXT("\\"), (LPVOID*)&info, &len);
	*major = HIWORD(info->dwFileVersionMS);
	*minor = LOWORD(info->dwFileVersionMS);
	free(data);
}

void set_tray_icon() {
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(total_unread_msgs_count ? IDI_ICON_UNREAD : IDI_ICON));
	NOTIFYICONDATA nid = {0};
	nid.cbSize = sizeof(nid);
	nid.hWnd = hMain;
	nid.uID = 1;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON;
	nid.hIcon = hIcon;
	if (total_unread_msgs_count) {
		get_lang_string("t_unr", lang_str, NULL);
		swprintf(nid.szTip, lang_str, total_unread_msgs_count);
	} else wcscpy(nid.szTip, L"Telegacy");
	Shell_NotifyIcon(NIM_ADD, &nid);
	DWORD minor, major;
	get_dll_version(L"shell32.dll", &minor, &major);
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	bool supports_balloon = (major >= 5);
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && (osvi.dwMajorVersion < 4 || (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion < 90))) {
		supports_balloon = false;
	}
	if (supports_balloon) {
		balloon_notifications_available = true;
		NOTIFYICONDATAV2 nid = {0};
		nid.cbSize = sizeof(NOTIFYICONDATAV2);
		nid.hWnd = hMain;
		nid.uID = 1;
		nid.uVersion = 3;
		Shell_NotifyIcon(0x00000004, (NOTIFYICONDATA*)&nid);
	} else {
		balloon_notifications_available = false;
		balloon_notifications = false;
	}
	dcInfoMain.ready = true;
}

void change_mute_all(int type, bool unmuted) {
	HMENU hMenuMute = GetSubMenu(GetSubMenu(hMenuBar, 0), 1);
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING;
	char* lang_id = NULL;
	if (type == 0) lang_id = unmuted ? "m_m0" : "m_u0";
	else if (type == 1) lang_id = unmuted ? "m_m1" : "m_u1";
	else lang_id = unmuted ? "m_m2" : "m_u2";
	get_lang_string(lang_id, lang_str, NULL);
	mii.dwTypeData = lang_str;
	SetMenuItemInfo(hMenuMute, type, TRUE, &mii);
}

void update_category_unread_count(int type, bool now_muted) {
	for (int i = 0; i < peers_count; i++) {
		if (get_peer_notify_type(&peers[i]) == type && peers[i].unread_msgs_count && !is_peer_individually_muted(&peers[i])) {
			update_total_unread_msgs_count(now_muted ? (0 - peers[i].unread_msgs_count) : peers[i].unread_msgs_count);
		}
	}
}

void sync_notifications() {
	if (!dcInfoMain.sock || dcInfoMain.sock == INVALID_SOCKET) return;
	BYTE unenc_query[64];
	BYTE enc_query[88];
	const int notify_constructors[3] = { 0x193b4417, 0x4a95e84e, 0xb1db7c7e };
	for (int k = 0; k < 3; k++) {
		internal_header(unenc_query, true);
		memcpy(notify_req_msg_id[k], unenc_query + 16, 8);
		write_le(unenc_query + 28, 8, 4);
		write_le(unenc_query + 32, 0x12b3ad31, 4);
		write_le(unenc_query + 36, notify_constructors[k], 4);
		fortuna_read(unenc_query + 40, 24, &prng);
		convert_message(unenc_query, enc_query, 64, 0);
		send_query(enc_query, 88);
	}

	// account.getNotifyExceptions
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, 8, 4);
	write_le(unenc_query + 32, 0x53577479, 4);
	write_le(unenc_query + 36, 0, 4);
	fortuna_read(unenc_query + 40, 24, &prng);
	convert_message(unenc_query, enc_query, 64, 0);
	send_query(enc_query, 88);
}

BYTE* find_peer(BYTE* peer_bytes, BYTE* type_bytes, bool has_peer_type, char* type) {
	static const int peer_cons_arr[] = {TL_USER, TL_USER_EMPTY, TL_CHAT, TL_CHAT_FORBIDDEN, TL_CHAT_EMPTY, TL_CHANNEL, TL_CHANNEL_FORBIDDEN};
	static const int ucons[] = {TL_USER, TL_USER_EMPTY};
	static const int gcons[] = {TL_CHAT, TL_CHAT_FORBIDDEN, TL_CHAT_EMPTY};
	static const int ccons[] = {TL_CHANNEL, TL_CHANNEL_FORBIDDEN};
	int idx = -1;
	if (has_peer_type) {
		if (*type == -1) {
			int peer_cons = read_le(type_bytes, 4);
			if (peer_cons == TL_PEER_USER) *type = 0;
			else if (peer_cons == TL_PEER_CHAT) *type = 1;
			else *type = 2;
		}
		if (*type == 0) {
			idx = array_find(peer_bytes, (BYTE*)ucons, 4, 2);
		} else if (*type == 1) {
			idx = array_find(peer_bytes, (BYTE*)gcons, 4, 3);
		} else {
			idx = array_find(peer_bytes, (BYTE*)ccons, 4, 2);
		}
	} else {
		idx = array_find(peer_bytes, (BYTE*)&peer_cons_arr[0], 4, 7);
	}
	if (idx < 0) return NULL;
	peer_bytes += idx;
	int idx2 = array_find(peer_bytes, type_bytes + 4, 8, 1);
	if (idx2 < 0) return NULL;
	peer_bytes += idx2;
	if (!has_peer_type) {
		int cons12 = read_le(peer_bytes - 12, 4);
		int cons8 = read_le(peer_bytes - 8, 4);
		if (cons12 == TL_USER || cons12 == TL_USER_EMPTY) {
			*type = 0;
			peer_bytes -= 12;
		} else if (cons12 == TL_CHANNEL) {
			*type = 2;
			peer_bytes -= 12;
		} else if (cons8 == TL_CHAT || cons8 == TL_CHAT_FORBIDDEN || cons8 == TL_CHAT_EMPTY) {
			*type = 1;
			peer_bytes -= 8;
		} else if (cons8 == TL_CHANNEL_FORBIDDEN) {
			*type = 2;
			peer_bytes -= 8;
		} else {
			peer_bytes -= 12;
		}
	} else {
		if (*type == 1 || read_le(peer_bytes - 8, 4) == TL_CHANNEL_FORBIDDEN) peer_bytes -= 8;
		else peer_bytes -= 12;
	}
	return peer_bytes;
}

int msgfwd_addname(BYTE* peer_bytes, BYTE* msgfwd, int pos_init, bool shortmsg) {
	int written_info = 0;
	int flags = read_le(msgfwd + 4, 4);
	int offset = 8;
	wchar_t* name = NULL;
	bool name_allocated = false;
	if (flags & (1 << 5)) {
		if (flags & (1 << 0)) offset += 12;
		name = read_string(msgfwd + offset, NULL);
		name_allocated = true;
	} else if (flags & (1 << 0)) {
		if (memcmp(msgfwd + 12, myself.id, 8) == 0) name = L"you";
		else if (!shortmsg) {
			char type = -1;
			peer_bytes = find_peer(peer_bytes, msgfwd + 8, true, &type);
			peer_set_name(peer_bytes, &name, type);
			name_allocated = true;
		}
	}
	if (name) {
		written_info += riched_write(chat, name);
		int deleted_wchars = 0;
		for (int j = 0; j < wcslen(name); j++) j = emoji_adder(j, name, pos_init, 13, chat, &deleted_wchars);
		written_info -= deleted_wchars;
		SendMessage(chat, EM_SETSEL, pos_init + written_info, pos_init + written_info);
	}
	if (name_allocated) free(name);
	return written_info;
}

void place_dialog_center(HWND hDlg, bool main) {
	RECT rcDlg, rcMain;
	GetWindowRect(hDlg, &rcDlg);
	if (main) GetWindowRect(hMain, &rcMain);
	else SystemParametersInfo(SPI_GETWORKAREA, 0, &rcMain, 0);
	int dlgWidth  = rcDlg.right  - rcDlg.left;
	int dlgHeight = rcDlg.bottom - rcDlg.top;
	int mainWidth  = rcMain.right  - rcMain.left;
	int mainHeight = rcMain.bottom - rcMain.top;
	int x = rcMain.left + (mainWidth  - dlgWidth)  / 2;
	int y = rcMain.top  + (mainHeight - dlgHeight) / 2;
	SetWindowPos(hDlg, NULL, x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
}

wchar_t* get_path(wchar_t* path, wchar_t* file_name) {
	if (!path || !file_name) return path;
	wchar_t* p = wcsrchr(path, L'\\');
	if (!p) p = wcsrchr(path, L'/');
	if (p) {
		wcscpy(p + 1, file_name);
	} else {
		if (path[0] != 0) wcscat(path, L"\\");
		wcscat(path, file_name);
	}
	return path;
}

void get_dialogs() {
	telegacy_log("[get_dialogs] lowest_date=%d, limit=100", get_dialogs_lowest_date);
	// messages.getDialogs
	BYTE unenc_query[80];
	BYTE enc_query[104];
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, 32, 4);
	write_le(unenc_query + 32, 0xa0f4cb4f, 4);
	memset(unenc_query + 36, 0, 12);
	write_le(unenc_query + 40, get_dialogs_lowest_date, 4);
	write_le(unenc_query + 48, 0x7f3b18ea, 4);
	write_le(unenc_query + 52, 100, 4);
	memset(unenc_query + 56, 0, 8);
	fortuna_read(unenc_query + 64, 16, &prng);
	convert_message(unenc_query, enc_query, 80, 0);
	send_query(enc_query, 104);
}

void get_folders() {
	telegacy_log("[get_folders] requesting messages.getDialogFilters");
	get_dialogs_lowest_date = 0;
	// messages.getDialogFilters (folders)
	BYTE unenc_query[48];
	BYTE enc_query[72];
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, 4, 4);
	write_le(unenc_query + 32, 0xefd48c89, 4);
	fortuna_read(unenc_query + 36, 12, &prng);
	convert_message(unenc_query, enc_query, 48, 0);
	send_query(enc_query, 72);
}

void files_show_dropdown() {
	if (files.size() > 0) {
		RECT rc;
		SendMessage(hToolbar, TB_GETITEMRECT, 11, (LPARAM)&rc);

		MapWindowPoints(hToolbar, HWND_DESKTOP, (LPPOINT)&rc, 2);

		HMENU hMenu = CreatePopupMenu();
		for (int i = 0; i < files.size(); i++) AppendMenu(hMenu, MF_STRING, 1000 + i,  files[i]);
		get_lang_string("m_clr", lang_str, NULL);
		AppendMenu(hMenu, MF_STRING, 999, lang_str);

		int screeny;
		if (nt6) {
			RECT rcWork;
			SystemParametersInfo(SPI_GETWORKAREA, 0, &rcWork, 0);
			screeny = rcWork.bottom;
		} else screeny = GetSystemMetrics(SM_CYSCREEN);

		if (rc.bottom + GetMenuItemCount(hMenu) * GetSystemMetrics(SM_CYMENU) > screeny)
			TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN, rc.right, rc.top, 0, hMain, NULL);
		else
			TrackPopupMenu(hMenu, TPM_RIGHTALIGN | TPM_TOPALIGN, rc.right, rc.bottom, 0, hMain, NULL);

		DestroyMenu(hMenu);
	}
}

void open_files_list() {
	if (ie4) {
		RECT rc;
		SendMessage(hToolbar, TB_GETRECT, 4, (LPARAM)&rc);
		POINT pt;
		pt.x = rc.right - 3;
		pt.y = rc.top + 3;
		SendMessage(hToolbar, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
		SendMessage(hToolbar, WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
	} else files_show_dropdown();
}

void remove_peer(Peer* peer) {
	if (peer == current_peer) MessageBox(NULL, L"This chat no longer exists, removing it from the list.", L"Information", MB_OK | MB_ICONINFORMATION);
	if (memcmp(forwarding_peer_id, peer->id, 8) == 0) SendMessage(hMain, WM_COMMAND, MAKEWPARAM(19, 0), 0);
	free(peer->name);
	if (peer->handle) free(peer->handle);
	if (peer->full && peer->about) free(peer->about);
	if (peer->type != 0 && peer->full) {
		if (peer->type == 1) {
			for (int j = 0; j < peer->chat_users->size(); j++) {
				free(peer->chat_users->at(j).name);
				if (peer->chat_users->at(j).handle) free(peer->chat_users->at(j).handle);
			}
			delete peer->chat_users;
		}
		if (peer->reaction_list != &reaction_list && peer->reaction_list != NULL) {
			if (is_valid_reaction_list(peer->reaction_list)) {
				for (int k = 0; k < peer->reaction_list->size(); k++) free(peer->reaction_list->at(k));
				delete peer->reaction_list;
			}
			peer->reaction_list = &reaction_list;
		}
		if (peer->topics != NULL) {
			if (is_valid_topics_vector(peer->topics)) {
				for (size_t t = 0; t < peer->topics->size(); t++) {
					if (peer->topics->at(t).title) free(peer->topics->at(t).title);
				}
				delete peer->topics;
			}
			peer->topics = NULL;
		}
	}
	int current_peer_pos = current_peer - peers;
	for (int i = 0; i < peers_count; i++) {
		if (peer == &peers[i]) {
			for (int j = i; j < peers_count - 1; j++) peers[j] = peers[j+1];
			for (j = 0; j < folders_count; j++) {
				for (int k = 0; k < folders[j].count; k++) {
					if (folders[j].peers[k] == i) {
						for (int l = k; l < folders[j].count - 1; l++) folders[j].peers[l] = folders[j].peers[l+1];
						folders[j].count--;
						folders[j].peers = (int*)realloc(folders[j].peers, 4*folders[j].count);
					}
					if (k < folders[j].count && folders[j].peers[k] > i) folders[j].peers[k]--;
				}
			}
			if (current_peer_pos > i) current_peer_pos--;
			peers_count--;
			peers = (Peer*)realloc(peers, sizeof(Peer)*peers_count);
			break;
		} 
	}
	if (peer == current_peer || !current_peer) {
		current_peer = NULL;
		EnableMenuItem(hMenuBar, 1, MF_BYPOSITION | MF_GRAYED);
		DrawMenuBar(hMain);
		SendMessage(hMain, WM_COMMAND, MAKELONG(2, CBN_SELCHANGE), (LPARAM)hComboBoxFolders);
	} else if (current_peer && current_peer_pos >= 0) {
		current_peer = NULL;
		SendMessage(hComboBoxChats, CB_RESETCONTENT, 0, 0);
		ChatsFolder* current_folder = (ChatsFolder*)SendMessage(hComboBoxFolders, CB_GETITEMDATA, SendMessage(hComboBoxFolders, CB_GETCURSEL, 0, 0), 0);
		int no = 0;
		for (int i = 0; i < current_folder->count; i++) {
			SendMessage(hComboBoxChats, CB_ADDSTRING, 0, (LPARAM)peers[current_folder->peers[i]].name);
			SendMessage(hComboBoxChats, CB_SETITEMDATA, i, (LPARAM)&peers[current_folder->peers[i]]);
			if (current_folder->peers[i] == current_peer_pos) no = i;
		}
		SendMessage(hComboBoxChats, CB_SETCURSEL, no, 0);
		current_peer = &peers[current_peer_pos];
	} 
}

void read_react_ment(bool react) {
	BYTE unenc_query[80];
	BYTE enc_query[104];
	internal_header(unenc_query, true);
	write_le(unenc_query + 32, react ? 0x54aa7f8e : 0x36e5bf4d, 4);
	memset(unenc_query + 36, 0, 4);
	int offset = 40 + place_peer(unenc_query + 40, current_peer, true);
	write_le(unenc_query + 28, offset - 32, 4);
	int padding_len = get_padding(offset);
	fortuna_read(unenc_query + offset, padding_len, &prng);
	offset += padding_len;
	convert_message(unenc_query, enc_query, offset, 0);
	send_query(enc_query, offset + 24);
}

void logout_cleanup() {
	dcInfoMain.authorized = false;
	dcInfoMain.dc = 0;
	closesocket(dcInfoMain.sock);
	DeleteFile(get_path(appdata_path, L"DCs.dat"));
	DeleteFile(get_path(appdata_path, L"database.dat"));
	DeleteFile(get_path(appdata_path, L"session.dat"));
	DestroyWindow(hMain);
}

void set_sep_width(int width) {
	if (!ie4) width += 13;
	TBBUTTON tbb = {0};
	tbb.idCommand = 9;
	tbb.fsStyle = TBSTYLE_SEP;
	tbb.iBitmap = width;
	SendMessage(hToolbar, TB_DELETEBUTTON, 7, 0);
	SendMessage(hToolbar, TB_INSERTBUTTONA, 7, (LPARAM)&tbb);
}

HBITMAP rgb_to_bmp(BYTE* rgb, bool alpha, int width, int height) {
	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = -height;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;
	bmi.bmiHeader.biCompression = BI_RGB;
	BYTE* dst = NULL;
	HBITMAP hBitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void**)&dst, NULL, 0);
	int stride = ((width * 3 + 3) & ~3);
	int channels = alpha ? 4 : 3;
	for (int y = 0; y < height; y++) {
		BYTE* dstRow = dst + y * stride;
		BYTE* srcRow = rgb + y * width * channels;
		for (int x = 0; x < width; x++) {
			if (alpha && srcRow[x*channels + 3] < 96) {
				dstRow[x*3 + 0] = GetBValue(colors[1]);
				dstRow[x*3 + 1] = GetGValue(colors[1]);
				dstRow[x*3 + 2] = GetRValue(colors[1]);
			} else {
				dstRow[x*3 + 0] = srcRow[x*channels + 2];
				dstRow[x*3 + 1] = srcRow[x*channels + 1];
				dstRow[x*3 + 2] = srcRow[x*channels + 0];
			}
		}
	}
	HDC hdcSrc = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcSrc);
	HDC hdcSrcMem = CreateCompatibleDC(hdcSrc);
	HBITMAP hClone = CreateCompatibleBitmap(hdcSrc, width, height);
	HBITMAP hOldSrc = (HBITMAP)SelectObject(hdcMem, hClone);
	HBITMAP hOldSrcBmp = (HBITMAP)SelectObject(hdcSrcMem, hBitmap);
	BitBlt(hdcMem, 0, 0, width, height, hdcSrcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hOldSrc);
	SelectObject(hdcSrcMem, hOldSrcBmp);
	DeleteDC(hdcMem);
	DeleteDC(hdcSrcMem);
	ReleaseDC(NULL, hdcSrc);
	DeleteObject(hBitmap);
	free(rgb);
	return hClone;
}

HICON load_icon_file(const wchar_t* path, int cx, int cy) {
	if (!path || GetFileAttributes(path) == (DWORD)-1) return NULL;
	char ansi_path[MAX_PATH];
	if (WideCharToMultiByte(CP_ACP, 0, path, -1, ansi_path, MAX_PATH, NULL, NULL) <= 0) return NULL;
	return (HICON)LoadImageA(NULL, ansi_path, IMAGE_ICON, cx, cy, LR_LOADFROMFILE);
}

bool is_valid_reaction_list(std::vector<wchar_t*>* ptr) {
	if (!ptr) return false;
	if (ptr == &reaction_list) return true;
	if (IsBadReadPtr(ptr, sizeof(std::vector<wchar_t*>))) return false;
	DWORD* p = (DWORD*)ptr;
	if (p[0] > p[1] || p[1] > p[2]) return false;
	int cnt = (int)(p[1] - p[0]) / sizeof(wchar_t*);
	if (cnt < 0 || cnt > 1000) return false;
	return true;
}

bool is_valid_topics_vector(std::vector<ForumTopic>* ptr) {
	if (!ptr) return false;
	if (IsBadReadPtr(ptr, sizeof(std::vector<ForumTopic>))) return false;
	DWORD* p = (DWORD*)ptr;
	if (p[0] > p[1] || p[1] > p[2]) return false;
	int cnt = (int)(p[1] - p[0]) / sizeof(ForumTopic);
	if (cnt < 0 || cnt > 10000) return false;
	return true;
}

bool draw_topic_icon(HDC hDC, const wchar_t* emoji_path, const RECT* rcIcon, int icon_size) {
	if (!emoji_path || !rcIcon) return false;
	HICON hIcon = load_icon_file(emoji_path, icon_size, icon_size);
	if (hIcon) {
		DrawIconEx(hDC, rcIcon->left, rcIcon->top, hIcon, icon_size, icon_size, 0, NULL, DI_NORMAL);
		DestroyIcon(hIcon);
		return true;
	}
	return false;
}

bool paint_emoji_bitmap(HDC hdc, wchar_t* path, RECT* rect) {
	if (!path || !rect) return false;
	int w = (rect->right > rect->left) ? (rect->right - rect->left) : 15;
	int h = (rect->bottom > rect->top) ? (rect->bottom - rect->top) : 15;
	HICON hIcon = load_icon_file(path, w, h);
	if (hIcon) {
		DrawIconEx(hdc, rect->left, rect->top, hIcon, w, h, 0, NULL, DI_NORMAL);
		DestroyIcon(hIcon);
		return true;
	}
	return false;
}

void paint_emoji_button(DRAWITEMSTRUCT* dis) {
	if (dis->itemAction == ODA_DRAWENTIRE) {
		FillRect(dis->hDC, &dis->rcItem, hBrushes[2]);
		wchar_t* code = (wchar_t*)GetWindowLongPtr(dis->hwndItem, GWLP_USERDATA);
		bool custom_emoji = code[0] == 1;
		wchar_t path[MAX_PATH];
		if (custom_emoji) {
			swprintf(path, L"%s\\%s.ico", get_path(appdata_path, L"custom_emojis"), code + 1);
		} else swprintf(path, L"%s\\%s.ico", get_path(exe_path, L"emojis"), code);
		RECT rc = {2, 2, 0, 0};
		if (!paint_emoji_bitmap(dis->hDC, path, &rc) && !custom_emoji) {
			try_to_add_fe0f(path);
			paint_emoji_bitmap(dis->hDC, path, &rc);
		}
	}
}

void paint_password_button(DRAWITEMSTRUCT* dis, bool options) {
	if (dis->itemAction == ODA_DRAWENTIRE) {
		FillRect(dis->hDC, &dis->rcItem, hBrushes[2]);
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

		SetBkColor(hdc, RGB(128, 0, 128));
		BitBlt(hdcMask, 0, 0, 16, 16, hdc, 0, 0, SRCCOPY);

		SetTextColor(dis->hDC, colors[3]);
		SetBkColor(dis->hDC, colors[2]);
		int offset = options ? 2 : 4;
		BitBlt(dis->hDC, offset, offset, 16, 16, hdcMask, 0, 0, SRCCOPY);

		SelectBitmap(hdc, hBmpOld);
		SelectBitmap(hdcMask, hBmpMaskOld);
		DeleteDC(hdc);
		DeleteDC(hdcMask);
		DeleteObject(hBmp);
		DeleteObject(hBmpMask);
		ReleaseDC(NULL, hdcRef);
	}
}

void bring_me_to_life() {
	if (!IsWindowVisible(hMain)) {
		ShowWindow(hMain, maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
		update_own_status(true);
		if (current_peer) SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
	} else if (IsIconic(hMain)) {
		ShowWindow(hMain, SW_RESTORE);
	}
	SetForegroundWindow(hMain);
}

bool string_has_cyrillic(const wchar_t* str) {
	if (!str) return false;
	for (int i = 0; str[i]; i++) {
		wchar_t c = str[i];
		if ((c >= 0x0400 && c <= 0x052F) || (c >= 0x2DE0 && c <= 0x2DFF) || (c >= 0xA640 && c <= 0xA69F)) {
			return true;
		}
	}
	return false;
}

static bool is_bmp_emoji_or_symbol(wchar_t c) {
	if (c >= 0x200B && c <= 0x200D) return true;
	if (c >= 0x2300 && c <= 0x23FF) return true;
	if (c >= 0x2600 && c <= 0x27BF) return true;
	if (c >= 0x2B00 && c <= 0x2BFF) return true;
	if (c >= 0xFE00 && c <= 0xFE0F) return true;
	return false;
}

void clean_title_for_combobox(const wchar_t* src, wchar_t* dst, int max_chars) {
	if (!src || !dst || max_chars <= 0) return;
	int d = 0;
	bool last_space = true;
	for (int s = 0; src[s] && d < max_chars - 1; s++) {
		wchar_t c = src[s];
		if (c >= 0xD800 && c <= 0xDBFF && src[s+1] >= 0xDC00 && src[s+1] <= 0xDFFF) {
			s++;
			if (!last_space && d < max_chars - 1) {
				dst[d++] = L' ';
				last_space = true;
			}
		} else if (c >= 0xD800 && c <= 0xDFFF) {
			// stray surrogate, skip
		} else if (is_bmp_emoji_or_symbol(c)) {
			if (!last_space && d < max_chars - 1) {
				dst[d++] = L' ';
				last_space = true;
			}
		} else {
			if (c == L' ' && last_space) continue;
			dst[d++] = c;
			last_space = (c == L' ');
		}
	}
	dst[d] = 0;
	while (d > 0 && dst[d - 1] == L' ') dst[--d] = 0;
}

void update_cyrillic_font() {
	if (hFontCyrillic) {
		DeleteObject(hFontCyrillic);
		hFontCyrillic = NULL;
	}
	LOGFONT lf = {0};
	if (hFonts[1]) {
		GetObject(hFonts[1], sizeof(LOGFONT), &lf);
	} else {
		lf.lfHeight = -MulDiv(9, dpi, 72);
		lf.lfWeight = FW_NORMAL;
	}
	lf.lfCharSet = RUSSIAN_CHARSET;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy(lf.lfFaceName, L"Arial");
	hFontCyrillic = CreateFontIndirect(&lf);
	if (!hFontCyrillic) {
		wcscpy(lf.lfFaceName, L"Tahoma");
		hFontCyrillic = CreateFontIndirect(&lf);
	}
	if (!hFontCyrillic) {
		lf.lfCharSet = DEFAULT_CHARSET;
		wcscpy(lf.lfFaceName, L"Arial");
		hFontCyrillic = CreateFontIndirect(&lf);
	}
}

void draw_combobox_text(HDC hDC, const wchar_t* str, RECT* rc, UINT format) {
	if (!str || !rc) return;
	int len = wcslen(str);
	if (len == 0) return;

	if (string_has_cyrillic(str)) {
		if (!hFontCyrillic) update_cyrillic_font();
		HGDIOBJ oldFont = NULL;
		if (hFontCyrillic) oldFont = SelectObject(hDC, hFontCyrillic);
		char ansi_buf[1024];
		char* pbuf = ansi_buf;
		int buf_size = sizeof(ansi_buf);
		if (len * 2 + 1 > buf_size) {
			buf_size = len * 2 + 10;
			pbuf = (char*)malloc(buf_size);
		}
		if (pbuf) {
			int ansi_len = WideCharToMultiByte(1251, 0, str, len, pbuf, buf_size - 1, NULL, NULL);
			if (ansi_len > 0) {
				pbuf[ansi_len] = 0;
				DrawTextA(hDC, pbuf, ansi_len, rc, format);
				if (pbuf != ansi_buf) free(pbuf);
				if (oldFont) SelectObject(hDC, oldFont);
				return;
			}
			if (pbuf != ansi_buf) free(pbuf);
		}
		if (oldFont) SelectObject(hDC, oldFont);
	}
	DrawTextW(hDC, str, len, rc, format);
}

void init_default_font(int index) {
	if (index == 0) {
		LOGFONT lf = {0};
		lf.lfHeight = -MulDiv(10, dpi, 72);
		lf.lfWeight = 400;
		lf.lfCharSet = DEFAULT_CHARSET;
		wcscpy(lf.lfFaceName, L"Arial");
		hFonts[0] = CreateFontIndirect(&lf);
	} else if (index == 1) {
		LOGFONT lf = {0};
		lf.lfHeight = -MulDiv(9, dpi, 72);
		lf.lfWeight = FW_NORMAL;
		lf.lfCharSet = DEFAULT_CHARSET;
		wcscpy(lf.lfFaceName, L"Tahoma");
		hFonts[1] = CreateFontIndirect(&lf);
		if (!hFonts[1]) {
			wcscpy(lf.lfFaceName, L"MS Sans Serif");
			hFonts[1] = CreateFontIndirect(&lf);
		}
		if (!hFonts[1]) hFonts[1] = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		update_cyrillic_font();
	} else if (index == 2) {
		if (nt3) {
			LOGFONT lf = {0};
			lf.lfHeight = -MulDiv(8, dpi, 72);
			lf.lfWeight = 700;
			lf.lfCharSet = DEFAULT_CHARSET;
			wcscpy(lf.lfFaceName, L"MS Sans Serif");
			hFonts[2] = CreateFontIndirect(&lf);
		} else hFonts[2] = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	}
}

int riched_write(HWND riched, wchar_t* str) {
	StreamData sd = {0};
	sd.buf = (BYTE*)str;
	sd.length = wcslen(str) * 2;
	EDITSTREAM es = {0};
	es.dwCookie = (DWORD_PTR)&sd;
	es.pfnCallback = StreamInCallback;
	if (riched) return SendMessage(riched, EM_STREAMIN, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&es) / 2;
	else {
		LRESULT res;
		textHost->textServices->TxSendMessage(EM_STREAMIN, SF_TEXT | SF_UNICODE | SFF_SELECTION, (LPARAM)&es, &res);
		return res / 2;
	}
}

void convert_negative_lfheight(LOGFONT* lf, int index) {
	if (lf->lfHeight > 0) {
		HDC hdcRef = GetDC(NULL);
		HFONT hFontOld = (HFONT)SelectObject(hdcRef, hFonts[index]);
		TEXTMETRIC tm;
		GetTextMetrics(hdcRef, &tm);
		SelectObject(hdcRef, hFontOld);
		ReleaseDC(NULL, hdcRef);
		lf->lfHeight = tm.tmInternalLeading - tm.tmHeight;
	}
}

void set_menu(HWND hWnd) {
	hMenuBar = CreateMenu();
	HMENU hMenuProfile = CreatePopupMenu();
	HMENU hMenuMute = CreatePopupMenu();
	HMENU hMenuChat = CreatePopupMenu();
	HMENU hMenuTheme = CreatePopupMenu();
	HMENU hMenuTools = CreatePopupMenu();
	HMENU hMenuHelp = CreatePopupMenu();
	get_lang_string("m_pvw", lang_str, NULL);
	AppendMenu(hMenuProfile, MF_STRING, 30, lang_str);
	AppendMenu(hMenuChat, MF_STRING | (current_peer == &myself ? MF_GRAYED : 0), 31, lang_str);
	get_lang_string("m_mu", lang_str, NULL);
	AppendMenu(hMenuProfile, MF_POPUP, (UINT_PTR)hMenuMute, lang_str);
	get_lang_string(muted_types[0] > 0 ? "m_u0" : "m_m0", lang_str, NULL);
	AppendMenu(hMenuMute, MF_STRING, 38, lang_str);
	get_lang_string(muted_types[1] > 0 ? "m_u1" : "m_m1", lang_str, NULL);
	AppendMenu(hMenuMute, MF_STRING, 39, lang_str);
	get_lang_string(muted_types[2] > 0 ? "m_u2" : "m_m2", lang_str, NULL);
	AppendMenu(hMenuMute, MF_STRING, 40, lang_str);
	get_lang_string("m_ext", lang_str, NULL);
	AppendMenu(hMenuProfile, MF_STRING, 37, lang_str);
	get_lang_string("m_lgo", lang_str, NULL);
	AppendMenu(hMenuProfile, MF_STRING, 36, lang_str);
	get_lang_string(current_peer && is_peer_muted(current_peer) ? "m_uc" : "m_mc", lang_str, NULL);
	AppendMenu(hMenuChat, MF_STRING, 41, lang_str);
	get_lang_string("m_thn", lang_str, NULL);
	AppendMenu(hMenuTheme, MF_STRING | MF_CHECKED, 600, lang_str);
	get_lang_string("m_ths", lang_str, NULL);
	AppendMenu(hMenuChat, MF_POPUP, (UINT_PTR)hMenuTheme, lang_str);
	get_lang_string("m_rf", lang_str, NULL);
	AppendMenu(hMenuChat, MF_STRING, 43, lang_str);
	get_lang_string("m_mf", lang_str, NULL);
	AppendMenu(hMenuTools, MF_STRING, 32, lang_str);
	get_lang_string("m_sup", lang_str, NULL);
	if (!ie4) AppendMenu(hMenuTools, MF_STRING, 42,  lang_str);
	AppendMenu(hMenuTools, MF_SEPARATOR, 0, NULL);
	get_lang_string("m_syn", lang_str, NULL);
	AppendMenu(hMenuTools, MF_STRING, 44, lang_str);
	AppendMenu(hMenuTools, MF_SEPARATOR, 0, NULL);
	get_lang_string("m_opt", lang_str, NULL);
	AppendMenu(hMenuTools, MF_STRING, 33, lang_str);
	get_lang_string("m_ug", lang_str, NULL);
	AppendMenu(hMenuHelp, MF_STRING, 34, lang_str);
	get_lang_string("m_abt", lang_str, NULL);
	AppendMenu(hMenuHelp, MF_STRING, 35, lang_str);
	get_lang_string("m_p", lang_str, NULL);
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuProfile, lang_str);
	get_lang_string("m_c", lang_str, NULL);
	AppendMenu(hMenuBar, MF_POPUP | (current_peer ? 0 : MF_GRAYED), (UINT_PTR)hMenuChat, lang_str);
	get_lang_string("m_t", lang_str, NULL);
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuTools, lang_str);
	get_lang_string("m_h", lang_str, NULL);
	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hMenuHelp, lang_str);
	SetMenu(hWnd, hMenuBar);
	DrawMenuBar(hWnd);
}

void create_service_msg(BYTE* message, wchar_t* sender, wchar_t* service_msg, bool channel) {
	int msgact_cons = read_le(message, 4);
	int offset_msg = 4;
	switch (msgact_cons) {
	case TL_ACTION_CHAT_CREATE:
	case TL_ACTION_CHAT_EDIT_TITLE: {
		wchar_t* chat_name = read_string(message + offset_msg, NULL);
		if (channel) {
			get_lang_string(msgact_cons == TL_ACTION_CHAT_CREATE ? "i_ccr" : "i_cch", lang_str, NULL);
			swprintf(service_msg, lang_str, chat_name);
		} else {
			get_lang_string(msgact_cons == TL_ACTION_CHAT_CREATE ? "i_gcr" : "i_gch", lang_str, NULL);
			swprintf(service_msg, lang_str, sender, chat_name);
		}
		free(chat_name);
		break;
	}
	case TL_ACTION_CHANNEL_CREATE: {
		wchar_t* chat_name = read_string(message + offset_msg, NULL);
		get_lang_string("i_ccr", lang_str, NULL);
		swprintf(service_msg, lang_str, chat_name);
		free(chat_name);
		break;
	}
	case TL_ACTION_CHAT_EDIT_PHOTO:
		get_lang_string(channel ? "i_cphc" : "i_phc", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_CHAT_DELETE_PHOTO:
		get_lang_string(channel ? "i_cphd" : "i_phd", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_CHAT_ADD_USER: {
		if (memcmp(message + 16, message + offset_msg + 8, 8) == 0) {
			get_lang_string("i_j", lang_str, NULL);
			swprintf(service_msg, lang_str, sender);
		} else {
			wchar_t* name = NULL;
			if (current_peer->type == 1 && current_peer->chat_users) for (int i = 0; i < current_peer->chat_users->size(); i++) {
				if (memcmp(message + offset_msg + 8, current_peer->chat_users->at(i).id, 8) == 0) {
					name = current_peer->chat_users->at(i).name;
					break;
				}
			}
			bool name_allocated = false;
			if (!name) {
				if (read_le(myself.id, 8) != 0 && memcmp(message + offset_msg + 8, myself.id, 8) == 0 && myself.name && wcslen(myself.name) > 0) {
					name = myself.name;
				} else {
					for (int i = 0; i < peers_count; i++) {
						if (memcmp(message + offset_msg + 8, peers[i].id, 8) == 0 && peers[i].name) {
							name = peers[i].name;
							break;
						}
					}
				}
			}
			if (!name) {
				char type = -1;
				BYTE* peer_bytes = find_peer(message + offset_msg + 16, message + offset_msg + 4, false, &type);
				peer_set_name(peer_bytes, &name, type);
				name_allocated = true;
			}
			if (!name) name = L"Unknown";
			get_lang_string("i_add", lang_str, NULL);
			swprintf(service_msg, lang_str, name, sender);
			if (name_allocated && name != L"Unknown") free(name);
		}
		break;
	}
	case TL_ACTION_CHAT_DELETE_USER:
		get_lang_string("i_l", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_CHAT_JOINED_BY_LINK:
		get_lang_string("i_jl", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_CUSTOM_ACTION:
		read_string(message + offset_msg, service_msg);
		break;
	case TL_ACTION_CHAT_MIGRATE_TO:
		get_lang_string("i_m", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_CHANNEL_MIGRATE_FROM:
		get_lang_string("i_m2", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_PIN_MESSAGE:
		get_lang_string(channel ? "i_cp" : "i_p", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_HISTORY_CLEAR:
		get_lang_string(channel ? "i_cd" : "i_d", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_PHONE_CALL: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 12;
		if (flags & (1 << 0)) {
			offset_msg += 4;
			if (flags & (1 << 2)) get_lang_string("i_vc", lang_str, NULL);
			else get_lang_string("i_c", lang_str, NULL);
		} else {
			if (flags & (1 << 2)) get_lang_string("i_ovc", lang_str, NULL);
			else get_lang_string("i_oc", lang_str, NULL);
		}
		if (flags & (1 << 1)) {
			wchar_t duration_str[10];
			int duration = read_le(message + offset_msg, 4);
			offset_msg += 4;
			int hours = duration / 3600;
			int minutes = (duration % 3600) / 60;
			int seconds = duration % 60;
			if (hours != 0) swprintf(duration_str, L"(%02d:%02d:%02d)", hours, minutes, seconds);
			else swprintf(duration_str, L"(%02d:%02d)", minutes, seconds);
			swprintf(service_msg, L"%s %s", lang_str, duration_str);
		} else wcscpy(service_msg, lang_str);
		break;
	}
	case TL_ACTION_SCREENSHOT_TAKEN:
		get_lang_string("i_s", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	case TL_ACTION_SET_CHAT_THEME:
		if (tlstr_len(message + offset_msg, false) == 0) {
			get_lang_string(channel ? "i_ctho" : "i_tho", lang_str, NULL);
			swprintf(service_msg, lang_str, sender);
		} else {
			get_lang_string(channel ? "i_cth" : "i_th", lang_str, NULL);
			wcscat(lang_str, L" ");
			int emoji_start = wcslen(lang_str);
			read_string(message + offset_msg, lang_str + emoji_start);
			wchar_t file_name[MAX_PATH];
			swprintf(file_name, L"%s\\", get_path(exe_path, L"emojis"));
			for (int j = emoji_start; j < wcslen(lang_str); j++) {
				int cr;
				if (lang_str[j] >= 0xD800 && lang_str[j] <= 0xDBFF) {
					cr = ((lang_str[j] - 0xD800) << 10) + (lang_str[j+1] - 0xDC00) + 0x10000;
					j++;
				} else cr = lang_str[j];
				if (file_name[7] == 0) swprintf(file_name, L"%s%x", file_name, cr);
				else swprintf(file_name, L"%s-%x", file_name, cr);
			}
			wcscat(file_name, L".ico");
			FILE* f = _wfopen(file_name, L"rb");
			if (!f) {
				wcscpy(file_name + wcslen(file_name) - 4, L"-fe0f.ico");
				f = _wfopen(file_name, L"rb");
				if (f) {
					lang_str[wcslen(lang_str) + 1] = 0;
					lang_str[wcslen(lang_str)] = 0xFE0F;
					fclose(f);
				}
			} else fclose(f);
			swprintf(service_msg, lang_str, sender);
		}
		break;
	case TL_ACTION_TOPIC_CREATE: {
		int flags = read_le(message + 4, 4);
		wchar_t* topic_name = read_string(message + 8, NULL);
		if ((flags & 2) || !topic_name || wcslen(topic_name) == 0) {
			swprintf(service_msg, L"%s created the topic General", sender);
		} else {
			swprintf(service_msg, L"%s created the topic \"%s\"", sender, topic_name);
		}
		if (topic_name) free(topic_name);
		break;
	}
	case TL_ACTION_TOPIC_EDIT: {
		int flags = read_le(message + 4, 4);
		int roff = 8;
		wchar_t* topic_name = NULL;
		if (flags & (1 << 0)) {
			topic_name = read_string(message + roff, NULL);
			roff += tlstr_len(message + roff, true);
		}
		if (flags & (1 << 1)) roff += 8;
		bool closed_present = (flags & (1 << 2)) != 0;
		bool is_closed = false;
		if (closed_present) {
			is_closed = (read_le(message + roff, 4) == TL_BOOL_TRUE);
			roff += 4;
		}
		bool hidden_present = (flags & (1 << 3)) != 0;
		bool is_hidden = false;
		if (hidden_present) {
			is_hidden = (read_le(message + roff, 4) == TL_BOOL_TRUE);
			roff += 4;
		}
		if (topic_name) {
			swprintf(service_msg, L"%s renamed the topic to \"%s\"", sender, topic_name);
			free(topic_name);
		} else if (closed_present) {
			swprintf(service_msg, is_closed ? L"%s closed the topic" : L"%s reopened the topic", sender);
		} else if (hidden_present) {
			swprintf(service_msg, is_hidden ? L"%s hid the topic" : L"%s unhid the topic", sender);
		} else {
			swprintf(service_msg, L"%s edited the topic", sender);
		}
		break;
	}
	case TL_ACTION_SET_CHAT_WALLPAPER: {
		get_lang_string(channel ? "i_cw" : "i_w", lang_str, NULL);
		swprintf(service_msg, lang_str, sender);
		break;
	}
	default:
		get_lang_string("i_no", lang_str, NULL);
	} 
}

void get_myself() {
	if (read_le(myself.id, 8) != 0 && myself.name && wcslen(myself.name) > 0) return;
	BYTE unenc_query[64];
	BYTE enc_query[88];
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

void show_main() {
	telegacy_log(">>> show_main() called - showing main window <<<");
	PostMessage(hMain, WM_USER + 101, 0, 0);
	set_tray_icon();
	get_myself();
	if (CHECKUPDATES) {
		unsigned threadID;
		_beginthreadex(NULL, 0, UpdateWorker, NULL, 0, &threadID);
	}
}

LRESULT back_brush(HDC hDC) {
	SetTextColor(hDC, colors[3]);
	SetBkColor(hDC, colors[2]);
	return (LRESULT)hBrushes[2];
}

void get_lang_string(char* id, wchar_t* wdest, char* dest) {
	if (wdest) {
		GetPrivateProfileStringA(LANG, id, id, lang_str_ansi, 100, lang_path);
		MultiByteToWideChar(lang_codepage, 0, lang_str_ansi, -1, wdest, 100);
	} else GetPrivateProfileStringA(LANG, id, id, dest, 100, lang_path);
}

void update_toolbar() {
	if (ie3) {
		COLORMAP cmaps[2];
		cmaps[0].from = RGB(128, 0, 128);
		if (theme_brush) {
			LOGBRUSH lb;
			GetObject(theme_brush, sizeof(lb), &lb);
			cmaps[0].to = lb.lbColor;
		} else cmaps[0].to = colors[0];
		cmaps[1].from = RGB(0, 0, 0);
		cmaps[1].to = colors[3];
		HBITMAP tbBmpNew = CreateMappedBitmap(GetModuleHandle(NULL), IDB_ICONS, 0, cmaps, 2);
		TBREPLACEBITMAP trb = {0};
		trb.nIDOld = (UINT)tbBmp;
		trb.nIDNew = (UINT)tbBmpNew;
		trb.nButtons = 14;
		SendMessage(hToolbar, TB_REPLACEBITMAP, NULL, (LPARAM)&trb);
		DeleteObject(tbBmp);
		tbBmp = tbBmpNew;
	}
}

void nt3_combobox_fit(HWND comboBox) {
	int count = SendMessage(comboBox, CB_GETCOUNT, NULL, NULL);
	int height_main = SendMessage(comboBox, CB_GETITEMHEIGHT, -1, NULL);
	int height_item = SendMessage(comboBox, CB_GETITEMHEIGHT, 0, NULL);
	RECT rc;
	GetWindowRect(comboBox, &rc);
	int width = rc.right - rc.left;
	int height = height_main + height_item * count + 4;
	if (height > 450) height = 450;
	SetWindowPos(comboBox, NULL, NULL, NULL, width, height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

BYTE pubkey_der[] = {
  0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
  0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xe8, 0xbb, 0x33,
  0x05, 0xc0, 0xb5, 0x2c, 0x6c, 0xf2, 0xaf, 0xdf, 0x76, 0x37, 0x31, 0x34,
  0x89, 0xe6, 0x3e, 0x05, 0x26, 0x8e, 0x5b, 0xad, 0xb6, 0x01, 0xaf, 0x41,
  0x77, 0x86, 0x47, 0x2e, 0x5f, 0x93, 0xb8, 0x54, 0x38, 0x96, 0x8e, 0x20,
  0xe6, 0x72, 0x9a, 0x30, 0x1c, 0x0a, 0xfc, 0x12, 0x1b, 0xf7, 0x15, 0x1f,
  0x83, 0x44, 0x36, 0xf7, 0xfd, 0xa6, 0x80, 0x84, 0x7a, 0x66, 0xbf, 0x64,
  0xac, 0xce, 0xc7, 0x8e, 0xe2, 0x1c, 0x0b, 0x31, 0x6f, 0x0e, 0xda, 0xfe,
  0x2f, 0x41, 0x90, 0x8d, 0xa7, 0xbd, 0x1f, 0x4a, 0x51, 0x07, 0x63, 0x8e,
  0xeb, 0x67, 0x04, 0x0a, 0xce, 0x47, 0x2a, 0x14, 0xf9, 0x0d, 0x9f, 0x7c,
  0x2b, 0x7d, 0xef, 0x99, 0x68, 0x8b, 0xa3, 0x07, 0x3a, 0xdb, 0x57, 0x50,
  0xbb, 0x02, 0x96, 0x49, 0x02, 0xa3, 0x59, 0xfe, 0x74, 0x5d, 0x81, 0x70,
  0xe3, 0x68, 0x76, 0xd4, 0xfd, 0x8a, 0x5d, 0x41, 0xb2, 0xa7, 0x6c, 0xbf,
  0xf9, 0xa1, 0x32, 0x67, 0xeb, 0x95, 0x80, 0xb2, 0xd0, 0x6d, 0x10, 0x35,
  0x74, 0x48, 0xd2, 0x0d, 0x9d, 0xa2, 0x19, 0x1c, 0xb5, 0xd8, 0xc9, 0x39,
  0x82, 0x96, 0x1c, 0xdf, 0xde, 0xda, 0x62, 0x9e, 0x37, 0xf1, 0xfb, 0x09,
  0xa0, 0x72, 0x20, 0x27, 0x69, 0x60, 0x32, 0xfe, 0x61, 0xed, 0x66, 0x3d,
  0xb7, 0xa3, 0x7f, 0x6f, 0x26, 0x3d, 0x37, 0x0f, 0x69, 0xdb, 0x53, 0xa0,
  0xdc, 0x0a, 0x17, 0x48, 0xbd, 0xaa, 0xff, 0x62, 0x09, 0xd5, 0x64, 0x54,
  0x85, 0xe6, 0xe0, 0x01, 0xd1, 0x95, 0x32, 0x55, 0x75, 0x7e, 0x4b, 0x8e,
  0x42, 0x81, 0x33, 0x47, 0xb1, 0x1d, 0xa6, 0xab, 0x50, 0x0f, 0xd0, 0xac,
  0xe7, 0xe6, 0xdf, 0xa3, 0x73, 0x61, 0x99, 0xcc, 0xaf, 0x93, 0x97, 0xed,
  0x07, 0x45, 0xa4, 0x27, 0xdc, 0xfa, 0x6c, 0xd6, 0x7b, 0xcb, 0x1a, 0xcf,
  0xf3, 0x02, 0x03, 0x01, 0x00, 0x01
};
BYTE pubkey_der_test[] = {
  0x30, 0x82, 0x01, 0x22, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86,
  0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x82, 0x01, 0x0f, 0x00,
  0x30, 0x82, 0x01, 0x0a, 0x02, 0x82, 0x01, 0x01, 0x00, 0xc8, 0xc1, 0x1d,
  0x63, 0x56, 0x91, 0xfa, 0xc0, 0x91, 0xdd, 0x94, 0x89, 0xae, 0xdc, 0xed,
  0x29, 0x32, 0xaa, 0x8a, 0x0b, 0xce, 0xfe, 0xf0, 0x5f, 0xa8, 0x00, 0x89,
  0x2d, 0x9b, 0x52, 0xed, 0x03, 0x20, 0x08, 0x65, 0xc9, 0xe9, 0x72, 0x11,
  0xcb, 0x2e, 0xe6, 0xc7, 0xae, 0x96, 0xd3, 0xfb, 0x0e, 0x15, 0xae, 0xff,
  0xd6, 0x60, 0x19, 0xb4, 0x4a, 0x08, 0xa2, 0x40, 0xcf, 0xdd, 0x28, 0x68,
  0xa8, 0x5e, 0x1f, 0x54, 0xd6, 0xfa, 0x5d, 0xea, 0xa0, 0x41, 0xf6, 0x94,
  0x1d, 0xdf, 0x30, 0x26, 0x90, 0xd6, 0x1d, 0xc4, 0x76, 0x38, 0x5c, 0x2f,
  0xa6, 0x55, 0x14, 0x23, 0x53, 0xcb, 0x4e, 0x4b, 0x59, 0xf6, 0xe5, 0xb6,
  0x58, 0x4d, 0xb7, 0x6f, 0xe8, 0xb1, 0x37, 0x02, 0x63, 0x24, 0x6c, 0x01,
  0x0c, 0x93, 0xd0, 0x11, 0x01, 0x41, 0x13, 0xeb, 0xdf, 0x98, 0x7d, 0x09,
  0x3f, 0x9d, 0x37, 0xc2, 0xbe, 0x48, 0x35, 0x2d, 0x69, 0xa1, 0x68, 0x3f,
  0x8f, 0x6e, 0x6c, 0x21, 0x67, 0x98, 0x3c, 0x76, 0x1e, 0x3a, 0xb1, 0x69,
  0xfd, 0xe5, 0xda, 0xaa, 0x12, 0x12, 0x3f, 0xa1, 0xbe, 0xab, 0x62, 0x1e,
  0x4d, 0xa5, 0x93, 0x5e, 0x9c, 0x19, 0x8f, 0x82, 0xf3, 0x5e, 0xae, 0x58,
  0x3a, 0x99, 0x38, 0x6d, 0x81, 0x10, 0xea, 0x6b, 0xd1, 0xab, 0xb0, 0xf5,
  0x68, 0x75, 0x9f, 0x62, 0x69, 0x44, 0x19, 0xea, 0x5f, 0x69, 0x84, 0x7c,
  0x43, 0x46, 0x2a, 0xbe, 0xf8, 0x58, 0xb4, 0xcb, 0x5e, 0xdc, 0x84, 0xe7,
  0xb9, 0x22, 0x6c, 0xd7, 0xbd, 0x7e, 0x18, 0x3a, 0xa9, 0x74, 0xa7, 0x12,
  0xc0, 0x79, 0xdd, 0xe8, 0x5b, 0x9d, 0xc0, 0x63, 0xb8, 0xa5, 0xc0, 0x8e,
  0x8f, 0x85, 0x9c, 0x0e, 0xe5, 0xdc, 0xd8, 0x24, 0xc7, 0x80, 0x7f, 0x20,
  0x15, 0x33, 0x61, 0xa7, 0xf6, 0x3c, 0xfd, 0x2a, 0x43, 0x3a, 0x1b, 0xe7,
  0xf5, 0x02, 0x03, 0x01, 0x00, 0x01
};
unsigned int pubkey_der_len = 294;