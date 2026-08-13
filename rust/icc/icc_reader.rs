// Copyright 2026 Google LLC
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#![cfg_attr(not(test), allow(dead_code))]

use std::ops::Range;

pub(crate) const MAX_PROFILE_SIZE: usize = 10 * 1024 * 1024;
const HEADER_SIZE: usize = 128;
const TAG_COUNT_OFFSET: usize = HEADER_SIZE;
const TAG_TABLE_OFFSET: usize = TAG_COUNT_OFFSET + 4;
const TAG_RECORD_SIZE: usize = 12;
const ACSP_SIGNATURE: u32 = u32::from_be_bytes(*b"acsp");

#[derive(Clone, Copy)]
pub(crate) struct Reader<'a> {
    data: &'a [u8],
}

impl<'a> Reader<'a> {
    pub(crate) fn new(data: &'a [u8]) -> Self {
        Self { data }
    }

    pub(crate) fn len(&self) -> usize {
        self.data.len()
    }

    pub(crate) fn bytes(&self, offset: usize, len: usize) -> Option<&'a [u8]> {
        let end = offset.checked_add(len)?;
        self.data.get(offset..end)
    }

    pub(crate) fn u8(&self, offset: usize) -> Option<u8> {
        self.data.get(offset).copied()
    }

    pub(crate) fn be_u16(&self, offset: usize) -> Option<u16> {
        Some(u16::from_be_bytes(self.bytes(offset, 2)?.try_into().ok()?))
    }

    pub(crate) fn be_u32(&self, offset: usize) -> Option<u32> {
        Some(u32::from_be_bytes(self.bytes(offset, 4)?.try_into().ok()?))
    }

    pub(crate) fn be_i32(&self, offset: usize) -> Option<i32> {
        Some(i32::from_be_bytes(self.bytes(offset, 4)?.try_into().ok()?))
    }

    pub(crate) fn s15_fixed16(&self, offset: usize) -> Option<f32> {
        Some(self.be_i32(offset)? as f32 / 65536.0)
    }
}

pub(crate) fn align_to_4(value: usize) -> Option<usize> {
    value.checked_add(3).map(|value| value & !3)
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub(crate) struct Tag<'a> {
    pub(crate) signature: u32,
    pub(crate) type_signature: u32,
    pub(crate) data: &'a [u8],
}

pub(crate) struct TagTable<'a> {
    reader: Reader<'a>,
    records: Range<usize>,
}

impl<'a> TagTable<'a> {
    pub(crate) fn parse(data: &'a [u8]) -> Option<Self> {
        if data.len() >= MAX_PROFILE_SIZE {
            return None;
        }

        let input = Reader::new(data);
        let profile_size = input.be_u32(0)? as usize;
        if profile_size < TAG_TABLE_OFFSET || profile_size > data.len() {
            return None;
        }

        let reader = Reader::new(data.get(..profile_size)?);
        if reader.be_u32(36)? != ACSP_SIGNATURE {
            return None;
        }

        let tag_count = reader.be_u32(TAG_COUNT_OFFSET)? as usize;
        let table_len = tag_count.checked_mul(TAG_RECORD_SIZE)?;
        let table_end = TAG_TABLE_OFFSET.checked_add(table_len)?;
        reader.bytes(0, table_end)?;

        Some(Self {
            reader,
            records: TAG_TABLE_OFFSET..table_end,
        })
    }

    pub(crate) fn get(&self, signature: u32) -> Result<Option<Tag<'a>>, ()> {
        let mut found = None;
        for record_offset in self.records.clone().step_by(TAG_RECORD_SIZE) {
            if self.reader.be_u32(record_offset).ok_or(())? != signature {
                continue;
            }

            let tag_offset = self.reader.be_u32(record_offset + 4).ok_or(())? as usize;
            let tag_len = self.reader.be_u32(record_offset + 8).ok_or(())? as usize;
            let data = self.reader.bytes(tag_offset, tag_len).ok_or(())?;
            let type_signature = Reader::new(data).be_u32(0).ok_or(())?;
            found = Some(Tag {
                signature,
                type_signature,
                data,
            });
        }
        Ok(found)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    const TEST_TAG: u32 = u32::from_be_bytes(*b"rXYZ");
    const TEST_TYPE: u32 = u32::from_be_bytes(*b"XYZ ");

