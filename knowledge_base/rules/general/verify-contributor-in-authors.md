---
name: skia/verify-contributor-in-authors
description: Before approving a change, verify the contributor is listed in AUTHORS and has signed the CLA, requiring an AUTHORS entry if new
rubric: readability
severity: blocking
---

**Rule.** Before approving or merging a change, verify that first-time or
external contributors (or their organization) are listed in the AUTHORS file and
have signed the Contributor License Agreement (CLA). If the author is not yet
listed, ensure the CLA is signed and require an alphabetical entry in the
AUTHORS file formatted as 'Name <email address>' for individuals or
'Organization <fnmatch pattern>' for corporate entities as part of the CL.

**Why.** Skia requires all contributors to execute a Contributor License
Agreement for copyright and licensing compliance, and automated commit-queue
checks verify that commit authors match entries in AUTHORS. Verifying
attribution before approval prevents licensing issues and prevents commit-queue
rejections.

## Examples

Bad:

```cpp
// Approving a change from a first-time contributor without verifying AUTHORS:
LGTM! Change looks good to land.
// (Contributor not listed in AUTHORS and CLA status unverified)
```

Good:

```cpp
// Verifying contributor in AUTHORS and requesting addition when absent:
Before we can merge, please ensure you've signed the Google CLA:
https://skia.org/docs/dev/contrib/
and add an entry in alphabetical order to the AUTHORS file in this CL:
Collabora Ltd <*@collabora.com>
```
