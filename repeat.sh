#!/bin/bash
seq 1 10 | xargs -I{} sh -c "./capture.sh && ./analyze.sh && mv mobi.csv repeat/mobi{}.csv"
seq 1 10 | xargs -I{} sh -c "mlr -I --csv put '\$file={};' repeat/mobi{}.csv"
mlr --csv cat repeat/mobi* > all_with_file.csv