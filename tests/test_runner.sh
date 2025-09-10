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
# test_sha256.sh - compare FeatherHash binaries against openssl dgst -sha256/384/512

set -eu

# Get the input path of this script as called
input_path="$0"
# Remove the trailing slash if present
input_path="${input_path%/}"
# Extract the directory name
PATH_ARG="${input_path%/*}"

if [ -d "$PATH_ARG" ] && [ ":$PATH:" != *":$PATH_ARG:"* ] ; then
	PATH="${PATH:+"$PATH:"}$PATH_ARG" ;
	export PATH ;
fi

unset input_path ;
unset PATH_ARG ;

# small test vectors
test_vectors() {
  # empty
  test_256sum.sh || return 1;
  test_384sum.sh || return 1;
  test_512sum.sh || return 1;

  return 0
}

if test_vectors; then
  echo "All tests passed"
  exit 0
else
  echo "Tests failed" >&2
  exit 1
fi
