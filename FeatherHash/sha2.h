/* CC0 1.0 Universal - sha2.h

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

 Minimal SHA-2 declarations for sha256/sha384/sha512.
*/
#ifndef FEATHERHASH_SHA2_H

/*!
 @header sha2.h
 @discussion
 Lightweight, portable SHA-256 implementation (API only). This component provides three primary functions:
 - sha256_init: initialize a context
 - sha256_update: feed bytes into the running hash
 - sha256_final: finalize and produce the 32-byte digest

 The implementation is intentionally compact and suitable for embedded or utility code. It does not allocate dynamic memory and assumes a little- or big-endian-independent byte-level input (the algorithm itself works with big-endian message words).

 The API operates on a caller-allocated sha256_ctx structure, defined as: ``sha256_ctx``.

 Thread safety:
 - Each sha256_ctx instance may be used by one thread at a time. The functions do not use global mutable state.

 Security notes:
 - The implementation zeroes the context in sha256_final to reduce lifetime of sensitive intermediate state.
 - The implementation is not hardened against side-channel attacks (timing, cache). Do not use for high-security requirements without appropriate hardening.

 Usage example:
 @code
 sha256_ctx ctx;
 uint8_t digest[32];
 const uint8_t *data = ...;
 size_t len = ...;

 sha256_init(&ctx);
 sha256_update(&ctx, data, len);
 sha256_final(&ctx, digest);
 @endcode
*/

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

#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark Utility Functions
#endif /* !__clang__ */

/**!
 Rotate-32 operation utility.

 - Discussion:  This version is an intuitive bitwise OR of right shifted bits and the left shifted 'overflow' bits (e.g., "carried over" if you will).
 There are certainly better versions out there than this lightweight implementation, but this version is intended for stage0 toolchains
 after all.

 - Parameter x: The ``uint32_t`` value to rotate right (32 bits).
 - Parameter n: The ``unsigned`` distance to rotate right by.
 - seealso: A more [general form](https://stackoverflow.com/a/776523) (that one is CC-BY-SA-4.0)
 - seealso: The two academic implementation of identical form (a cool coincidence)
 * By John Regehr (Publicly available academic implementation; unknown copyright)
 on [his blog](https://blog.regehr.org/archives/1063)
 * And by Brad Conte (public domain implementation)
 on [his github](https://github.com/B-Con/crypto-algorithms/blob/cfbde48414baacf51fc7c74f275190881f037d32/sha256.c#L22)
 - seealso: ``rotr64``
 - seealso: clang's built-in [builtin_rotateright32](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-rotateright)
 */
static inline uint32_t rotr32(uint32_t x, unsigned n);

/**!
 Rotate-64 operation utility.

 - Discussion:  This version is an intuitive bitwise OR of right shifted bits and the left shifted 'overflow' bits (e.g., "carried over" if you will).
 There are certainly better versions out there than this lightweight implementation, but this version is intended for stage0 toolchains
 after all.

 - Parameter x: The ``uint64_t`` value to rotate right (64 bits).
 - Parameter n: The ``unsigned`` distance to rotate right by.
 - seealso: A more [general form](https://stackoverflow.com/a/776523) (that one is CC-BY-SA-4.0)
 - seealso: ``rotr32``
 - seealso: clang's built-in [builtin_rotateright32](https://clang.llvm.org/docs/LanguageExtensions.html#builtin-rotateright)
 */
static inline uint64_t rotr64(uint64_t x, unsigned n);

#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark Secure Hash Algorithm Functions
#endif /* !__clang__ */

/* Context sizes:
 - SHA-256: block 64, state 8 x uint32_t
 - SHA-512/384: block 128, state 8 x uint64_t
 */

/* SHA-256 */
struct _sha256_ctx {
	uint32_t state[8]; /* internal state (A..H) */
	uint64_t bitlen;   /* total number of message bits processed */
	uint8_t buf[64];   /* partial-block buffer */
	size_t buflen;     /* number of bytes currently in buf */
};
typedef struct _sha256_ctx sha256_ctx;

/*!
 Initialize a ``sha256_ctx`` to begin hashing a new message.

 The function sets initial state, zeroes counters and buffer length. Must be
 called before ``sha256_update`` / ``sha256_final``.

 - Parameter c: Pointer to caller-allocated ``sha256_ctx``.
 */
void sha256_init(sha256_ctx *c);

/*!
 Process input bytes into the running SHA-256 computation.

 This function may be called multiple times to process streaming data.
 - The function updates the context's internal bit length and buffers partial blocks.
 - When a full 64-byte block is accumulated, it is processed immediately.
 - The caller remains responsible for supplying all message bytes; call ``sha256_final`` to produce the digest.

 - Parameter c: Pointer to an initialized ``sha256_ctx``.
 - Parameter data: Pointer to input bytes; may be ``NULL`` only when `len` is 0.
 - Parameter len: The ``size_t`` of bytes to process.
 */
void sha256_update(sha256_ctx *c, const void *data, size_t len);

