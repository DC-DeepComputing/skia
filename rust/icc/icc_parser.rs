// Copyright 2026 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#![cfg_attr(not(test), allow(dead_code))]

use crate::ffi;
use crate::icc_curve::{self, EmptyCurve};
use crate::icc_lut::{self, Direction, ParsedLut};
use crate::icc_reader::{Reader, TagTable};

const RGB_SIGNATURE: u32 = u32::from_be_bytes(*b"RGB ");
const GRAY_SIGNATURE: u32 = u32::from_be_bytes(*b"GRAY");
const XYZ_SIGNATURE: u32 = u32::from_be_bytes(*b"XYZ ");
const LAB_SIGNATURE: u32 = u32::from_be_bytes(*b"Lab ");
const YCBCR_SIGNATURE: u32 = u32::from_be_bytes(*b"YCbr");
const INPUT_CLASS_SIGNATURE: u32 = u32::from_be_bytes(*b"scnr");
const DISPLAY_CLASS_SIGNATURE: u32 = u32::from_be_bytes(*b"mntr");
const CICP_TYPE_SIGNATURE: u32 = u32::from_be_bytes(*b"cicp");

const R_XYZ: u32 = u32::from_be_bytes(*b"rXYZ");
const G_XYZ: u32 = u32::from_be_bytes(*b"gXYZ");
const B_XYZ: u32 = u32::from_be_bytes(*b"bXYZ");
const R_TRC: u32 = u32::from_be_bytes(*b"rTRC");
const G_TRC: u32 = u32::from_be_bytes(*b"gTRC");
const B_TRC: u32 = u32::from_be_bytes(*b"bTRC");
const K_TRC: u32 = u32::from_be_bytes(*b"kTRC");
const CICP: u32 = u32::from_be_bytes(*b"cicp");
const A2B_TAGS: [u32; 3] = [
    u32::from_be_bytes(*b"A2B0"),
    u32::from_be_bytes(*b"A2B1"),
    u32::from_be_bytes(*b"A2B2"),
];
const B2A_TAGS: [u32; 3] = [
    u32::from_be_bytes(*b"B2A0"),
    u32::from_be_bytes(*b"B2A1"),
    u32::from_be_bytes(*b"B2A2"),
];

struct Header {
    profile_class: u32,
    data_color_space: ffi::skcms_Signature,
    data_color_space_raw: u32,
    connection_space: ffi::skcms_Signature,
    connection_space_raw: u32,
    illuminant: [f32; 3],
}

pub(crate) fn parse_profile(data: &[u8]) -> Option<ffi::IccProfile> {
    let tags = TagTable::parse(data)?;
    let header = parse_header(data)?;
    let mut profile = empty_profile(header.data_color_space, header.connection_space);

    if header.data_color_space_raw == GRAY_SIGNATURE
        && header.connection_space_raw == XYZ_SIGNATURE
    {
        profile.to_xyzd50 = diagonal_matrix(header.illuminant);
        profile.has_to_xyzd50 = true;
    } else if header.data_color_space_raw == RGB_SIGNATURE {
        parse_rgb_matrix(&tags, &mut profile)?;
    }

    if header.data_color_space_raw == GRAY_SIGNATURE {
        parse_gray_trc(&tags, &mut profile)?;
    } else if header.data_color_space_raw == RGB_SIGNATURE {
        parse_rgb_trcs(&tags, &mut profile)?;
    }

    parse_cicp(&tags, &header, &mut profile)?;
    parse_luts(&tags, &header, &mut profile)?;
    Some(profile)
}

