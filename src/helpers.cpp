/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

int current_time() {
	return time(NULL) + time_diff;
}

char get_padding(int len) {
	char padding_len = 16 - len % 16;
	if (padding_len < 12) padding_len += 16;
	return padding_len;
}


void send_query(DCInfo* dcInfo, BYTE* enc_query, int length) {
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
	while (sent < length) sent += send(dcInfo->sock, (char*)(enc_query + sent), length - sent, 0);
	if (dcInfo == &dcInfoMain) LeaveCriticalSection(&csSock);
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
				fwrite(&dcInfo->current_seq_no, 4, 1, f);
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
void send_query(BYTE* enc_query, int length) {
	send_query(&dcInfoMain, enc_query, length);
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
	if (docstemp->min == 6) write_le(unenc_query, 0x1e287d04, 4);
	else write_le(unenc_query, 0x5b38c6c1, 4);
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

void update_own_status(bool status) {
	BYTE unenc_query[64];
	BYTE enc_query[88];
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, 8, 4);
	write_le(unenc_query + 32, 0x6628562c, 4);
	if (status) write_le(unenc_query + 36, 0xbc799737, 4);
	else write_le(unenc_query + 36, 0x997275b5, 4);
	fortuna_read(unenc_query + 40, 24, &prng);
	convert_message(unenc_query, enc_query, 64, 0);
	send_query(enc_query, 88);
}

void get_history() {
	BYTE unenc_query[112];
	BYTE enc_query[136];
	internal_header(unenc_query, true);
	write_le(unenc_query + 32, 0x4423e6c5, 4);
	char offset = place_peer(unenc_query + 36, current_peer, true);
	memset(unenc_query + 36 + offset, 0, 8);
	write_le(unenc_query + 44 + offset, messages.size(), 4);
	write_le(unenc_query + 48 + offset, 10, 4);
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

void set_typing(int cons, int add) {
	if (!current_peer || memcmp(myself.id, current_peer->id, 8) == 0) return;
	BYTE unenc_query[80];
	BYTE enc_query[104];
	internal_header(unenc_query, true);
	write_le(unenc_query + 32, 0x58943ee2, 4);
	memset(unenc_query + 36, 0, 4);
	char offset = place_peer(unenc_query + 40, current_peer, true) + 40;
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
	BYTE unenc_query[80];
	BYTE enc_query[104];
	char peer_len = place_peer(unenc_query + 36, current_peer, (current_peer->type == 2) ? false : true);
	char padding_len = get_padding(40+peer_len);
	char len = 40 + peer_len + padding_len;
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, len-32-padding_len, 4);
	write_le(unenc_query + 32, (current_peer->type == 2) ? 0xcc104937 : 0xe306d3a, 4);
	write_le(unenc_query + 36 + peer_len, message->id, 4);
	fortuna_read(unenc_query + 40 + peer_len, padding_len, &prng);
	convert_message(unenc_query, enc_query, len, 0);
	send_query(enc_query, len+24);
	message->seen = true;
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
			if (addmsg) {
				message_handler(true, rf->message, false, false, true);
				if (si.nPos >= (int)(si.nMax - si.nPage) - 15) SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
				ft->chrg.cpMin = messages[rf->i+1].end_char + 12;
				ft->chrg.cpMax = messages[rf->i+1].end_char + 13;
			}
			diff = set_reply(addmsg ? 0 : rf->j, messages[rf->i+addmsg].end_char + 12, SendMessage(chat, EM_FINDTEXTEX, FR_DOWN, (LPARAM)ft) == -1 ? (BYTE*)-1 : NULL, true);
			if (addmsg) delete_message(0, false);
			messages[rf->i].end_footer += diff;
			messages[rf->i].start_reactions += diff;
			messages[rf->i].reply_needed = 0;
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
		else SendMessage(chat, EM_SETSEL, -1, 0);

		if (ischatscrolling) {
			si.nPos = si.nTrackPos;
			ischatscrolling = false;
		}
		if (drawchat) {
			if (si.nPos >= (int)(si.nMax - si.nPage) - 15) SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
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

int array_find(BYTE* buf, BYTE* find, int find_len, int find_count) {
	for (int i = 0; ; i += 4) {
		for (int j = 0; j < find_count; j++) {
			if (memcmp(buf + i, find + j * find_len, find_len) == 0) return i;
		}
	}
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
	ft.chrg.cpMin = messages[i].end_char + 12;
	ft.chrg.cpMax = messages[i].end_char + 13;
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
		if (preposition) wcscpy(buf, L"at ");
	}
	else if (t.tm_year == t_now.tm_year && t.tm_yday == t_now.tm_yday-1) wcscpy(buf, L"yesterday at ");
	else {
		if (preposition) wcscpy(buf, L"on ");
		GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, buf + (preposition ? 3 : 0), 100);
		wcscat(buf, L" ");
	}
	GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, buf + wcslen(buf), 100);
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
		if (hIcon || (nt3 && GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES)) {
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
			HDC hdcRef = GetDC(NULL);
			pmfp->xExt = MulDiv(size, 2540, GetDeviceCaps(hdcRef, LOGPIXELSX));
			pmfp->yExt = MulDiv(size, 2540, GetDeviceCaps(hdcRef, LOGPIXELSY));
			ReleaseDC(NULL, hdcRef);
			pmfp->hMF = hWmf;
			GlobalUnlock(pmp);
			insert_image(richedit, pmp, NULL);
			return true;
		} else return false;
	} else {
		wchar_t placeholder[] = {0xFE0F, 0};
		HRESULT hr;
		if (richedit) SendMessage(richedit, EM_REPLACESEL, FALSE, (LPARAM)placeholder);
		else textHost->textServices->TxSendMessage(EM_REPLACESEL, FALSE, (LPARAM)placeholder, &hr);
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
			wchar_t placeholder[] = {0xFFFF, 0};
			if (chat) SendMessage(chat, EM_REPLACESEL, 0, (LPARAM)placeholder);
			else textHost->textServices->TxSendMessage(EM_REPLACESEL, 0, (LPARAM)placeholder, &res);
		}
		*deleted_wchars += compound_chars + surr_pair + specials;
		i += compound_chars + surr_pair + specials;
	}
	return i;
}

