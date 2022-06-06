#!/bin/bash

set -x

mkdir -p data/tmp
mkdir -p ./data/p

./array_1d.py 1000 const -o ./data/tmp/const.csv
./array_1d.py 1000 linear -o ./data/tmp/linear.csv
./array_1d.py 1000 square -o ./data/tmp/square.csv
./array_1d.py 1000 uniform -o ./data/tmp/uniform.csv
./array_1d.py 1000 normal 500 200 -o ./data/tmp/normal.csv


MERGE="./merge_csv.py -H weight,profit"

merge() {
    W=${1}
    P=${2}
    size=${3}
    ${MERGE} ${size} ./data/tmp/${W}.csv ./data/tmp/${P}.csv -o ./data/p/${size}_${W}_${P}.csv
}

merge_shuffle() {
    W=${1}
    P=${2}
    size=${3}
    ${MERGE} ${size} ./data/tmp/${W}.csv ./data/tmp/${P}.csv -o ./data/p/${size}_${W}_${P}_shuffle.csv --shuffle
}


merge linear square 200

merge const uniform 300

merge linear normal 400

merge square const 500

merge_shuffle square linear 600

merge_shuffle linear square 700

merge square uniform 800

merge uniform uniform 800

merge normal uniform 900


all='const linear square uniform normal'

for w in ${all}; do
    for p in ${all}; do
        merge ${w} ${p} 1000
    done
done

shuffle='linear square'

for w in ${shuffle}; do
    for p in ${shuffle}; do
        merge_shuffle ${w} ${p} 1000
    done
done



rm -rf ./data/tmp

