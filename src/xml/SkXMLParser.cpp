/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/xml/SkXMLParser.h"

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkOnlyStackAllocatable.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkRandom.h"
#include "src/core/SkTime.h"
#include "src/core/SkUtils.h"

#include <expat.h>

#include <array>
#include <vector>

static const auto gErrorStrings = std::to_array<const char *>({
    "empty or missing file ",
    "unknown element ",
    "unknown attribute name ",
    "error in attribute value ",
    "duplicate ID ",
    "unknown error "
,});

SkXMLParserError::SkXMLParserError() : fCode(kNoError), fLineNumber(-1),
    fNativeCode(-1)
{
    reset();
}

SkXMLParserError::~SkXMLParserError()
{
    // need a virtual destructor for our subclasses
}

void SkXMLParserError::getErrorString(SkString* str) const
{
    SkASSERT(str);
    SkString temp;
    if (fCode != kNoError) {
        if ((unsigned)fCode < std::size(gErrorStrings))
            temp.set(gErrorStrings[fCode - 1]);
        temp.append(fNoun);
    } else
        SkXMLParser::GetNativeErrorString(fNativeCode, &temp);
    str->append(temp);
}

void SkXMLParserError::reset() {
    fCode = kNoError;
    fLineNumber = -1;
    fNativeCode = -1;
}

////////////////

namespace {

// Return a random number based on the clock to be consistent
// across platforms yet hard to guess and not leaking information.
static uint32_t get_hash_salt() {
    static const uint32_t s_salt = []() {
        double nsecs = SkTime::GetNSecs();
        uint64_t timeVal = sk_bit_cast<uint64_t>(nsecs);
        uint32_t seed = static_cast<uint32_t>(timeVal ^ timeVal >> 32);

        SkRandom rand(seed);
        uint32_t salt = rand.nextU();
        while (salt == 0) [[unlikely]] {
            // Expat hash seed must be non-zero or the default cPRNG will be used.
            salt = rand.nextU();
        }
        return salt;
    }();
    return s_salt;
}

const XML_Memory_Handling_Suite sk_XML_alloc = {
    sk_malloc_throw,
    sk_realloc_throw,
    sk_free
};

} // anonymous namespace

struct SkXMLParser::ParsingContext {
    SK_ONLY_STACK_ALLOCATABLE;

    explicit ParsingContext(SkXMLParser* parser)
        : fXMLParser(XML_ParserCreate_MM(nullptr, &sk_XML_alloc, nullptr))
        , fParser(parser) {
        fParser->fContext = this;
    }

    ~ParsingContext() {
        fParser->fContext = nullptr;
    }

    static void XMLCALL start_element_handler(void* data,
                                              const char* tag,
                                              const char** attributes) {
        auto* ctx = static_cast<ParsingContext*>(data);
        ctx->flushText();

        ctx->fBufferedAttrs.clear();
        for (size_t i = 0; attributes[i]; i += 2) {
            ctx->fBufferedAttrs.push_back({attributes[i], attributes[i + 1]});
        }
        ctx->fParser->onStartElement(tag, ctx->fBufferedAttrs);
    }

    static void XMLCALL end_element_handler(void* data, const char* tag) {
        auto* ctx = static_cast<ParsingContext*>(data);
        ctx->flushText();

        ctx->fParser->onEndElement(tag);
    }

    static void XMLCALL text_handler(void* data, const char* txt, int len) {
        auto* ctx = static_cast<ParsingContext*>(data);
        ctx->appendText(txt, SkTo<size_t>(len));
    }

    static void XMLCALL entity_decl_handler(void* data,
                                            const XML_Char* entityName,
                                            int is_parameter_entity,
                                            const XML_Char* value,
                                            int value_length,
                                            const XML_Char* base,
                                            const XML_Char* systemId,
                                            const XML_Char* publicId,
                                            const XML_Char* notationName) {
        auto* ctx = static_cast<ParsingContext*>(data);
        SkDEBUGF("'%s' entity declaration found, stopping processing", entityName);
        XML_StopParser(ctx->fXMLParser, XML_FALSE);
    }

