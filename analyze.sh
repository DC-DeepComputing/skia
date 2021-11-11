#!/bin/bash
./trace_processor_shell --run-metrics=./metrics/nanobench_power.sql ./nanobench.perfetto-trace --metrics-output=json 
# ./trace_processor  nanobench.perfetto-trace --query-file power.sql > power.csv