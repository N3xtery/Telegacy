/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "telegacy.h"

class CImageDataObject : IDataObject {
private:
	ULONG m_ulRefCnt;
	BOOL  m_bRelease;
	STGMEDIUM m_stgmed;
	FORMATETC m_format;
public:
	CImageDataObject() {
		m_ulRefCnt = 1;
	}
	~CImageDataObject() {
		if (m_bRelease) ReleaseStgMedium(&m_stgmed);
	}
	STDMETHOD(QueryInterface)(REFIID iid, void ** ppvObject) {
		if (iid == IID_IUnknown || iid == IID_IDataObject) {
		  *ppvObject = this;
		  AddRef();
		  return S_OK;
		} else return E_NOINTERFACE;
	}
	STDMETHOD_(ULONG, AddRef)(void) {
		m_ulRefCnt++;
		return m_ulRefCnt;
	}
	STDMETHOD_(ULONG, Release)(void) {
		delete this;
		return m_ulRefCnt;
	}
	STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium) {
		HANDLE hDst;
		if (pformatetcIn->cfFormat == CF_METAFILEPICT && (pformatetcIn->tymed & TYMED_MFPICT)) {
			hDst = OleDuplicateData(m_stgmed.hMetaFilePict, CF_METAFILEPICT, NULL);
			if (hDst == NULL) return E_HANDLE;
			pmedium->tymed = TYMED_MFPICT;
			pmedium->hMetaFilePict = (HMETAFILEPICT)hDst;
			pmedium->pUnkForRelease = NULL;
		} else if (pformatetcIn->cfFormat == CF_BITMAP && (pformatetcIn->tymed & TYMED_GDI)) {
			hDst = OleDuplicateData(m_stgmed.hBitmap, CF_BITMAP, NULL);
			if (hDst == NULL) return E_HANDLE;
			pmedium->tymed = TYMED_GDI;
			pmedium->hBitmap = (HBITMAP)hDst;
			pmedium->pUnkForRelease = NULL;
		} else return DV_E_FORMATETC;
		return S_OK;
	}
	STDMETHOD(GetDataHere)(FORMATETC* pformatetc, STGMEDIUM*  pmedium) {
		return E_NOTIMPL;
	}
	STDMETHOD(QueryGetData)(FORMATETC*  pformatetc ) {
		if (m_stgmed.tymed == TYMED_MFPICT && (pformatetc->cfFormat == CF_METAFILEPICT) && (pformatetc->tymed & TYMED_MFPICT)) return S_OK;
		else if (m_stgmed.tymed == TYMED_GDI && (pformatetc->cfFormat == CF_BITMAP) && (pformatetc->tymed & TYMED_GDI)) return S_OK;
		return DV_E_FORMATETC;
	}
	STDMETHOD(GetCanonicalFormatEtc)(FORMATETC*  pformatectIn, FORMATETC* pformatetcOut) {
		return E_NOTIMPL;
	}
	STDMETHOD(SetData)(FORMATETC* pformatetc, STGMEDIUM*  pmedium, BOOL  fRelease) {
		m_format = *pformatetc;
		m_stgmed = *pmedium;
		m_bRelease = fRelease;
		return S_OK;
	}
	STDMETHOD(EnumFormatEtc)(DWORD dwDirection, IEnumFORMATETC**  ppenumFormatEtc) {
		return E_NOTIMPL;
	}
	STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection) {
		return E_NOTIMPL;
	}
	STDMETHOD(DUnadvise)(DWORD dwConnection) {
		return E_NOTIMPL;
	}
	STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise) {
		return E_NOTIMPL;
	}
};

