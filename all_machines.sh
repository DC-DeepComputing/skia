#!/bin/bash

cat machines.json | jq '.[].Dimensions | {"os": .os[-1], "device_type": .device_type[-1], , "gpu": .gpu[-1], "device_os": .device_os[-1], "machine_type": .machine_type[-1], "release_version": .release_version[-1]}'


cat ./infra/bots/tasks.json | jq '.tasks | to_entries[] | .value | .dimensions[]' | sort | uniq -c