---
name: skia/tests/assert-conservation-invariants-on-cache-stats
description: When verifying thread-safe caches with concurrent lookups and insertions, assert conservation invariants on cache statistics relating misses to additions and races
rubric: testing
severity: should_fix
---

**Rule.** When testing thread-safe caches under concurrent lookups and
insertions, assert conservation invariants on recorded cache statistics with
REPORTER_ASSERT. Specifically, verify that total cache misses equal the sum of
successful cache additions and lost insertion races (misses == additions +
races), and that total probes equal hits plus misses, ensuring all query
outcomes are accounted for without silent drops or double counting.

**Why.** In concurrent caches using query-then-insert patterns, each lookup miss
that attempts to insert must either successfully add a new entry or lose a race
to a concurrent thread inserting the same key. Asserting conservation invariants
validates the integrity of internal cache statistics accounting and ensures
concurrent requests neither drop outcomes nor count duplicate insertions.

## Examples

Bad:

```cpp
GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == kNumPipelines);
REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
```

Good:

```cpp
GlobalCache::PipelineStats stats = context->priv().globalCache()->getStats();

REPORTER_ASSERT(reporter, stats.fGraphicsCacheAdditions == kNumPipelines);
REPORTER_ASSERT(reporter, stats.fGraphicsRaces > 0);
REPORTER_ASSERT(reporter, stats.fGraphicsCacheMisses ==
                          stats.fGraphicsCacheAdditions + stats.fGraphicsRaces);
REPORTER_ASSERT(reporter, numProbes == stats.fGraphicsCacheHits + stats.fGraphicsCacheMisses);
```
