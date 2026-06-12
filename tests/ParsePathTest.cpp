/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/utils/SkParsePath.h"
#include "src/core/SkGeometry.h"
#include "tests/Test.h"

#include <array>
#include <cstddef>
#include <regex>

static void test_to_from(skiatest::Reporter* reporter,
                         const SkPath& path,
                         bool allow_relative_drift = false) {
    for (auto encoding :
         {SkParsePath::PathEncoding::Absolute, SkParsePath::PathEncoding::Relative}) {
        skiatest::ReporterContext c(
                reporter,
                encoding == SkParsePath::PathEncoding::Absolute ? "absolute" : "relative");
        SkString str1 = SkParsePath::ToSVGString(path, encoding);

        auto path2 = SkParsePath::FromSVGString(str1.c_str());
        REPORTER_ASSERT(reporter, path2.has_value());

        SkString str2 = SkParsePath::ToSVGString(*path2, encoding);

        if (str1 != str2) {
            bool is_nearly_match = false;
            if (allow_relative_drift && encoding == SkParsePath::PathEncoding::Relative) {
                is_nearly_match = true;
                // A relative path may have slightly different line segments in Conic shapes when
                // they are quadified. We test them by comparing the SVG path string piece by piece
                // instead of as a whole, so we can compare numbers within a tolerance.
                std::regex pattern(
                        R"re(([+-]?(?:\d+\.\d+|\.\d+|\d+)(?:[eE][+-]?\d+)?)|[a-zA-Z ,])re");
                std::string_view sv1(str1.c_str());
                std::string_view sv2(str2.c_str());
                std::cmatch m1, m2;
                auto it1 = sv1.cbegin();
                auto it2 = sv2.cbegin();
                while (std::regex_search(it1,
                                         sv1.cend(),
                                         m1,
                                         pattern,
                                         std::regex_constants::match_continuous) &&
                       std::regex_search(it2,
                                         sv2.cend(),
                                         m2,
                                         pattern,
                                         std::regex_constants::match_continuous)) {
                    // Advance both iterators to the end of their match.
                    it1 = m1[0].second;
                    it2 = m2[0].second;
                    if (m1.str() != m2.str()) {
                        auto cstr1 = m1.str().c_str();
                        auto cstr2 = m2.str().c_str();
                        auto ch1 = cstr1[0];
                        auto ch2 = cstr2[0];
                        bool are_nums = !(std::isalpha(ch1) || std::isalpha(ch2) ||
                                          std::isspace(ch1) || std::isspace(ch2));
                        is_nearly_match &= are_nums;
                        // Numerical matches are tested against Skia's epsilon threshold.
                        if (are_nums) {
                            double n1 = std::atof(cstr1);
                            double n2 = std::atof(cstr2);
                            bool nearly_equal = SkScalarNearlyEqual(n1, n2);
                            REPORTER_ASSERT(reporter,
                                            nearly_equal,
                                            "%s (pos: %ld, len: %lu) !~= %s (pos: %ld, len: %lu)",
                                            cstr1,
                                            it1 - sv1.cbegin(),
                                            sv1.size(),
                                            cstr2,
                                            it2 - sv2.cbegin(),
                                            sv2.size());
                            is_nearly_match &= nearly_equal;
                        }
                        // Any non-number non-matches should just fail.
                        REPORTER_ASSERT(reporter,
                                        are_nums,
                                        "%s (pos: %ld, len: %lu) != %s (pos: %ld, len: %lu)",
                                        cstr1,
                                        it1 - sv1.cbegin(),
                                        sv1.size(),
                                        cstr2,
                                        it2 - sv2.cbegin(),
                                        sv2.size());
                    }
                }
                REPORTER_ASSERT(reporter,
                                it1 == sv1.cend(),
                                "pos: %ld, remaining: %s",
                                it1 - str1.c_str(),
                                it1);
                REPORTER_ASSERT(reporter,
                                it2 == sv2.cend(),
                                "pos: %ld, remaining: %s",
                                it2 - str2.c_str(),
                                it2);
            }
            REPORTER_ASSERT(reporter,
                            str1 == str2 || is_nearly_match,
                            "str1: %s\n\nstr2: %s",
                            str1.c_str(),
                            str2.c_str());
        }
    }
#if 0  // closed paths are not equal, the iter explicitly gives the closing
       // edge, even if it is not in the path.
    REPORTER_ASSERT(reporter, path == path2);
    if (path != path2) {
        SkDebugf("str1=%s\nstr2=%s\n", str.c_str(), str2.c_str());
    }
#endif
}

