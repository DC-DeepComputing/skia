/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_XML)
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "src/xml/SkDOM.h"
#include "src/xml/SkXMLParser.h"
#include "tests/Test.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

static const SkDOM::Node* check_node(skiatest::Reporter* r, const SkDOM& dom,
                                     const SkDOM::Node* node, const char* expectedName,
                                     SkDOM::Type expectedType) {
    REPORTER_ASSERT(r, node);
    if (node) {
        REPORTER_ASSERT(r, !strcmp(dom.getName(node), expectedName));
        REPORTER_ASSERT(r, dom.getType(node) == expectedType);
    }
    return node;
}

DEF_TEST(SkDOM_test, r) {
    static const char gDoc[] =
        "<root a='1' b='2'>"
            "<elem1 c='3' />"
            "<elem2 d='4' />"
            "<elem3 e='5'>"
                "<subelem1>Some text.</subelem1>"
                "<subelem2 f='6' g='7'/>"
                "<subelem3>Some more text.</subelem3>"
            "</elem3>"
            "<elem4 h='8'/>"
        "</root>"
        ;

    SkMemoryStream docStream(gDoc, sizeof(gDoc) - 1);

    SkDOM   dom;
    REPORTER_ASSERT(r, !dom.getRootNode());

    const SkDOM::Node* root = dom.build(docStream);
    REPORTER_ASSERT(r, root && dom.getRootNode() == root);

    const char* v = dom.findAttr(root, "a");
    REPORTER_ASSERT(r, v && !strcmp(v, "1"));
    v = dom.findAttr(root, "b");
    REPORTER_ASSERT(r, v && !strcmp(v, "2"));
    v = dom.findAttr(root, "c");
    REPORTER_ASSERT(r, v == nullptr);

    REPORTER_ASSERT(r, dom.getFirstChild(root, "elem1"));
    REPORTER_ASSERT(r, !dom.getFirstChild(root, "subelem1"));

    {
        const auto* elem1 = check_node(r, dom, dom.getFirstChild(root),
                                       "elem1", SkDOM::kElement_Type);
        const auto* elem2 = check_node(r, dom, dom.getNextSibling(elem1),
                                       "elem2", SkDOM::kElement_Type);
        const auto* elem3 = check_node(r, dom, dom.getNextSibling(elem2),
                                       "elem3", SkDOM::kElement_Type);
        {
            const auto* subelem1 = check_node(r, dom, dom.getFirstChild(elem3),
                                              "subelem1", SkDOM::kElement_Type);
            {
                check_node(r, dom, dom.getFirstChild(subelem1),
                           "Some text.", SkDOM::kText_Type);
            }
            const auto* subelem2 = check_node(r, dom, dom.getNextSibling(subelem1),
                                              "subelem2", SkDOM::kElement_Type);
            const auto* subelem3 = check_node(r, dom, dom.getNextSibling(subelem2),
                                              "subelem3", SkDOM::kElement_Type);
            {
                check_node(r, dom, dom.getFirstChild(subelem3),
                           "Some more text.", SkDOM::kText_Type);
            }
        }
        check_node(r, dom, dom.getNextSibling(elem3),
                   "elem4", SkDOM::kElement_Type);
    }
}

DEF_TEST(SkXMLParser_CallbacksAndPosition, r) {
    class RecordingParser : public SkXMLParser {
    public:
        struct Event {
            enum class Type { kStart, kText, kEnd } fType;
            std::string fText;
            std::vector<std::pair<std::string, std::string>> fAttrs;
        };

        std::vector<Event> fEvents;
        int fMaxLine = -1;
        int fMaxCol = -1;

    protected:
        void onStartElement(std::string_view elem, SkSpan<const Attribute> attributes) override {
            Event event{Event::Type::kStart, std::string(elem), {}};
            for (const auto& [name, value] : attributes) {
                event.fAttrs.emplace_back(name, value);
            }
            fEvents.push_back(std::move(event));
            fMaxLine = std::max(fMaxLine, this->getCurrentLineNumber());
            fMaxCol = std::max(fMaxCol, this->getCurrentColumnNumber());
        }
        void onText(std::string_view text) override {
            fEvents.push_back({Event::Type::kText, std::string(text), {}});
        }
        void onEndElement(std::string_view elem) override {
            fEvents.push_back({Event::Type::kEnd, std::string(elem), {}});
        }
    };

    static const char kDoc[] =
        "<root a='1'\n"
        "      b='2'>"
        "<child c='3'>hello</child>"
        "</root>";

    RecordingParser parser;
    REPORTER_ASSERT(r, parser.parse(kDoc, sizeof(kDoc) - 1));
    REPORTER_ASSERT(r, parser.fEvents.size() == 5);
    if (parser.fEvents.size() == 5) {
        using Type = RecordingParser::Event::Type;
        REPORTER_ASSERT(r, parser.fEvents[0].fType == Type::kStart);
        REPORTER_ASSERT(r, parser.fEvents[0].fText == "root");
        REPORTER_ASSERT(r, parser.fEvents[0].fAttrs.size() == 2);
        REPORTER_ASSERT(r,
                        parser.fEvents[0].fAttrs[0] ==
                                std::make_pair(std::string("a"), std::string("1")));
        REPORTER_ASSERT(r,
                        parser.fEvents[0].fAttrs[1] ==
                                std::make_pair(std::string("b"), std::string("2")));

        REPORTER_ASSERT(r, parser.fEvents[1].fType == Type::kStart);
        REPORTER_ASSERT(r, parser.fEvents[1].fText == "child");
        REPORTER_ASSERT(r, parser.fEvents[1].fAttrs.size() == 1);
        REPORTER_ASSERT(r,
                        parser.fEvents[1].fAttrs[0] ==
                                std::make_pair(std::string("c"), std::string("3")));

        REPORTER_ASSERT(r, parser.fEvents[2].fType == Type::kText);
        REPORTER_ASSERT(r, parser.fEvents[2].fText == "hello");

        REPORTER_ASSERT(r, parser.fEvents[3].fType == Type::kEnd);
        REPORTER_ASSERT(r, parser.fEvents[3].fText == "child");

        REPORTER_ASSERT(r, parser.fEvents[4].fType == Type::kEnd);
        REPORTER_ASSERT(r, parser.fEvents[4].fText == "root");
    }
    REPORTER_ASSERT(r, parser.fMaxLine >= 2);
    REPORTER_ASSERT(r, parser.fMaxCol >= 1);
    REPORTER_ASSERT(r, parser.getCurrentLineNumber() == -1);
    REPORTER_ASSERT(r, parser.getCurrentColumnNumber() == -1);
}

#endif // SK_XML
