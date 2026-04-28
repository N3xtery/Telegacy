/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

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
			write_string(unenc_query + offset, L"1.0.3");
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
				write_string(unenc_query + 60, L"1.0.3");
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
		if (memcmp(last_rpcresult_msgid, difference_msg_id, 8) == 0 && !closed_logged_out) {
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
			if (!closed_logged_out) {
				closed_logged_out = true;
				MessageBox(hMain, L"Not logged in! Cleaning up and exiting...", L"Error", MB_OK | MB_ICONERROR);
				logout_cleanup();
			}
			return;
		} else if (error_code == 420) {
			memcpy(flood_msg_id, unenc_response - 8, 8);
			wchar_t* wait_time_str = wcsrchr(error_message, L'_') + 1;
			int wait_time = wcstol(wait_time_str, NULL, 10);
			SetTimer(hMain, 4, wait_time * 1000, NULL);
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
		if (closed_logged_out) return;
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
				get_dialogs();

				// users.getUsers (get current user)
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
		if (channel) memset(msg_count ? peers[0].channel_msg_id : channel->channel_msg_id, 0, 8);
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
		get_dialogs_lowest_date = 2147483647;
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
				if (date < get_dialogs_lowest_date) get_dialogs_lowest_date = date;
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

		if (peers_count < total_peers_count) get_dialogs();
		else get_folders();
		break;			 
	}
	case 0x2ad93719: { // messages.dialogFilters (folders)
		if (peers_count == 0) break;
		folders_count = read_le(unenc_response + 12, 4);
		if (folders_count > 1) {
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
				if (peers[i].name == NULL && memcmp(unenc_response + offset2, peers[i].id, 8) == 0) {
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
		qrcodegen_encodeText(token, temp, qrcode, qrcodegen_Ecc_LOW, 5, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
		int size = qrcodegen_getSize(qrcode);
		int scale = 185 / size;
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
			//if (current_peer && memcmp(update + 8, current_peer->id, 8) == 0) changing_status_bar = true;
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
			else if (peer->name) {
				if (peer->full) get_full_peer(peer);
				free(peer->name);
				if (peer->handle) free(peer->handle);
				if (peer->full && peer->about) free(peer->about);
				set_peer_info(peer_bytes, peer, false);
			}
		} else if (update_constructor == 0x635b4c09) update_chats_order(update + 4, update + 4, 2);
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