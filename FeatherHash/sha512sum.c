/* CC0 1.0 Universal - sha512sum.c

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 Minimal sha512sum command-line utility. */
#include "sha2.h"
#include "feather.h"

static const uint64_t SHA512_IV[8] = {
	0x6a09e667f3bcc908ULL,0xbb67ae8584caa73bULL,0x3c6ef372fe94f82bULL,0xa54ff53a5f1d36f1ULL,
	0x510e527fade682d1ULL,0x9b05688c2b3e6c1fULL,0x1f83d9abfb41bd6bULL,0x5be0cd19137e2179ULL
};

static int hash_file_sha512(const char *path, unsigned char out[64]) {
	sha512_ctx ctx;
	sha512_init(&ctx, SHA512_IV);
	FILE *f = stdin;
	int using_stdin = 0;
	if (path && strcmp(path, "-") != 0) {
		f = fopen(path, "rb");
		if (!f) return 1;
	} else {
		using_stdin = 1;
	}
	unsigned char buf[8192];
	size_t r;
	while ((r = fread(buf, 1, sizeof(buf), f)) > 0) {
		sha512_update(&ctx, buf, r);
	}
	if (!using_stdin && f) fclose(f);
	if (ferror(stdin) && using_stdin) return 2;
	sha512_final(&ctx, out);
	return 0;
}

static void print_hex(const unsigned char *d, size_t len) {
	for (size_t i = 0; i < len; ++i) printf("%02x", d[i]);
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		unsigned char out[64];
		if (hash_file_sha512("-", out) != 0) return 1;
		print_hex(out, 64);
		printf("  -\n");
		return 0;
	}
	int exitcode = 0;
	for (int i = 1; i < argc; ++i) {
		unsigned char out[64];
		int r = hash_file_sha512(argv[i], out);
		if (r != 0) {
			fprintf(stderr, "sha512sum: %s: cannot open/read\n", argv[i]);
			exitcode = 2;
			continue;
		}
		print_hex(out, 64);
		printf("  %s\n", argv[i]);
	}
	return exitcode;
}
