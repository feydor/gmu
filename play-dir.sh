#!/bin/sh
# loop files in a directory through gmu

gmu_bin="./gmu-arm64-apple-darwin23.3.0"

dir="$1"
if [[ -z "$dir" ]] || [[ ! -d "$dir" ]]; then
    echo "Missing directory"
    exit 1
fi

IFS=$'\n'
list=($(find -s "$dir" -type f -iname "*.vgm" ))

for file in "${list[@]}"; do
    "$gmu_bin" "$file"
done
