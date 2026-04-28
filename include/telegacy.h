/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#define _WIN32_WINNT 0x0400
#define WINVER 0x0500
#define WM_TRAYICON (WM_USER + 1)
#include <vector>
#include <deque>
#include <list>
#include <windows.h>
#include <windowsx.h>
#include <wincrypt.h>
#include <commctrl.h>
#include <winsock.h>
#include <math.h>
#include <time.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>
#include <richedit.h>
#include <oleidl.h>
#include <richole.h>
#include <dimm.h>
#include <algorithm>
#include <tomcrypt.h>
#include <tommath.h>
#include <miniz.h>
#include <ocidl.h>
#include <textserv.h>
#include <src/webp/decode.h>
#include <qrcodegen.h>
#include <htmlhelp.h>
#include <shlobj.h>
#include <jpeglib.h>
#include <ddeml.h>
#include "../res/resource.h"
#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "tomcrypt.lib")
#pragma comment(lib, "miniz.lib")
#pragma comment(lib, "libwebp.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "riched20.lib")
#pragma comment(lib, "qrcodegen.lib")
#pragma comment(lib, "htmlhelp.lib")
#pragma comment(lib, "libjpeg.lib")
extern "C" int __cdecl _fseeki64(FILE*, __int64, int);
extern "C" __int64 __cdecl _ftelli64(FILE*);

struct Permissions {
	bool cansendmsg;
	bool cansendmed;
	bool cansendvoice;
	bool cansendphoto;
	bool cansendvideo;
	bool cansendaudio;
	bool cansenddocs;
	bool canchangedesc;
};

struct Peer {
	BYTE id[8];
	BYTE access_hash[8];
	wchar_t* name;
	wchar_t* handle;
	wchar_t* about;
	BYTE birthday[4];
	int online;
	int last_read;
	int last_recv;
	int unread_msgs_count;
	int mute_until;
	BYTE photo[8];
	int photo_dc;
	std::vector<wchar_t*>* reaction_list;
	std::vector<Peer>* chat_users;
	BYTE theme_id[8];
	int theme_set_time;
	int name_set_time;
	int pfp_set_time;
	int channel_pts;
	BYTE channel_msg_id[8];
	Permissions perm;
	bool amadmin;
	bool full;
	bool status_updated;
	char type; // 0 - user, 1 - group, 2 - channel
};

struct ChatsFolder {
	BYTE id[4];
	int count;
	int pinned_count;
	int* peers;
	wchar_t* name;
};

struct Document {
	int min, max;
	__int64 size;
	BYTE photo_msg_id[8];
	BYTE id[8];
	BYTE access_hash[8];
	BYTE* file_reference;
	int dc;
	wchar_t* filename;
	char photo_size;
	bool visible;
};

struct Message {
	int start_char, end_header, end_char, start_reactions, end_footer, reply_needed, id;
	std::vector<wchar_t*> reacted;
	bool outgoing;
	bool seen;
};

struct Theme {
	BYTE id[8];
	COLORREF color;
	BYTE* emoji_id;
};

struct CustomEmojiPlacement {
	BYTE peer_id[8];
	int msg_id;
	int pos;
	int size;
	wchar_t* path;
};

struct RequestedCustomEmoji {
	__int64 id;
	__int64 access_hash;
	BYTE file_reference[32];
	int dc;
	BYTE msg_id[8];
	std::vector<CustomEmojiPlacement> ceps;
	bool reaction;
};

struct ICONDIR {
    WORD idReserved;
    WORD idType;
    WORD idCount;
};

struct ICONDIRENTRY {
    BYTE bWidth;
    BYTE bHeight;
    BYTE bColorCount;
    BYTE bReserved;
    WORD wPlanes;
    WORD wBitCount;
    DWORD dwBytesInRes;
    DWORD dwImageOffset;
};

struct StreamData {
	BYTE* buf;
	int written;
	int length;
};

