/*
Copyright © 2026 N3xtery

This file is part of Telegacy.

Telegacy is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Telegacy is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Telegacy. If not, see <https://www.gnu.org/licenses/>. 
*/

// The code in this file is pretty much AI generated

#include <telegacy.h>

void aes_ige(const unsigned char *in, unsigned char *out, size_t length,
			 const unsigned char *key, const unsigned char *iv, int encrypt) {
    symmetric_key skey;
    unsigned char iv1[16], iv2[16], tmp[16];
    size_t i;

    if (length % 16 != 0) {
		MessageBox(NULL, L"AES IGE mismatch!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    memcpy(iv1, iv, 16);
    memcpy(iv2, iv + 16, 16);

    aes_setup(key, 32, 0, &skey);

    for (i = 0; i < length; i += 16) {
        if (encrypt) {
            for (int j = 0; j < 16; ++j)
                tmp[j] = in[i + j] ^ iv1[j];

            aes_ecb_encrypt(tmp, tmp, &skey);

            for (j = 0; j < 16; ++j)
                out[i + j] = tmp[j] ^ iv2[j];

            memcpy(iv1, out + i, 16);
            memcpy(iv2, in + i, 16);
        } else {
            for (int j = 0; j < 16; ++j)
                tmp[j] = in[i + j] ^ iv2[j];

            aes_ecb_decrypt(tmp, tmp, &skey);

            for (j = 0; j < 16; ++j)
                out[i + j] = tmp[j] ^ iv1[j];

            memcpy(iv2, out + i, 16);
            memcpy(iv1, in + i, 16);
        }
    }

    aes_done(&skey);
}

unsigned __int64 pq_gcd(unsigned __int64 a, unsigned __int64 b) {
    if (a == 0) return b;
    while ((a & 1) == 0) a >>= 1;
    while ((b & 1) == 0) b >>= 1;

    while (a != b) {
        if (a > b) {
            a = (a - b) >> 1;
            while ((a & 1) == 0) a >>= 1;
        } else {
            b = (b - a) >> 1;
            while ((b & 1) == 0) b >>= 1;
        }
    }
    return a;
}

unsigned __int64 pq_add_mul(unsigned __int64 c, unsigned __int64 a, unsigned __int64 b, unsigned __int64 pq) {
    while (b) {
        if (b & 1) {
            c += a;
            if (c >= pq) c -= pq;
        }
        a += a;
        if (a >= pq) a -= pq;
        b >>= 1;
    }
    return c;
}

unsigned __int64 pq_factorize(unsigned __int64 pq) {
    if (pq <= 2 || pq >> 63) return 1;
    if ((pq & 1) == 0) return 2;

    unsigned __int64 g = 0;
    int i, iter = 0;

    for (i = 0; i < 3 || iter < 1000; i++) {
        unsigned __int64 q = (rand() % 15 + 17) % (pq - 1);
        unsigned __int64 x = ((unsigned __int64)rand() << 32 | rand()) % (pq - 1) + 1;
        unsigned __int64 y = x;
        int lim = 1 << ((i < 5 ? i : 5) + 18);
        int j;

        for (j = 1; j < lim; j++) {
            iter++;
            x = pq_add_mul(q, x, x, pq);
            unsigned __int64 z = (x < y) ? pq + x - y : x - y;
            g = pq_gcd(z, pq);
            if (g != 1) break;
            if ((j & (j - 1)) == 0) y = x;
        }
        if (g > 1 && g < pq) break;
    }

    if (g != 0) {
        unsigned __int64 other = pq / g;
        if (other < g) g = other;
    }

    return g;
}
