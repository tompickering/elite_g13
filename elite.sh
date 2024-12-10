#!/bin/zsh

dir="$(dirname $0)"
elitepath="${dir}/elite"

journal_dir="${1:-/home/tom/games/elite_log}"

pipename="pipe"

mkfifo "${pipename}" 2>/dev/null

trap "trap - SIGTERM && kill -- -$$" SIGINT SIGTERM EXIT

pkill -x elite

$elitepath < "$pipename" &

while true; do
    echo "Searching for journal..."
    journal="$(find "$journal_dir"/ -type f -name Journal.\* -exec stat --format "%Y %n" {} \; | sort -nr | cut -d ' ' -f 2- | head -n 1)"

    if [ -z "$journal" ]; then
        echo "Could not find journal in '$journal_dir'"
        exit 1
    fi

    echo "Found journal '$journal'"

    (tail -f "$journal" | stdbuf -i0 -o0 dos2unix >> "$pipename")&

    inotifywait -e create "$journal_dir"
    pkill -x tail
done
