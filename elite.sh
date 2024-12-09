#!/bin/bash

dir="$(dirname $0)"
elitepath="${dir}/elite"

journal_dir="${1:-/home/tom/games/elite_log}"

journal="$(find "$journal_dir"/ -type f -name Journal.\* -exec stat --format "%Y %n" {} \; | sort -nr | cut -d ' ' -f 2- | head -n 1)"

if [ -z $journal ]; then
    echo "Could not find journal in '$journal_dir'"
    exit 1

fi

echo "Found journal '$journal'"

tail -f "$journal" | dos2unix | $elitepath $journaldir