fn parse_luts(
    tags: &TagTable<'_>,
    header: &Header,
    profile: &mut ffi::IccProfile,
) -> Option<()> {
    let pcs_is_xyz = header.connection_space_raw == XYZ_SIGNATURE;
    for signature in A2B_TAGS {
        let Some(tag) = tags.get(signature).ok()? else {
            continue;
        };
        if !is_lut_type(tag.type_signature, Direction::A2B) {
            continue;
        }
        let ParsedLut::A2B(a2b) = icc_lut::parse_lut(tag.data, pcs_is_xyz, Direction::A2B)? else {
            return None;
        };
        profile.a2b = a2b;
        profile.has_a2b = true;
        break;
    }

    for signature in B2A_TAGS {
        let Some(tag) = tags.get(signature).ok()? else {
            continue;
        };
        if !is_lut_type(tag.type_signature, Direction::B2A) {
            continue;
        }
        let ParsedLut::B2A(b2a) = icc_lut::parse_lut(tag.data, pcs_is_xyz, Direction::B2A)? else {
            return None;
        };
        profile.b2a = b2a;
        profile.has_b2a = true;
        break;
    }
    Some(())
}

fn is_lut_type(signature: u32, direction: Direction) -> bool {
    matches!(signature, 0x6d667431 | 0x6d667432)
        || matches!(
            (signature, direction),
            (0x6d414220, Direction::A2B) | (0x6d424120, Direction::B2A)
        )
}

fn parse_header(data: &[u8]) -> Option<Header> {
    let reader = Reader::new(data);
    let version = reader.be_u32(8)?;
    if !matches!(
        version,
        0x00000000
            | 0x02000000
            | 0x02100000
            | 0x02200000
            | 0x02300000
            | 0x02400000
            | 0x04000000
            | 0x04100000
            | 0x04200000
            | 0x04300000
            | 0x04400000
    ) {
        return None;
    }

    let profile_class = reader.be_u32(12)?;
    if !matches!(
        profile_class,
        0x73636e72 | 0x6d6e7472 | 0x70727472 | 0x6c696e6b | 0x73706163 | 0x61627374 | 0x6e6d636c
    ) {
        return None;
    }

    if reader.be_u32(64)? > 3 {
        return None;
    }

    let data_color_space_raw = reader.be_u32(16)?;
    let connection_space_raw = reader.be_u32(20)?;
    Some(Header {
        profile_class,
        data_color_space: map_data_color_space(data_color_space_raw)?,
        data_color_space_raw,
        connection_space: map_connection_space(connection_space_raw)?,
        connection_space_raw,
        illuminant: [
            reader.s15_fixed16(68)?,
            reader.s15_fixed16(72)?,
            reader.s15_fixed16(76)?,
        ],
    })
}

fn map_data_color_space(signature: u32) -> Option<ffi::skcms_Signature> {
    Some(match signature {
        0x434d594b => ffi::skcms_Signature::skcms_Signature_CMYK,
        0x47524159 => ffi::skcms_Signature::skcms_Signature_Gray,
        0x52474220 => ffi::skcms_Signature::skcms_Signature_RGB,
        0x4c616220 => ffi::skcms_Signature::skcms_Signature_Lab,
        0x58595a20 => ffi::skcms_Signature::skcms_Signature_XYZ,
        0x4c757620 => ffi::skcms_Signature::skcms_Signature_CIELUV,
        0x59436272 => ffi::skcms_Signature::skcms_Signature_YCbCr,
        0x59787920 => ffi::skcms_Signature::skcms_Signature_CIEYxy,
        0x48535620 => ffi::skcms_Signature::skcms_Signature_HSV,
        0x484c5320 => ffi::skcms_Signature::skcms_Signature_HLS,
        0x434d5920 => ffi::skcms_Signature::skcms_Signature_CMY,
        0x32434c52 => ffi::skcms_Signature::skcms_Signature_2CLR,
        0x33434c52 => ffi::skcms_Signature::skcms_Signature_3CLR,
        0x34434c52 => ffi::skcms_Signature::skcms_Signature_4CLR,
        0x35434c52 => ffi::skcms_Signature::skcms_Signature_5CLR,
        0x36434c52 => ffi::skcms_Signature::skcms_Signature_6CLR,
        0x37434c52 => ffi::skcms_Signature::skcms_Signature_7CLR,
        0x38434c52 => ffi::skcms_Signature::skcms_Signature_8CLR,
        0x39434c52 => ffi::skcms_Signature::skcms_Signature_9CLR,
        0x41434c52 => ffi::skcms_Signature::skcms_Signature_10CLR,
        0x42434c52 => ffi::skcms_Signature::skcms_Signature_11CLR,
        0x43434c52 => ffi::skcms_Signature::skcms_Signature_12CLR,
        0x44434c52 => ffi::skcms_Signature::skcms_Signature_13CLR,
        0x45434c52 => ffi::skcms_Signature::skcms_Signature_14CLR,
        0x46434c52 => ffi::skcms_Signature::skcms_Signature_15CLR,
        _ => return None,
    })
}

