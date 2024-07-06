#!/bin/bash
#Roy Ronen 215229865

dirPath=$1
wordToFind=$2
flag=$3
# Array to store matching files
files=()
if [[ "$1" = '' || "$2" = '' ]]; then
    echo "Not enough parameters"
    exit
fi

if [ "$flag" = "-r" ]; then
    while IFS= read -r -d '' file; do
        rm "$file"
    done < <(find "$dirPath" -type f -name "*.out" -print0)

    while IFS= read -r -d '' file; do
        if grep -q -i -w "$wordToFind" "$file"; then
            files+=("$file")
        fi
    done < <(find "$dirPath" -type f -name "*.c" -print0)

else
    for file in "$dirPath/"*.out; do
        if [ -e "$file" ]; then
            rm "$file"
        fi
    done

    for file in "$dirPath/"*.c; do
        if [ -e "$file" ]; then
            if grep -q -i -w "$wordToFind" "$file"; then
                files+=("$file")
            fi
        fi
    done
fi

# Print the matching files

for file in "${files[@]}"; do
    dir="$(dirname -- "$file")"
    cd "$dir"
    f="$(basename -s .c -- "$file")"
    gcc "$f.c" -w -o "$f.out"
    cd "$OLDPWD"
done