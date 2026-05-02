/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

WNDPROC oldMsgInputProc, oldChatProc, oldEmojiScrollProc, oldEmojiStaticProc, oldReactionStaticProc, oldEmojiScrollFallbackProc, oldSplitterProc, oldReactionButtonProc;
WNDPROC oldNameProc, oldHandleProc, oldAboutProc, oldBirthdayProc, oldOptionsTabsProc;
HWND name, handle, about, birthday, hOptionsTabs, downloadEdit = NULL;
HWND sound_edits[3] = {0};
HWND font_edits[3] = {0};
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

void write_font_name(int index) {
	wchar_t buf[100];
	LOGFONT lf = {0};
	GetObject(hFonts[index], sizeof(lf), &lf);
	convert_negative_lfheight(&lf, index);
	int pt = MulDiv(-lf.lfHeight, 72, dpi);
	swprintf(buf, L"%s %dpt", lf.lfFaceName, pt);
	if (lf.lfWeight >= FW_BOLD) wcscat(buf, L", Bold");
	if (lf.lfItalic) wcscat(buf, L", Italic");
	SendMessage(font_edits[index], EM_SETSEL, 0, -1);
	SendMessage(font_edits[index], EM_REPLACESEL, FALSE, (LPARAM)buf);
}

void apply_fonts(HWND parent) {
	HWND child = GetWindow(parent, GW_CHILD);
	while (child != NULL) {
		HWND next = GetWindow(child, GW_HWNDNEXT);
		SendMessage(child, WM_SETFONT, (WPARAM)hFonts[2], TRUE);
		child = next;
	}
}

void update_fonts(int index) {
	if (index == 0) SendMessage(msgInput, WM_SETFONT, (WPARAM)hFonts[0], TRUE);
	else if (index == 1) {
		SendMessage(hComboBoxChats, WM_SETFONT, (WPARAM)hFonts[1], TRUE);
		SendMessage(hComboBoxFolders, WM_SETFONT, (WPARAM)hFonts[1], TRUE);
	} else {
		apply_fonts(hOptionsTabs);
		apply_fonts(current_dialog);
		InvalidateRect(hStatus, NULL, TRUE);
	}
}