fn map_connection_space(signature: u32) -> Option<ffi::skcms_Signature> {
    match signature {
        XYZ_SIGNATURE => Some(ffi::skcms_Signature::skcms_Signature_XYZ),
        LAB_SIGNATURE => Some(ffi::skcms_Signature::skcms_Signature_Lab),
        _ => None,
    }
}

fn parse_rgb_matrix(tags: &TagTable<'_>, profile: &mut ffi::IccProfile) -> Option<()> {
    let tags = [tags.get(R_XYZ).ok()?, tags.get(G_XYZ).ok()?, tags.get(B_XYZ).ok()?];
    let [Some(red), Some(green), Some(blue)] = tags else {
        return Some(());
    };
    let red = icc_curve::parse_xyz(red.data)?;
    let green = icc_curve::parse_xyz(green.data)?;
    let blue = icc_curve::parse_xyz(blue.data)?;

    profile.to_xyzd50 = ffi::Matrix3x3 {
        vals: [
            [red[0], green[0], blue[0]],
            [red[1], green[1], blue[1]],
            [red[2], green[2], blue[2]],
        ],
    };
    profile.has_to_xyzd50 = matrix_columns_are_nonzero(&profile.to_xyzd50);
    Some(())
}

fn parse_gray_trc(tags: &TagTable<'_>, profile: &mut ffi::IccProfile) -> Option<()> {
    let Some(tag) = tags.get(K_TRC).ok()? else {
        return Some(());
    };
    let Some(curve) =
        icc_curve::parse_curve(tag.data, EmptyCurve::Reject).map(|parsed| parsed.curve)
    else {
        if tag.type_signature == u32::from_be_bytes(*b"curv")
            && Reader::new(tag.data).be_u32(8) == Some(0)
        {
            return Some(());
        }
        return None;
    };

    profile.trc_r = clone_curve(&curve);
    profile.trc_g = clone_curve(&curve);
    profile.trc_b = curve;
    profile.has_trc = true;
    Some(())
}

fn parse_rgb_trcs(tags: &TagTable<'_>, profile: &mut ffi::IccProfile) -> Option<()> {
    let tags = [tags.get(R_TRC).ok()?, tags.get(G_TRC).ok()?, tags.get(B_TRC).ok()?];
    let curves = [
        parse_top_level_curve(tags[0])?,
        parse_top_level_curve(tags[1])?,
        parse_top_level_curve(tags[2])?,
    ];
    match curves {
        [Some(red), Some(green), Some(blue)] => {
            profile.trc_r = red;
            profile.trc_g = green;
            profile.trc_b = blue;
            profile.has_trc = true;
            Some(())
        }
        _ => Some(()),
    }
}

fn parse_top_level_curve(tag: Option<crate::icc_reader::Tag<'_>>) -> Option<Option<ffi::Curve>> {
    let Some(tag) = tag else {
        return Some(None);
    };
    if tag.type_signature == u32::from_be_bytes(*b"curv")
        && Reader::new(tag.data).be_u32(8) == Some(0)
    {
        return Some(None);
    }
    icc_curve::parse_curve(tag.data, EmptyCurve::Reject)
        .map(|parsed| Some(parsed.curve))
}