/*!
 Finalize hashing and write the 32-byte (256-bit) digest in big-endian order to out.

 After this call, the context is zeroed and must be re-initialized with ``sha256_init`` before reuse.

 - Parameter c: Pointer to a ``sha256_ctx`` previously passed to ``sha256_init`` and ``sha256_update``.
 - Parameter out: Pointer to a 32-byte buffer (e.g., `uint8_t[32]`) that will receive the digest.
*/
void sha256_final(sha256_ctx *c, uint8_t out[32]);

/* --- Internal notes (for maintainers) ---
 - K256: round constants per FIPS-180-4.
 - sha256_transform: processes a single 512-bit block and updates 'state'.
 - The message schedule w[0..63] is computed in-place; SIG0/SIG1/EP0/EP1/CH/MAJ are provided as macros.
 - Endianness: input bytes are combined to big-endian 32-bit words in sha256_transform.
 - The implementation appends the 64-bit message length in big-endian as required by the spec.
 - Zeroing the sha256_ctx in sha256_final includes state, counters and buffer to limit exposure of intermediate values.
 - Keep the transform function static/internal to prevent inadvertent external use.
 */

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

/* --- SHA-256 helper macros --- */
#ifndef FEATHERHASH_SHA2_MACROS

#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark SHA2 Macros
#endif /* !__clang__ */

#if defined(HAS_TYPEOF) && HAS_TYPEOF
// ROT32 left and right helper macros
#define ROTLEFT(a, b) ({ __typeof__(a) __temp_a = (a); __typeof__(b) __temp_b = (b); (__typeof__(a))(rotl32(__temp_a, __temp_b)); })
#define ROTRIGHT(a, b) ({ __typeof__(a) __temp_a = (a); __typeof__(b) __temp_b = (b); (__typeof__(a))(rotr32(__temp_a, __temp_b)); })

// W to SIG?() with x,y,z helper macros
#define WTSIG(w, x, y, z) ({ __typeof__(w) __temp_w = (w); __typeof__(x) __temp_x = (x); __typeof__(y) __temp_y = (y); __typeof__(z) __temp_z = (z); (__typeof__(w))((ROTRIGHT(__temp_w, __temp_x) ^ ROTRIGHT(__temp_w, __temp_y)) ^ (__temp_w >> __temp_z)); })
// W to EP?() with x,y,z helper macros
#define WTEP(w, x, y, z) ({ __typeof__(w) __temp_w = (w); __typeof__(x) __temp_x = (x); __typeof__(y) __temp_y = (y); __typeof__(z) __temp_z = (z); (__typeof__(w))((ROTRIGHT(__temp_w, __temp_x) ^ ROTRIGHT(__temp_w, __temp_y)) ^ ROTRIGHT(__temp_w, __temp_z)); })
// SHA-2 EPx functions
#define EP0(n) ({ __typeof__(n) __temp_n = (n); (__typeof__(n))(WTEP(__temp_n, 2, 13, 22)); })
#define EP1(n) ({ __typeof__(n) __temp_n = (n); (__typeof__(n))(WTEP(__temp_n, 6, 11, 25)); })
// SHA-2 SIGx functions
#define SIG0(n) ({ __typeof__(n) __temp_n = (n); (__typeof__(n))(WTSIG(__temp_n, 7, 18, 3)); })
#define SIG1(n) ({ __typeof__(n) __temp_n = (n); (__typeof__(n))(WTSIG(__temp_n, 17, 19, 10)); })
// SHA-2 CH function
#define CH(x, y, z) ({ __typeof__(x) __temp_x = (x); __typeof__(y) __temp_y = (y); __typeof__(z) __temp_z = (z); ((__temp_x & __temp_y) ^ (~__temp_x & __temp_z)); })
// SHA-2 MAJ function
#define MAJ(x, y, z) ({ __typeof__(x) __temp_x = (x); __typeof__(y) __temp_y = (y); __typeof__(z) __temp_z = (z); ((__temp_x & __temp_y) ^ (__temp_x & __temp_z) ^ (__temp_y & __temp_z)); })
#else /* !HAS_TYPEOF */
// Fallback implementation without __typeof__
#define ROTLEFT(a, b) (rotl32((a), (b)))
#define ROTRIGHT(a, b) (rotr32((a), (b)))

// W to SIG?() with x,y,z helper macro
#define WTSIG(w, x, y, z) ((ROTRIGHT((w), (x)) ^ ROTRIGHT((w), (y))) ^ ((w) >> (z)))
// W to EP?() with x,y,z helper macros
#define WTEP(w, x, y, z) ((ROTRIGHT((w), (x)) ^ ROTRIGHT((w), (y))) ^ ROTRIGHT((w), (z)))
// SHA-2 EPx functions
#define EP0(n) (WTEP(n,7,18,3))
#define EP1(n) (WTEP(n,17,19,10))
// SHA-2 SIGx functions
#define SIG0(n) (WTSIG(n,7,18,3))
#define SIG1(n) (WTSIG(n,17,19,10))
// SHA-2 CH function
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
// SHA-2 MAJ function
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#endif /* !defined(HAS_TYPEOF) */

#define FEATHERHASH_SHA2_MACROS ""

#endif /* FEATHERHASH_SHA2_MACROS */

#endif /* FEATHERHASH_SHA2_H */