int set_reply(int i, int start_footer, BYTE* quote_text, bool setformat) {
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
	SETTEXTEX st;
	st.flags = ST_SELECTION;
	st.codepage = 1200;
	int pos_init = start_footer + written_info;
	if (quote_text != (BYTE*)-1) {
		if (quote_text) {
			wchar_t* quote = read_string(quote_text, NULL);
			if (wcslen(quote) > 78) wcscpy(quote + 75, L"...");
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)quote);
			int deleted_wchars = 0;
			for (int j = 0; j < wcslen(quote); j++) j = emoji_adder(j, quote, pos_init, 13, chat, &deleted_wchars);
			written_info -= deleted_wchars;
			free(quote);
		} else if (toobig) written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"...");
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
		cf.dwMask = CFM_COLOR | CFM_BOLD | CFM_SIZE | CFM_LINK | CFM_ITALIC | CFM_UNDERLINE | CFM_STRIKEOUT | CFM_FACE | CFM_BACKCOLOR;
		wcscpy(cf.szFaceName, L"Arial");
		cf.dwEffects = CFE_ITALIC;
		cf.crTextColor = 0;
		cf.crBackColor = RGB(255, 255, 255);
		cf.yHeight = 160;
		SendMessage(chat, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
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
					HDC hdcRef = GetDC(NULL);
					pmfp->xExt = MulDiv(13, 2540, GetDeviceCaps(hdcRef, LOGPIXELSX));
					pmfp->yExt = MulDiv(13, 2540, GetDeviceCaps(hdcRef, LOGPIXELSY));
					ReleaseDC(NULL, hdcRef);
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

void status_bar_status(Peer* peer) {
	if (peer->type == 0) {
		swprintf(status_str, L"%s ", peer->name);
		switch (peer->online) {
		case -1:
			SendMessage(hStatus, SB_SETTEXTA, 0, (LPARAM)"");
			return;
		case 0:
			wcscat(status_str, L"is online");
			break;
		case 1:
			wcscat(status_str, L"was online recently");
			break;
		case 2:
			wcscat(status_str, L"was online last week");
			break;
		case 3:
			wcscat(status_str, L"was online last month");
			break;
		default:
			wcscat(status_str, L"was online ");
			get_date(&status_str[wcslen(status_str)], peer->online, true);
			break;
		}
	} else if (peer->type == 1) {
		int count = peer->chat_users->size();
		if (count == 1) swprintf(status_str, L"%d participant", count);
		else swprintf(status_str, L"%d participants", count);
	} else {
		int count = (int)peer->chat_users;
		if (peer->perm.cansendmsg) {
			if (count == 1) swprintf(status_str, L"%d participant", count);
			else swprintf(status_str, L"%d participants", count);
		} else {
			if (count == 1) swprintf(status_str, L"%d subscriber", count);
			else swprintf(status_str, L"%d subscribers", count);
		}
	}
	SendMessage(hStatus, SB_SETTEXTA, 0 | SBT_OWNERDRAW, wcslen(peer->name));
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
	if (theme_brush) DeleteObject(theme_brush);
	if (i == 0) {
		theme_brush = NULL;
		SetClassLongPtr(hMain, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));
	} else {
		theme_brush = CreateSolidBrush(themes[i - 1].color);
		SetClassLongPtr(hMain, GCLP_HBRBACKGROUND, (LONG_PTR)theme_brush);
	}
	InvalidateRect(hMain, NULL, TRUE);
	InvalidateRect(tbSeparatorHider, NULL, TRUE);
	HMENU hMenuTheme = GetSubMenu(GetSubMenu(hMenuBar, 1), 2);
	for (int j = 0; j < themes.size() + 1; j++) CheckMenuItem(hMenuTheme, j, MF_BYPOSITION | MF_UNCHECKED);
	CheckMenuItem(hMenuTheme, i, MF_BYPOSITION | MF_CHECKED);
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
		if (nt3) hIcon = (HICON)(GetFileAttributes(file_name) != INVALID_FILE_ATTRIBUTES);
		if (!hIcon) {
			wcscpy(file_name + wcslen(file_name) - 4, L"-fe0f.ico");
			hIcon = (HICON)LoadImage(NULL, file_name, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
			if (nt3) hIcon = (HICON)(GetFileAttributes(file_name) != INVALID_FILE_ATTRIBUTES);
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
	bool service = (read_le(unenc_response + offset, 4) == 0xd3d28540) ? true : false;
	if (service) message_handler(true, unenc_response + offset, false, false, false); // for themes
	int flags_msg = read_le(unenc_response + offset + 4, 4);
	if (service) offset += 12;
	else offset += 16;
	if (msg_id) *msg_id = &unenc_response[offset-4];
	if (flags_msg & (1 << 8)) offset += 12;
	if (flags_msg & (1 << 29)) offset += 4;
	offset += 4;
	*id = &unenc_response[offset];
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
	for (int j = 0; j < current_folder->count; j++) {
		if (current_folder->peers[j] > i && j >= current_folder->pinned_count) break;
		else if (current_folder->peers[j] == i) {
			SendMessage(hComboBoxChats, CB_DELETESTRING, j, NULL);
			SendMessage(hComboBoxChats, CB_INSERTSTRING, j, (LPARAM)peers[i].name);
			SendMessage(hComboBoxChats, CB_SETITEMDATA, j, (LPARAM)&peers[i]);
			if (current_peer == &peers[i]) {
				SendMessage(hComboBoxChats, CB_SETCURSEL, j, 0);
				if (peers[i].type == 0) status_bar_status(&peers[i]);
			}
			break;
		}
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
	if (!message_footer) message_footer = &messages[read_le(reactions - 4, 4)];
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
			SETTEXTEX st;
			st.flags = ST_SELECTION;
			st.codepage = 1200;
			written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)divider);
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
					SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"?");
					unknown_custom_emoji_solver(msg_id, written_info, 13, custom_emoji_id, false);
				} else {
					try_to_add_fe0f(file_name);
					if (!insert_emoji(file_name, 13, chat)) SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)L"?");
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
	if (type == 0) {
		int flags = read_le(unenc_response + 4, 4);
		int offset = 20;
		if (flags & (1 << 0)) offset += 8;
		if (flags & (1 << 2)) offset += set_name(unenc_response + offset, name);
		else *name = read_string(unenc_response + offset, NULL);
	} else if (type == 1) *name = read_string(unenc_response + 16, NULL);
	else {
		int flags = read_le(unenc_response + 4, 4);
		if (flags & (1 << 13)) unenc_response += 8;
		*name = read_string(unenc_response + 20, NULL);
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

void get_config() {
	BYTE unenc_query[48];
	BYTE enc_query[72];
	internal_header(unenc_query, true);
	write_le(unenc_query + 28, 4, 4);
	write_le(unenc_query + 32, 0xc4f9186b, 4);
	fortuna_read(unenc_query + 36, 12, &prng);
	convert_message(unenc_query, enc_query, 48, 0);
	send_query(enc_query, 72);
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
		update_own_status(false);
	}
}

