#!/bin/bash

CLANGFORMAT="clang-format"
CLANGFORMATARGS=(-style='{BasedOnStyle: Google, DerivePointerAlignment: false, PointerAlignment: Left, AlignOperands: true}')

exit_status=0

for file in "$@"; do
    if [[ ! -f "$file" ]]; then
        echo "Файл $file не существует."
        exit_status=1
        continue
    fi

    $CLANGFORMAT "${CLANGFORMATARGS[@]}" "$file" >temp.format

    if ! diff -q "$file" temp.format >/dev/null; then
        echo "Файлы $file не отформатирован:"
        diff "$file" temp.format
        exit_status=1
    fi

    rm temp.format
done

if [[ $exit_status -ne 0 ]]; then
    echo "Некоторые файлы не прошли проверку."
    exit 1
else
    echo "Все файлы прошли проверку."
    exit 0
fi
