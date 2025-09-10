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
# test_sha512.sh - compare FeatherHash binary against openssl dgst -sha512

set -eu

BINARY=${1:-./out/bin/sha512sum}
OPENSSL=${OPENSSL:-openssl}

if [ ! -x "$BINARY" ]; then
  echo "Binary not found or not executable: $BINARY" >&2
  exit 2
fi

# helper to normalize openssl output: "SHA256(filename)= <hex>" or "(stdin)= <hex>"
osum() {
  INPUT="$1"
  if [ "$INPUT" = "-" ]; then
    # read stdin from here-document; use printf to avoid echo issues
    printf "%s" "$2" | ${OPENSSL} dgst -sha512 | awk '{print $2}'
  else
    ${OPENSSL} dgst -sha512 "$INPUT" | awk '{print $2}'
  fi
}

# small test vectors
test_vectors() {
  # empty
  printf "" > /tmp/fh_empty
  fh=$(printf "" | "$BINARY" | awk '{print $1}')
  os=$(osum - '' )
  if [ "$fh" != "$os" ]; then
    printf "%s\n" "Mismatch empty string: fh=$fh os=$os" >&2; return 1
  fi

  # abc
  printf "abc" > /tmp/fh_abc
  fh=$("$BINARY" /tmp/fh_abc | awk '{print $1}')
  os=$(osum /tmp/fh_abc)
  if [ "$fh" != "$os" ]; then
    printf "%s\n" "Mismatch 'abc': fh=$fh os=$os" >&2; return 1
  fi

  # random file
  dd if=/dev/urandom of=/tmp/fh_rand bs=1k count=16 >/dev/null 2>&1 || head -c 1073741824 /dev/urandom > /tmp/fh_rand
  fh=$("$BINARY" /tmp/fh_rand | awk '{print $1}')
  os=$(osum /tmp/fh_rand)
  if [ "$fh" != "$os" ]; then
    printf "%s\n" "Mismatch random file" >&2; return 1
  fi

  # stdin test
  fh=$(printf "stream-data-1234" | "$BINARY" | awk '{print $1}')
  os=$(printf "stream-data-1234" | ${OPENSSL} dgst -sha512 | awk '{print $2}')
  if [ "$fh" != "$os" ]; then
    printf "%s\n" "Mismatch stdin" >&2; return 1
  fi

  return 0
}

cleanup_test_artifacts() {
  if [ ! -r /tmp/fh_empty ]; then
    rm -f /tmp/fh_empty 2>/dev/null ;
  fi
  if [ ! -r /tmp/fh_abc ]; then
    rm -f /tmp/fh_abc 2>/dev/null ;
  fi
  if [ ! -r /tmp/fh_rand ]; then
    rm -f /tmp/fh_rand 2>/dev/null ;
  fi
  return 0
}

if test_vectors; then
  echo "All sha512 tests passed"
  cleanup_test_artifacts ;
  exit 0
else
  cleanup_test_artifacts ;
  echo "sha512 Tests failed" >&2
  exit 1
fi
