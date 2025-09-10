/* CC0 1.0 Universal - feather.h

 Permission to use, copy, modify, and/or distribute this software for any
 purpose with or without fee is hereby granted.

 THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

*/
#ifndef FEATHERHASH_MAIN_H
#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark FeatherHashHeader
#endif /* !__clang__ */
///Defined whenever ``feather.h`` is imported.
#define FEATHERHASH_MAIN_H "feather.h"

#if defined(__has_include)
#if defined(__clang__) && __clang__
#pragma mark -
#pragma mark Imports
#endif /* !__clang__ */

#if defined(Compat_h)
/* compat.h from XCMB is apache-2.0 so may not always be available */
#if defined(XCMB_USE_STD_LOG)
#define HAVE_STDIO_H XCMB_USE_STD_LOG
#if defined(XCMB_USE_STD_LOG)
#define HAVE_STDLIB_H XCMB_USE_STD_LIB
#endif /* !defined(XCMB_USE_STD_LOG) */
#endif /* !defined(XCMB_USE_STD_LIB) */
#endif /* !defined(Compat_h) */

#if __has_include(<stdio.h>)
#ifndef HAVE_STDIO_H
#import <stdio.h>
#define HAVE_STDIO_H 1
#if __has_include(<stdlib.h>)
#ifndef HAVE_STDLIB_H
#import <stdlib.h>
#define HAVE_STDLIB_H 1
#else
#warning Use of stdio without stdlib is unusual. Compiler Gods Help You.
#define HAVE_STDLIB_H 0
#endif /* !HAVE_STDLIB_H */
#endif /* !__has_include(<stdlib.h>) */
#else
#define HAVE_STDIO_H 0
#define HAVE_STDLIB_H 0
#endif /* !HAVE_STDIO_H */
#endif /* !__has_include(<stdio.h>) */

#if __has_include(<string.h>)
#include <string.h>
#define HAVE_STRING_H 1
#endif /* !__has_include(<string.h>) */

#else /* no __has_include: include and assume availability */

/* If any header macros are missing, try to include them unconditionally to
 produce a diagnostic early. This keeps behavior consistent for compilers
 without __has_include. */
#ifndef HAVE_STDIO_H
#include <stdio.h>
#define HAVE_STDIO_H 1
#endif /* !HAVE_STDIO_H */

#ifndef HAVE_STDLIB_H
#include <stdlib.h>
#define HAVE_STDLIB_H 1
#endif /* !HAVE_STDLIB_H */

#ifndef HAVE_STRING_H
#include <string.h>
#define HAVE_STRING_H 1
#endif /* !HAVE_STRING_H */

#endif /* !defined(__has_include) */

#endif /* !FEATHERHASH_MAIN_H */
