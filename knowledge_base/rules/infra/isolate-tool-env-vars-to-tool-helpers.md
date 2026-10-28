---
name: skia/infra/isolate-tool-env-vars-to-tool-helpers
description: When configuring task builder specs in infra/bots, attach tool-specific environment variables and dependencies within dedicated tool helpers rather than generic task constructors
rubric: design
severity: should_fix
---

**Rule.** When generating bot tasks in infra/bots (such as in gen_tasks_logic),
attach tool-specific environment variables, CIPD packages, and PATH entries
within dedicated tool helper methods (e.g., usesBazel, usesCCache) rather than
in generic task constructors (such as newTaskBuilder). Keep generic task
constructors scoped strictly to baseline properties common to all tasks.

**Why.** Setting tool-specific environment variables or dependencies in generic
task constructors pollutes the generated task specifications across all bots,
bloating tasks.json and risking unintended side effects or configuration
conflicts in tasks that do not use the tool.

## Examples

Bad:

```cpp
func newTaskBuilder(b *jobBuilder, name string) *taskBuilder {
	return &taskBuilder{
		jobBuilder: b,
		Name:       name,
		Spec: &specs.TaskSpec{
			Environment: map[string]string{
				"USE_BAZEL_FALLBACK_VERSION": "error",
			},
		},
	}
}
```

Good:

```cpp
func (b *taskBuilder) usesBazel(hostOSArch string) {
	pkg, ok := archToPkg[hostOSArch]
	if !ok {
		panic("Unsupported osAndArch for bazelisk: " + hostOSArch)
	}
	b.cipd(b.MustGetCipdPackageFromAsset(pkg))
	b.addToPATH(pkg)
	b.env("USE_BAZEL_FALLBACK_VERSION", "error")
}
```
