# Copyright 2026 Google LLC
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Inclusive sample shares from an xctrace time-profile export, for the SkoreText bench
import re
import sys
import xml.etree.ElementTree as ET

tree = ET.parse(sys.argv[1])
root = tree.getroot()
frames = {}
backtraces = {}
tagged = {}


def frame_name(el):
    if el.get('ref'):
        return frames[el.get('ref')]
    name = el.get('name', '?')
    frames[el.get('id')] = name
    return name


def bt_frames(el):
    if el.get('ref'):
        return backtraces[el.get('ref')]
    names = [frame_name(f) for f in el.findall('frame')]
    backtraces[el.get('id')] = names
    return names


patterns = {
    'ParagraphImpl::layout': r'ParagraphImpl::layout\(',
    'ParagraphBuilderImpl::Build': r'ParagraphBuilderImpl::Build',
    '  computeCodeUnitProperties (SkUnicode)': r'computeCodeUnitProperties',
    '  shapeTextIntoEndlessLine': r'shapeTextIntoEndlessLine',
    '    CoreTextLineShaper::shape': r'CoreTextLineShaper::shape\(',
    '      CoreTextAttrStringBuilder (all)': r'CoreTextAttrStringBuilder::',
    '      CTTypesetterCreateWithAttributedString': r'^CTTypesetterCreateWithAttributedString',
    '      CTTypesetterCreateLine': r'^CTTypesetterCreateLine',
    '    applySpacingAndBuildClusterTable': r'applySpacingAndBuildClusterTable',
    '  breakShapedTextIntoLines': r'breakShapedTextIntoLines',
    '  formatLines': r'ParagraphImpl::formatLines',
}
counts = {k: 0 for k in patterns}
total = 0
for row in root.iter('row'):
    tb = row.find('tagged-backtrace')
    if tb is None:
        continue
    if tb.get('ref'):
        names = tagged[tb.get('ref')]
    else:
        names = bt_frames(tb.find('backtrace'))
        tagged[tb.get('id')] = names
    total += 1
    for k, p in patterns.items():
        if any(re.search(p, n) for n in names):
            counts[k] += 1
print(f'samples: {total}')
for k, v in counts.items():
    print(f'{k:45s} {v:6d}  {100.0 * v / total:5.1f}%')

# Callee histogram below shapeTextIntoEndlessLine for samples outside the typesetter and builder
from collections import Counter
callees = Counter()
tagged_counts = Counter()
for row in root.iter('row'):
    tb = row.find('tagged-backtrace')
    if tb is None:
        continue
    key = tb.get('ref') or tb.get('id')
    tagged_counts[key] += 1
for key, n in tagged_counts.items():
    names = tagged[key]
    idx = [i for i, f in enumerate(names) if 'shapeTextIntoEndlessLine' in f]
    if not idx:
        continue
    i = idx[0]
    chain = ' <- '.join(f[:60] for f in names[max(0, i - 3):i])
    callees[chain] += n
for chain, n in callees.most_common(25):
    print(f'{n:6d}  {chain}')

print('--- categories inside shapeTextIntoEndlessLine ---')
cats = Counter()
order = [('CT shaping: CTTypesetterCreateWithAttributedString',
          r'^CTTypesetterCreateWithAttributedString'),
         ('CTLine creation', r'TypesetterCreateLine'),
         ('CoreTextAttrStringBuilder', r'CoreTextAttrStringBuilder::'),
         ('buildClusterTable / spacing', r'buildClusterTable|applySpacingAndBuildClusterTable'),
         ('CFRelease of CT objects', r'^_?CFRelease$')]
for key, n in tagged_counts.items():
    names = tagged[key]
    if not any('shapeTextIntoEndlessLine' in f for f in names):
        continue
    for label, pat in order:
        if any(re.search(pat, f) for f in names):
            cats[label] += n
            break
    else:
        if any('CoreTextLineShaper::shapeText' in f for f in names):
            cats['CTRun -> Run conversion (rest of shapeTextRange/Region)'] += n
        else:
            cats['other'] += n
lay = counts['ParagraphImpl::layout']
for label, n in cats.most_common():
    print(f'{label:58s} {n:6d}  {100.0 * n / lay:5.1f}% of layout')