bool pass_pushed = false;
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
		case 31:
			if (pass_pushed) SendMessage(hProxyPassword, EM_SETPASSWORDCHAR, (WPARAM)'*', NULL);
			else SendMessage(hProxyPassword, EM_SETPASSWORDCHAR, NULL, NULL);
			InvalidateRect(hProxyPassword, NULL, TRUE);
			pass_pushed = !pass_pushed;
			break;
		case 35:
		case 36:
		case 37: {
			int index = LOWORD(wParam) - 35;
			LOGFONT lf = {0};
			GetObject(hFonts[index], sizeof(lf), &lf);
			CHOOSEFONT cf = {0};
			cf.lStructSize = sizeof(cf);
			cf.hwndOwner = current_dialog;
			cf.lpLogFont = &lf;
			cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST;
			if (ChooseFont(&cf)) {
				wchar_t buf[100];
				wchar_t* sections[] = {L"FontChat", L"FontSystem", L"FontUI"};
				WritePrivateProfileString(sections[index], L"face", lf.lfFaceName, get_path(appdata_path, L"options.ini"));
				swprintf(buf, L"%d", lf.lfHeight);
				WritePrivateProfileString(sections[index], L"height", buf, appdata_path);
				swprintf(buf, L"%d", lf.lfWeight);
				WritePrivateProfileString(sections[index], L"weight", buf, appdata_path);
				swprintf(buf, L"%d", lf.lfItalic);
				WritePrivateProfileString(sections[index], L"italic", buf, appdata_path);
				DeleteObject(hFonts[index]);
				hFonts[index] = CreateFontIndirect(&lf);
				write_font_name(index);
				update_fonts(index);
			}
			break;
		}
		case 38:
		case 39:
		case 40: {
			int index = LOWORD(wParam) - 38;
			DeleteObject(hFonts[index]);
			init_default_font(index);
			write_font_name(index);
			wchar_t* sections[] = {L"FontChat", L"FontSystem", L"FontUI"};
			WritePrivateProfileString(sections[index], L"face", L"", get_path(appdata_path, L"options.ini"));
			update_fonts(index);
			break;
		}
		}
		break;
	case WM_ERASEBKGND:
		if (nt3) {
			RECT rc;
			GetClientRect(hWnd, &rc);
			FillRect((HDC)wParam, &rc, (HBRUSH)GetStockObject(WHITE_BRUSH));
			return TRUE;
		}
		break;
	case WM_DRAWITEM:
		paint_password_button((DRAWITEMSTRUCT*)lParam, true);
		break;
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
			if (attrs != -1 && !(attrs & FILE_ATTRIBUTE_DIRECTORY) && _wcsicmp(wcsrchr(path, L'.'), L".wav") == 0) {
				wcscpy(sound_paths[i], path);
			}
		}
	}
	if (hProxyIP) {
		wchar_t buf[256];
		if (ie4) {
			DWORD ip_bin;
			SendMessage(hProxyIP, IPM_GETADDRESS, 0, (LPARAM)&ip_bin);
			if (ip_bin) swprintf(buf, L"%d.%d.%d.%d", FIRST_IPADDRESS(ip_bin), SECOND_IPADDRESS(ip_bin), THIRD_IPADDRESS(ip_bin), FOURTH_IPADDRESS(ip_bin));
			else buf[0] = 0;
		} else GetWindowText(hProxyIP, buf, 16);
		WritePrivateProfileString(L"Proxy", L"ip", buf, get_path(appdata_path, L"options.ini"));
		GetWindowText(hProxyPort, buf, 6);
		WritePrivateProfileString(L"Proxy", L"port", buf, appdata_path);
		GetWindowText(hProxyUsername, buf, 256);
		WritePrivateProfileString(L"Proxy", L"username", buf, appdata_path);
		GetWindowText(hProxyPassword, buf, 256);
		WritePrivateProfileString(L"Proxy", L"password", buf, appdata_path);
		hProxyIP = NULL;
		if (!nt3) {
			HICON hIconPass = (HICON)SendMessage(hProxyHidePassword, BM_GETIMAGE, IMAGE_ICON, 0);
			if (hIconPass) DestroyIcon(hIconPass);
		}
	}
}

