---
title: "Perf (Performance Dashboard)"
linkTitle: "Perf"
weight: 10
---

**Perf** (also known as the **Performance Dashboard**) is a distributed system for monitoring and analyzing performance benchmarking metrics produced across Continuous Integration (CI) and postsubmit testing workflows.

While originally developed by the Skia project to track 2D graphics performance and nanobenchmarks, Perf is maintained by the Browser Performance Infrastructure team and serves as the standard performance analysis dashboard across multiple open-source projects.

---

## Core Capabilities

* **Continuous Metric Ingestion**: Stores time-series data from automated benchmark suites run across various platforms, architectures, and hardware configurations.
* **Anomaly & Regression Detection**: Algorithms monitor incoming telemetry for significant changepoints and regressions, automatically filing alerts and issue reports.
* **Interactive Data Exploration**: Web interfaces for plotting metrics over time, comparing commits, viewing blame lists, and exploring cluster trends.

---

## Topics in this Section

* [User & Triage Guides](./user_guide.md): Learn how to use the Multigraph view (`/m`) and format benchmark data for ingestion.
* [Active Dashboard Instances](./instances.md): A directory of live public production dashboard instances across Chromium, Skia, V8, AndroidX, Flutter, Fuchsia, and more.
* [Help & Support](./help.md): Support channels, bug filing resources, and community contact points.

For documentation specifically focused on the Skia instance configurations, see [Skia Perf](../skiaperf.md).
