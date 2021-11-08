#!/bin/bash
./trace_processor  nanobench.perfetto-trace --query-file slices.sql > slices.csv

./trace_processor  nanobench.perfetto-trace --query-file power.sql > power.csv