bool options_modal = false;
INT_PTR CALLBACK DlgProcOptions(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_INITDIALOG: {
		hOptionsTabs = CreateWindowEx(WS_EX_CONTROLPARENT, WC_TABCONTROL, NULL, WS_CHILD | WS_VISIBLE | (nt3 ? TCS_OWNERDRAWFIXED : 0), 5, 5, 390, 190, hDlg, (HMENU)5, NULL, NULL);
		oldOptionsTabsProc = (WNDPROC)SetWindowLongPtr(hOptionsTabs, GWLP_WNDPROC, (LONG_PTR)WndProcOptionsTabs);

		TCITEMA tie = {0};
		tie.mask = TCIF_TEXT;

		tie.pszText = "General";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 0, (LPARAM)&tie);
		tie.pszText = "Content";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 1, (LPARAM)&tie);
		tie.pszText = "Fonts";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 2, (LPARAM)&tie);
		tie.pszText = "Sounds";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 3, (LPARAM)&tie);
		tie.pszText = "Voice";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 4, (LPARAM)&tie);
		tie.pszText = "Proxy";
		SendMessage(hOptionsTabs, TCM_INSERTITEMA, 5, (LPARAM)&tie);

		if (lParam == 1) {
			options_modal = true;
			TabCtrl_SetCurSel(hOptionsTabs, 5);
		} else {
			options_modal = false;
			TabCtrl_SetCurSel(hOptionsTabs, 0);
		}
		NMHDR hdr;
		hdr.hwndFrom = hOptionsTabs;
		hdr.code = TCN_SELCHANGE;
		SendMessage(hDlg, WM_NOTIFY, NULL, (LPARAM)&hdr);

		HWND buttonOK = CreateWindowEx(0, L"BUTTON", L"OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 260, 200, 65, 21, hDlg, (HMENU)IDOK, 0, NULL);
		HWND buttonCancel = CreateWindowEx(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 330, 200, 65, 21, hDlg, (HMENU)IDCANCEL, 0, NULL);
		
		apply_fonts(hDlg);
		SetWindowText(hDlg, L"Options");
		place_dialog_center(hDlg, true);
		SetFocus(hOptionsTabs);
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL || LOWORD(wParam) == IDOK) options_modal ? EndDialog(hDlg, 0) : DestroyWindow(hDlg);
		break;
	case WM_HELP: {
		HELPINFO* hi = (HELPINFO*)lParam;
		if (hi->iContextType == HELPINFO_WINDOW) {
			int id = GetDlgCtrlID((HWND)hi->hItemHandle);
			if (id >= 6 && id <= 40) {
				if (GetFileAttributes(get_path(exe_path, L"help.hlp")) == -1 || LOBYTE(LOWORD(GetVersion())) >= 6 || !WinHelp(hDlg, exe_path, HELP_CONTEXTPOPUP, id)) {
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
				apply_fonts(hOptionsTabs);

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
				apply_fonts(hOptionsTabs);
				SendMessage(imagehwnds[IMAGELOADPOLICY], BM_SETCHECK, BST_CHECKED, 0);
				if (EMOJIS) SendMessage(emojiCheckbox, BM_SETCHECK, BST_CHECKED, 0);
				if (SPOILERS) SendMessage(spoilerCheckbox, BM_SETCHECK, BST_CHECKED, 0);
				break;
			}
			case 2: {
				HWND chatFontInfo = CreateWindow(L"STATIC", L"Chat font:", WS_CHILD | WS_VISIBLE, 10, 30, 370, 15, hOptionsTabs, (HMENU)32, NULL, NULL);
				font_edits[0] = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? WS_BORDER : 0) | WS_TABSTOP | ES_AUTOHSCROLL, 10, 45, 320, 20, hOptionsTabs, (HMENU)32, NULL, NULL);
				HWND browseChatFont = CreateWindowEx(0, L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 335, 45, 20, 20, hOptionsTabs, (HMENU)35, 0, NULL);
				HWND resetChatFont = CreateWindowEx(0, L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 45, 20, 20, hOptionsTabs, (HMENU)38, 0, NULL);

				HWND systemFontInfo = CreateWindow(L"STATIC", L"Chat list font:", WS_CHILD | WS_VISIBLE, 10, 70, 370, 15, hOptionsTabs, (HMENU)33, NULL, NULL);
				font_edits[1] = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? WS_BORDER : 0) | WS_TABSTOP | ES_AUTOHSCROLL, 10, 85, 320, 20, hOptionsTabs, (HMENU)33, NULL, NULL);
				HWND browseSystemFont = CreateWindowEx(0, L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 335, 85, 20, 20, hOptionsTabs, (HMENU)36, 0, NULL);
				HWND resetSystemFont = CreateWindowEx(0, L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 85, 20, 20, hOptionsTabs, (HMENU)39, 0, NULL);

				HWND uiFontInfo = CreateWindow(L"STATIC", L"UI font:", WS_CHILD | WS_VISIBLE, 10, 110, 370, 15, hOptionsTabs, (HMENU)34, NULL, NULL);
				font_edits[2] = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | (nt3 ? WS_BORDER : 0) | WS_TABSTOP | ES_AUTOHSCROLL, 10, 125, 320, 20, hOptionsTabs, (HMENU)34, NULL, NULL);
				HWND browseUIFont = CreateWindowEx(0, L"BUTTON", L"...", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 335, 125, 20, 20, hOptionsTabs, (HMENU)37, 0, NULL);
				HWND resetUIFont = CreateWindowEx(0, L"BUTTON", L"X", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 360, 125, 20, 20, hOptionsTabs, (HMENU)40, 0, NULL);

				apply_fonts(hOptionsTabs);
				write_font_name(0);
				write_font_name(1);
				write_font_name(2);
				break;
			}
			case 3: {
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

				apply_fonts(hOptionsTabs);

				for (int i = 0; i < 3; i++) {
					if (sound_paths[i][0] == 0) SendMessage(sound_edits[i], EM_REPLACESEL, FALSE, (LPARAM)L"None");
					else SendMessage(sound_edits[i], EM_REPLACESEL, FALSE, (LPARAM)sound_paths[i]);
					SendMessage(sound_edits[i], EM_SETSEL, 0, 0);
				}
				break;
			}
			case 4: {
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

				apply_fonts(hOptionsTabs);
				SetFocus(hOptionsTabs);
				break;
			}
			case 5:
				HWND ip_label = CreateWindow(L"STATIC", L"IP address:", WS_CHILD | WS_VISIBLE, 10, 30, 150, 15, hOptionsTabs, (HMENU)27, NULL, NULL);
				if (ie4) hProxyIP = CreateWindow(WC_IPADDRESS, L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 45, 150, 20, hOptionsTabs, (HMENU)27, NULL, NULL);
				else {
					hProxyIP = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | (nt3 ? WS_BORDER : 0) | ES_CENTER, 10, 45, 150, 20, hOptionsTabs, (HMENU)27, NULL, NULL);
					SendMessage(hProxyIP, EM_LIMITTEXT, 15, 0);
				}
				HWND port_label = CreateWindow(L"STATIC", L"Port:", WS_CHILD | WS_VISIBLE, 170, 30, 50, 15, hOptionsTabs, (HMENU)28, NULL, NULL);
				hProxyPort = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | (nt3 ? WS_BORDER : 0) | ES_CENTER | ES_NUMBER, 170, 45, 50, 20, hOptionsTabs, (HMENU)28, NULL, NULL);
				SendMessage(hProxyPort, EM_LIMITTEXT, 5, 0);
				HWND username_label = CreateWindow(L"STATIC", L"Username:", WS_CHILD | WS_VISIBLE, 10, 70, 168, 15, hOptionsTabs, (HMENU)29, NULL, NULL);
				hProxyUsername = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | (nt3 ? WS_BORDER : 0), 10, 85, 168, 20, hOptionsTabs, (HMENU)29, NULL, NULL);
				SendMessage(hProxyUsername, EM_LIMITTEXT, 255, 0);
				HWND password_label = CreateWindow(L"STATIC", L"Password:", WS_CHILD | WS_VISIBLE, 188, 70, 167, 15, hOptionsTabs, (HMENU)30, NULL, NULL);
				hProxyPassword = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL | (nt3 ? WS_BORDER : 0) | ES_PASSWORD, 188, 85, 167, 20, hOptionsTabs, (HMENU)30, NULL, NULL);
				SendMessage(hProxyPassword, EM_LIMITTEXT, 255, 0);
				pass_pushed = false;
				hProxyHidePassword = CreateWindow(L"BUTTON", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | (nt3 ? BS_OWNERDRAW : BS_ICON), 360, 85, 20, 20, hOptionsTabs, (HMENU)31, 0, NULL);
				if (!nt3) {
					HIMAGELIST hImg = ImageList_LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ICONS), 16, 0, RGB(128, 0, 128));
					HICON hIcon = ImageList_GetIcon(hImg, 6, ILD_NORMAL);
					ImageList_Destroy(hImg);
					SendMessage(hProxyHidePassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				}
				HWND note_label = CreateWindow(L"STATIC", L"Note: Only SOCKS5 proxies are supported.", WS_CHILD | WS_VISIBLE, 10, 165, 300, 15, hOptionsTabs, NULL, NULL, NULL);
				apply_fonts(hOptionsTabs);

				wchar_t buf[256];
				GetPrivateProfileString(L"Proxy", L"ip", L"", buf, 16, get_path(appdata_path, L"options.ini"));
				if (ie4) {
					char ip[16];
					WideCharToMultiByte(CP_ACP, 0, buf, -1, ip, wcslen(buf) + 1, NULL, NULL);
					DWORD ip_bin = inet_addr(ip);
					if (ip[0]) SendMessage(hProxyIP, IPM_SETADDRESS, 0, MAKEIPADDRESS(FOURTH_IPADDRESS(ip_bin), THIRD_IPADDRESS(ip_bin), SECOND_IPADDRESS(ip_bin), FIRST_IPADDRESS(ip_bin)));
				} else SetWindowText(hProxyIP, buf);
				GetPrivateProfileString(L"Proxy", L"port", L"", buf, 6, appdata_path);
				SetWindowText(hProxyPort, buf);
				GetPrivateProfileString(L"Proxy", L"username", L"", buf, 256, appdata_path);
				SetWindowText(hProxyUsername, buf);
				GetPrivateProfileString(L"Proxy", L"password", L"", buf, 256, appdata_path);
				SetWindowText(hProxyPassword, buf);
				break;
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
		infoLabel = CreateWindow(L"STATIC", about ? L"v1.0.3  |  Copyright © 2026 N3xtery  |  GNU GPL v3 License" : L"Connecting to the server...",
			WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 117, 384, 75, hDlg, NULL, NULL, NULL);
		apply_fonts(hDlg);

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
		SetForegroundWindow(hDlg);
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
		PARAFORMAT2 pf;
		pf.cbSize = sizeof(pf);
		pf.dwMask = PFM_ALIGNMENT;
		pf.wAlignment = PFA_CENTER;
		SendMessage(name, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
		SendMessage(handle, EM_SETPARAFORMAT, 0, (LPARAM)&pf);
		HWND hLabelHandle = CreateWindow(L"STATIC", L"Handle", WS_CHILD | WS_VISIBLE | SS_CENTER, 180, 40, 120, 15, hDlg, NULL, NULL, NULL);
		HWND hLabelAbout = CreateWindow(L"STATIC", L"About", WS_CHILD | WS_VISIBLE | SS_CENTER, 310, 40, 170, 15, hDlg, NULL, NULL, NULL);
		HWND hLabelBirthday = CreateWindow(L"STATIC", L"Birthday", WS_CHILD | WS_VISIBLE | SS_CENTER, 180, 85, 120, 15, hDlg, NULL, NULL, NULL);
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
				SendMessage(birthday, DTM_SETMCFONT, (WPARAM)hFonts[2], 0);
			} else birthday = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_CENTER | ES_MULTILINE | (nt3 ? WS_BORDER : 0), 180, 100, 120, 25, hDlg, NULL, NULL, NULL);
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
		dlgPic = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, 10, 10, 160, 160, hDlg, NULL, NULL, NULL);
		SendMessage(name, EM_REPLACESEL, FALSE, (LPARAM)peer->name);
		int name_len = wcslen(peer->name);
		int deleted_wchars = 0;
		for (int i = 0; i < name_len; i++) i = emoji_adder(i, peer->name, 0, 15, name, &deleted_wchars);
		if (peer->handle) SendMessage(handle, EM_REPLACESEL, FALSE, (LPARAM)peer->handle);
		if (peer->full && peer->about) {
			SendMessage(about, EM_REPLACESEL, FALSE, (LPARAM)peer->about);
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

		apply_fonts(hDlg);
		SendMessage(name, WM_SETFONT, (WPARAM)hFonts[1], FALSE);
		SendMessage(handle, WM_SETFONT, (WPARAM)hFonts[1], FALSE);
		SendMessage(birthday, WM_SETFONT, (WPARAM)hFonts[1], FALSE);
		SendMessage(about, WM_SETFONT, (WPARAM)hFonts[0], FALSE);
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
	case EM_STREAMIN: 
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
				riched_write(msgInput, NULL, buf);
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

			riched_write(msgInput, NULL, buf + offset);
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
			else if (submsg == SB_BOTTOM) {
				SCROLLINFO si;
				si.cbSize = sizeof(si);
				si.fMask = SIF_RANGE | SIF_PAGE;
				GetScrollInfo(chat, SB_VERT, &si);
				SendMessage(chat, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, si.nMax - si.nPage), 0);
				return 0;
			} else if (submsg == SB_LINEDOWN) {
				SCROLLINFO si;
				si.cbSize = sizeof(si);
				si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
				GetScrollInfo(chat, SB_VERT, &si);
				if (si.nPos > (int)(si.nMax - si.nPage) - 20) {
					SendMessage(chat, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, si.nMax - si.nPage), 0);
					return 0;
				}
			}
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

