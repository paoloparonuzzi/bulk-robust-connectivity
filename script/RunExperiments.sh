#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

cd "$PROJECT_ROOT/build"

EXE="./bulkRobustSpan"
TIME_LIMIT=3600
OUTPUT_FILE="$PROJECT_ROOT/results/bulk-robust-connectivity.txt"

mkdir -p "$PROJECT_ROOT/results"
: > "$OUTPUT_FILE"

if [ ! -x "$EXE" ]; then
    echo "Executable $EXE not found. Please compile the project first."
    exit 1
fi

echo "Running LP experiments..."

nVertex=40
while [ "$nVertex" -lt 161 ]
do
    for alg in 3 4
    do
        for seed in {0..9}
        do
            lp=1
            nEdge1=$((nVertex * 2 / 10))
            nEdge2=$((nVertex * 3 / 10))
            nEdge3=$((nVertex * 4 / 10))
            nEdge4=$((nVertex * 5 / 10))

            for nEdge in "$nEdge1" "$nEdge2" "$nEdge3" "$nEdge4"
            do
                "$EXE" "../instances/Flex_${nVertex}_${seed}.txt" "$TIME_LIMIT" "$alg" "$lp" "$nEdge" "$OUTPUT_FILE"
            done
        done
    done
    nVertex=$((nVertex + 40))
done

echo "Running MIP experiments..."

nVertex=40
while [ "$nVertex" -lt 161 ]
do
    for alg in 3 4
    do
        for seed in {0..9}
        do
            lp=0
            nEdge1=$((nVertex * 2 / 10))
            nEdge2=$((nVertex * 3 / 10))
            nEdge3=$((nVertex * 4 / 10))
            nEdge4=$((nVertex * 5 / 10))

            for nEdge in "$nEdge1" "$nEdge2" "$nEdge3" "$nEdge4"
            do
                "$EXE" "../instances/Flex_${nVertex}_${seed}.txt" "$TIME_LIMIT" "$alg" "$lp" "$nEdge" "$OUTPUT_FILE"
            done
        done
    done
    nVertex=$((nVertex + 40))
done

echo "All experiments completed successfully."