int save_dcs(BYTE* unenc_response, BYTE* this_dc) {
	int dc_count = read_le(unenc_response, 4);
	FILE* f = _wfopen(get_path(appdata_path, L"DCs.dat"), L"wb");
	fwrite(this_dc, 1, 4, f); // this dc
	fseek(f, 8, SEEK_SET);
	int offset = 4;
	int actual_dc_count = 0;
	for (int i = 0; i < dc_count; i++) {
		int flags = read_le(unenc_response + offset + 4, 4);
		if (flags) offset += 12 + tlstr_len(unenc_response + offset + 12, true);
		else {
			char ip[16] = {0};
			memcpy(ip, unenc_response + offset + 13, unenc_response[offset + 12]);
			offset += 12 + tlstr_len(unenc_response + offset + 12, true);
			fwrite(ip, 1, 16, f);
			fwrite(unenc_response + offset, 1, 4, f);
			actual_dc_count++;
		}
		offset += 4;
		if (flags & (1 << 10)) offset += tlstr_len(unenc_response + offset, true);
	}
	fseek(f, 4, SEEK_SET);
	fwrite(&actual_dc_count, 1, 4, f); // dc count
	fclose(f);
	return offset;
}

void apply_notifysettings(BYTE* unenc_response, int* mute_until, __int64 id) {
	int notify_flags = read_le(unenc_response + 4, 4);
	int offset_dlg = 8;
	if (notify_flags & (1 << 0)) offset_dlg += 4;
	if (notify_flags & (1 << 1)) offset_dlg += 4;
	if (notify_flags & (1 << 2)) *mute_until = read_le(unenc_response + offset_dlg, 4);
	else *mute_until = 0;
	if (*mute_until && *mute_until != 2147483647) {
		int time_diff = *mute_until - current_time();
		if (time_diff > 0) {
			UnmuteTimer ut;
			ut.peer_id = id;
			ut.timer_id = 50 + unmuteTimers.size();
			unmuteTimers.push_back(ut);
			SetTimer(hMain, ut.timer_id, time_diff * 1000, NULL);
		} else *mute_until = 0;
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
	InvalidateRect(hComboBoxChats, NULL, FALSE);
}

void click_on_notification() {
	bool found = false;
	if (current_peer && memcmp(current_peer->id, notification_peer_id, 8) == 0) {
		found = true;
		SendMessage(chat, WM_VSCROLL, SB_BOTTOM, 0);
		SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
		remove_notification();
	}
	if (!found) for (int i = 0; i < current_folder->count; i++) {
		if (memcmp(notification_peer_id, peers[current_folder->peers[i]].id, 8) == 0) {
			SendMessage(hComboBoxChats, CB_SETCURSEL, i, 0);
			SendMessage(hMain, WM_COMMAND, MAKEWPARAM(3, CBN_SELCHANGE), (LPARAM)hComboBoxChats);
			found = true;
			break;
		}
	}
	if (!found && current_folder != &folders[0]) for (int i = 0; i < folders[0].count; i++) {
		if (memcmp(notification_peer_id, peers[folders[0].peers[i]].id, 8) == 0) {
			SendMessage(hComboBoxFolders, CB_SETCURSEL, 0, 0);
			SendMessage(hMain, WM_COMMAND, MAKEWPARAM(2, CBN_SELCHANGE), (LPARAM)hComboBoxFolders);
			SendMessage(hComboBoxChats, CB_SETCURSEL, i, 0);
			SendMessage(hMain, WM_COMMAND, MAKEWPARAM(3, CBN_SELCHANGE), (LPARAM)hComboBoxChats);
			found = true;
			break;
		}
	}
	if (found) bring_me_to_life();
}

INT_PTR CALLBACK DlgProcNotification(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_INITDIALOG: {
		NOTIFYICONDATAV2* nid = (NOTIFYICONDATAV2*)lParam;
		HDC hdc = GetDC(hDlg);
		SetBkColor(hdc, GetSysColor(COLOR_MENU));

		LOGFONT lf;
		GetObject(hDefaultFont, sizeof(LOGFONT), &lf);
		lf.lfWeight = FW_BOLD;
		HFONT hFontBold = CreateFontIndirect(&lf);

		RECT rcName = {10, 10, 300, 0};
		HFONT oldFont = (HFONT)SelectObject(hdc, hFontBold);
		DrawText(hdc, nid->szInfoTitle, -1, &rcName, DT_WORDBREAK | DT_CALCRECT);

		RECT rcMsg = {10, rcName.bottom + 5, 300, 0};
		SelectObject(hdc, hDefaultFont);
		DrawText(hdc, nid->szInfo, -1, &rcMsg, DT_WORDBREAK | DT_CALCRECT);

		int width = (rcMsg.right > rcName.right ? rcMsg.right : rcName.right) + 15;
		int height = rcMsg.bottom + 15;
		RECT work;
		SystemParametersInfo(SPI_GETWORKAREA, 0, &work, 0);
		int x = work.right - width - 10;
		int y = work.bottom - height - 10;
		SetWindowPos(hDlg, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW | SWP_NOACTIVATE);

		SelectObject(hdc, hFontBold);
		DrawText(hdc, nid->szInfoTitle, -1, &rcName, DT_WORDBREAK);
		SelectObject(hdc, hDefaultFont);
		DrawText(hdc, nid->szInfo, -1, &rcMsg, DT_WORDBREAK);

		SelectObject(hdc, oldFont);
		ReleaseDC(hDlg, hdc);
		DeleteObject(hFontBold);
		SetTimer(hDlg, 1, 10000, NULL);
		break;
	}
	case WM_TIMER:
		if (wParam == 1) remove_notification();
		break;
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSTATIC:
		if (nt3) return (long)GetStockObject(WHITE_BRUSH);
		break;
	case WM_LBUTTONDOWN:
		click_on_notification();
		break;
	}
	return FALSE;
}

