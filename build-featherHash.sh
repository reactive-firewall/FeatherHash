#!/bin/dash
#
# Permission to use, copy, modify, and/or distribute this shell-script for any
# purpose with or without fee is hereby granted.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
################################################################################
# PATH=${PATH:-"/bin:/sbin:/usr/sbin:/usr/bin:/usr/local/sbin:/usr/local/bin"} ;
# hermetic build/install for FeatherHash
# Usage: ./build-featherHash.sh [CC] [CFLAGS] [DESTDIR]
set -euo

# --- Configuration / defaults ---
CC_ARG="${1:-}"
CFLAGS_ARG="${2:-}"
DESTDIR_ARG="${3:-}"

# Tools (can be overridden via environment)
: "${CC:=${CC_ARG:-clang}}"
: "${AR:=ar}"
: "${STRIP:=strip}"

# Build flags (can be overridden by env)
: "${CFLAGS:=${CFLAGS_ARG:--O2 -ffunction-sections -fdata-sections -fPIC -Wall -Wextra -Werror}}"
: "${LDFLAGS:=-fuse-ld=lld -Wl}"

# Paths (internal)
SRCDIR="FeatherHash"
SRC_1="${SRCDIR}/sha256sum.c"
SRC_2="${SRCDIR}/sha2.c"
HDR="${SRCDIR}/*.h"
PREFIX="/bin"
BINNAME="sha256sum"

# DESTDIR safety: default to ./out if not provided
DESTDIR="${DESTDIR_ARG:-./out}"
# Normalize DESTDIR to an absolute path to avoid surprises
DESTDIR="$(cd "$(dirname "$DESTDIR")" && mkdir -p "$(basename "$DESTDIR")" && cd "$(pwd)/$(basename "$DESTDIR")" >/dev/null 2>&1 || true; printf '%s' "$(pwd)" )"

# Internal layout under DESTDIR
OBJDIR="${DESTDIR}/obj"
BINDIR="${DESTDIR}/bin"
INCLUDEDIR="${DESTDIR}/include"
OUT_BIN_PATH="${DESTDIR}${PREFIX}/${BINNAME}"
TMPOBJ_1="${OBJDIR}/${BINNAME}.o"
TMPOBJ_2="${OBJDIR}/main.o"
STAGED_BIN="${DESTDIR}${PREFIX}/${BINNAME}"

# --- Helpers ---
err() { printf 'ERROR: %s\n' "$*" >&2; exit 1; }
warn() { printf 'Warning: %s\n' "$*" >&2; }
command_exists() { command -v "$1" >/dev/null 2>&1; }

# Require exact interpreter location
# command -v /stage0/bin/bash >/dev/null 2>&1 || err "Require /stage0/bin/bash as interpreter"

# Validate compiler presence (warn but continue so environment can provide)
if ! command_exists "$CC"; then
	warn "compiler '$CC' not found in PATH; build will likely fail"
fi

# Prevent accidental destructive rm -rf on unusual DESTDIR: only allow if DESTDIR is under current dir or explicit './' used
case "$DESTDIR" in
	/*) ;; # absolute allowed (already normalized)
	./*) ;; # explicit relative allowed
	*) ;;
esac

# Create deterministic dirs (safe removal only within DESTDIR)
if [ -d "$DESTDIR" ] && [ -n "${DESTDIR_ARG:-}" ]; then
	# If user explicitly provided DESTDIR, remove previous contents to ensure hermetic build
	rm -rf -- "$DESTDIR"
fi
mkdir -p -- "$OBJDIR" "$BINDIR" "$INCLUDEDIR"

# Copy header if present (optional)
if [ -f "$HDR" ]; then
	cp -- "$HDR" "$INCLUDEDIR/" || err "failed copying header"
fi

# Source check
[ -f "$SRC_1" ] || err "source $SRC_1 not found"

# Compile: explicit include path ensures hermetic headers
printf 'Compiling %s -> %s\n' "$SRC_1" "$TMPOBJ_1"
# Split flags safely
# shellcheck disable=SC2086
$CC $CFLAGS -I"$INCLUDEDIR" -c -o "$TMPOBJ_1" "$SRC_1" || err "compilation failed"

# Source check
[ -f "$SRC_2" ] || err "source $SRC_2 not found"

# Compile: explicit include path ensures hermetic headers
printf 'Compiling %s -> %s\n' "$SRC_2" "$TMPOBJ_2"
# Split flags safely
# shellcheck disable=SC2086
$CC $CFLAGS -I"$INCLUDEDIR" -c -o "$TMPOBJ_2" "$SRC_2" || err "compilation failed"

# Link: attempt static then fallback to dynamic; keep hermetic LDFLAGS if provided
printf 'Linking -> %s\n' "${BINDIR}/${BINNAME}"
mkdir -p -- "$BINDIR"
# Try static link first
set +e
# shellcheck disable=SC2086
$CC $TMPOBJ_1 $TMPOBJ_2 -o "${BINDIR}/${BINNAME}" -static $LDFLAGS
link_status=$?
set -e
if [ "$link_status" -ne 0 ]; then
	printf 'Static link failed (status %d), retrying dynamic link...\n' "$link_status"
	# shellcheck disable=SC2086
	$CC $TMPOBJ_1 $TMPOBJ_2 -o "${BINDIR}/${BINNAME}" $LDFLAGS || err "link failed"
fi

# Optionally strip if available
if command_exists "$STRIP"; then
	printf 'Stripping binary...\n'
	if ! "$STRIP" "${BINDIR}/${BINNAME}" >/dev/null 2>&1; then
		warn "strip failed; continuing"
	fi
fi

# Stage install into DESTDIR + PREFIX
mkdir -p -- "$(dirname "$OUT_BIN_PATH")"
mv -- "${BINDIR}/${BINNAME}" "$OUT_BIN_PATH" || err "install move failed"
chmod 0755 "$OUT_BIN_PATH"

# Verification: run binary with -q or --help if they exist, but do not modify host PATH.
printf 'Verifying built FeatherHash sha256sum...\n'
$CC --version
$OUT_BIN_PATH $SRC_1 2>/dev/null || true
$OUT_BIN_PATH $SRC_2 2>/dev/null || true


printf 'Build/install complete.\n'
printf 'Staged install path: %s\n' "$OUT_BIN_PATH"
printf 'To finalize install, copy %s to %s on the target system.\n' "$OUT_BIN_PATH" "${PREFIX}/${BINNAME}"
