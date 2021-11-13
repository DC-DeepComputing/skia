#!/bin/bash
rm reapeted_with_file.csv
seq 1 10 | xargs -I{} sh -c " mlr --csv put '\$file={};' repeat/mobi{}.csv" >> reapeted_with_file.csv