/* CC0 1.0 Universal - sha2.c
 Minimal SHA-256 and SHA-512 implementations per FIPS 180-4.
 No dynamic allocation. Portable C (C11/C14/C23). */
#include "sha2.h"

#if defined(__has_include)

#if __has_include(<string.h>)
#include <string.h> /* memcpy, memset */
#define HAVE_STRING_H 1
#endif /* !__has_include(<string.h>) */

#else /* no __has_include: include and assume availability */

#ifndef HAVE_STRING_H
#include <string.h>
#define HAVE_STRING_H 1
#endif /* !HAVE_STRING_H */

#endif /* !defined(__has_include) */

/* --- Utility macros --- */
static inline uint32_t rotr32(uint32_t x, unsigned n) {
	return (x >> n) | (x << (32 - n));
}
static inline uint64_t rotr64(uint64_t x, unsigned n) {
	return (x >> n) | (x << (64 - n));
}

/* --- SHA-256 implementation --- */
static const uint32_t K256[64] = {
	0x428a2f98u,0x71374491u,0xb5c0fbcfu,0xe9b5dba5u,0x3956c25bu,0x59f111f1u,0x923f82a4u,0xab1c5ed5u,
	0xd807aa98u,0x12835b01u,0x243185beu,0x550c7dc3u,0x72be5d74u,0x80deb1feu,0x9bdc06a7u,0xc19bf174u,
	0xe49b69c1u,0xefbe4786u,0x0fc19dc6u,0x240ca1ccu,0x2de92c6fu,0x4a7484aau,0x5cb0a9dcu,0x76f988dau,
	0x983e5152u,0xa831c66du,0xb00327c8u,0xbf597fc7u,0xc6e00bf3u,0xd5a79147u,0x06ca6351u,0x14292967u,
	0x27b70a85u,0x2e1b2138u,0x4d2c6dfcu,0x53380d13u,0x650a7354u,0x766a0abbu,0x81c2c92eu,0x92722c85u,
	0xa2bfe8a1u,0xa81a664bu,0xc24b8b70u,0xc76c51a3u,0xd192e819u,0xd6990624u,0xf40e3585u,0x106aa070u,
	0x19a4c116u,0x1e376c08u,0x2748774cu,0x34b0bcb5u,0x391c0cb3u,0x4ed8aa4au,0x5b9cca4fu,0x682e6ff3u,
	0x748f82eeu,0x78a5636fu,0x84c87814u,0x8cc70208u,0x90befffau,0xa4506cebu,0xbef9a3f7u,0xc67178f2u
};

void sha256_init(sha256_ctx *c) {
	c->state[0] = 0x6a09e667u;
	c->state[1] = 0xbb67ae85u;
	c->state[2] = 0x3c6ef372u;
	c->state[3] = 0xa54ff53au;
	c->state[4] = 0x510e527fu;
	c->state[5] = 0x9b05688cu;
	c->state[6] = 0x1f83d9abu;
	c->state[7] = 0x5be0cd19u;
	c->bitlen = 0;
	c->buflen = 0;
}

static void sha256_transform(uint32_t state[8], const uint8_t block[64]) {
	uint32_t w[64];
	for (int t = 0; t < 16; ++t) {
		w[t] = ((uint32_t)block[t*4] << 24) |
		((uint32_t)block[t*4 + 1] << 16) |
		((uint32_t)block[t*4 + 2] << 8) |
		((uint32_t)block[t*4 + 3]);
	}
	for (int t = 16; t < 64; ++t) {
		uint32_t s0 = rotr32(w[t-15], 7) ^ rotr32(w[t-15], 18) ^ (w[t-15] >> 3);
		uint32_t s1 = rotr32(w[t-2], 17) ^ rotr32(w[t-2], 19) ^ (w[t-2] >> 10);
		w[t] = w[t-16] + s0 + w[t-7] + s1;
	}
	uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
	uint32_t e = state[4], f = state[5], g = state[6], h = state[7];
	for (int t = 0; t < 64; ++t) {
		uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
		uint32_t ch = (e & f) ^ ((~e) & g);
		uint32_t temp1 = h + S1 + ch + K256[t] + w[t];
		uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
		uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
		uint32_t temp2 = S0 + maj;
		h = g; g = f; f = e; e = d + temp1;
		d = c; c = b; b = a; a = temp1 + temp2;
	}
	state[0] += a; state[1] += b; state[2] += c; state[3] += d;
	state[4] += e; state[5] += f; state[6] += g; state[7] += h;
}

void sha256_update(sha256_ctx *c, const void *data, size_t len) {
	const uint8_t *p = (const uint8_t*)data;
	c->bitlen += (uint64_t)len * 8;
	while (len > 0) {
		size_t take = (64 - c->buflen) < len ? (64 - c->buflen) : len;
		memcpy(c->buf + c->buflen, p, take);
		c->buflen += take;
		p += take;
		len -= take;
		if (c->buflen == 64) {
			sha256_transform(c->state, c->buf);
			c->buflen = 0;
		}
	}
}

