#!/bin/bash
./trace_processor_shell --run-metrics=./metrics/nanobench_power.sql ./nanobench.perfetto-trace --metrics-output=json  | jq '."perfetto.protos.nanobench_power"[]' | json2csv > mobi.csv
# ./trace_processor  nanobench.perfetto-trace --query-file power.sql > power.csv