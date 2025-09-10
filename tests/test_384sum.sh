#!/bin/dash
# test_sha384.sh - compare FeatherHash binary against openssl dgst -sha384
set -eu

BINARY=${1:-./out/bin/sha384sum}
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
    printf "%s" "$2" | ${OPENSSL} dgst -sha384 | awk '{print $2}'
  else
    ${OPENSSL} dgst -sha384 "$INPUT" | awk '{print $2}'
  fi
}

# small test vectors
test_vectors() {
  # empty
  printf "" > /tmp/fh_empty
  fh=$(printf "" | "$BINARY" | awk '{print $1}')
  os=$(osum - '' )
  if [ "$fh" != "$os" ]; then
    echo "Mismatch empty string: fh=$fh os=$os" >&2; return 1
  fi

  # abc
  printf "abc" > /tmp/fh_abc
  fh=$("$BINARY" /tmp/fh_abc | awk '{print $1}')
  os=$(osum /tmp/fh_abc)
  if [ "$fh" != "$os" ]; then
    echo "Mismatch 'abc': fh=$fh os=$os" >&2; return 1
  fi

  # random file
  dd if=/dev/urandom of=/tmp/fh_rand bs=1k count=16 >/dev/null 2>&1 || head -c 16384 /dev/urandom > /tmp/fh_rand
  fh=$("$BINARY" /tmp/fh_rand | awk '{print $1}')
  os=$(osum /tmp/fh_rand)
  if [ "$fh" != "$os" ]; then
    echo "Mismatch random file" >&2; return 1
  fi

  # stdin test
  fh=$(printf "stream-data-1234" | "$BINARY" | awk '{print $1}')
  os=$(printf "stream-data-1234" | ${OPENSSL} dgst -sha384 | awk '{print $2}')
  if [ "$fh" != "$os" ]; then
    echo "Mismatch stdin" >&2; return 1
  fi

  return 0
}

if test_vectors; then
  echo "All sha384 tests passed"
  exit 0
else
  echo "sha384 Tests failed" >&2
  exit 1
fi