void sha256_final(sha256_ctx *c, uint8_t out[32]) {
	size_t i = c->buflen;
	c->buf[i++] = 0x80u;
	if (i > 56) {
		while (i < 64) c->buf[i++] = 0;
		sha256_transform(c->state, c->buf);
		i = 0;
	}
	while (i < 56) c->buf[i++] = 0;
	/* append length in bits big-endian */
	uint64_t bitlen_be = c->bitlen;
	for (int j = 0; j < 8; ++j) {
		c->buf[63 - j] = (uint8_t)(bitlen_be & 0xFFu);
		bitlen_be >>= 8;
	}
	sha256_transform(c->state, c->buf);
	for (int t = 0; t < 8; ++t) {
		out[t*4 + 0] = (uint8_t)(c->state[t] >> 24);
		out[t*4 + 1] = (uint8_t)(c->state[t] >> 16);
		out[t*4 + 2] = (uint8_t)(c->state[t] >> 8);
		out[t*4 + 3] = (uint8_t)(c->state[t]);
	}
	/* zero sensitive state */
	memset(c, 0, sizeof(*c));
}

/* --- SHA-512 (core used for SHA-512 and SHA-384) --- */
static const uint64_t K512[80] = {
	0x428a2f98d728ae22ULL,0x7137449123ef65cdULL,0xb5c0fbcfec4d3b2fULL,0xe9b5dba58189dbbcULL,
	0x3956c25bf348b538ULL,0x59f111f1b605d019ULL,0x923f82a4af194f9bULL,0xab1c5ed5da6d8118ULL,
	0xd807aa98a3030242ULL,0x12835b0145706fbeULL,0x243185be4ee4b28cULL,0x550c7dc3d5ffb4e2ULL,
	0x72be5d74f27b896fULL,0x80deb1fe3b1696b1ULL,0x9bdc06a725c71235ULL,0xc19bf174cf692694ULL,
	0xe49b69c19ef14ad2ULL,0xefbe4786384f25e3ULL,0x0fc19dc68b8cd5b5ULL,0x240ca1cc77ac9c65ULL,
	0x2de92c6f592b0275ULL,0x4a7484aa6ea6e483ULL,0x5cb0a9dcbd41fbd4ULL,0x76f988da831153b5ULL,
	0x983e5152ee66dfabULL,0xa831c66d2db43210ULL,0xb00327c898fb213fULL,0xbf597fc7beef0ee4ULL,
	0xc6e00bf33da88fc2ULL,0xd5a79147930aa725ULL,0x06ca6351e003826fULL,0x142929670a0e6e70ULL,
	0x27b70a8546d22ffcULL,0x2e1b21385c26c926ULL,0x4d2c6dfc5ac42aedULL,0x53380d139d95b3dfULL,
	0x650a73548baf63deULL,0x766a0abb3c77b2a8ULL,0x81c2c92e47edaee6ULL,0x92722c851482353bULL,
	0xa2bfe8a14cf10364ULL,0xa81a664bbc423001ULL,0xc24b8b70d0f89791ULL,0xc76c51a30654be30ULL,
	0xd192e819d6ef5218ULL,0xd69906245565a910ULL,0xf40e35855771202aULL,0x106aa07032bbd1b8ULL,
	0x19a4c116b8d2d0c8ULL,0x1e376c085141ab53ULL,0x2748774cdf8eeb99ULL,0x34b0bcb5e19b48a8ULL,
	0x391c0cb3c5c95a63ULL,0x4ed8aa4ae3418acbULL,0x5b9cca4f7763e373ULL,0x682e6ff3d6b2b8a3ULL,
	0x748f82ee5defb2fcULL,0x78a5636f43172f60ULL,0x84c87814a1f0ab72ULL,0x8cc702081a6439ecULL,
	0x90befffa23631e28ULL,0xa4506cebde82bde9ULL,0xbef9a3f7b2c67915ULL,0xc67178f2e372532bULL,
	0xca273eceea26619cULL,0xd186b8c721c0c207ULL,0xeada7dd6cde0eb1eULL,0xf57d4f7fee6ed178ULL,
	0x06f067aa72176fbaULL,0x0a637dc5a2c898a6ULL,0x113f9804bef90daeULL,0x1b710b35131c471bULL,
	0x28db77f523047d84ULL,0x32caab7b40c72493ULL,0x3c9ebe0a15c9bebcULL,0x431d67c49c100d4cULL,
	0x4cc5d4becb3e42b6ULL,0x597f299cfc657e2aULL,0x5fcb6fab3ad6faecULL,0x6c44198c4a475817ULL
};

