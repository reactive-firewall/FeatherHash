/*
 make0.c - Nullforge

 Public Domain

 */
/* sha256.c - FeatherHash SHA-256 implementation (public-domain) */
#include "sha256.h"
#include <string.h>
#include <stdio.h>

static const uint32_t k[64] = {
	0x428a2f98ul,0x71374491ul,0xb5c0fbcful,0xe9b5dba5ul,0x3956c25bul,0x59f111f1ul,0x923f82a4ul,0xab1c5ed5ul,
	0xd807aa98ul,0x12835b01ul,0x243185beul,0x550c7dc3ul,0x72be5d74ul,0x80deb1feul,0x9bdc06a7ul,0xc19bf174ul,
	0xe49b69c1ul,0xefbe4786ul,0x0fc19dc6ul,0x240ca1ccul,0x2de92c6ful,0x4a7484aaul,0x5cb0a9dcul,0x76f988daul,
	0x983e5152ul,0xa831c66dul,0xb00327c8ul,0xbf597fc7ul,0xc6e00bf3ul,0xd5a79147ul,0x06ca6351ul,0x14292967ul,
	0x27b70a85ul,0x2e1b2138ul,0x4d2c6dfcul,0x53380d13ul,0x650a7354ul,0x766a0abbul,0x81c2c92eul,0x92722c85ul,
	0xa2bfe8a1ul,0xa81a664bul,0xc24b8b70ul,0xc76c51a3ul,0xd192e819ul,0xd6990624ul,0xf40e3585ul,0x106aa070ul,
	0x19a4c116ul,0x1e376c08ul,0x2748774cul,0x34b0bcb5ul,0x391c0cb3ul,0x4ed8aa4aul,0x5b9cca4ful,0x682e6ff3ul,
	0x748f82eeul,0x78a5636ful,0x84c87814ul,0x8cc70208ul,0x90befffaul,0xa4506cebul,0xbef9a3f7ul,0xc67178f2ul
};

static uint32_t rotr32(uint32_t x, unsigned n) { return (x >> n) | (x << (32 - n)); }

static void sha256_transform(fh_sha256_ctx *ctx, const uint8_t data[])
{
	uint32_t w[64];
	unsigned i;
	for (i = 0; i < 16; ++i) {
		w[i] = ((uint32_t)data[i*4] << 24) | ((uint32_t)data[i*4+1] << 16) |
		((uint32_t)data[i*4+2] << 8) | ((uint32_t)data[i*4+3]);
	}
	for (i = 16; i < 64; ++i) {
		uint32_t s0 = rotr32(w[i-15], 7) ^ rotr32(w[i-15], 18) ^ (w[i-15] >> 3);
		uint32_t s1 = rotr32(w[i-2], 17) ^ rotr32(w[i-2], 19) ^ (w[i-2] >> 10);
		w[i] = w[i-16] + s0 + w[i-7] + s1;
	}

	uint32_t a = ctx->state[0];
	uint32_t b = ctx->state[1];
	uint32_t c = ctx->state[2];
	uint32_t d = ctx->state[3];
	uint32_t e = ctx->state[4];
	uint32_t f = ctx->state[5];
	uint32_t g = ctx->state[6];
	uint32_t h = ctx->state[7];

	for (i = 0; i < 64; ++i) {
		uint32_t S1 = rotr32(e,6) ^ rotr32(e,11) ^ rotr32(e,25);
		uint32_t ch = (e & f) ^ (~e & g);
		uint32_t temp1 = h + S1 + ch + k[i] + w[i];
		uint32_t S0 = rotr32(a,2) ^ rotr32(a,13) ^ rotr32(a,22);
		uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
		uint32_t temp2 = S0 + maj;

		h = g;
		g = f;
		f = e;
		e = d + temp1;
		d = c;
		c = b;
		b = a;
		a = temp1 + temp2;
	}

	ctx->state[0] += a;
	ctx->state[1] += b;
	ctx->state[2] += c;
	ctx->state[3] += d;
	ctx->state[4] += e;
	ctx->state[5] += f;
	ctx->state[6] += g;
	ctx->state[7] += h;
}

void fh_sha256_init(fh_sha256_ctx *ctx)
{
	ctx->datalen = 0;
	ctx->bitlen = 0;
	ctx->state[0] = 0x6a09e667ul;
	ctx->state[1] = 0xbb67ae85ul;
	ctx->state[2] = 0x3c6ef372ul;
	ctx->state[3] = 0xa54ff53aul;
	ctx->state[4] = 0x510e527ful;
	ctx->state[5] = 0x9b05688cul;
	ctx->state[6] = 0x1f83d9abul;
	ctx->state[7] = 0x5be0cd19ul;
}

void fh_sha256_update(fh_sha256_ctx *ctx, const uint8_t *data, size_t len)
{
	unsigned i;
	for (i = 0; i < len; ++i) {
		ctx->data[ctx->datalen++] = data[i];
		if (ctx->datalen == 64) {
			sha256_transform(ctx, ctx->data);
			ctx->bitlen += 512;
			ctx->datalen = 0;
		}
	}
}

void fh_sha256_final(fh_sha256_ctx *ctx, uint8_t hash[32])
{
	unsigned i;
	uint64_t bitlen_be;
	i = ctx->datalen++;

	if (ctx->datalen > 56) {
		ctx->data[ctx->datalen - 1] = 0x80;
		while (ctx->datalen < 64) ctx->data[ctx->datalen++] = 0x00;
		sha256_transform(ctx, ctx->data);
		ctx->datalen = 0;
	} else {
		ctx->data[ctx->datalen - 1] = 0x80;
	}
	while (ctx->datalen < 56) ctx->data[ctx->datalen++] = 0x00;

	ctx->bitlen += (uint64_t)(i - 1) * 8;
	bitlen_be = ctx->bitlen;
	ctx->data[56] = (uint8_t)(bitlen_be >> 56);
	ctx->data[57] = (uint8_t)(bitlen_be >> 48);
	ctx->data[58] = (uint8_t)(bitlen_be >> 40);
	ctx->data[59] = (uint8_t)(bitlen_be >> 32);
	ctx->data[60] = (uint8_t)(bitlen_be >> 24);
	ctx->data[61] = (uint8_t)(bitlen_be >> 16);
	ctx->data[62] = (uint8_t)(bitlen_be >> 8);
	ctx->data[63] = (uint8_t)(bitlen_be);
	sha256_transform(ctx, ctx->data);

	for (i = 0; i < 8; ++i) {
		hash[i*4]     = (uint8_t)(ctx->state[i] >> 24);
		hash[i*4 + 1] = (uint8_t)(ctx->state[i] >> 16);
		hash[i*4 + 2] = (uint8_t)(ctx->state[i] >> 8);
		hash[i*4 + 3] = (uint8_t)(ctx->state[i]);
	}
}
