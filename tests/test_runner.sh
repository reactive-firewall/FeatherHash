#!/bin/dash
# test_sha256.sh - compare FeatherHash binary against openssl dgst -sha256
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
