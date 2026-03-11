#!/usr/bin/env bash
set -euo pipefail

EXE="./bulkRobustMatch"
TIME_LIMIT=3600
OUTPUT_FILE="../results/bulk-robust-assignment.txt"

mkdir -p ../results
: > "$OUTPUT_FILE"

echo "Running LP experiments..."

nVertex=20
while [ "$nVertex" -lt 81 ]
do
    for alg in 0 2 3 4
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
                "$EXE" "../instances/Match_${nVertex}_${seed}.txt" "$TIME_LIMIT" "$alg" "$lp" "$nEdge" "$OUTPUT_FILE"
            done
        done
    done
    nVertex=$((nVertex + 20))
done

echo "Running MIP experiments..."

nVertex=20
while [ "$nVertex" -lt 121 ]
do
    for alg in 0 3
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
                "$EXE" "../instances/Match_${nVertex}_${seed}.txt" "$TIME_LIMIT" "$alg" "$lp" "$nEdge" "$OUTPUT_FILE"
            done
        done
    done
    nVertex=$((nVertex + 20))
done

echo "All experiments completed successfully."