void insert_image(HWND hRichEdit, HMETAFILEPICT hMetaFilePict, HBITMAP hBitmap) {
    HRESULT hr;
    LPRICHEDITOLE pRichEditOle;
    if (hRichEdit) SendMessage(hRichEdit, EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOle);
	else textHost->textServices->TxSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&pRichEditOle, &hr);

    LPOLECLIENTSITE pClientSite;
    hr = pRichEditOle->GetClientSite(&pClientSite);
	
	LPLOCKBYTES pLockBytes = NULL;
	hr = CreateILockBytesOnHGlobal(NULL, TRUE, &pLockBytes);
	LPSTORAGE pStorage;
	hr = StgCreateDocfileOnILockBytes(pLockBytes, STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_READWRITE, 0, &pStorage);

	CImageDataObject* pImageDataObject = new CImageDataObject();
	STGMEDIUM stg = {0};
	if (hMetaFilePict) {
		FORMATETC fmt = {CF_METAFILEPICT, NULL, DVASPECT_CONTENT, -1, TYMED_MFPICT};
		stg.tymed = TYMED_MFPICT;
		stg.hMetaFilePict = hMetaFilePict;
		hr = pImageDataObject->SetData(&fmt, &stg, TRUE);
	} else {
		FORMATETC fmt = {CF_BITMAP, NULL, DVASPECT_CONTENT, -1, TYMED_GDI};
		stg.tymed = TYMED_GDI;
		stg.hBitmap = hBitmap;
		hr = pImageDataObject->SetData(&fmt, &stg, TRUE);
	}

	IDataObject* pDataObject;
	hr = pImageDataObject->QueryInterface(IID_IDataObject, (void**)&pDataObject);
	LPOLEOBJECT pObject;
	hr = OleCreateStaticFromData(pDataObject, IID_IOleObject, OLERENDER_DRAW, NULL, pClientSite, pStorage, (void**)&pObject);
	OleSetContainedObject(pObject, TRUE);

	CLSID clsid;
    hr = pObject->GetUserClassID(&clsid);

    REOBJECT reobject = { sizeof(REOBJECT) };
    reobject.clsid = clsid;
    reobject.cp = REO_CP_SELECTION;
    reobject.dvaspect = DVASPECT_CONTENT;
    reobject.dwFlags = hMetaFilePict ? REO_BELOWBASELINE : 0;
	reobject.dwUser = 0;
    reobject.poleobj = pObject;
    reobject.polesite = pClientSite;
    reobject.pstg = pStorage;
    SIZEL sizel = {0};
    reobject.sizel = sizel;

    hr = pRichEditOle->InsertObject(&reobject);
	pDataObject->Release();
    pObject->Release();
	pClientSite->Release();
	pStorage->Release();
	pLockBytes->Release();
    pRichEditOle->Release();
}

int utf8_to_wide(BYTE* src, wchar_t* str, int length) {
    int str_pos = 0;
    for (int i = 0; i < length; i++) {
        unsigned int code;
        int cont_bytes;

        if (src[i] < 0x80) {
            code = src[i];
            cont_bytes = 0;
        } else if ((src[i] & 0xE0) == 0xC0) {
            code = src[i] & 0x1F;
            cont_bytes = 1;
        } else if ((src[i] & 0xF0) == 0xE0) {
            code = src[i] & 0x0F;
            cont_bytes = 2;
        } else if ((src[i] & 0xF8) == 0xF0) {
            code = src[i] & 0x07;
            cont_bytes = 3;
        };

		for (int j = 0; j < cont_bytes; j++)
            code = (code << 6) | (src[i + 1 + j] & 0x3F);

        i += cont_bytes;
        if (str == NULL) {
			str_pos += (code <= 0xFFFF) ? 1 : 2;
            continue;
        }
        if (code <= 0xFFFF) str[str_pos++] = (wchar_t)code;
        else {
            code -= 0x10000;
            str[str_pos++] = (wchar_t)(0xD800 + (code >> 10));
            str[str_pos++] = (wchar_t)(0xDC00 + (code & 0x3FF));
        }
    }
    return str_pos;
}

