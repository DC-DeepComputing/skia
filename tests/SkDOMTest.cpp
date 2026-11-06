/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_XML)
#include "include/core/SkStream.h"
#include "src/xml/SkDOM.h"
#include "src/xml/SkXMLParser.h"
#include "src/xml/SkXMLWriter.h"
#include "tests/Test.h"

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
        int fAttrsDoneCount = 0;
        int fMaxLine = -1;
        int fMaxCol = -1;
        int fAttrsDoneMaxLine = -1;
        int fAttrsDoneMaxCol = -1;
        int fStartCount = 0;
        bool fInOpenTag = false;
        bool fOrderValid = true;

    protected:
        void onStartElement(const char elem[]) override {
            if (fInOpenTag) {
                fOrderValid = false;
            }
            fInOpenTag = true;
            ++fStartCount;
            fMaxLine = std::max(fMaxLine, this->getCurrentLineNumber());
            fMaxCol = std::max(fMaxCol, this->getCurrentColumnNumber());
        }
        void onAddAttribute(const char name[], const char value[]) override {
            if (!fInOpenTag) {
                fOrderValid = false;
            }
        }
        void onAttributesDone() override {
            if (!fInOpenTag) {
                fOrderValid = false;
            }
            fInOpenTag = false;
            ++fAttrsDoneCount;
            fAttrsDoneMaxLine = std::max(fAttrsDoneMaxLine, this->getCurrentLineNumber());
            fAttrsDoneMaxCol = std::max(fAttrsDoneMaxCol, this->getCurrentColumnNumber());
        }
        void onText(const char text[], int len) override {
            if (fInOpenTag) {
                fOrderValid = false;
            }
        }
        void onEndElement(const char elem[]) override {
            if (fInOpenTag) {
                fOrderValid = false;
            }
        }
    };

    static const char kDoc[] =
        "<root a='1' b='2'>\n"
        "  <child c='3'/>\n"
        "</root>";

    RecordingParser parser;
    REPORTER_ASSERT(r, parser.parse(kDoc, sizeof(kDoc) - 1));
    REPORTER_ASSERT(r, parser.fStartCount == 2);
    REPORTER_ASSERT(r, parser.fAttrsDoneCount == 2);
    REPORTER_ASSERT(r, !parser.fInOpenTag && parser.fOrderValid);
    REPORTER_ASSERT(r, parser.fMaxLine >= 2);
    REPORTER_ASSERT(r, parser.fMaxCol >= 1);
    REPORTER_ASSERT(r, parser.fAttrsDoneMaxLine >= 2);
    REPORTER_ASSERT(r, parser.fAttrsDoneMaxCol >= 1);
    REPORTER_ASSERT(r, parser.getCurrentLineNumber() == -1);
    REPORTER_ASSERT(r, parser.getCurrentColumnNumber() == -1);

    SkMemoryStream domStream(kDoc, sizeof(kDoc) - 1);
    SkDOM dom;
    const SkDOM::Node* root = dom.build(domStream);
    REPORTER_ASSERT(r, root);

    RecordingParser domParser;
    REPORTER_ASSERT(r, domParser.parse(dom, root));
    REPORTER_ASSERT(r, domParser.fStartCount == 2);
    REPORTER_ASSERT(r, domParser.fAttrsDoneCount == 2);
    REPORTER_ASSERT(r, !domParser.fInOpenTag && domParser.fOrderValid);

    RecordingParser writerParser;
    {
        SkXMLParserWriter writer(&writerParser);
        writer.startElement("root");
        writer.addAttribute("a", "1");
        writer.startElement("textElem");
        writer.addAttribute("b", "2");
        writer.addText("hello", 5);
        writer.endElement();
        writer.startElement("leaf");
        writer.addAttribute("c", "3");
        writer.endElement();
        writer.endElement();
    }
    REPORTER_ASSERT(r, writerParser.fStartCount == 3);
    REPORTER_ASSERT(r, writerParser.fAttrsDoneCount == 3);
    REPORTER_ASSERT(r, !writerParser.fInOpenTag && writerParser.fOrderValid);
}

#endif // SK_XML
