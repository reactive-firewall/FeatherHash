/* CC0 1.0 Universal - sha384sum.c
   Minimal sha384sum command-line utility. Implements SHA-384 by using SHA-512 core
   with SHA-384 initial IV and truncating output to 48 bytes. */
#include "sha2.h"
#include "feather.h"

static const uint64_t SHA384_IV[8] = {
	0xcbbb9d5dc1059ed8ULL,0x629a292a367cd507ULL,0x9159015a3070dd17ULL,0x152fecd8f70e5939ULL,
	0x67332667ffc00b31ULL,0x8eb44a8768581511ULL,0xdb0c2e0d64f98fa7ULL,0x47b5481dbefa4fa4ULL
};

static int hash_file_sha384(const char *path, unsigned char out48[48]) {
	sha512_ctx ctx;
	sha512_init(&ctx, SHA384_IV);
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
	unsigned char out64[64];
	sha512_final(&ctx, out64);
	memcpy(out48, out64, 48);
	return 0;
}

static void print_hex(const unsigned char *d, size_t len) {
	for (size_t i = 0; i < len; ++i) printf("%02x", d[i]);
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		unsigned char out[48];
		if (hash_file_sha384("-", out) != 0) return 1;
		print_hex(out, 48);
		printf("  -\n");
		return 0;
	}
	int exitcode = 0;
	for (int i = 1; i < argc; ++i) {
		unsigned char out[48];
		int r = hash_file_sha384(argv[i], out);
		if (r != 0) {
			fprintf(stderr, "sha384sum: %s: cannot open/read\n", argv[i]);
			exitcode = 2;
			continue;
		}
		print_hex(out, 48);
		printf("  %s\n", argv[i]);
	}
	return exitcode;
}