void new_msg_notification(Peer* peer, BYTE* msg_bytes, bool groupmed) {
	if (memcmp(peer->id, myself.id, 8) == 0) return;
	peer->unread_msgs_count++;
	if (!peer->mute_until && !muted_types[peer->type]) {
		update_total_unread_msgs_count(1);
		if (!groupmed && dcInfoMain.ready) {
			if (peer->type == 2 && peer->channel_msg_id[7]) return;
			memcpy(notification_peer_id, peer->id, 8);
			NOTIFYICONDATAV2 nid = {0};
			wcscpy(nid.szInfoTitle, peer->name ? peer->name : L"New message");
			if (msg_bytes) {
				int msg_len = tlstr_to_str_len(msg_bytes);
				if (msg_len == 0) wcscpy(nid.szInfo, L"[Attachment]");
				else if (msg_len < 256) read_string(msg_bytes, nid.szInfo);
				else {
					wchar_t* msg_temp = read_string(msg_bytes, NULL);
					wcsncpy(nid.szInfo, msg_temp, 252);
					wcscpy(nid.szInfo + 252, L"...");
					free(msg_temp);
				}
			} else wcscpy(nid.szInfo, L"[Service Message]");
			if (balloon_notifications) {
				nid.cbSize = sizeof(nid);
				nid.hWnd = hMain;
				nid.uID = 1;
				nid.uFlags = 0x00000010;
				nid.dwInfoFlags = 0x00000001 | 0x00000010;
				nid.uTimeout = 10000;
				nid.uVersion = 3;
				Shell_NotifyIcon(NIM_MODIFY, (NOTIFYICONDATA*)&nid);
			} else {
				BYTE buffer[24] = {0};
				DLGTEMPLATE *dlg = (DLGTEMPLATE*)buffer;
				dlg->style = WS_POPUP | DS_MODALFRAME;
				if (current_notification) remove_notification();
				current_notification = CreateDialogIndirectParam(GetModuleHandle(NULL), dlg, hMain, DlgProcNotification, (LONG)&nid);
			}
			if (sound_paths[0][0]) PlaySound(sound_paths[0], NULL, SND_FILENAME | SND_ASYNC);
		}
	}
}