    fn profile_with_tag(tag_offset: u32, tag_len: u32) -> Vec<u8> {
        let mut data = vec![0; 160];
        let profile_size = data.len() as u32;
        data[0..4].copy_from_slice(&profile_size.to_be_bytes());
        data[36..40].copy_from_slice(b"acsp");
        data[128..132].copy_from_slice(&1u32.to_be_bytes());
        data[132..136].copy_from_slice(&TEST_TAG.to_be_bytes());
        data[136..140].copy_from_slice(&tag_offset.to_be_bytes());
        data[140..144].copy_from_slice(&tag_len.to_be_bytes());
        data[144..148].copy_from_slice(&TEST_TYPE.to_be_bytes());
        data
    }

    #[test]
    fn reader_checks_ranges_and_endianness() {
        let data = [0x12, 0x34, 0x56, 0x78, 0xff, 0xff, 0x00, 0x00];
        let reader = Reader::new(&data);

        assert_eq!(reader.u8(0), Some(0x12));
        assert_eq!(reader.be_u16(0), Some(0x1234));
        assert_eq!(reader.be_u32(0), Some(0x12345678));
        assert_eq!(reader.be_i32(4), Some(-65536));
        assert_eq!(reader.s15_fixed16(4), Some(-1.0));
        assert_eq!(reader.bytes(6, 2), Some(&data[6..8]));
        assert_eq!(reader.bytes(7, 2), None);
        assert_eq!(reader.bytes(usize::MAX, 2), None);
    }

    #[test]
    fn alignment_is_checked() {
        assert_eq!(align_to_4(0), Some(0));
        assert_eq!(align_to_4(1), Some(4));
        assert_eq!(align_to_4(4), Some(4));
        assert_eq!(align_to_4(usize::MAX), None);
    }

    #[test]
    fn tag_table_resolves_bounded_tag() {
        let data = profile_with_tag(144, 16);
        let tags = TagTable::parse(&data).unwrap();
        let tag = tags.get(TEST_TAG).unwrap().unwrap();

        assert_eq!(tag.signature, TEST_TAG);
        assert_eq!(tag.type_signature, TEST_TYPE);
        assert_eq!(tag.data, &data[144..160]);
    }

    #[test]
    fn tag_table_rejects_invalid_envelope() {
        assert!(TagTable::parse(&[]).is_none());

        let mut invalid_signature = profile_with_tag(144, 16);
        invalid_signature[36..40].copy_from_slice(b"nope");
        assert!(TagTable::parse(&invalid_signature).is_none());

        let mut truncated_table = profile_with_tag(144, 16);
        truncated_table[128..132].copy_from_slice(&3u32.to_be_bytes());
        truncated_table.truncate(144);
        assert!(TagTable::parse(&truncated_table).is_none());

        let mut oversized_declaration = profile_with_tag(144, 16);
        oversized_declaration[0..4].copy_from_slice(&161u32.to_be_bytes());
        assert!(TagTable::parse(&oversized_declaration).is_none());

        let mut undersized_declaration = profile_with_tag(144, 16);
        undersized_declaration[0..4].copy_from_slice(&143u32.to_be_bytes());
        assert!(TagTable::parse(&undersized_declaration).is_none());
    }

    #[test]
    fn selected_tag_must_be_in_bounds() {
        let data = profile_with_tag(156, 8);
        let tags = TagTable::parse(&data).unwrap();
        assert!(tags.get(TEST_TAG).is_err());

        let mut outside_declared_size = profile_with_tag(144, 16);
        outside_declared_size.extend([0; 16]);
        outside_declared_size[0..4].copy_from_slice(&148u32.to_be_bytes());
        let tags = TagTable::parse(&outside_declared_size).unwrap();
        assert!(tags.get(TEST_TAG).is_err());
    }

    #[test]
    fn malformed_unknown_tag_is_ignored() {
        let data = profile_with_tag(u32::MAX, u32::MAX);
        let tags = TagTable::parse(&data).unwrap();
        assert!(tags
            .get(u32::from_be_bytes(*b"cicp"))
            .unwrap()
            .is_none());
    }
}