fn parse_cicp(
    tags: &TagTable<'_>,
    header: &Header,
    profile: &mut ffi::IccProfile,
) -> Option<()> {
    let class_supports_cicp = matches!(
        header.profile_class,
        INPUT_CLASS_SIGNATURE | DISPLAY_CLASS_SIGNATURE
    );
    let space_supports_cicp = matches!(
        header.data_color_space_raw,
        RGB_SIGNATURE | YCBCR_SIGNATURE | XYZ_SIGNATURE
    );
    if !class_supports_cicp || !space_supports_cicp {
        return Some(());
    }

    let Some(tag) = tags.get(CICP).ok()? else {
        return Some(());
    };
    if tag.type_signature != CICP_TYPE_SIGNATURE {
        return Some(());
    }

    let reader = Reader::new(tag.data);
    let matrix_coefficients = reader.u8(10)?;
    if matrix_coefficients > 14 {
        return None;
    }
    profile.cicp = ffi::Cicp {
        color_primaries: reader.u8(8)?,
        transfer_characteristics: reader.u8(9)?,
        matrix_coefficients,
        video_full_range_flag: u8::from(reader.u8(11)? == 1),
    };
    profile.has_cicp = true;
    Some(())
}

fn matrix_columns_are_nonzero(matrix: &ffi::Matrix3x3) -> bool {
    (0..3).all(|column| (0..3).any(|row| matrix.vals[row][column] != 0.0))
}

fn diagonal_matrix(diagonal: [f32; 3]) -> ffi::Matrix3x3 {
    ffi::Matrix3x3 {
        vals: [
            [diagonal[0], 0.0, 0.0],
            [0.0, diagonal[1], 0.0],
            [0.0, 0.0, diagonal[2]],
        ],
    }
}

fn clone_curve(curve: &ffi::Curve) -> ffi::Curve {
    ffi::Curve {
        table_entries: curve.table_entries,
        parametric: curve.parametric,
        table_data: curve.table_data.clone(),
    }
}

fn empty_curve() -> ffi::Curve {
    ffi::Curve {
        table_entries: 0,
        parametric: ffi::TransferFunction::default(),
        table_data: Vec::new(),
    }
}

fn empty_a2b() -> ffi::A2B {
    ffi::A2B {
        input_curves: Vec::new(),
        input_channels: 0,
        grid_points: [0; 4],
        grid_data: Vec::new(),
        is_16bit_grid: false,
        matrix_curves: Vec::new(),
        matrix: ffi::Matrix3x3 { vals: [[0.0; 3]; 3] },
        matrix_bias: [0.0; 3],
        matrix_channels: 0,
        output_curves: Vec::new(),
        output_channels: 0,
    }
}

fn empty_b2a() -> ffi::B2A {
    ffi::B2A {
        input_curves: Vec::new(),
        input_channels: 0,
        matrix: ffi::Matrix3x3 { vals: [[0.0; 3]; 3] },
        matrix_bias: [0.0; 3],
        matrix_curves: Vec::new(),
        matrix_channels: 0,
        grid_points: [0; 4],
        grid_data: Vec::new(),
        is_16bit_grid: false,
        output_curves: Vec::new(),
        output_channels: 0,
    }
}

