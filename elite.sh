#!/bin/zsh

dir="$(dirname $0)"
elitepath="${dir}/elite"

journal_dir="${1:-/home/tom/games/elite_log}"

while true; do
    journal="$(find "$journal_dir"/ -type f -name Journal.\* -exec stat --format "%Y %n" {} \; | sort -nr | cut -d ' ' -f 2- | head -n 1)"

    if [ -z $journal ]; then
        echo "Could not find journal in '$journal_dir'"
        exit 1

    fi

    echo "Found journal '$journal'"

    (inotifywait -e close "$journal" && kill -TERM $(pidof elite) && kill $(pidof tail))&
    tail -f -n0 "$journal" | dos2unix | $elitepath $journaldir
done
