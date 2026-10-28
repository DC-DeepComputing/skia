---
name: skia/api/prefix-boolean-query-methods-with-auxiliary-verbs
description: When declaring boolean query or predicate functions, use concise auxiliary verbs (is, has, can, should, supports) rather than imperative verbs, bare nouns, or is-able adjectives
rubric: readability
severity: should_fix
---

**Rule.** Name boolean query and predicate functions or methods using concise
auxiliary verbs or predicate prefixes (such as is.../Is..., has.../Has...,
can.../Can..., should.../Should..., did.../Did..., supports...) or natural,
fluent verb phrases with fully spelled terms (e.g., appendsVertices()). For
capability and compatibility queries, prefer concise modal verb prefixes like
Can<Verb> (e.g., CanInterpolate) over multi-syllable adjectives ending in
'-able' (e.g., isInterpolatable). In Caps classes, prefix feature queries with
supports... or is... rather than bare nouns (e.g.,
supportsOcclusionQueryPrecise()). Avoid imperative action verbs (e.g.,
draw_layer_as_sprite(); use can_layer_be_drawn_as_sprite()), verbose procedural
prefixes (DetermineIf...), declarative past-tense phrases
(asyncCompilationFailed()), and awkward noun-stacked jargon
(isAppendRateVert()).

**Why.** Boolean predicates represent assertions, capabilities, or conditions
evaluated at conditional call sites. Direct modal verbs like Can<Verb> are more
concise and idiomatic than multi-syllable '-able' adjectives. Imperative verbs
mislead callers into expecting mutating operations with side effects, while bare
nouns resemble getters, procedural prefixes add clutter, and past-tense or
noun-stacked phrases obscure query semantics.

## Examples

Bad:

```cpp
// Multi-syllable '-able' adjective is less direct than modal Can<Verb>
bool isInterpolatable(const SkPath& compare) const;

// Imperative action verb falsely implies performing an operation rather than a boolean query
static bool draw_layer_as_sprite(const SkMatrix& matrix, const SkISize& size);

// Declarative past-tense phrase resembles an event callback rather than a query
bool asyncCompilationFailed() const override;

// Procedural prefix adds unnecessary verbosity to boolean predicate
bool DetermineIfDstReadRequired(const Caps*, std::optional<SkBlendMode>, Coverage);

// Awkward noun-stacked jargon and truncated terms hinder readability
bool isAppendRateVert() const { return SkToBool(fFlags & Flags::kAppendVertices); }

// Bare noun or raw API property name resembles a value getter rather than a capability query
bool occlusionQueryPrecise() const { return fOcclusionQueryPrecise; }
```

Good:

```cpp
// Direct modal auxiliary verb is concise, readable, and idiomatic
bool CanInterpolate(const SkPath& from, const SkPath& to);

// Auxiliary/interrogative prefix clarifies that the function is a non-mutating query
static bool can_layer_be_drawn_as_sprite(const SkMatrix& matrix, const SkISize& size);

// Auxiliary verb clarifies that the method is a boolean query
bool didAsyncCompilationFail() const override;

// Concise predicate prefix reads naturally in conditional expressions
bool IsDstReadRequired(const Caps*, std::optional<SkBlendMode>, Coverage);

// Natural verb phrase with fully spelled words reads fluently at call sites
bool appendsVertices() const { return SkToBool(fFlags & Flags::kAppendVertices); }

// Predicate prefix clarifies feature capability check
bool supportsOcclusionQueryPrecise() const { return fOcclusionQueryPrecise; }
```
