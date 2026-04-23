/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

#include <telegacy.h>

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
	case 0x9de7a269:
		offset_msg += 16;
		break;
	case 0x861cc8a0:
	case 0xe67f520e:
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
			case 0x75c78e60:
				offset_msg += 12;
				break;
			case 0x21e1ad6:
				offset_msg += 8;
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				break;
			case 0xe0b0bc2e:
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				break;
			case 0xfa3efb95:
				offset_msg += 12;
				offset_msg += read_le(unenc_response + offset_msg, 4) * 4 + 4;
				break;
			case 0xd8214d41:
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
			case 0xde33b094: {
				int flags_videosize = read_le(unenc_response + offset_msg, 4);
				offset_msg += 4;
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				offset_msg += 12;
				if (flags_videosize & (1 << 0)) offset_msg += 8;
				break;
			}
			case 0xf85c413c:
				offset_msg += 12;
				offset_msg += read_le(unenc_response + offset_msg, 4) * 4 + 4;
				break;
			case 0xda082fe:
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
	case 0x6c37c15c:
		offset_msg += 8;
		break;
	case 0x6319d612: {
		int flags_docattstick = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += inputstickerset_offset(unenc_response + offset_msg);
		if (flags_docattstick & (1 << 0)) offset_msg += 32;
		break;
	}
	case 0x43c57c48: {
		int flags_docattvid = read_le(unenc_response + offset_msg, 4);
		offset_msg += 20;
		if (flags_docattvid & (1 << 2)) offset_msg += 4;
		if (flags_docattvid & (1 << 4)) offset_msg += 8;
		if (flags_docattvid & (1 << 5)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case 0x9852f9c6: {
		int flags_docattaud = read_le(unenc_response + offset_msg, 4);
		offset_msg += 8;
		if (flags_docattaud & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_docattaud & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_docattaud & (1 << 2)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case 0x15590068:
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case 0xfd149899:
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
	if (doc_cons == 0x36f8c871) offset_msg += 8;
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
	if (read_le(unenc_response + offset_msg, 4) == 0x2331b22d)
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
	case 0xdc3d824f:
		break;
	case 0x744694e0:
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case 0x3c2884c1:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 8;
		break;
	case 0xde5a0dd6:
	case 0x1ccb966a:
	case 0x35553762:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case 0x7e6260d7: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += richtext_offset(unenc_response + offset_msg);
		break;
	}
	case 0x81ccf4f:
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
	case 0x35a95cb9:
		offset_msg += 8;
		break;
	case 0xdde8a54c:
	case 0x27bcbbfc:
		offset_msg += 16;
		break;
	case 0xa87b0a1c:
	case 0xbd2a0840:
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
	if (chatphoto_cons == 0x1c6e1c11 || chatphoto_cons == 0x82d1f706) {
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
	if (geo_cons == 0xb2a2f663) {
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
	if (inputchannel_cons == 0xf35aec28) offset_msg += 16;
	else if (inputchannel_cons == 0x5b934f9d) {
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
	case 0x13567e8a:
	case 0xdb20b188:
		break;
	case 0xbaafe5e0:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += 4;
		break;
	case 0xc070d93e:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case 0xce0d37b0:
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case 0xe4e88011: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int pageblock_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (pageblock_cons == 0xb92fb6cd) offset_msg += richtext_offset(unenc_response + offset_msg);
			else {
				int count2 = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int j = 0; j < count2; j++) offset_msg += pageblock_offset(unenc_response + offset_msg);
			}
		}
		break;
	}
	case 0x263d7c26:
	case 0x4f4456d3:
		offset_msg += richtext_offset(unenc_response + offset_msg);
		offset_msg += richtext_offset(unenc_response + offset_msg);
		break;
	case 0x1759c560: {
		int flags = read_le(unenc_response + offset_msg, 4);
		offset_msg += 12;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		if (flags & (1 << 0)) {
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 8;
		}
		break;
	}
	case 0x7c8fe7b6:
		offset_msg += 12;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	case 0x39f23300:
		offset_msg += pageblock_offset(unenc_response + offset_msg);
		break;
	case 0xa8718dc5: {
		int flags = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags & (1 << 2)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags & (1 << 4)) offset_msg += 8;
		if (flags & (1 << 5)) offset_msg += 8;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	}
	case 0xf259a80b: {
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
	case 0x65a0fa4d:
	case 0x31f9590: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += pageblock_offset(unenc_response + offset_msg);
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	}
	case 0xef1751b5: {
		int chat_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		switch (chat_cons) {
		case 0x29562865:
			offset_msg += 8;
			break;
		case 0x41cbf256: {
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
		case 0x6592a1a7:
			offset_msg += 8;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		case 0xe00998b7: {
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
				if (emoji_cons == 0x929b619d) offset_msg += 8;
				else if (emoji_cons == 0xfa30a8c7) offset_msg += 12;
			}
			if (flags2 & (1 << 10)) offset_msg += 4;
			if (flags2 & (1 << 11)) offset_msg += 4;
			break;
		}
		case 0x17d493d5: {
			int flags = read_le(unenc_response + offset_msg, 4);
			offset_msg += 20;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags & (1 << 16)) offset_msg += 4;
			break;
		}
		}
		break;
	}
	case 0x804361ea:
		offset_msg += 8;
		offset_msg += pagecaption_offset(unenc_response + offset_msg);
		break;
	case 0xbf4dea82: {
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
	case 0x9a8ae1e1: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int list_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (list_cons == 0x5e068047) offset_msg += richtext_offset(unenc_response + offset_msg);
			else {
				int count2 = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int j = 0; j < count2; j++) offset_msg += pageblock_offset(unenc_response + offset_msg);
			}
		}
		break;
	}
	case 0x76768bed: {
		offset_msg += 4;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += pageblock_offset(unenc_response + offset_msg);
		offset_msg += richtext_offset(unenc_response + offset_msg);
		break;
	}
	case 0x16115a96: {
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
	case 0xa44f3ef6:
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
	if (reaction_cons == 0x1b2286b8) offset_msg += tlstr_len(unenc_response + offset_msg, true);
	else if (reaction_cons == 0x8935fc73) offset_msg += 8;
	return offset_msg;
}

