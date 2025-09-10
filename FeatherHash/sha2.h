/* CC0 1.0 Universal - sha2.h
 Public domain / CC0. Minimal SHA-2 declarations for sha256/sha384/sha512.
 */
#ifndef FEATHERHASH_SHA2_H
#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark sha2Header
// __typeof__ is supported
#define HAS_TYPEOF 1
#else /* !__clang__ */
#define HAS_TYPEOF 0
#endif /* !HAS_TYPEOF */
///Defined whenever ``sha2.h`` is imported.
#define FEATHERHASH_SHA2_H "sha2.h"

#if defined(__has_include)
#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark Imports
#endif /* !__clang__ */

#if __has_include(<stddef.h>)
#include <stddef.h>
#define HAVE_STDDEF_H 1
#endif /* !__has_include(<stddef.h>) */

#if __has_include(<stdint.h>)
#include <stdint.h>
#define HAVE_STDINT_H 1
#endif /* !__has_include(<stdint.h>) */

#else /* no __has_include: include and assume availability */

/* If any header macros are missing, try to include them unconditionally to
 produce a diagnostic early. This keeps behavior consistent for compilers
 without __has_include. */

#ifndef HAVE_STDDEF_H
#include <stddef.h>
#define HAVE_STDDEF_H 1
#endif /* !HAVE_STDDEF_H */

#ifndef HAVE_STDINT_H
#include <stdint.h>
#define HAVE_STDINT_H 1
#endif /* !HAVE_STDINT_H */

#endif /* !defined(__has_include) */

#ifdef __cplusplus
extern "C" {
#endif /* !defined(__cplusplus) */

/* Context sizes:
 - SHA-256: block 64, state 8 x uint32_t
 - SHA-512/384: block 128, state 8 x uint64_t
 */

/* SHA-256 */
struct _sha256_ctx {
	uint32_t state[8];
	uint64_t bitlen;
	uint8_t buf[64];
	size_t buflen;
};
typedef struct _sha256_ctx sha256_ctx;

void sha256_init(sha256_ctx *c);
void sha256_update(sha256_ctx *c, const void *data, size_t len);
void sha256_final(sha256_ctx *c, uint8_t out[32]);

/* SHA-512 core (used for SHA-512 and SHA-384) */
typedef struct {
	uint64_t state[8];
	uint64_t bitlen_high;
	uint64_t bitlen_low;
	uint8_t buf[128];
	size_t buflen;
} sha512_ctx;

void sha512_init(sha512_ctx *c, const uint64_t iv[8]);
void sha512_update(sha512_ctx *c, const void *data, size_t len);
void sha512_final(sha512_ctx *c, uint8_t out[64]);

#ifdef __cplusplus
}
#endif /* !defined(__cplusplus) */

#endif /* FEATHERHASH_SHA2_H */


#if defined(HAS_TYPEOF) && HAS_TYPEOF
#define ROTLEFT(a, b) ({ __typeof__(a) __temp_a = (a); __typeof__(b) __temp_b = (b); ((__temp_a << __temp_b) | (__temp_a >> ((sizeof(__temp_a) * 8) - __temp_b))); })
#define ROTRIGHT(a, b) ({ __typeof__(a) __temp_a = (a); __typeof__(b) __temp_b = (b); ((__temp_a >> __temp_b) | (__temp_a << ((sizeof(__temp_a) * 8) - __temp_b))); })

#define CH(x, y, z) ({ __typeof__(x) __temp_x = (x); __typeof__(y) __temp_y = (y); __typeof__(z) __temp_z = (z); ((__temp_x & __temp_y) ^ (~__temp_x & __temp_z)); })
#define MAJ(x, y, z) ({ __typeof__(x) __temp_x = (x); __typeof__(y) __temp_y = (y); __typeof__(z) __temp_z = (z); ((__temp_x & __temp_y) ^ (__temp_x & __temp_z) ^ (__temp_y & __temp_z)); })
#else /* !HAS_TYPEOF */
// Fallback implementation without __typeof__
#define ROTLEFT(a,b) (((a) << (b)) | ((a) >> ((32)-(b))))
#define ROTRIGHT(a,b) (((a) >> (b)) | ((a) << ((32)-(b))))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#endif /* !defined(HAS_TYPEOF) */

#define EP0(x) (ROTRIGHT(x,2) ^ ROTRIGHT(x,13) ^ ROTRIGHT(x,22))
#define EP1(x) (ROTRIGHT(x,6) ^ ROTRIGHT(x,11) ^ ROTRIGHT(x,25))
#define SIG0(x) (ROTRIGHT(x,7) ^ ROTRIGHT(x,18) ^ ((x) >> 3))
#define SIG1(x) (ROTRIGHT(x,17) ^ ROTRIGHT(x,19) ^ ((x) >> 10))
