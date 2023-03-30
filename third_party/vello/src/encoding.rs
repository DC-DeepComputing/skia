// Copyright 2023 Google Inc.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

use {
    peniko::{Brush, Color, kurbo::Affine},
    std::pin::Pin,
    vello_encoding::{Encoding as VelloEncoding, Transform},
};
use crate::ffi;

pub(crate) struct Encoding {
    encoding: VelloEncoding,
}

pub(crate) fn new_encoding() -> Box<Encoding> {
    Box::new(Encoding::new())
}

impl Encoding {
    fn new() -> Encoding {
        Encoding {
            encoding: VelloEncoding::default(),
        }
    }

    pub fn fill(self: &mut Encoding,
                style: ffi::Fill,
                transform: ffi::Affine,
                brush: &ffi::Brush,
                mut path_iter: Pin<&mut ffi::PathIterator>) {
        self.encoding.encode_transform(Transform::from_kurbo(&transform.into())); 
        self.encoding.encode_linewidth(match style {
            ffi::Fill::NonZero => -1.0,
            ffi::Fill::EvenOdd => -2.0,
            _ => panic!("invalid fill type"),
        });
        let mut path_enc = self.encoding.encode_path(/*is_fill=*/true);
        let mut path_el = ffi::PathElement::default();
        let segments = unsafe {
            while path_iter.as_mut().next_element(&mut path_el) {
                match path_el.verb {
                    ffi::PathVerb::MoveTo => {
                        let p = &path_el.points[0];
                        path_enc.move_to(p.x, p.y);
                    },
                    ffi::PathVerb::LineTo => {
                        let p = &path_el.points[0];
                        path_enc.line_to(p.x, p.y);
                    },
                    ffi::PathVerb::QuadTo => {
                        let p0 = &path_el.points[0];
                        let p1 = &path_el.points[1];
                        path_enc.quad_to(p0.x, p0.y, p1.x, p1.y);
                    },
                    ffi::PathVerb::CurveTo => {
                        let p0 = &path_el.points[0];
                        let p1 = &path_el.points[1];
                        let p2 = &path_el.points[2];
                        path_enc.cubic_to(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y);
                    },
                    ffi::PathVerb::Close => path_enc.close(),
                    _ => panic!("invalid path verb"),
                }
            }
            path_enc.finish(/*insert_path_marker=*/true)
        };
        if segments != 0 {
            self.encoding.encode_brush(&Brush::from(brush), 1.0)
        }
    }
}

impl Default for ffi::PathVerb {
    fn default() -> Self {
        Self::MoveTo
    }
}

impl From<ffi::Affine> for Affine {
    fn from(src: ffi::Affine) -> Self {
        Self::new([
            src.matrix[0] as f64,
            src.matrix[1] as f64,
            src.matrix[2] as f64,
            src.matrix[3] as f64,
            src.matrix[4] as f64,
            src.matrix[5] as f64,
        ])
    }
}

impl From<&ffi::Color> for Color {
    fn from(src: &ffi::Color) -> Self {
        Self {
            r: src.r,
            g: src.g,
            b: src.b,
            a: src.a,
        }
    }
}

impl From<&ffi::Brush> for Brush {
    fn from(src: &ffi::Brush) -> Brush {
        match src.kind {
            ffi::BrushKind::Solid => Brush::Solid(Color::from(&src.data.solid)),
            _ => panic!("invalid brush kind"),
        }
    }
}