int msgextmed_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int msgextmed_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (msgextmed_cons == 0xad628cc8) {
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
	if (inputuser_cons == 0xf21158c6) offset_msg += 16;
	else if (inputuser_cons == 0x1da448e2) {
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
	case 0xdc7b1140:
		offset_msg += 16;
		break;
	case 0x208e68c9: {
		offset_msg += 8;
		offset_msg += inputuser_offset(unenc_response + offset_msg);
		break;
	}
	case 0xbd610bc9:
	case 0x826f8b60:
	case 0x9c4e7e8b:
	case 0xbf0693d4:
	case 0xf1ccaaac:
	case 0x28a20571:
	case 0x32ca960f:
	case 0x6ed02538:
	case 0x76a6d327:
	case 0x73924be0:
	case 0xc8cf05f8: {
		if (msgent_cons == 0xf1ccaaac) offset_msg += 4;
		int format_start = read_le(unenc_response + offset_msg, 4);
		int format_length = read_le(unenc_response + offset_msg + 4, 4);
		int index = 0;
		if (msgent_cons == 0x826f8b60) index = 1;
		else if (msgent_cons == 0x9c4e7e8b) index = 2;
		else if (msgent_cons == 0xbf0693d4) index = 3;
		else if (msgent_cons == 0xf1ccaaac) index = 4;
		else if (msgent_cons == 0x28a20571) index = 5;
		else if (msgent_cons == 0x73924be0) {
			index = 5;
			offset_msg += tlstr_len(unenc_response + offset_msg + 8, true);
		} else if (msgent_cons == 0x32ca960f) index = 6;
		else if (msgent_cons == 0x6ed02538) index = 7;
		else if (msgent_cons == 0x76a6d327) {
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
		if (msgent_cons == 0xc8cf05f8) {
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
	if (story_cons == 0x51e6ee4f) offset_msg += 4;
	else if (story_cons == 0xffadc913) offset_msg += 16;
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
				case 0xbe82db9c: {
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += geo_offset(unenc_response + offset_msg);
					for (int l = 0; l < 5; l++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
					break;
				}
				case 0xb282217f:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += 8;
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
					break;
				case 0xcad5452d: {
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
				case 0x14455871:
					offset_msg += 4;
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += reaction_offset(unenc_response + offset_msg);
					break;
				case 0x770416af:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += 12;
					break;
				case 0x2271f2bf:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += inputchannel_offset(unenc_response + offset_msg);
					offset_msg += 4;
					break;
				case 0x37381085:
					offset_msg += medareacoords_offset(unenc_response + offset_msg);
					offset_msg += tlstr_len(unenc_response + offset_msg, true);
					break;
				case 0x49a6549c:
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
				case 0xb8905fb2:
				case 0xe4621141:
				case 0x6b134e8e:
				case 0x41c87565: {
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
	case 0x695150d7: {
		int flags_medphoto = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_medphoto & (1 << 0)) offset_msg += photo_offset(unenc_response + offset_msg);
		if (flags_medphoto & (1 << 2)) offset_msg += 4;
		break;
	}
	case 0x56e0d474: {
		offset_msg += geo_offset(unenc_response + offset_msg);
		break;
	}
	case 0x70322949: {
		for (int j = 0; j < 4; j++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += 8;
		break;
	}
	case 0xdd570bd5: {
		int flags_doc1 = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_doc1 & (1 << 0)) offset_msg += doc_offset(unenc_response + offset_msg);
		if (flags_doc1 & (1 << 5)) {
			int count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int j = 0; j < count; j++) offset_msg += doc_offset(unenc_response + offset_msg);
		}
		if (flags_doc1 & (1 << 2)) offset_msg += 4;
		break;
	}
	case 0xddf10c3b: {
		offset_msg += 4;
		int web_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		int flags_web = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		switch (web_cons) {
		case 0x211a1788:
			offset_msg += 8;
			if (flags_web & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		case 0xb0d13e47:
			offset_msg += 8;
			if (flags_web & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			break;
		case 0xe89c45b2: {
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
				for (int j = 0; j < count; j++) offset_msg += pageblock_offset(unenc_response + offset_msg);
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
					case 0x54b56617: {
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
					case 0x2e94c3e7: {
						int flags_webatt = read_le(unenc_response + offset_msg, 4);
						offset_msg += 20;
						if (flags_webatt & (1 << 0)) offset_msg += story_offset(unenc_response + offset_msg);
						break;
					}
					case 0x50cc03d3: {
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
		case 0x7311ca11: {
			int flags_web = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_web & (1 << 0)) offset_msg += 4;
			break;
		}
		}
		break;
	}
	case 0x2ec0533f: {
		offset_msg += geo_offset(unenc_response + offset_msg);
		for (int i = 0; i < 5; i++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case 0xfdb19008: {
		offset_msg += 4;
		int flags_game = read_le(unenc_response + offset_msg, 4);
		offset_msg += 20;
		for (int i = 0; i < 3; i++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		offset_msg += photo_offset(unenc_response + offset_msg);
		if (flags_game & (1 << 0)) offset_msg += doc_offset(unenc_response + offset_msg);
		break;
	}
	case 0xf6a548d3: {
		int flags_invoice = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		for (int i = 0; i < 2; i++) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_invoice & (1 << 0)) {
			int webdoc_cons = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += (webdoc_cons == 0x1c570ed1) ? 12 : 4;
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
	case 0xb940c666: {
		int flags_geolive = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		offset_msg += geo_offset(unenc_response + offset_msg);
		if (flags_geolive & (1 << 0)) offset_msg += 4;
		offset_msg += 4;
		if (flags_geolive & (1 << 1)) offset_msg += 4;
		break;
	}
	case 0x4bd6e798: {
		offset_msg += 12;
		int flags_poll = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		offset_msg += textwithent_offset(unenc_response + offset_msg);
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			offset_msg += 4;
			offset_msg += textwithent_offset(unenc_response + offset_msg);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
		}
		if (flags_poll & (1 << 4)) offset_msg += 4;
		if (flags_poll & (1 << 5)) offset_msg += 4;
		offset_msg += 4;
		int flags_pollres = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_pollres & (1 << 1)) {
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) {
				offset_msg += 8;
				offset_msg += tlstr_len(unenc_response + offset_msg, true);
				offset_msg += 4;
			}
		}
		if (flags_pollres & (1 << 2)) offset_msg += 4;
		if (flags_pollres & (1 << 3)) {
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += 12;
		}
		if (flags_pollres & (1 << 4)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		if (flags_pollres & (1 << 4)) {
			count = read_le(unenc_response + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += msgent_offset(unenc_response + offset_msg, NULL);
		}
		break;
	}
	case 0x3f7ee58b:
		offset_msg += 4;
		offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	case 0x68cb6283: {
		int flags_story = read_le(unenc_response + offset_msg, 4);
		offset_msg += 20;
		if (flags_story & (1 << 0)) offset_msg += story_offset(unenc_response + offset_msg);
		break;
	}
	case 0xaa073beb: {
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
	case 0xceaa3ea1: {
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
	case 0xa8852491: {
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
	case 0x5f3b8a00:
		if (flags_rpt & (1 << 0)) offset_msg += 4;
		if (flags_rpt & (1 << 1)) offset_msg += 4;
		break;
	case 0xc9f06e1b:
		if (flags_rpt & (1 << 3)) offset_msg += 4;
		if (flags_rpt & (1 << 4)) offset_msg += 4;
		if (flags_rpt & (1 << 1)) offset_msg += 8;
		if (flags_rpt & (1 << 2)) offset_msg += 8;
		break;
	case 0x339bef6c:
		if (flags_rpt & (1 << 3)) offset_msg += 4;
		if (flags_rpt & (1 << 1)) offset_msg += 8;
		if (flags_rpt & (1 << 2)) offset_msg += 8;
		break;
	}
	return offset_msg;
}

int keybutrow_offset(BYTE* unenc_response) {
	int offset_msg = 4;
	int count = read_le(unenc_response + offset_msg + 4, 4);
	offset_msg += 8;
	for (int i = 0; i < count; i++) {
		int keybut_cons = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		switch (keybut_cons) {
		case 0xa2fa4880:
		case 0xb16a6c29:
		case 0xfc796b3f:
		case 0x50f41ccf:
		case 0xafd93fbb:
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		case 0x35bbdb6b:
			offset_msg += 4;
		case 0x258aff05:
		case 0x13767230:
		case 0xa0c0505c:
		case 0x75d2698e:
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		case 0x93b9fbb5: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_keybut & (1 << 1)) {
				int count2 = read_le(unenc_response + offset_msg + 4, 4);
				offset_msg += 8;
				for (int j = 0; j < count; j++) offset_msg += 4;
			}
			break;
		}
		case 0x10b78d29: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_keybut & (1 << 0)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			break;	 
		}
		case 0xd02e7fd4: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			if (flags_keybut & (1 << 1)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += inputuser_offset(unenc_response + offset_msg);
			break;	 		 
		}
		case 0xbbc7515d: {
			int flags_keybut = read_le(unenc_response + offset_msg, 4);
			offset_msg += 4;
			if (flags_keybut & (1 << 0)) offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			break;
		}
		case 0xe988037b: 
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += inputuser_offset(unenc_response + offset_msg);
			break;
		case 0x308660c1:
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 8;
			break;
		case 0x53d7bfd8:
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			offset_msg += requestpeertype_offset(unenc_response + offset_msg);
			offset_msg += 4;
			break;
		case 0xc9662d05:
			offset_msg += 4;
			offset_msg += tlstr_len(unenc_response + offset_msg, true);
			offset_msg += 4;
			offset_msg += requestpeertype_offset(unenc_response + offset_msg);
			offset_msg += 4;
			break;
		}
	}
	return offset_msg;
}

int replymarkup_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int reply_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	switch (reply_cons) {
	case 0xa03e5b85:
		offset_msg += 4;
		break;
	case 0x86b40b08: {
		int flags_reply = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		if (flags_reply & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case 0x85dd99d1: {
		int flags_reply = read_le(unenc_response + offset_msg, 4);
		offset_msg += 4;
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += keybutrow_offset(unenc_response + offset_msg);
		if (flags_reply & (1 << 3)) offset_msg += tlstr_len(unenc_response + offset_msg, true);
		break;
	}
	case 0x48a30254: {
		int count = read_le(unenc_response + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += keybutrow_offset(unenc_response + offset_msg);
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
	for (int i = 0; i < count; i++) {
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
	if (read_le(message + offset_msg - 4, 4) == 0x64199744) return offset_msg;
	offset_msg += 32;
	for (int j = 0; j < 2; j++) offset_msg += tlstr_len(message + offset_msg, true);
	return offset_msg;
}

int wallpaper_offset(BYTE* unenc_response) {
	int offset_msg = 0;
	int wallpaper_cons = read_le(unenc_response + offset_msg, 4);
	offset_msg += 4;
	if (wallpaper_cons == 0xa437c3ed) {
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
	if (stargift_cons == 0x2cc73c8) {
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
			if (stargiftatt_cons == 0x39d99013 || stargiftatt_cons == 0x13acff19) {
				offset_msg += tlstr_len(message + offset_msg, true);
				offset_msg += doc_offset(message + offset_msg);
				offset_msg += 4;
			} else if (stargiftatt_cons == 0x94271762) {
				offset_msg += tlstr_len(message + offset_msg, true);
				offset_msg += 20;
			} else if (stargiftatt_cons == 0xc02c4f4b) {
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

int msgact_offset(BYTE* message, int offset_msg, wchar_t** service_msg, bool* service_msg_allocated) {
	bool message_adding = (service_msg == NULL) ? false : true;
	int msgact_cons = read_le(message + offset_msg, 4);
	offset_msg += 4;
	switch (msgact_cons) {
	case 0xbd47cbad:
	case 0xb5a1ce5a: {
		if (message_adding) {
			wchar_t* chat_name = read_string(message + offset_msg, NULL);
			wchar_t* info = (msgact_cons == 0xbd47cbad) ? L"This group was created with the name" : L"This group's name was changed to";
			int length = wcslen(chat_name) + wcslen(info) + 10;
			*service_msg = (wchar_t*)malloc(2*length);
			*service_msg_allocated = true;
			swprintf(*service_msg, L"%s \"%s\" by %%s", info, chat_name);
			free(chat_name);
		}
		offset_msg += tlstr_len(message + offset_msg, true);
		if (msgact_cons == 0xbd47cbad) {
			int count = read_le(message + offset_msg + 4, 4);
			offset_msg += 8;
			for (int i = 0; i < count; i++) offset_msg += 8;
		}
		break;
	}
	case 0x95d2ac92: {
		if (message_adding) {
			wchar_t* chat_name = read_string(message + offset_msg, NULL);
			wchar_t* info = L"This channel was created with the name";
			int length = wcslen(chat_name) + wcslen(info) + 4;
			*service_msg = (wchar_t*)malloc(2*length);
			*service_msg_allocated = true;
			swprintf(*service_msg, L"%s \"%s\"", info, chat_name);
			free(chat_name);
		}
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case 0x7fcb13a8:
		if (message_adding) *service_msg = L"The chat's photo was changed by %s";
		offset_msg += photo_offset(message + offset_msg);
		break;
	case 0x95e3fbef:
		if (message_adding) *service_msg = L"The chat's photo was deleted by %s";
		break;
	case 0x15cefd00: {
		if (message_adding) {
			if (memcmp(message + 16, message + offset_msg + 8, 8) == 0) *service_msg = L"%s joined the chat";
			else {
				wchar_t* name = NULL;
				if (current_peer->type == 1) for (int i = 0; i < current_peer->chat_users->size(); i++) {
					if (memcmp(message + offset_msg + 8, current_peer->chat_users->at(i).id, 8) == 0) {
						name = current_peer->chat_users->at(i).name;
						break;
					}
				}
				bool name_allocated = false;
				if (!name) {
					char type = -1;
					BYTE* peer_bytes = find_peer(message + offset_msg + 16, message + offset_msg + 4, false, &type);
					peer_set_name(peer_bytes, &name, type);
					name_allocated = true;
				}
				wchar_t* base = L" was added by %s";
				*service_msg = (wchar_t*)malloc(2 * (wcslen(base) + wcslen(name) + 1));
				swprintf(*service_msg, L"%s%s", name, base);
				*service_msg_allocated = true;
				if (name_allocated) free(name);
			}
		}
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 8;
		break;
	}
	case 0xa43f30cc:
		if (message_adding) *service_msg = L"%s left the chat";
		offset_msg += 8;
		break;
	case 0x31224c3:
		if (message_adding) *service_msg = L"%s joined the chat by a link";
		offset_msg += 8;
		break;
	case 0xfae69f56:
		if (message_adding) {
			*service_msg = read_string(message + offset_msg, NULL);
			*service_msg_allocated = true;
		}
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case 0xb4c38cb5:
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case 0xe1037f92:
		if (message_adding) *service_msg = L"This chat was migrated to a supergroup by %s";
		offset_msg += 8;
		break;
	case 0xea3948e9:
		if (message_adding) *service_msg = L"This supergroup was migrated from a chat by %s";
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		break;
	case 0x94bd38ed:
		if (message_adding) *service_msg = L"A message was pinned";
		break;
	case 0x9fbab604:
		if (message_adding) *service_msg = L"The chat's history was deleted by %s";
		break;
	case 0x92a72876:
	case 0x87e2f155:
		offset_msg += 12;
		break;
	case 0xffa00ccc: {
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
	case 0xc624b16e: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 4)) offset_msg += 4;
		break;
	}
	case 0x80e11a7f: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 12;
		wchar_t* info;
		if (flags & (1 << 0)) {
			offset_msg += 4;
			if (message_adding) {
				if (flags & (1 << 2)) info = L"Video call";
				else info = L"Call";
			}
		} else if (message_adding) {
			if (flags & (1 << 2)) info = L"Ongoing video call";
			else info = L"Ongoing call";
		}
		if (message_adding) {
			if (flags & (1 << 1)) {
				wchar_t duration_str[10];
				int duration = read_le(message + offset_msg, 4);
				offset_msg += 4;
				int hours = duration / 3600;
				int minutes = (duration % 3600) / 60;
				int seconds = duration % 60;
				if (hours != 0) swprintf(duration_str, L"(%02d:%02d:%02d)", hours, minutes, seconds);
				else swprintf(duration_str, L"(%02d:%02d)", minutes, seconds);
				int length = wcslen(duration_str) + wcslen(info) + 2;
				*service_msg = (wchar_t*)malloc(2*length);
				*service_msg_allocated = true;
				swprintf(*service_msg, L"%s %s", info, duration_str);
			} else *service_msg = info;
		} else if (flags & (1 << 1)) offset_msg += 4;
		break;
	}
	case 0x4792929b:
		if (message_adding) *service_msg = L"A screenshot was taken by %s";
		break;
	case 0xc516d679: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 2)) {
			int bot_cons = read_le(message + offset_msg, 4);
			offset_msg += 4;
			if (bot_cons == 0x5da674b7) break;
			int flags_bot = read_le(message + offset_msg, 4);
			offset_msg += 20;
			for (int i = 0; i < 3; i++) offset_msg += tlstr_len(message + offset_msg, true);
			offset_msg += photo_offset(message + offset_msg);
			if (flags_bot & (1 << 0)) offset_msg += doc_offset(message + offset_msg);
			offset_msg += 8;
		}
		break;
	}
	case 0x1b287353: {
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			offset_msg += 4;
			int flags = read_le(message + offset_msg, 4);
			offset_msg += 8;
			if (flags & (1 << 0)) {
				offset_msg += 4;
				for (int j = 0; j < 3; j++) offset_msg += tlstr_len(message + offset_msg, true);
			}
			for (int i = 1; i < 4; i++) if (flags & (1 << i)) offset_msg += securefile_offset(message + offset_msg);
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
	case 0xd95c6154: {
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 4;
		break;			 
	}
	case 0x98e0d697:
		offset_msg += 28;
		break;
	case 0x7a0d7f42: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 24;
		if (flags & (1 << 0)) offset_msg += 4;
		break;
	}
	case 0x502f92f7: {
		offset_msg += 20;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 8;
		break;
	}
	case 0x3c134d7b: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 8;
		if (flags & (1 << 0)) offset_msg += 8;
		break;
	}
	case 0xb3a07661:
		offset_msg += 24;
		break;
	case 0xaa786345:
		if (message_adding) {
			if (tlstr_len(message + offset_msg, false) == 0) *service_msg = L"The chat's theme was turned off by %s";
			else {
				wchar_t msg[45] = L"The chat's theme was changed by %s to ";
				int emoji_start = wcslen(msg);
				read_string(message + offset_msg, msg + emoji_start);
				wchar_t file_name[MAX_PATH];
				swprintf(file_name, L"%s\\", get_path(exe_path, L"emojis"));
				for (int j = emoji_start; j < wcslen(msg); j++) {
					int cr;
					if (msg[j] >= 0xD800 && msg[j] <= 0xDBFF) {
						cr = ((msg[j] - 0xD800) << 10) + (msg[j+1] - 0xDC00) + 0x10000;
						j++;
					} else cr = msg[j];
					if (file_name[7] == 0) swprintf(file_name, L"%s%x", file_name, cr);
					else swprintf(file_name, L"%s-%x", file_name, cr);
				}
				wcscat(file_name, L".ico");
				FILE* f = _wfopen(file_name, L"rb");
				if (!f) {
					wcscpy(file_name + wcslen(file_name) - 4, L"-fe0f.ico");
					f = _wfopen(file_name, L"rb");
					if (f) {
						msg[wcslen(msg) + 1] = 0;
						msg[wcslen(msg)] = 0xFE0F;
						fclose(f);
					}
				} else fclose(f);
				*service_msg = (wchar_t*)malloc(2*(wcslen(msg)+1));
				wcscpy(*service_msg, msg);
				*service_msg_allocated = true;
			}
		}
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case 0x47dd8079: {
		for (int i = 0; i < 2; i++) offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case 0x6c6274fa: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 12;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 0)) offset_msg += 8;
		if (flags & (1 << 1)) offset_msg += textwithent_offset(message + offset_msg);
		break;
	}
	case 0xd999256: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += 8;
		break;
	}
	case 0xc0944820: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 1)) offset_msg += 8;
		if (flags & (1 << 2)) offset_msg += 4;
		if (flags & (1 << 3)) offset_msg += 4;
		break;
	}
	case 0x57de635e:
		offset_msg += photo_offset(message + offset_msg);
		break;
	case 0x31518e9b: {
		offset_msg += 4;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) offset_msg += 12;
		break;
	}
	case 0x5060a3f4: {
		if (message_adding) *service_msg = L"The chat's wallpaper was set by %s";
		offset_msg += 4;
		offset_msg += wallpaper_offset(message + offset_msg);
		break;
	}
	case 0x56d03994: {
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
	case 0xa80f51e4 : {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		if (flags & (1 << 0)) offset_msg += 8;
		break;
	}
	case 0xcc02aa6d:
		offset_msg += 4;
		break;
	case 0x93b31848: {
		offset_msg += 4;
		int count = read_le(message + offset_msg + 4, 4);
		offset_msg += 8;
		for (int i = 0; i < count; i++) {
			int rpeer_cons = read_le(message + offset_msg, 4);
			offset_msg += 4;
			int flags = read_le(message + offset_msg, 4);
			offset_msg += 12;
			if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
			if (rpeer_cons == 0xd62ff46a) {
				if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
				if (flags & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
			} else if (rpeer_cons == 0x8ba403e4) {
				if (flags & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
			}
			if (flags & (1 << 2)) offset_msg += photo_offset(message + offset_msg);
		}
		break;
	}
	case 0x41b3e202: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 16;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 8;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 4;
		for (int i = 0; i < 2; i++) offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case 0x45d5b021: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 4;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 16;
		if (flags & (1 << 0)) offset_msg += tlstr_len(message + offset_msg, true);
		if (flags & (1 << 0)) offset_msg += 8;
		if (flags & (1 << 1)) offset_msg += tlstr_len(message + offset_msg, true);
		break;
	}
	case 0xb00c47a2:
		offset_msg += 12;
		offset_msg += tlstr_len(message + offset_msg, true);
		offset_msg += 16;
		break;
	case 0xd8f4f0a7: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 8;
		offset_msg += stargift_offset(message + offset_msg);
		if (flags & (1 << 1)) offset_msg += textwithent_offset(message + offset_msg);
		if (flags & (1 << 4)) offset_msg += 8;
		if (flags & (1 << 5)) offset_msg += 4;
		if (flags & (1 << 8)) offset_msg += 8;
		break;
	}
	case 0x26077b99: {
		int flags = read_le(message + offset_msg, 4);
		offset_msg += 8;
		offset_msg += stargift_offset(message + offset_msg);
		if (flags & (1 << 3)) offset_msg += 4;
		if (flags & (1 << 4)) offset_msg += 8;
		break;
	}
	}
	if (message_adding && *service_msg == (wchar_t*)1) *service_msg = L"This service message isn't supported in Telegacy"; 
	return offset_msg;
}

int msgrpl_offset(BYTE* message) {
	int offset_msg = 0;
	if (read_le(message + offset_msg, 4) == 0xe5af939) offset_msg += 20;
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
			for (int j = 0; j < count; j++) offset_msg += msgent_offset(message + offset_msg, false);
		}
		if (flags_msgrpl & (1 << 10)) offset_msg += 4;
	}
	return offset_msg;
}

int sendmsgaction_offset(BYTE* message) {
	int offset_msg = 4;
	int cons = read_le(message, 4);
	switch (cons) {
	case 0xe9763aec:
	case 0xf351d7ab:
	case 0xd1d34a26:
	case 0x243e1c66:
	case 0xdbda9246:
		offset_msg += 4;
		break;
	case 0xb665902e:
		offset_msg += tlstr_len(message + offset_msg, true);
		break;
	case 0x25972bcb:
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
				if (notsound_cons == 0x830b9ae4) {
					offset += tlstr_len(unenc_response + offset, true);
					offset += tlstr_len(unenc_response + offset, true);
				} else if (notsound_cons == 0xff6c8049) offset += 8;
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
		if (botmenu_cons == 0xc7b57ce6) {
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
	if (exchatinv_cons == 0xa22cbd96) {
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
	if (reaction_cons == 0x661d4037) {
		int count = read_le(unenc_response + offset + 4, 4);
		offset += 8;
		if (peer) peer->reaction_list = new std::vector<wchar_t*>();
		for (int k = 0; k < count; k++) {
			int reactiontype_cons = read_le(unenc_response + offset, 4);
			offset += 4;
			if (reactiontype_cons == 0x1b2286b8) {
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
			} else if (reactiontype_cons == 0x8935fc73) {
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
	} else if (reaction_cons == 0xeafc32bc && peer) peer->reaction_list = NULL;
	else offset += 4;
	return offset;
}