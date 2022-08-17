# Skia Infrastructure

This directory contains infrastructure elements.


## Tasks and Jobs

Files in this directory define a DAG of tasks which can run against Skia commits. A
task is a small, self-contained unit which runs via Swarming on a machine in the
pool. Tasks may be chained together, e.g. one task to compile test binaries and
another to actually run them.

Jobs are collections of related tasks which help define sub-sections of the DAG,
for example, to be used as try jobs. Each job is defined as an entry point into
the DAG.

tasks.json in this directory is the list of tasks and jobs for the repo. Note
that this file is NEVER edited by hand but generated via gen_tasks.go and the
input files enumerated below. The [Task
Scheduler](https://skia.googlesource.com/buildbot/+/main/task_scheduler/README.
md) reads tasks.json at each commit to determine which jobs to run.

Jobs with a `"cq_config": {}` property are run as gating ones on Gerrit CLs.
Most of the rest are run as backfill ("waterfall" in buildbot terminology),
reaching back in time to qualify already-landed CLs as resources permit.

Jobs with an `OnDemand` chunk in their names are run only only when manually
requested, not as part of routine backfill. They are mostly canaries: tests that
try a CL against, for example, a full Chrome build to see if it's going to break
Chrome.

For convenience, gen_tasks.go is provided to generate tasks.json and also to
test it for correct syntax, cycles, and orphaned tasks. Always edit gen_tasks.go
or one of the following input JSON files rather than tasks.json directly:

  * cfg.json - Basic configuration information for gen_tasks.go.
  * jobs.json - The list of all jobs to run. Edit this to add or remove
      bots.

Whenever gen_tasks.go, any of the above JSON files, or assets are changed, you
need to run gen_tasks.go to regenerate tasks.json:

	$ go run infra/bots/gen_tasks.go

Or:

	$ cd infra/bots; make train

There is also a test mode which performs consistency checks and verifies that
tasks.json is unchanged:

	$ go run infra/bots/gen_tasks.go --test

Or:

	$ cd infra/bots; make test

### Creating Jobs

When you add a new job, you can test it before merging; Gerrit's UI for running
check jobs reads the tasks.json in the CL, not the one from tip-of-tree.
Alternatively, you can trigger a job from the commandline using the `sk` tool.
For example...

    $ sk try Test-Mac12-Clang-MacMini7.1-CPU-AVX2-x86_64-Debug-All-NativeFonts_ASAN

## Recipes

Recipes are the framework used by Skia's infrastructure to perform work inside
of Swarming tasks. The main elements are:

  * recipes.py - Used for running and testing recipes.
  * recipes - These are the entry points for each type of task, eg. compiling
      or running tests.
  * recipe_modules - Shared modules which are used by recipes.
  * .recipe_deps - Recipes and modules may depend on modules from other repos.
      The recipes.py script automatically syncs those dependencies in this
      directory.


## Isolate Files

These files determine which parts of the repository are transferred to the bot
when a Swarming task is triggered. The
[Isolate tool](https://github.com/luci/luci-py/tree/main/appengine/isolate/doc)
hashes each file and will upload any new/changed files. Bots maintain a cache so
that they can efficiently download only the files they don't have.


## Assets

Artifacts used by the infrastructure are versioned here, along with scripts for
recreating/uploading/downloading them. See the README in that directory for more
information. Any time an asset used by the bots changes, you need to re-run
gen_tasks.go.


## Tools

Assorted other infrastructure-related tools, eg. isolate and CIPD binaries.


## CT

Helpers for running Skia tasks in Cluster Telemetry.