void sha512_init(sha512_ctx *c, const uint64_t iv[8]) {
	for (int i = 0; i < 8; ++i) c->state[i] = iv[i];
	c->bitlen_high = 0;
	c->bitlen_low = 0;
	c->buflen = 0;
}

static void sha512_transform(uint64_t state[8], const uint8_t block[128]) {
	uint64_t w[80];
	for (int t = 0; t < 16; ++t) {
		uint64_t hi = ((uint64_t)block[t*8] << 56) | ((uint64_t)block[t*8+1] << 48) |
		((uint64_t)block[t*8+2] << 40) | ((uint64_t)block[t*8+3] << 32);
		uint64_t lo = ((uint64_t)block[t*8+4] << 24) | ((uint64_t)block[t*8+5] << 16) |
		((uint64_t)block[t*8+6] << 8) | ((uint64_t)block[t*8+7]);
		w[t] = (hi << 32) | lo;
	}
	for (int t = 16; t < 80; ++t) {
		uint64_t s0 = rotr64(w[t-15], 1) ^ rotr64(w[t-15], 8) ^ (w[t-15] >> 7);
		uint64_t s1 = rotr64(w[t-2], 19) ^ rotr64(w[t-2], 61) ^ (w[t-2] >> 6);
		w[t] = w[t-16] + s0 + w[t-7] + s1;
	}
	uint64_t a = state[0], b = state[1], c2 = state[2], d = state[3];
	uint64_t e = state[4], f = state[5], g = state[6], h = state[7];
	for (int t = 0; t < 80; ++t) {
		uint64_t S1 = rotr64(e, 14) ^ rotr64(e, 18) ^ rotr64(e, 41);
		uint64_t ch = (e & f) ^ ((~e) & g);
		uint64_t temp1 = h + S1 + ch + K512[t] + w[t];
		uint64_t S0 = rotr64(a, 28) ^ rotr64(a, 34) ^ rotr64(a, 39);
		uint64_t maj = (a & b) ^ (a & c2) ^ (b & c2);
		uint64_t temp2 = S0 + maj;
		h = g; g = f; f = e; e = d + temp1;
		d = c2; c2 = b; b = a; a = temp1 + temp2;
	}
	for (int i = 0; i < 8; ++i) state[i] += (uint64_t[]){a,b,c2,d,e,f,g,h}[i]; /* expand */
	/* Note: The inline array above is a concise expression; to avoid non-portable compound literals in C89,
	 we use it here because C11/C14/C23 allow it. It's also purely internal. */
}

/* Maintain 128-bit bit length via two 64-bit counters */
void sha512_update(sha512_ctx *c, const void *data, size_t len) {
	const uint8_t *p = (const uint8_t*)data;
	/* update 128-bit bit length */
	uint64_t add_bits = (uint64_t)len * 8;
	c->bitlen_low += add_bits;
	if (c->bitlen_low < add_bits) c->bitlen_high += 1;
	while (len > 0) {
		size_t take = (128 - c->buflen) < len ? (128 - c->buflen) : len;
		memcpy(c->buf + c->buflen, p, take);
		c->buflen += take;
		p += take;
		len -= take;
		if (c->buflen == 128) {
			sha512_transform(c->state, c->buf);
			c->buflen = 0;
		}
	}
}

void sha512_final(sha512_ctx *c, uint8_t out[64]) {
	size_t i = c->buflen;
	c->buf[i++] = 0x80u;
	if (i > 112) {
		while (i < 128) c->buf[i++] = 0;
		sha512_transform(c->state, c->buf);
		i = 0;
	}
	while (i < 112) c->buf[i++] = 0;
	/* append 128-bit length big-endian: high then low */
	uint64_t high = c->bitlen_high;
	uint64_t low = c->bitlen_low;
	for (int j = 0; j < 8; ++j) c->buf[15 - j] = (uint8_t)(low & 0xFFu), low >>= 8;
	for (int j = 0; j < 8; ++j) c->buf[23 - j] = (uint8_t)(high & 0xFFu), high >>= 8;
	sha512_transform(c->state, c->buf);
	for (int t = 0; t < 8; ++t) {
		uint64_t v = c->state[t];
		out[t*8 + 0] = (uint8_t)(v >> 56);
		out[t*8 + 1] = (uint8_t)(v >> 48);
		out[t*8 + 2] = (uint8_t)(v >> 40);
		out[t*8 + 3] = (uint8_t)(v >> 32);
		out[t*8 + 4] = (uint8_t)(v >> 24);
		out[t*8 + 5] = (uint8_t)(v >> 16);
		out[t*8 + 6] = (uint8_t)(v >> 8);
		out[t*8 + 7] = (uint8_t)(v);
	}
	memset(c, 0, sizeof(*c));
}
