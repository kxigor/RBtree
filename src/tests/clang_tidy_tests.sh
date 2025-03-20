#!/bin/bash

CLANG_TIDY="clang-tidy-19 -p build/ -extra-arg=-Wno-unknown-warning-option -extra-arg=-Wno-unused-command-line-argument -extra-arg=-Wno-invalid-command-line-argument"
CLANG_TIDY_VERSION=19
CLANG_TIDY_ARGS="--quiet"

exit_status=0

for file in "$@"; do
    if [[ ! -f "$file" ]]; then
        echo "File $file does not exist."
        exit_status=1
        continue
    fi

    $CLANG_TIDY $CLANG_TIDY_ARGS "$file"

    if [[ $? -ne 0 ]]; then
        exit_status=1
    fi
done

if [[ $exit_status -ne 0 ]]; then
    echo "Some files did not pass the check."
    exit 1
else
    echo "All files passed the check."
    exit 0
fi