void wide_to_utf8_one(int cr, int* str_pos, BYTE* out) {
	if (out == NULL) {
		if (cr < 0x80) (*str_pos)++;
		else if (cr < 0x800) (*str_pos) += 2;
		else if (cr < 0x10000) (*str_pos) += 3;
		else if (cr < 0x110000) (*str_pos) += 4;
	} else {
		if (cr < 0x80) out[(*str_pos)++] = (char)cr;
		else if (cr < 0x800) {
			out[(*str_pos)++] = 0xC0 | (cr >> 6);
			out[(*str_pos)++] = 0x80 | (cr & 0x3F);
		} else if (cr < 0x10000) {
			out[(*str_pos)++] = 0xE0 | (cr >> 12);
			out[(*str_pos)++] = 0x80 | ((cr >> 6) & 0x3F);
			out[(*str_pos)++] = 0x80 | (cr & 0x3F);
		}
		else if (cr < 0x110000) {
			out[(*str_pos)++] = 0xF0 | (cr >> 18);
			out[(*str_pos)++] = 0x80 | ((cr >> 12) & 0x3F);
			out[(*str_pos)++] = 0x80 | ((cr >> 6)  & 0x3F);
			out[(*str_pos)++] = 0x80 | (cr & 0x3F);
		}
	}
}

int wide_to_utf8(wchar_t* str, BYTE* out) {
	int str_pos = 0, ole_count = 0;
	IRichEditOle* ole = NULL;
	int str_len = wcslen(str);
    for (int i = 0; i < str_len; i++) {
		if (str[i] == 0xFFFC) {
			if (ole == NULL) SendMessage(writing_str_from, EM_GETOLEINTERFACE, 0, (LPARAM)&ole);
			REOBJECT reo = { sizeof(REOBJECT) };
			if (SUCCEEDED(ole->GetObject(ole_count, &reo, REO_GETOBJ_POLEOBJ))) {
				IDataObject* data;
				if (SUCCEEDED(reo.poleobj->QueryInterface(IID_IDataObject, (void**)&data))) {
					FORMATETC fmt = { CF_METAFILEPICT, NULL, DVASPECT_CONTENT, -1, TYMED_MFPICT };
					STGMEDIUM stg = {0};
					if (SUCCEEDED(data->GetData(&fmt, &stg))) {
						ole_count++;
						METAFILEPICT* mp = (METAFILEPICT*)GlobalLock(stg.hMetaFilePict);
						int size = GetMetaFileBitsEx(mp->hMF, 0, NULL);
						char* wmf_data1 = (char*)malloc(size);
						char* wmf_data2 = wmf_data1;
						GetMetaFileBitsEx(mp->hMF, size, wmf_data2);
						DeleteMetaFile(mp->hMF);
						GlobalUnlock(stg.hMetaFilePict);
						wmf_data2 += sizeof(METAHEADER);
						while (true) {
							METARECORD* mr = (METARECORD*)wmf_data2;
							if (LOBYTE(mr->rdFunction) == LOBYTE(META_ESCAPE) && mr->rdParm[0] == MFCOMMENT) {
								wmf_data2 = (char*)&mr->rdParm[2];
								break;
							} else wmf_data2 += mr->rdSize * 2;
						}
						wchar_t* wmf_data = (wchar_t*)wmf_data2;
						wchar_t emojis[10] = {0};
						while (true) {
							wchar_t* end_ptr = wcschr(wmf_data, L'-');
							int number = (int)wcstol(wmf_data, &end_ptr, 16);
							if (out != (BYTE*)-1) wide_to_utf8_one(number, &str_pos, out);
							else {
								if (number <= 0xFFFF) emojis[wcslen(emojis)] = number;
								else {
									number -= 0x10000;
									emojis[wcslen(emojis)] = 0xD800 + (number >> 10);
									emojis[wcslen(emojis)] = 0xDC00 + (number & 0x3FF);
								}
							}
							if (end_ptr[0] != L'\0') wmf_data = end_ptr + 1;
							else break;
						}
						if (out == (BYTE*)-1) {
							int emoji_len = wcslen(emojis);
							wcsncpy(str + i + emoji_len, str + i + 1, str_len - i);
							wcsncpy(str + i, emojis, emoji_len);
							i += emoji_len - 1;
							str_len += emoji_len - 1;
						}
						free(wmf_data1);
					}
					data->Release();
				}
				reo.poleobj->Release();
			}
		} else if (out != (BYTE*)-1) wide_to_utf8_one((int)str[i], &str_pos, out);
    }
	if (ole != NULL) ole->Release();
    return str_pos;
}