void update_total_unread_msgs_count(int new_total_unread_msgs_count) {
	int old_total_unread_msgs_count = total_unread_msgs_count;
	total_unread_msgs_count += new_total_unread_msgs_count;
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
		swprintf(nid.szTip, L"Telegacy (%d)", total_unread_msgs_count);
		if (total_unread_msgs_count == 1) swprintf(nid.szTip, L"Telegacy | %d unread message", total_unread_msgs_count);
		else swprintf(nid.szTip, L"Telegacy | %d unread messages", total_unread_msgs_count);
	} else wcscpy(nid.szTip, L"Telegacy");
	Shell_NotifyIcon(NIM_ADD, &nid);
	DWORD minor, major;
	get_dll_version(L"shell32.dll", &minor, &major);
	if (major >= 5) {
		balloon_notifications_available = true;
		NOTIFYICONDATAV2 nid = {0};
		nid.cbSize = sizeof(NOTIFYICONDATAV2);
		nid.hWnd = hMain;
		nid.uID = 1;
		nid.uVersion = 3;
		Shell_NotifyIcon(0x00000004, (NOTIFYICONDATA*)&nid);
	} else balloon_notifications = false;
	dcInfoMain.ready = true;
}

void change_mute_all(int type, bool unmuted) {
	HMENU hMenuMute = GetSubMenu(GetSubMenu(hMenuBar, 0), 1);
	MENUITEMINFO mii = {0};
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_STRING;
	if (type == 0) mii.dwTypeData = unmuted ? L"Mute all users" : L"Unmute all users";
	else if (type == 1) mii.dwTypeData = unmuted ? L"Mute all groups" : L"Unmute all groups";
	else mii.dwTypeData = unmuted ? L"Mute all channels" : L"Unmute all channels";
	SetMenuItemInfo(hMenuMute, type, TRUE, &mii);
}