struct DCInfo {
	int dc_file_index;
	int dc;
	SOCKET sock;
	BYTE auth_key[256];
	BYTE auth_key_id[8];
	BYTE session_id[8];
	BYTE server_salt[8];
	BYTE last_msg_id_sent[8];
	BYTE last_msg_id[8];
	BYTE future_salt[8];
	int current_seq_no;
	bool authorized;
	bool ready;
};

struct UnmuteTimer {
	__int64 peer_id;
	int timer_id;
};

struct ReplyFront {
	int i;
	int j;
	BYTE* message;
};

struct uploadThreadEvent {
	HANDLE event;
	BYTE lastId[8];
};

struct NOTIFYICONDATAV2 {
    DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    WCHAR  szTip[128];
    DWORD dwState;
    DWORD dwStateMask;
    WCHAR  szInfo[256];
    union {
        UINT  uTimeout;
        UINT  uVersion;
    } DUMMYUNIONNAME;
    WCHAR  szInfoTitle[64];
    DWORD dwInfoFlags;
};

extern BYTE pubkey_der[];
extern BYTE pubkey_der_test[];
extern unsigned int pubkey_der_len;

extern prng_state prng;
extern hash_state md;
extern CRITICAL_SECTION csSock, csCM;

extern DCInfo dcInfoMain;
extern int time_diff;
extern int pts;
extern int qts;
extern int date;

extern int database_version;
extern bool test_server;
extern bool drawchat;
extern bool closing;
extern HWND current_dialog, current_notification, current_info, current_about;
extern HWND dlgPic, infoLabel;
extern HBITMAP infoBmp, infoBmpMask;
extern int width;
extern int height;
extern bool maximized;
extern wchar_t appdata_path[MAX_PATH];
extern wchar_t exe_path[MAX_PATH];

extern BYTE* phone_number_bytes;
extern BYTE* phone_code_hash;
extern BYTE* qrCodeToken;

extern HWND hComboBoxChats, hComboBoxFolders, msgInput, chat, hMain, hStatus, hToolbar, tbSeparatorHider, hTabs, emojiStatic, emojiScroll, hOverlayTabs, reactionStatic, splitter;
extern HWND hNumber, hNumberBtn, hCode, hCodeBtn, hQRCode, h2FA, hPass, h2FAHint, hProxyIP, hProxyPort, hProxyUsername, hProxyPassword, hProxyHidePassword;
extern IActiveIMMApp* g_pAIMM;
extern HMENU hMenuBar;
extern HFONT hFonts[3];

extern wchar_t* last_tofront_sender;
extern BYTE group_id_tofront[8];
extern BYTE group_id[8];

extern Peer myself;
extern Peer* peers;
extern int peers_count;
extern int total_peers_count;
extern ChatsFolder* folders;
extern Peer* current_peer;
extern Peer* old_peer;
extern ChatsFolder* current_folder;
extern int folders_count;
extern std::deque<Document> documents;
extern std::vector<Document> downloading_docs;
extern std::vector<TEXTRANGE> links;
extern std::deque<Message> messages;
extern std::deque<wchar_t*> fav_emojis;
extern std::vector<wchar_t*> reaction_list;
extern int reaction_hash;
extern __int64 theme_hash;
extern int sel_msg_id;
extern int editing_msg_id;
extern int replying_msg_id;
extern int forwarding_msg_id;
extern BYTE forwarding_peer_id[8];
extern std::vector<wchar_t*> files;
extern std::vector<Theme> themes;
extern HBRUSH theme_brush;
extern std::vector<uploadThreadEvent*> uploadEvents;
extern BYTE last_rpcresult_msgid[8];
extern BYTE* sendMultiMedia;
extern BYTE pfp_msgid[8];
extern BYTE handle_msgid[8];
extern BYTE flood_msg_id[8];
extern Peer dlg_peer;
extern HWND writing_str_from;
extern wchar_t status_str[100];
extern std::vector<RequestedCustomEmoji> rces;
extern std::list<DCInfo> active_dcs;
extern int muted_types[3];
extern int total_unread_msgs_count;
extern BYTE notification_peer_id[8];
extern BYTE difference_msg_id[8];
extern std::vector<UnmuteTimer> unmuteTimers;
extern bool dragging;
extern int edits_border_offset;
extern bool dontlosefocus;
extern bool needtosetuplogin;
extern bool minimized;
extern bool ischatscrolling;
extern bool ie4;
extern bool ie3;
extern bool nt3;
extern bool hint_needed;
extern wchar_t* hint;
extern bool no_more_msgs;
extern int get_dialogs_lowest_date;
extern bool closed_logged_out;
extern int dpi;

