/*
 * Copyright 2025 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>
#include <optional>
#include <vector>

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTo.h"
#include "include/utils/SkParsePath.h"
#include "modules/jsonreader/SkJSONReader.h"

namespace {

template <typename T>
struct MapEntry {
    const char* name;
    T           value;
};

template <typename T>
std::optional<T> ParseEnum(const skjson::StringValue* jname,
                           SkSpan<const MapEntry<T>> map) {
    if (!jname) { return std::nullopt; }

    for (const auto& e : map) {
        if (!strcmp(jname->begin(), e.name)) {
            return e.value;
        }
    }

    return std::nullopt;
}

SkPaint ParsePaint(const skjson::ObjectValue& jcmd) {
    SkPaint paint;

    if (const skjson::ArrayValue* jcolor = jcmd["p_color"]) {
        SkColor4f c = SkColors::kBlack;
        for (size_t i = 0; i < std::min(jcolor->size(), 4lu); ++i) {
            if (const skjson::NumberValue* jcomponent = (*jcolor)[i]) {
                c[i] = **jcomponent;
            }
        }
        paint.setColor4f(c);
    }

    if (const skjson::NumberValue* jswidth = jcmd["s_width"]) {
        paint.setStrokeWidth(**jswidth);
        paint.setStyle(SkPaint::kStroke_Style);
    }

    if (auto cap = ParseEnum<SkPaint::Cap>(jcmd["s_cap"], {
        { "butt"  , SkPaint::kButt_Cap   },
        { "round" , SkPaint::kRound_Cap  },
        { "square", SkPaint::kSquare_Cap },
    })) {
        paint.setStrokeCap(*cap);
    }

    if (auto join = ParseEnum<SkPaint::Join>(jcmd["s_join"], {
        { "miter", SkPaint::kMiter_Join },
        { "round", SkPaint::kRound_Join },
        { "bevel", SkPaint::kBevel_Join },
    })) {
        paint.setStrokeJoin(*join);
    }

    return paint;
}

class VGXParser {
public:
    bool parse(const sk_sp<SkData>& vgx_data, SkCanvas* canvas) {
        const skjson::DOM jdom(static_cast<const char*>(vgx_data->data()), vgx_data->size());
        const skjson::ObjectValue* jroot = jdom.root();
        if (!jroot) {
            return false;
        }

        return this->parse_cmds((*jroot)["cmds"], canvas);
    }

private:
    bool parse_cmds(const skjson::ArrayValue* jcmds, SkCanvas* canvas) {
        using CmdProc = bool (VGXParser::*)(const skjson::ObjectValue&, SkCanvas*);
        auto find_proc = [](const skjson::ObjectValue* jcmd) -> CmdProc {
            static constexpr struct {
                const char* fCmd;
                CmdProc     fProc;
            } gParserProcs[] = {
                { "definePath", &VGXParser::cmd_definepath },
                { "drawPath"  , &VGXParser::cmd_drawpath   },
            };

            if (!jcmd) {
                return nullptr;
            }

            const skjson::StringValue* jcmd_name = (*jcmd)["cmd"];
            if (!jcmd_name) {
                return nullptr;
            }

            for (const auto& proc : gParserProcs) {
                if (!strcmp(proc.fCmd, jcmd_name->begin())) {
                    return proc.fProc;
                }
            }

            return nullptr;
        };

        if (!jcmds) {
            return false;
        }

        for (const skjson::ObjectValue* jcmd : *jcmds) {
            CmdProc proc = find_proc(jcmd);
            if (!proc || !(this->*proc)(*jcmd, canvas)) {
                return false;
            }
        }

        return true;
    }

    bool cmd_definepath(const skjson::ObjectValue& jcmd, SkCanvas*) {
        const skjson::StringValue* jd = jcmd["d"];
        if (!jd) {
            return false;
        }

        SkPath path;
        if (!SkParsePath::FromSVGString(jd->begin(), &path)) {
            return false;
        }

        fPaths.push_back(path);

        return true;
    }

    bool cmd_drawpath(const skjson::ObjectValue& jcmd, SkCanvas* canvas) {
        const skjson::NumberValue* jindex = jcmd["index"];
        if (!jindex) {
            return false;
        }
        const size_t idx = SkToSizeT(**jindex);
        if (idx >= fPaths.size()) {
            return false;
        }

        canvas->drawPath(fPaths[idx], ParsePaint(jcmd));

        return true;
    }

    std::vector<SkPath> fPaths;
};

}  // namespace

namespace skvgx {

bool Parse(SkStream* stream, SkCanvas* canvas) {
    const auto data = SkData::MakeFromStream(stream, stream->getLength());
    if (!data) {
        return false;
    }

    return VGXParser().parse(data, canvas);
}

}  // namespace skvgx