BYTE* find_peer(BYTE* peer_bytes, BYTE* type_bytes, bool has_peer_type, char* type) {
	int peer_cons_arr[] = {0x4b46c37e, 0x41cbf256, 0xe00998b7};
	if (has_peer_type) {
		if (*type == -1) {
			int peer_cons = read_le(type_bytes, 4);
			if (peer_cons == 0x59511722) *type = 0;
			else if (peer_cons == 0x36c6019a) *type = 1;
			else *type = 2;
		}
		peer_bytes += array_find(peer_bytes, (BYTE*)&peer_cons_arr[*type], 4, 1);
	} else peer_bytes += array_find(peer_bytes, (BYTE*)&peer_cons_arr[0], 4, 3);
	peer_bytes += array_find(peer_bytes, type_bytes + 4, 8, 1);
	if (!has_peer_type) {
		int cons = read_le(peer_bytes - 12, 4);
		*type = 1;
		if (cons == 0x4b46c37e) *type = 0;
		else if (cons == 0xe00998b7) *type = 2;
		else peer_bytes += 4;
		peer_bytes -= 12;
	} else {
		if (*type == 1) peer_bytes -= 8;
		else peer_bytes -= 12;
	}
	return peer_bytes;
}

int msgfwd_addname(BYTE* peer_bytes, BYTE* msgfwd, int pos_init, bool shortmsg) {
	SETTEXTEX st;
	st.flags = ST_SELECTION;
	st.codepage = 1200;
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
		written_info += SendMessage(chat, EM_SETTEXTEX, (WPARAM)&st, (LPARAM)name);
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
	wcscpy(wcsrchr(path, L'\\') + 1, file_name);
	return path;
}