extern HWAVEIN hWaveIn;
extern WAVEHDR hdr_buf[4];
extern std::vector<BYTE> recorded;

extern bool balloon_notifications_available;
extern bool balloon_notifications;
extern bool SENDMEDIAASFILES;
extern bool CLOSETOTRAY;
extern int IMAGELOADPOLICY;
extern bool EMOJIS;
extern bool SPOILERS;
extern wchar_t sound_paths[3][MAX_PATH];
extern int SAMPLERATE;
extern int BITSPERSAMPLE;
extern int CHANNELS;

class CTextHost : public ITextHost {
public:
	ULONG refCount;
    ITextServices* textServices;

    CTextHost() {
		refCount = 1;
		IUnknown* unknown;
		CreateTextServices(NULL, this, &unknown);
		const IID IID_ITextServices_fixed = {0x8d33f740, 0xcf58, 0x11ce, {0xa8, 0x9d, 0x00, 0xaa, 0x00, 0x6c, 0xad, 0xc5}};
		unknown->QueryInterface(IID_ITextServices_fixed, (void**)&textServices);
		unknown->Release();
	}
	
	STDMETHODIMP QueryInterface(REFIID riid, void** ppvObject) {
		if (riid == IID_IUnknown || riid == IID_ITextHost) {
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

	HDC TxGetDC() { return 0; }
	INT TxReleaseDC(HDC) { return 1; }
	HRESULT TxGetClientRect(LPRECT) { return E_FAIL; }
	COLORREF TxGetSysColor(int nIndex) { return GetSysColor(nIndex); }
	HBRUSH TxGetSysColorBrush(int nIndex) { return GetSysColorBrush(nIndex); }
	void TxInvalidateRect(LPCRECT, BOOL) {}
	BOOL TxSetScrollRange(int, LONG, int, BOOL) { return TRUE; }
	BOOL TxSetScrollPos(int, int, BOOL) { return TRUE; }
	BOOL TxShowScrollBar(INT, BOOL) { return FALSE; }
	BOOL TxEnableScrollBar(INT, INT) { return FALSE; }
	void TxScrollWindowEx(INT, INT, LPCRECT, LPCRECT, HRGN, LPRECT, UINT) {}
	void TxSetCapture(BOOL) {}
	void TxSetFocus() {}
	void TxSetCursor(HCURSOR, BOOL) {}
	BOOL TxScreenToClient(LPPOINT) { return FALSE; }
	BOOL TxClientToScreen(LPPOINT) { return FALSE; }
	HRESULT TxActivate(LONG*) { return S_OK; }
	HRESULT TxDeactivate(LONG) { return S_OK; }
	HRESULT TxGetCharFormat(const CHARFORMATW**) { return E_NOTIMPL; }
	HRESULT TxGetParaFormat(const PARAFORMAT**) { return E_NOTIMPL; }
	HRESULT TxGetViewInset(LPRECT prc) {
		memset(prc, 0, sizeof(RECT));
		return S_OK;
	}
	HRESULT TxGetBackStyle(TXTBACKSTYLE *pstyle) {
		*pstyle = TXTBACK_TRANSPARENT;
		return S_OK;
	}
	HRESULT TxGetMaxLength(DWORD *plength) {
		*plength = 1024*1024*16;
		return S_OK;
	}
	HRESULT TxGetScrollBars(DWORD *pdwScrollBar) {
		*pdwScrollBar = 0;
		return S_OK;
	}
	HRESULT TxGetPasswordChar(TCHAR*) { return S_FALSE; }
	HRESULT TxGetAcceleratorPos(LONG* pcp) {
		*pcp = -1;
		return S_OK;
	}
	HRESULT TxGetExtent(LPSIZEL) { return E_NOTIMPL; }
	HRESULT OnTxCharFormatChange(const CHARFORMATW*) { return S_OK; }
	HRESULT OnTxParaFormatChange(const PARAFORMAT*) { return S_OK; }
	HRESULT TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits) {
		DWORD bits = TXTBIT_RICHTEXT | TXTBIT_WORDWRAP;
		*pdwBits = bits & dwMask;
		return S_OK;
	}
	HRESULT TxNotify(DWORD, void*) { return S_OK; }
	HIMC TxImmGetContext() { return 0; }
	void TxImmReleaseContext(HIMC) {}
	HRESULT TxGetSelectionBarWidth(LONG *lSelBarWidth) {
		*lSelBarWidth = 0;
		return S_OK;
	}
	BOOL TxCreateCaret(HBITMAP, int, int) { return TRUE; }
	BOOL TxSetCaretPos(int, int) { return TRUE; }
	BOOL TxShowCaret(BOOL) { return TRUE; }
	BOOL TxDestroyCaret() { return TRUE; }
	BOOL TxSetTimer(UINT, UINT) { return TRUE; }
	void TxKillTimer(UINT) {}
	void TxViewChange(BOOL) {}
	BOOL TxIsVisible() { return TRUE; }
};

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
extern CTextHost* textHost;

// calculations.cpp
void aes_ige(const unsigned char *in, unsigned char *out, size_t length, const unsigned char *key, const unsigned char *iv, int encrypt);
unsigned __int64 pq_gcd(unsigned __int64 a, unsigned __int64 b);
unsigned __int64 pq_add_mul(unsigned __int64 c, unsigned __int64 a, unsigned __int64 b, unsigned __int64 pq);
unsigned __int64 pq_factorize(unsigned __int64 pq);

// conversions.cpp
void insert_image(HWND hRichEdit, HMETAFILEPICT hMetaFilePict, HBITMAP hBitmap);
void write_be(BYTE* buf, unsigned __int64 val, int count);
void write_le(BYTE* buf, unsigned __int64 val, int count);
__int64 read_be(BYTE* buf, int count);
__int64 read_le(BYTE* buf, int count);
int str_to_tlstr_len(wchar_t* text);
int tlstr_len(BYTE* buf, bool full);
int tlstr_to_str_len(BYTE* buf);
void write_string(BYTE* buf, wchar_t* text);
wchar_t* read_string(BYTE* buf, wchar_t* location);
void wide_to_utf8_one(int cr, int* str_pos, BYTE* out);
int wide_to_utf8(wchar_t* str, BYTE* out);
void wemoji_to_path(wchar_t* emoji_str, wchar_t* file_name, bool dir);

// helpers.cpp
int current_time();
char get_padding(int len);
void send_query(BYTE* enc_query, int length);
void send_query(DCInfo* dcInfo, BYTE* enc_query, int length);
void create_msg_id(BYTE* buf);
void create_msg_id(DCInfo* dcInfo, BYTE* buf);
void create_msg_key(BYTE* unenc_query, int length, int x, BYTE* msg_key);
void create_msg_key(DCInfo* dcInfo, BYTE* unenc_query, int length, int x, BYTE* msg_key);
bool convert_message(BYTE* unenc, BYTE* enc, int length, int x);
bool convert_message(DCInfo* dcInfo, BYTE* unenc, BYTE* enc, int length, int x);
void create_seq_no(BYTE* buf, bool content_related);
void create_seq_no(DCInfo* dcInfo, BYTE* buf, bool content_related);
void internal_header(BYTE* unenc_query, bool content_related);
void internal_header(DCInfo* dcInfo, BYTE* unenc_query, bool content_related);
int set_peer_info(BYTE* unenc_response, Peer* peer, bool just_update);
void update_chats_order(BYTE* id, BYTE* msg_id, char type);
void download_file(DCInfo* dcInfo, Document* document);
int folder_handler(BYTE* unenc_response, ChatsFolder* folder, int i, bool update);
DWORD CALLBACK StreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
DWORD CALLBACK StreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
int insert_format(BYTE* unenc_query, int format_count, std::vector<int>* format_vecs);
wchar_t* files_i(wchar_t* file_name);
int place_inputmedia(BYTE* unenc_query, Document* docstemp, int index);
void get_future_salt(DCInfo* dcInfo);
void update_own_status(bool status);
void get_history();
void set_typing(int cons, int add);
void make_seen(Message* message);
void update_positions(int diff, int pos, int new_links);
int replace_in_chat(FINDTEXTEX* ft, CHARRANGE* cr, wchar_t* replacement, HBITMAP hBitmap, BYTE* reactions, CustomEmojiPlacement* cep, ReplyFront* rf);
int array_find(BYTE* buf, BYTE* find, int find_len, int find_count);
int place_peer(BYTE* unenc_query, Peer* peer, bool peer_name);
void set_reply_tofront(int i, BYTE* message, int j);
void get_date(wchar_t* buf, int date_init, bool preposition);
int set_name(BYTE* unenc_response, wchar_t** name);
int compound_emoji_checker(wchar_t* msg, int chars_left);
bool insert_emoji(wchar_t* path, int size, HWND richedit);
int emoji_adder(int i, wchar_t* msg, int pos, int size, HWND chat, int* deleted_wchars);
int set_reply(int i, int start_footer, BYTE* quote_text, bool setformat);
void status_bar_status(Peer* peer);
void user_status_updated(BYTE* userStatus, Peer* peer);
void delete_message(int j, bool scroll);
void apply_theme(int i);
void register_themes();
void set_permissions(BYTE* unenc_response, Peer* peer);
void get_channel_difference(Peer* peer);
void get_peerid_from_msg(BYTE* unenc_response, BYTE** id, BYTE** msg_id);
HBITMAP jpg_to_bmp(BYTE* unenc_response, int myjpg_size);
void write_md5(BYTE* unenc_query, FILE* f);
void get_full_peer(Peer* peer);
void update_name_in_list(int i);
void get_state(bool problem);
void get_message(int id, Peer* peer);
int set_reactions(BYTE* reactions, Message* message_footer, std::vector<int>* format_vecs, int modifier, int msg_id);
void try_to_add_fe0f(wchar_t* file_name);
void update_pts(int new_pts);
void peer_set_name(BYTE* unenc_response, wchar_t** name, char type);
void get_photo(RequestedCustomEmoji* rce, Document* document, DCInfo* dcInfo);
void send_ping(DCInfo* dcInfo);
void get_config();
void get_pfp(DCInfo* dcInfo, Peer* peer);
void unknown_custom_emoji_solver(int msg_id, int pos, int size, __int64 custom_emoji_id, bool reaction);
void get_unknown_custom_emojis();
void exit_telegacy();
int save_dcs(BYTE* unenc_response, BYTE* this_dc);
void apply_notifysettings(BYTE* unenc_response, int* mute_until, __int64 id);
void new_msg_notification(Peer* peer, BYTE* msg_bytes, bool groupmed);
void update_total_unread_msgs_count(int new_total_unread_msgs_count);
void click_on_notification();
void remove_notification();
void get_dll_version(wchar_t* dll, DWORD* minor, DWORD* major);
void set_tray_icon();
void change_mute_all(int type, bool unmuted);
BYTE* find_peer(BYTE* peer_bytes, BYTE* type_bytes, bool has_peer_type, char* type);
int msgfwd_addname(BYTE* peer_bytes, BYTE* msgfwd, int pos_init, bool shortmsg);
void place_dialog_center(HWND hDlg, bool main);
wchar_t* get_path(wchar_t* path, wchar_t* file_name);
void get_dialogs();
void get_folders();
void files_show_dropdown();
void open_files_list();
void remove_peer(Peer* peer);
void read_react_ment(bool react);
void logout_cleanup();
void set_sep_width(int width);
HBITMAP rgb_to_bmp(BYTE* rgb, bool alpha, int width, int height);
bool paint_emoji_bitmap(HDC hdc, wchar_t* path, RECT* rect);
void paint_emoji_button(DRAWITEMSTRUCT* dis);
void paint_password_button(DRAWITEMSTRUCT* dis, bool options);
void bring_me_to_life();
void init_default_font(int index);

// message.cpp
int message_handler(bool to_front, BYTE* message, bool update_order, bool editing, bool rplhelper);
void message_adder(bool service, bool to_front, int flags, BYTE* msg_id, BYTE* msg_bytes, wchar_t* service_msg, EDITSTREAM* es, BYTE* chat_member_id, std::vector<int>* format_vecs, BYTE* reactions, BYTE* msgrpl, BYTE* msgfwd, BYTE* views, bool groupmed_end, bool footer, bool editing, int date);

// response.cpp
void response_handler(DCInfo* dcInfo, BYTE* unenc_response, bool acknowledgement, int length);
int update_handler(BYTE* update);

// procs.cpp
extern WNDPROC oldMsgInputProc, oldChatProc, oldEmojiScrollProc, oldEmojiStaticProc, oldReactionStaticProc, oldEmojiScrollFallbackProc, oldSplitterProc, oldReactionButtonProc;
extern LRESULT CALLBACK WndProcMsgInput(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WndProcChat(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WndProcEmojiScroll(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WndProcEmojiScrollFallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WndProcEmojiStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WndProcReactionStatic(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WndProcReactionButton(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK WndProcSplitter(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcLogin(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProc2FA(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcInfo(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProcOptions(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK DlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

// telegacy.cpp
int init_connection(DCInfo* dcInfo, bool socketworkerreconnect);
void reconnect(DCInfo* dcInfo);
void create_auth_key(DCInfo* dcInfo);
unsigned __stdcall SocketWorker(void* param);
unsigned __stdcall FileSenderWorker(void* param);

// offsets.cpp
int msgfwd_offset(BYTE* unenc_response);
int messagemedia_offset(BYTE* unenc_response);
int msgrpl_offset(BYTE* unenc_response);
int msgent_offset(BYTE* unenc_response, std::vector<int>* format_vecs);
int photo_video_size_offset(BYTE* unenc_response, bool photo, bool photo_vec, bool video);
int docatt_offset(BYTE* unenc_response);
int replymarkup_offset(BYTE* unenc_response);
int msgreact_offset(BYTE* unenc_response);
int textwithent_offset(BYTE* unenc_response);
int wallpaper_offset(BYTE* unenc_response);
int msgact_offset(BYTE* message, int offset_msg, wchar_t** service_msg, bool* service_msg_allocated);
int sendmsgaction_offset(BYTE* message);
int doc_offset(BYTE* unenc_response);
int photo_offset(BYTE* unenc_response);
int peernotifyset_offset(BYTE* unenc_response);
int botinfo_offset(BYTE* unenc_response);
int exchatinv_offset(BYTE* unenc_response);
int chatreactions_offset(BYTE* unenc_response, Peer* peer);
int geo_offset(BYTE* unenc_response);
int chatphoto_offset(BYTE* unenc_response);
int inputchannel_offset(BYTE* unenc_response);
int story_offset(BYTE* unenc_response);