    SkAutoTCallVProc<std::remove_pointer_t<XML_Parser>, XML_ParserFree> fXMLParser;

private:
    void flushText() {
        if (!fBufferedText.empty()) {
            fParser->onText(std::string_view(fBufferedText.data(), fBufferedText.size()));
            fBufferedText.clear();
        }
    }

    void appendText(const char* txt, size_t len) {
        fBufferedText.insert(fBufferedText.end(), txt, &txt[len]);
    }

    SkXMLParser* const fParser;
    std::vector<char> fBufferedText;
    std::vector<Attribute> fBufferedAttrs;
};

SkXMLParser::SkXMLParser(SkXMLParserError* parserError) : fError(parserError)
{
}

SkXMLParser::~SkXMLParser()
{
}

bool SkXMLParser::parse(SkStream& docStream)
{
    ParsingContext ctx(this);
    if (!ctx.fXMLParser) {
        SkDEBUGF("could not create XML parser\n");
        return false;
    }

    // Expat calls to rand_s if no salt is set, which is not allowed on Windows.
    // https://crbug.com/40088338
    // Setting the salt helps prevent DOS with a known hash sequence. Generating a
    // random salt once per process oursevles ensures the salt is non-deterministic
    // and unguessable, while preventing calls to rand_s inside sandboxed processes
    // on Windows.
    XML_SetHashSalt(ctx.fXMLParser, get_hash_salt());

    XML_SetUserData(ctx.fXMLParser, &ctx);
    XML_SetElementHandler(ctx.fXMLParser,
                          ParsingContext::start_element_handler,
                          ParsingContext::end_element_handler);
    XML_SetCharacterDataHandler(ctx.fXMLParser, ParsingContext::text_handler);

    // Disable entity processing, to inhibit internal entity expansion. See expat CVE-2013-0340.
    XML_SetEntityDeclHandler(ctx.fXMLParser, ParsingContext::entity_decl_handler);

    XML_Status status = XML_STATUS_OK;
    if (docStream.getMemoryBase() && docStream.hasLength()) {
        const char* base = reinterpret_cast<const char*>(docStream.getMemoryBase());
        status = XML_Parse(ctx.fXMLParser,
                           base + docStream.getPosition(),
                           docStream.getLength() - docStream.getPosition(),
                           true);
    } else {
        static constexpr int kBufferSize = 4096;
        bool done = false;
        do {
            void* buffer = XML_GetBuffer(ctx.fXMLParser, kBufferSize);
            if (!buffer) {
                SkDEBUGF("could not buffer enough to continue\n");
                return false;
            }

            size_t len = docStream.read(buffer, kBufferSize);
            done = docStream.isAtEnd();
            status = XML_ParseBuffer(ctx.fXMLParser, SkToS32(len), done);
            if (XML_STATUS_ERROR == status) {
                break;
            }
        } while (!done);
    }
    if (XML_STATUS_ERROR == status) {
#if defined(SK_DEBUG)
        XML_Error error = XML_GetErrorCode(ctx.fXMLParser);
        int line = XML_GetCurrentLineNumber(ctx.fXMLParser);
        int column = XML_GetCurrentColumnNumber(ctx.fXMLParser);
        const XML_LChar* errorString = XML_ErrorString(error);
        SkDEBUGF("parse error @%d:%d: %d (%s).\n", line, column, error, errorString);
#endif
        return false;
    }

    return true;
}

bool SkXMLParser::parse(const char doc[], size_t len)
{
    SkMemoryStream docStream(doc, len);
    return this->parse(docStream);
}

int SkXMLParser::getCurrentLineNumber() const {
    return fContext ? SkToInt(XML_GetCurrentLineNumber(fContext->fXMLParser)) : -1;
}

int SkXMLParser::getCurrentColumnNumber() const {
    return fContext ? SkToInt(XML_GetCurrentColumnNumber(fContext->fXMLParser)) : -1;
}

void SkXMLParser::GetNativeErrorString(int error, SkString* str)
{

}

////////////////////////////////////////////////////////////////////////////////

void SkXMLParser::onStartElement(std::string_view elem, SkSpan<const Attribute> attributes) {}
void SkXMLParser::onEndElement(std::string_view elem) {}
void SkXMLParser::onText(std::string_view text) {}
