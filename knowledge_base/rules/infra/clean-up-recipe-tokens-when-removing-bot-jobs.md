---
name: skia/infra/clean-up-recipe-tokens-when-removing-bot-jobs
description: When removing bot configurations or builder tokens, delete obsolete checks, make negated conditions unconditional, and regenerate recipe expectations
rubric: correctness
severity: should_fix
---

**Rule.** When removing bot jobs or tryjob configurations, delete matching
builder token checks (such as extra_tokens in recipes or ExtraConfig in
gen_tasks_logic). When removing checks on eliminated configurations, preserve
and make unconditional any code guarded by negated conditions (e.g.,
!b.ExtraConfig(...) or 'token' not in extra_tokens) rather than deleting the
guarded block. Run make train in infra/bots to delete obsolete simulation test
cases and regenerate recipe expectations.

**Why.** Negated conditions guard functionality that must run for all
configurations except the excluded bot; when that bot is eliminated, the
condition becomes unconditionally true for all remaining builders, and deleting
the block inadvertently drops required assets or properties. Removing obsolete
token checks and running make train prevents dead code and stale expectations
across the infra subsystem.

## Examples

Bad:

```cpp
// In infra/bots/gen_tasks_logic/nano_flags.go when retiring "Valgrind":
-	if !b.ExtraConfig("Valgrind") {
-		b.asset("svg")
-		b.recipeProp("svgs", "true")
-	}

# In infra/bots/recipe_modules/build/default.py after removing Vello jobs from jobs.json:
if 'Vello' in extra_tokens:
  args['skia_enable_vello_shaders'] = 'true'
```

Good:

```cpp
// Retain negated condition body unconditionally when retiring the configuration:
-	if !b.ExtraConfig("Valgrind") {
-		b.asset("svg")
-		b.recipeProp("svgs", "true")
-	}
+	b.asset("svg")
+	b.recipeProp("svgs", "true")

# Clean up obsolete builder token checks and run `make train` to update expectations:
if 'Fontations' in extra_tokens:
  args['skia_use_fontations'] = 'true'
```