INT_PTR CALLBACK DlgProc2FA(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
	case WM_INITDIALOG: {
		HWND h2FALbl = CreateWindow(L"STATIC", L"Enter your password:", WS_CHILD | WS_VISIBLE, 10, 10, 155, 15, hDlg, NULL, NULL, NULL);
		h2FA = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_PASSWORD | WS_TABSTOP | (nt3 ? WS_BORDER : 0), 10, 25, 130, 25, hDlg, NULL, NULL, NULL);
		SendMessage(h2FA, EM_LIMITTEXT, 63, 0);
		hPass = CreateWindow(L"BUTTON", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | (nt3 ? BS_OWNERDRAW : BS_ICON), 145, 25, 25, 25, hDlg, (HMENU)5, NULL, NULL);
		if (!nt3) {
			HIMAGELIST hImg = ImageList_LoadBitmap(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_ICONS), 16, 0, RGB(128, 0, 128));
			HICON hIcon = ImageList_GetIcon(hImg, 6, ILD_NORMAL);
			ImageList_Destroy(hImg);
			SendMessage(hPass, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		}

		h2FAHint = CreateWindow(L"BUTTON", L"Hint?", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 28, 60, 55, 25, hDlg, (HMENU)3, NULL, NULL);
		HWND h2FABtn = CreateWindow(L"BUTTON", L"Log in", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 93, 60, 55, 25, hDlg, (HMENU)4, NULL, NULL);
		apply_fonts(hDlg);
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
	case WM_DRAWITEM:
		paint_password_button((DRAWITEMSTRUCT*)lParam, false);
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
			HWND hLabelPhone = CreateWindow(L"STATIC", L"Your phone number:", WS_CHILD | WS_VISIBLE, 10, 10, 125, 15, hDlg, NULL, NULL, NULL);
			HWND hLabelCode = CreateWindow(L"STATIC", L"Code:", WS_CHILD | WS_VISIBLE, 10, 55, 125, 15, hDlg, NULL, NULL, NULL);
			HWND hLabelQr = CreateWindow(L"STATIC", L"Or, scan this QR code to log in:", WS_CHILD | WS_VISIBLE, 10, 100, 190, 15, hDlg, NULL, NULL, NULL);
			hNumber = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | (nt3 ? WS_BORDER : 0), 10, 25, 125, 25, hDlg, NULL, NULL, NULL);
			hNumberBtn = CreateWindow(L"BUTTON", L"Get code", WS_CHILD | WS_VISIBLE | WS_TABSTOP, 140, 25, 59, 25, hDlg, (HMENU)3, NULL, NULL);
			hCode = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_TABSTOP | (nt3 ? WS_BORDER : 0), 10, 70, 125, 25, hDlg, NULL, NULL, NULL);
			hCodeBtn = CreateWindow(L"BUTTON", L"Log in", WS_CHILD | WS_VISIBLE | WS_DISABLED | WS_TABSTOP, 140, 70, 59, 25, hDlg, (HMENU)4, NULL, NULL);
			hQRCode = CreateWindowEx(WS_EX_CLIENTEDGE, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY, 10, 115, 185, 185, hDlg, NULL, NULL, NULL);
			apply_fonts(hDlg);

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