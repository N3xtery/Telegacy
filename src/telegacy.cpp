/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

prng_state prng;
hash_state md;
CRITICAL_SECTION csSock, csCM;

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

wchar_t* last_tofront_sender;
BYTE group_id_tofront[8] = {0};
BYTE group_id[8] = {0};

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
std::vector<uploadThreadEvent*> uploadEvents;
BYTE last_rpcresult_msgid[8];
BYTE* sendMultiMedia = NULL;
BYTE pfp_msgid[8];
BYTE handle_msgid[8];
BYTE flood_msg_id[8];
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
CTextHost* textHost = NULL;

HWAVEIN hWaveIn = NULL;
WAVEHDR hdr_buf[4];
std::vector<BYTE> recorded;

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

				for (int i = 0; i < 4; i++) {
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
				for (int i = 0; i < 4; i++) {
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
		} else if (wParam == 4) {
			if (memcmp(flood_msg_id, rces[0].msg_id, 8) == 0) get_photo(&rces[0], NULL, &dcInfoMain); 
			else for (int i = 0; i < documents.size(); i++) {
				if (memcmp(flood_msg_id, documents[i].photo_msg_id, 8) == 0) {
					get_photo(NULL, &documents[i], &dcInfoMain); 
					break;
				}
			}
			KillTimer(hWnd, 4);
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