void get_folders() {
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
		AppendMenu(hMenu, MF_STRING, 999,  L"Clear this list");

		if (rc.bottom + GetMenuItemCount(hMenu) * GetSystemMetrics(SM_CYMENU) > GetSystemMetrics(SM_CYSCREEN))
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
			for (int k = 0; k < peer->reaction_list->size(); k++) free(peer->reaction_list->at(k));
			delete peer->reaction_list;
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
				dstRow[x*3 + 0] = 255;
				dstRow[x*3 + 1] = 255;
				dstRow[x*3 + 2] = 255;
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

bool paint_emoji_bitmap(HDC hdc, wchar_t* path, RECT* rect) {
	FILE* f = _wfopen(path, L"rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		int size_buf = ftell(f);
		rewind(f);
		BYTE buf[1386];
		fread(buf, 1, size_buf, f);
		fclose(f);
		ICONDIRENTRY* entry = (ICONDIRENTRY*)(buf + sizeof(ICONDIR));
		BYTE* dib = &buf[entry->dwImageOffset];
		BITMAPINFOHEADER* bih = (BITMAPINFOHEADER*)(dib);
		int width = bih->biWidth;
		int height = bih->biHeight / 2;
		int palettesize = 0;
		if (bih->biBitCount <= 8) palettesize = (bih->biClrUsed ? bih->biClrUsed : (1 << bih->biBitCount)) * sizeof(RGBQUAD);
		BYTE* xorBits = dib + bih->biSize + palettesize;
		int xorstride = ((width * bih->biBitCount + 31) / 32) * 4;
		int xorsize = xorstride * height;
		BYTE* andBits = xorBits + xorsize;
		bih->biHeight = 15;
		HDC hdcRef = GetDC(NULL);
		HBITMAP hXor = CreateCompatibleBitmap(hdcRef, width, height);
		SetDIBits(NULL, hXor, 0, height, xorBits, (BITMAPINFO*)bih, DIB_RGB_COLORS);

		BITMAPINFO bmi = {0};
		bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmi.bmiHeader.biWidth = width;
		bmi.bmiHeader.biHeight = height;
		bmi.bmiHeader.biPlanes = 1;
		bmi.bmiHeader.biBitCount = 1;
		bmi.bmiHeader.biCompression = BI_RGB;
		HBITMAP hAnd = CreateBitmap(width, height, 1, 1, NULL);
		SetDIBits(NULL, hAnd, 0, height, andBits, &bmi, DIB_RGB_COLORS);

		HDC hdcMem = CreateCompatibleDC(hdcRef);
		HBITMAP hBmpOld = (HBITMAP)SelectObject(hdcMem, hAnd);
		SetTextColor(hdc, 0x00000000L);
		SetBkColor(hdc, 0x00FFFFFFL);
		BitBlt(hdc, rect->left, rect->top, 15, 15, hdcMem, 0, 0, SRCAND);
		SelectObject(hdcMem, hXor);
		BitBlt(hdc, rect->left, rect->top, 15, 15, hdcMem, 0, 0, SRCINVERT);
		SelectObject(hdcMem, hBmpOld);
		DeleteDC(hdcMem);
		ReleaseDC(NULL, hdcRef);
		DeleteObject(hXor);
		DeleteObject(hAnd);
		return true;
	} else return false;
}

void paint_emoji_button(DRAWITEMSTRUCT* dis) {
	if (dis->itemAction == ODA_DRAWENTIRE) {
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

void bring_me_to_life() {
	if (!IsWindowVisible(hMain)) {
		ShowWindow(hMain, maximized ? SW_SHOWMAXIMIZED : SW_SHOW);
		update_own_status(true);
		if (current_peer) SendMessage(chat, WM_VSCROLL, MAKELONG(SB_ENDSCROLL, 0), 0);
	} else if (IsIconic(hMain)) ShowWindow(hMain, SW_RESTORE);
	else SetForegroundWindow(hMain);
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