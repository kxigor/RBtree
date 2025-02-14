#!/bin/bash

CLANGTIDY="clang-tidy-15 -p build/ -extra-arg=-Wno-unknown-warning-option -extra-arg=-Wno-unused-command-line-argument -extra-arg=-Wno-invalid-command-line-argument" 
CLANGTIDYAGRS="--quiet"

exit_status=0

for file in "$@"; do
    if [[ ! -f "$file" ]]; then
        echo "Файл $file не существует."
        exit_status=1
        continue
    fi

    $CLANGTIDY "${CLANGTIDYAGRS[@]}" "$file"

    if [[ ! $? -eq 0 ]]
    then 
      exit_status=1
    fi

done

if [[ $exit_status -ne 0 ]]; then
    echo "Некоторые файлы не прошли проверку."
    exit 1
else
    echo "Все файлы прошли проверку."
    exit 0
fi
