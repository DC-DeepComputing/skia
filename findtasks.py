#!/usr/bin/env python

MACHINE_TYPE = "n1-standard-32"

import json

with open("infra/bots/tasks.json") as f:
    tasks_json = json.loads(f.read())

target_tasks = []

for name, body in tasks_json["tasks"].items():
    if "machine_type:" + MACHINE_TYPE in body["dimensions"] and \
        "os:Debian-10.3" in body["dimensions"] and \
        "Nightly" not in name:
        target_tasks.append(name)

print("Tasks to run:")
for name in target_tasks:
    print("    " + name)
print()

command = "git cl try -B skia.primary \\\n"
extra_args = ["    -b " + name for name in target_tasks]
command += " \\\n".join(extra_args)

print("Command:\n")
print(command)
