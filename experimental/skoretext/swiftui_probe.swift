// Copyright 2026 Google LLC
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// SwiftUI Text kerning / tracking widths, to compare with the UILabel column of trak_probe.m
// Build and run in the iOS simulator: see README.md
import SwiftUI
import UIKit
@MainActor func w(_ v: some View) -> CGFloat {
    let host = UIHostingController(rootView: v.fixedSize())
    return host.sizeThatFits(in: CGSize(width: 10000, height: 10000)).width
}
@MainActor func run() {
    for s in ["Hello World", "你好世界", "שלום"] {
        for size in [17.0, 28.0] {
            for x in [0.0, -0.41, 1.0, 5.0] {
                let base = Text(s).font(.system(size: size))
                print(String(format: "%@ %.0f x=%.2f plain=%.3f kerning=%.3f tracking=%.3f",
                             s, size, x, w(base), w(base.kerning(x)), w(base.tracking(x))))
            }
        }
    }
}
MainActor.assumeIsolated { run() }