void write_be(BYTE* buf, unsigned __int64 val, int count) {
	for (int i = 0; i < count; i++) {
		buf[i] = (unsigned char)(val >> (count-1-i)*8);
	}
}

void write_le(BYTE* buf, unsigned __int64 val, int count) {
	memcpy(buf, &val, count);
}

__int64 read_be(BYTE* buf, int count) {
	__int64 converted = 0;
	for (int i = 0; i < count; i++) {
		converted |= ((__int64)buf[i] << (count-1-i)*8);
	}
	return converted;
}

__int64 read_le(BYTE* buf, int count) {
	__int64 converted = 0;
	memcpy(&converted, buf, count);
	return converted;
}

int str_to_tlstr_len(wchar_t* text) {
	int size = wide_to_utf8(text, NULL);
	int size_len = (size < 254) ? 1 : 4;
	int padding_length = (4 - (size + size_len) % 4) % 4;
	return size + size_len + padding_length;
}

int tlstr_len(BYTE* buf, bool full) {
	int str_len = 0;
	if (buf[0] == 254) {
		str_len = read_le(buf + 1, 3);
		if (full) str_len += 4;
	} else {
		str_len = buf[0];
		if (full) str_len++;
	}
	if (full) str_len += (4 - str_len % 4) % 4;
	return str_len;
}

int tlstr_to_str_len(BYTE* buf) {
	int str_len = tlstr_len(buf, false);
	int str_start = str_len >= 254 ? 4 : 1;
	return utf8_to_wide(buf + str_start, NULL, str_len);
}

void write_string(BYTE* buf, wchar_t* text) {
	int size = wide_to_utf8(text, NULL);
	int size_len = (size < 254) ? 1 : 4;
	int padding_length = (4 - (size + size_len) % 4) % 4;
	if (size < 254) {
		buf[0] = size;
	} else {
		buf[0] = 254;
		write_le(buf + 1, size, 3);
	}
	wide_to_utf8(text, buf + size_len);
	memset(buf + size_len + size, 0, padding_length);
}

wchar_t* read_string(BYTE* buf, wchar_t* location) {
	int str_len = tlstr_len(buf, false);
	int str_start = str_len >= 254 ? 4 : 1;
	int wide_len = utf8_to_wide(buf + str_start, NULL, str_len);
	if (location == NULL) {
		wchar_t* str = (wchar_t*)malloc((wide_len + 1) * sizeof(wchar_t));
		utf8_to_wide(buf + str_start, str, str_len);
		str[wide_len] = 0;
		return str;
	}
	else {
		utf8_to_wide(buf + str_start, location, str_len);
		location[wide_len] = 0;
		return NULL;
	}
}

void wemoji_to_path(wchar_t* emoji_str, wchar_t* file_name, bool dir) {
	bool firstdone = false;
	for (int j = 0; j < wcslen(emoji_str); j++) {
		int cr;
		if (emoji_str[j] >= 0xD800 && emoji_str[j] <= 0xDBFF) {
			cr = ((emoji_str[j] - 0xD800) << 10) + (emoji_str[j+1] - 0xDC00) + 0x10000;
			j++;
		} else cr = emoji_str[j];
		if (!firstdone) {
			swprintf(file_name, L"%s%x", file_name, cr);
			firstdone = true;
		} else swprintf(file_name, L"%s-%x", file_name, cr);
	}
	if (dir) wcscat(file_name, L".ico");
}