static struct {
    const char* fStr;
    const SkRect fBounds;
} gRec[] = {
        {"M1,1 l-2.58-2.828-3.82-0.113, 1.9-3.3223-1.08-3.6702, 3.75,0.7744,3.16-2.1551,"
         "0.42,3.8008,3.02,2.3384-3.48,1.574-1.29,3.601z",
         {-5.39999962f, -10.3142f, 5.77000046f, 1.f}},
        {"", {0, 0, 0, 0}},
        {"M0,0L10,10", {0, 0, SkIntToScalar(10), SkIntToScalar(10)}},
        {"M-5.5,-0.5 Q 0 0 6,6.50", {-5.5f, -0.5f, 6, 6.5f}},
        {"M0,0 l10,0 0,10 z m5,5 l5,5 z", {0, 0, 10, 10}},
        {"M5,5 h10 v0 v10 h-10 z", {5, 5, 15, 15}},
        {"M-2,-2 l4,0 0,4 -4,0 z m1,1 l2,2 z", {-2, -2, 2, 2}},
        {"M1,1 l0,0 h0 v0 m5,5 m1,1 l2,2", {1.f, 1.f, 9.f, 9.f}},
};

DEF_TEST(ParsePath, reporter) {
    for (size_t i = 0; i < std::size(gRec); i++) {
        skiatest::ReporterContext c(reporter, SkStringPrintf("gRec[%lu]", i));
        auto path = SkParsePath::FromSVGString(gRec[i].fStr);
        REPORTER_ASSERT(reporter, path.has_value());
        const SkRect& expectedBounds = gRec[i].fBounds;
        const SkRect& pathBounds = path->getBounds();
        REPORTER_ASSERT(reporter, expectedBounds == pathBounds);

        test_to_from(reporter, *path);
    }

    SkRect r;
    r.setLTRB(0, 0, 10, 10.5f);
    SkPathBuilder p;
    p.addRect(r);
    {
        skiatest::ReporterContext c(reporter, "after rect");
        test_to_from(reporter, p.snapshot());
    }
    p.addOval(r);
    {
        skiatest::ReporterContext c(reporter, "after oval");
        test_to_from(reporter, p.snapshot(), /* allow_relative_drift= */ true);
    }
    p.addRRect(SkRRect::MakeRectXY(r, 4, 4.5));
    {
        skiatest::ReporterContext c(reporter, "after rrect");
        test_to_from(reporter, p.snapshot(), /* allow_relative_drift= */ true);
    }
}

static void testInvalidPath(skiatest::Reporter* reporter,
                            const std::string& name,
                            const std::string& input) {
    skiatest::ReporterContext subtest(reporter, name);
    auto path = SkParsePath::FromSVGString(input.c_str());
    REPORTER_ASSERT(reporter, !path.has_value());
}

DEF_TEST(ParsePath_InvalidDoesNotCrash, r) {
    testInvalidPath(r, "empty move", "M");
    testInvalidPath(r, "partial move", "M 5");
    testInvalidPath(r, "partial vertical line", "V");  // oss-fuzz:68723
    testInvalidPath(r, "partial horizontal line", "H");
    testInvalidPath(r, "partial cubic", "C 1 2");
    testInvalidPath(r, "partial continued cubic", "S 6 7");
    testInvalidPath(r, "partial quad", "Q 3 4 5");
    testInvalidPath(r, "partial continued quad", "T");
    testInvalidPath(r, "partial arc", "A 1 2 3 4 5 6");
    testInvalidPath(r, "partial ~", "~ 7 6 5");
}

