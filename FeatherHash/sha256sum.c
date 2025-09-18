/* CC0 1.0 Universal - sha256sum.c

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 Minimal sha256sum command-line utility. */
#include "sha2.h"
#include "feather.h"

static int hash_file_sha256(const char *path, unsigned char out[32]) {
	sha256_ctx ctx;
	sha256_init(&ctx);
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
		sha256_update(&ctx, buf, r);
	}
	if (!using_stdin && f) fclose(f);
	if (ferror(stdin) && using_stdin) return 2;
	sha256_final(&ctx, out);
	return 0;
}

static void print_hex(const unsigned char *d, size_t len) {
	for (size_t i = 0; i < len; ++i) printf("%02x", d[i]);
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		unsigned char out[32];
		if (hash_file_sha256("-", out) != 0) return 1;
		print_hex(out, 32);
		printf("  -\n");
		return 0;
	}
	int exitcode = 0;
	for (int i = 1; i < argc; ++i) {
		unsigned char out[32];
		int r = hash_file_sha256(argv[i], out);
		if (r != 0) {
			fprintf(stderr, "sha256sum: %s: cannot open/read\n", argv[i]);
			exitcode = 2;
			continue;
		}
		print_hex(out, 32);
		printf("  %s\n", argv[i]);
	}
	return exitcode;
}