fn empty_profile(
    data_color_space: ffi::skcms_Signature,
    connection_space: ffi::skcms_Signature,
) -> ffi::IccProfile {
    ffi::IccProfile {
        data_color_space,
        connection_space,
        to_xyzd50: ffi::Matrix3x3 { vals: [[0.0; 3]; 3] },
        has_to_xyzd50: false,
        trc_r: empty_curve(),
        trc_g: empty_curve(),
        trc_b: empty_curve(),
        has_trc: false,
        cicp: ffi::Cicp {
            color_primaries: 0,
            transfer_characteristics: 0,
            matrix_coefficients: 0,
            video_full_range_flag: 0,
        },
        has_cicp: false,
        a2b: empty_a2b(),
        has_a2b: false,
        b2a: empty_b2a(),
        has_b2a: false,
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::path::PathBuf;

    struct ProfileBuilder {
        data: Vec<u8>,
        tags: Vec<(u32, Vec<u8>)>,
    }

    impl ProfileBuilder {
        fn rgb() -> Self {
            let mut data = vec![0; 132];
            data[8..12].copy_from_slice(&0x02100000u32.to_be_bytes());
            data[12..16].copy_from_slice(b"mntr");
            data[16..20].copy_from_slice(b"RGB ");
            data[20..24].copy_from_slice(b"XYZ ");
            data[36..40].copy_from_slice(b"acsp");
            data[68..72].copy_from_slice(&fixed(0.9642));
            data[72..76].copy_from_slice(&fixed(1.0));
            data[76..80].copy_from_slice(&fixed(0.8249));
            Self { data, tags: Vec::new() }
        }

        fn tag(mut self, signature: &[u8; 4], data: Vec<u8>) -> Self {
            self.tags.push((u32::from_be_bytes(*signature), data));
            self
        }

        fn build(mut self) -> Vec<u8> {
            let table_end = 132 + self.tags.len() * 12;
            self.data.resize(table_end, 0);
            self.data[128..132].copy_from_slice(&(self.tags.len() as u32).to_be_bytes());

            let mut offset = table_end;
            for (index, (signature, tag)) in self.tags.into_iter().enumerate() {
                let record = 132 + index * 12;
                self.data[record..record + 4].copy_from_slice(&signature.to_be_bytes());
                self.data[record + 4..record + 8]
                    .copy_from_slice(&(offset as u32).to_be_bytes());
                self.data[record + 8..record + 12]
                    .copy_from_slice(&(tag.len() as u32).to_be_bytes());
                self.data.extend(&tag);
                while self.data.len() % 4 != 0 {
                    self.data.push(0);
                }
                offset = self.data.len();
            }
            let size = self.data.len() as u32;
            self.data[0..4].copy_from_slice(&size.to_be_bytes());
            self.data
        }
    }

    fn fixed(value: f32) -> [u8; 4] {
        ((value * 65536.0) as i32).to_be_bytes()
    }

    fn xyz(x: f32, y: f32, z: f32) -> Vec<u8> {
        let mut data = Vec::from(*b"XYZ \0\0\0\0");
        data.extend(fixed(x));
        data.extend(fixed(y));
        data.extend(fixed(z));
        data
    }

    fn gamma(value: u16) -> Vec<u8> {
        let mut data = Vec::from(*b"curv\0\0\0\0\0\0\0\x01");
        data.extend(value.to_be_bytes());
        data
    }

    fn rgb_matrix_profile() -> Vec<u8> {
        ProfileBuilder::rgb()
            .tag(b"rXYZ", xyz(1.0, 0.0, 0.0))
            .tag(b"gXYZ", xyz(0.0, 1.0, 0.0))
            .tag(b"bXYZ", xyz(0.0, 0.0, 1.0))
            .tag(b"rTRC", gamma(0x0233))
            .tag(b"gTRC", gamma(0x0233))
            .tag(b"bTRC", gamma(0x0233))
            .tag(b"cicp", Vec::from(*b"cicp\0\0\0\0\x09\x10\0\x01"))
            .build()
    }

    fn lut16() -> Vec<u8> {
        let mut data = vec![0; 52];
        data[0..4].copy_from_slice(b"mft2");
        data[8] = 3;
        data[9] = 3;
        data[10] = 2;
        for index in 0..3 {
            data[12 + (index * 3 + index) * 4..16 + (index * 3 + index) * 4]
                .copy_from_slice(&fixed(1.0));
        }
        data[48..50].copy_from_slice(&2u16.to_be_bytes());
        data[50..52].copy_from_slice(&2u16.to_be_bytes());
        for index in 0..(3 * 2 + 2usize.pow(3) * 3 + 3 * 2) {
            data.extend((index as u16).to_be_bytes());
        }
        data
    }

    fn resource_profile(name: &str) -> ffi::IccProfile {
        let path = PathBuf::from(std::env::var_os("TEST_SRCDIR").unwrap())
            .join(std::env::var_os("TEST_WORKSPACE").unwrap())
            .join("resources/icc_profiles")
            .join(name);
        let data = std::fs::read(&path)
            .unwrap_or_else(|error| panic!("failed to read {}: {error}", path.display()));
        parse_profile(&data).unwrap_or_else(|| panic!("failed to parse {}", path.display()))
    }

    #[test]
    fn matrix_trc_and_cicp_survive_ffi_entry_point() {
        let data = rgb_matrix_profile();
        let inline = parse_profile(&data).unwrap();
        let mut bridged = empty_profile(
            ffi::skcms_Signature::skcms_Signature_RGB,
            ffi::skcms_Signature::skcms_Signature_XYZ,
        );
        assert!(crate::parse_icc_profile(&data, &mut bridged));

        assert_eq!(inline.data_color_space, bridged.data_color_space);
        assert_eq!(inline.connection_space, bridged.connection_space);
        assert_eq!(inline.has_to_xyzd50, bridged.has_to_xyzd50);
        assert_eq!(inline.to_xyzd50.vals, bridged.to_xyzd50.vals);
        assert_eq!(inline.has_trc, bridged.has_trc);
        assert_eq!(inline.trc_r.parametric.g, bridged.trc_r.parametric.g);
        assert_eq!(inline.trc_g.parametric.g, bridged.trc_g.parametric.g);
        assert_eq!(inline.trc_b.parametric.g, bridged.trc_b.parametric.g);
        assert_eq!(inline.has_cicp, bridged.has_cicp);
        assert_eq!(inline.cicp.color_primaries, bridged.cicp.color_primaries);
        assert_eq!(
            inline.cicp.transfer_characteristics,
            bridged.cicp.transfer_characteristics
        );
        assert_eq!(inline.cicp.matrix_coefficients, bridged.cicp.matrix_coefficients);
        assert_eq!(
            inline.cicp.video_full_range_flag,
            bridged.cicp.video_full_range_flag
        );
    }

    #[test]
    fn lut_priority_and_conversion_survive_ffi_entry_point() {
        let data = ProfileBuilder::rgb()
            .tag(b"A2B1", lut16())
            .tag(b"B2A2", lut16())
            .build();
        let inline = parse_profile(&data).unwrap();
        let mut bridged = empty_profile(
            ffi::skcms_Signature::skcms_Signature_RGB,
            ffi::skcms_Signature::skcms_Signature_XYZ,
        );
        assert!(crate::parse_icc_profile(&data, &mut bridged));

        assert!(inline.has_a2b);
        assert!(inline.has_b2a);
        assert_eq!(inline.a2b.input_channels, bridged.a2b.input_channels);
        assert_eq!(inline.a2b.output_channels, bridged.a2b.output_channels);
        assert_eq!(inline.a2b.grid_points, bridged.a2b.grid_points);
        assert_eq!(inline.a2b.grid_data, bridged.a2b.grid_data);
        assert_eq!(inline.a2b.matrix.vals, bridged.a2b.matrix.vals);
        assert_eq!(inline.b2a.input_channels, bridged.b2a.input_channels);
        assert_eq!(inline.b2a.output_channels, bridged.b2a.output_channels);
        assert_eq!(inline.b2a.grid_points, bridged.b2a.grid_points);
        assert_eq!(inline.b2a.grid_data, bridged.b2a.grid_data);
        assert_eq!(inline.b2a.matrix.vals, bridged.b2a.matrix.vals);
    }

    #[test]
    fn incomplete_or_malformed_rgb_tags_do_not_form_profile_data() {
        let incomplete = ProfileBuilder::rgb()
            .tag(b"rXYZ", xyz(1.0, 0.0, 0.0))
            .tag(b"rTRC", gamma(0x0233))
            .build();
        let parsed = parse_profile(&incomplete).unwrap();
        assert!(!parsed.has_to_xyzd50);
        assert!(!parsed.has_trc);

        let malformed = ProfileBuilder::rgb()
            .tag(b"rTRC", Vec::from(*b"nope\0\0\0\0\0\0\0\0"))
            .build();
        assert!(parse_profile(&malformed).is_none());

        let malformed_colorants = ProfileBuilder::rgb()
            .tag(b"rXYZ", Vec::from(*b"XYZ \0\0\0\0\0\0\0\0"))
            .tag(b"gXYZ", xyz(0.0, 1.0, 0.0))
            .tag(b"bXYZ", xyz(0.0, 0.0, 1.0))
            .build();
        assert!(parse_profile(&malformed_colorants).is_none());
    }

    #[test]
    fn gray_profile_uses_header_illuminant() {
        let mut builder = ProfileBuilder::rgb();
        builder.data[16..20].copy_from_slice(b"GRAY");
        let data = builder.tag(b"kTRC", gamma(0x0100)).build();
        let profile = parse_profile(&data).unwrap();
        assert!(profile.has_to_xyzd50);
        assert!(profile.has_trc);
        assert!((profile.to_xyzd50.vals[0][0] - 0.9642).abs() < 0.0001);
        assert_eq!(profile.to_xyzd50.vals[1][1], 1.0);
        assert!((profile.to_xyzd50.vals[2][2] - 0.8249).abs() < 0.0001);
        assert_eq!(profile.trc_r.parametric.g, 1.0);
        assert_eq!(profile.trc_g.parametric.g, 1.0);
        assert_eq!(profile.trc_b.parametric.g, 1.0);
    }

    #[test]
    fn all_zero_version_is_accepted_narrowly() {
        let mut zero_version = rgb_matrix_profile();
        zero_version[8..12].copy_from_slice(&0u32.to_be_bytes());
        assert!(parse_profile(&zero_version).is_some());
        let mut bridged = empty_profile(
            ffi::skcms_Signature::skcms_Signature_RGB,
            ffi::skcms_Signature::skcms_Signature_XYZ,
        );
        assert!(crate::parse_icc_profile(&zero_version, &mut bridged));

        let mut invalid_v0_version = rgb_matrix_profile();
        invalid_v0_version[8..12].copy_from_slice(&0x00100000u32.to_be_bytes());
        assert!(parse_profile(&invalid_v0_version).is_none());
    }

    #[test]
    fn header_and_cicp_validation_match_current_contract() {
        let mut invalid_version = rgb_matrix_profile();
        invalid_version[8..12].copy_from_slice(&0x05000000u32.to_be_bytes());
        assert!(parse_profile(&invalid_version).is_none());

        let invalid_cicp = ProfileBuilder::rgb()
            .tag(b"cicp", Vec::from(*b"cicp\0\0\0\0\x01\x0d\xff\x01"))
            .build();
        assert!(parse_profile(&invalid_cicp).is_none());
    }

    #[test]
    fn parses_checked_in_icc_profile_corpus() {
        for name in [
            "AdobeRGB1998.icc",
            "HP_Z32x.icc",
            "HP_ZR30w.icc",
            "apng19.icc",
            "mu_gray.icc",
            "pq_hdr.icc",
            "srgb_lab_pcs.icc",
            "swapped.icc",
            "tiles.icc",
            "upperLeft.icc",
            "upperRight.icc",
        ] {
            resource_profile(name);
        }

        assert!(resource_profile("apng19.icc").has_a2b);

        let gray = resource_profile("mu_gray.icc");
        assert!(gray.has_to_xyzd50);
        assert!(gray.has_trc);

        for name in ["swapped.icc", "tiles.icc"] {
            let profile = resource_profile(name);
            assert!(profile.has_trc);
            assert_eq!(profile.trc_r.table_entries, 1024);
            assert_eq!(profile.trc_r.table_data.len(), 2048);
        }

        let pq = resource_profile("pq_hdr.icc");
        assert!(pq.has_a2b);
        assert!(pq.has_b2a);
        assert_eq!(pq.b2a.output_channels, 0);
    }
}