DEF_TEST(ParsePathOptionalCommand, r) {
    struct {
        const char* fStr;
        int fVerbs;
        int fPoints;
    } gTests[] = {
            {"", 0, 0},

            {"H100 200 ", 3, 3},
            {"H-100-200", 3, 3},
            {"H+100+200", 3, 3},
            {"H.10.20", 3, 3},
            {"H-.10-.20", 3, 3},
            {"H+.10+.20", 3, 3},

            {"L100 100 200 200", 3, 3},
            {"L-100-100-200-200", 3, 3},
            {"L+100+100+200+200", 3, 3},
            {"L.10.10.20.20", 3, 3},
            {"L-.10-.10-.20-.20", 3, 3},
            {"L+.10+.10+.20+.20", 3, 3},

            {"C100 100 200 200 300 300 400 400 500 500 600 600", 3, 7},
            {"C100-100-200-200-300-300-400-400-500-500-600-600", 3, 7},
            {"C100+100+200+200+300+300+400+400+500+500+600+600", 3, 7},
            {"C.10.10.20.20.30.30.40.40.50.50.60.60", 3, 7},
            {"C-.10-.10-.20-.20-.30-.30-.40-.40-.50-.50-.60-.60", 3, 7},
            {"C+.10+.10+.20+.20+.30+.30+.40+.40+.50+.50+.60+.60", 3, 7},

            {"c-1.49.71-2.12 2.5-1.4 4 .71 1.49 2.5 2.12 4 1.4z", 4, 7},

            {"M10 10 L20 20 Z M30 30 L40 40", 5, 4},
            {"M10 10 L20 20 z m20 20 l10 10", 5, 4},
            {"L20 20 z m10 10 l10 10", 5, 4},
    };

    for (size_t i = 0; i < std::size(gTests); ++i) {
        skiatest::ReporterContext c(r, SkStringPrintf("gTests[%lu]", i));
        auto path = SkParsePath::FromSVGString(gTests[i].fStr);
        REPORTER_ASSERT(r, path.has_value());
        REPORTER_ASSERT(r, path->countVerbs() == gTests[i].fVerbs);
        REPORTER_ASSERT(r, path->countPoints() == gTests[i].fPoints);

        // Ensure both relative and absolute evaluate to the same path.
        test_to_from(r, *path);
    }
}

DEF_TEST(ParsePathArcFlags, r) {
    const char* arcs = "M10 10a2.143 2.143 0 100-4.285 2.143 2.143 0 000 4.286";
    auto path = SkParsePath::FromSVGString(arcs);
    REPORTER_ASSERT(r, path.has_value());
    // Arcs decompose to two conics.
    REPORTER_ASSERT(r, path->countVerbs() == 5);
    // One for move, 2x per conic.
    REPORTER_ASSERT(r, path->countPoints() == 9);
}

DEF_TEST(ParsePath_RelativeClosedSubpaths, r) {
    SkPathBuilder builder;
    // Test that each path verb works relatively then updates the start point.
    builder.lineTo(20, 20);
    builder.close();  // Returns to {0, 0}
    builder.moveTo(10, 10);
    builder.lineTo(20, 20);
    builder.close();  // Returns to {10, 10}
    builder.moveTo(30, 30);
    builder.cubicTo(35, 25, 45, 25, 50, 40);
    builder.close();  // Returns to {30, 30}
    builder.moveTo(60, 60);
    builder.quadTo(65, 50, 70, 70);
    builder.close();  // Returns to {60, 60}
    builder.moveTo(80, 80);
    builder.lineTo(90, 90);
    SkPath path = builder.detach();

    SkString relStr = SkParsePath::ToSVGString(path, SkParsePath::PathEncoding::Relative);
    const char* expected =
            "m0 0l20 20Zm10 10l10 10Zm20 20c5 -5 15 -5 20 10Zm30 30q5 -10 10 10Zm20 20l10 10";
    REPORTER_ASSERT(r, relStr.equals(expected), "\ngot : %s\nwant: %s", relStr.c_str(), expected);

    auto parsedPath = SkParsePath::FromSVGString(relStr.c_str());
    REPORTER_ASSERT(r, parsedPath.has_value());

    test_to_from(r, *parsedPath);
}
