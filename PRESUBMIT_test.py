#!/usr/bin/env python3
# Copyright 2023 Google LLC
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import unittest
from unittest import mock

import PRESUBMIT

from PRESUBMIT_test_mocks import MockFile, MockAffectedFile
from PRESUBMIT_test_mocks import MockInputApi, MockOutputApi


class ReleaseNotesTest(unittest.TestCase):
    def testNoEditTopReleaseNotesNoWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('README.chromium', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckTopReleaseNotesChanged(
            mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))

    def testUpdateTopReleaseNotesIssuesWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('RELEASE_NOTES.md', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckTopReleaseNotesChanged(
            mock_input_api, mock_output_api)

        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], mock_output_api.PresubmitPromptWarning, 'Not a warning')
        self.assertTrue(results[0].message.startswith(
            'Do not edit RELEASE_NOTES.md'))

    def testUpdateTopReleaseNotesNoWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('RELEASE_NOTES.md', ''),
            MockFile('relnotes/deleted_note.md', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckTopReleaseNotesChanged(
            mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))

    def testUpdatePublicHeaderAndNoReleaseNoteGeneratesWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('include/core/SkDrawable.h', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckReleaseNotesForPublicAPI(
            mock_input_api, mock_output_api)

        self.assertEqual(1, len(results))
        self.assertIsInstance(
            results[0], mock_output_api.PresubmitPromptWarning, 'Not a warning')

    def testUpdatePublicHeaderAndReleaseNoteGeneratesNoWarning(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('include/core/SkDrawable.h', ''),
            MockFile('relnotes/new_note.md', ''),
        ]

        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckReleaseNotesForPublicAPI(
            mock_input_api, mock_output_api)

        self.assertEqual(0, len(results))


class RunCommandAndCheckDiffTest(unittest.TestCase):
    def setUp(self):
        self.foo_file = MockAffectedFile('foo.txt', new_contents=['foo'])
        self.bar_file = MockAffectedFile('bar.txt', new_contents=['bar'])

        self.mock_input_api = MockInputApi()
        self.mock_input_api.files = [self.foo_file, self.bar_file]
        self.mock_output_api = MockOutputApi()

    def setContents(self, file, contents):
        file._new_contents = contents

    @mock.patch('subprocess.check_output')
    def testNoChangesReturnsNoResults(self, mock_subprocess):
        results = PRESUBMIT._RunCommandAndCheckDiff(self.mock_output_api, [], [])
        self.assertEqual(results, [])

    @mock.patch('subprocess.check_output')
    def testChangingIrrelevantFilesReturnsNoResults(self, mock_subprocess):
        mock_subprocess.side_effect = lambda *args, **kwargs: self.setContents(self.bar_file, ['foo'])
        results = PRESUBMIT._RunCommandAndCheckDiff(
            self.mock_output_api, ['cmd'], [self.foo_file],
        )
        self.assertEqual(results, [])

    @mock.patch('subprocess.check_output')
    def testChangingRelevantFilesReturnsDiff(self, mock_subprocess):
        mock_subprocess.side_effect = lambda *args, **kwargs: self.setContents(self.foo_file, ['bar'])
        results = PRESUBMIT._RunCommandAndCheckDiff(
            self.mock_output_api, ['cmd'], [self.foo_file],
        )
        self.assertEqual(len(results), 1)
        self.assertEqual(results[0].message,
"""Diffs found after running "cmd":

--- foo.txt
+++ foo.txt
@@ -1 +1 @@
-foo
+bar

Please commit or discard the above changes.""")


class MacroStyleTest(unittest.TestCase):
    def testValidMacrosPass(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('src/gpu/ganesh/gl/GrGLDefines.h', [
                '#ifndef GrGLDefines_DEFINED',
                '#define GrGLDefines_DEFINED',
                '#define GR_GL_TEXTURE0 0xdeadbeef',
                '#if GR_GO_SLOWER',
                '#endif',
                '#endif',
            ]),
        ]
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckMacroStyle(mock_input_api, mock_output_api)
        self.assertEqual(0, len(results))

    def testGaneshIfdefGeneratesError(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('src/gpu/ganesh/GrFoo.cpp', [
                '#ifdef GR_GO_SLOWER',
                '#endif',
            ]),
        ]
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckMacroStyle(mock_input_api, mock_output_api)
        self.assertEqual(1, len(results))
        self.assertIsInstance(results[0], mock_output_api.PresubmitError)
        self.assertIn('Ganesh prefers `#if GR_GO_SLOWER`', results[0].message)

    def testLowercaseAndUnprefixedHeaderMacrosWarn(self):
        mock_input_api = MockInputApi()
        mock_input_api.files = [
            MockFile('src/gpu/ganesh/gl/GrGLUtil.h', [
                '#define my_macro 1',
                '#define GR_TEXTURE0 0xdeadbeef',
            ]),
        ]
        mock_output_api = MockOutputApi()
        results = PRESUBMIT._CheckMacroStyle(mock_input_api, mock_output_api)
        self.assertEqual(1, len(results))
        self.assertIsInstance(results[0], mock_output_api.PresubmitPromptWarning)
        self.assertIn('ALL_CAPS_WITH_UNDERSCORES (my_macro)', results[0].message)
        self.assertIn('prefixed with SK or GR (my_macro)', results[0].message)
        self.assertIn('prefixed GR_GL (GR_TEXTURE0)', results[0].message)


if __name__ == '__main__':
    unittest.main()
