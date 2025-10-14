/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathMeasure.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkFloatingPoint.h"
#include "src/base/SkRandom.h"
#include "tools/ToolUtils.h"
#include "tools/timer/TimeUtils.h"

#include <vector>
#include <queue>

#if 0

Next time:
  have 3 methods: currentBDay, next, prev



Prompt: We're going to create a birthday reminder app that, on start up, is going to consume a family tree
and then, when asked, it will display the next relative's birthdate along with their name. The user
can also iteratively tap a next button to scroll through the upcoming birthdays in order.

Intially the input is going to be a family tree rooted at the user and just containing their
direct ancestors. So, the user, then their parents, then their parents' parents, etc. Here is
the data structure:

struct Person {
    std::string name;
    int64_t birthday_month;
    int64_t birthday_day_of_the_month;
    Person* parentA;
    Person* parentB;
};

Your job is to create a BirthdayManager class that accepts the family tree in its constructor
and has a nextBirthday method that returns the next birthday. The nextBirthday method should
address both the initial next birth day use case and the scrolling through the birthdays use
cases. We want it to be pretty efficient.

Follow on:
Initial reviews of our app complain that it doesn't support spouses or childen. How would you
modify the Person structure to accomadate them.

Okay - update your constructor to add the spouses and children to the app.

Follow on:
How does your implementation deal with 2nd cousin marriages. Such a situation could have one
person appearing twice - once as a descendant of a grand uncle/aunt and again as a spouse.

Follow on:
How does your implementation deal with people with the same birthday? Users have cried out
for a feature that lets the ordering be determined by distance from them in the tree. How would
you do that? How does this interact w/ the 2nd cousin problem.

#endif

struct Person {
    Person(const std::string& name1, int month, int day, Person* pA, Person* pB)
            : name(name1)
            , birthday_month(month)
            , birthday_day_of_the_month(day)
            , parentA(pA)
            , parentB(pB) {}

    std::string name;
    int birthday_month;
    int birthday_day_of_the_month;
    Person* parentA;
    Person* parentB;
};

class Mgr {
public:
    Mgr(Person* client) {
        if (client) {
            this->visit(client->parentA);
            this->visit(client->parentB);
        }
    }

    void doit() {
        while (!fDays.empty()) {
            Person* tmp = fDays.top();
            SkDebugf("%s %d %d\n",
                     tmp->name.c_str(),
                     tmp->birthday_month,
                     tmp->birthday_day_of_the_month);
            fDays.pop();
        }
    }

private:
    void visit(Person* p) {
        if (p == nullptr) {
            return;
        }
        fDays.push(p);
        this->visit(p->parentA);
        this->visit(p->parentB);
    }

    class Compare {
    public:
        bool operator()(const Person* a, const Person* b) {
            if (a->birthday_month > b->birthday_month) {
                return true;
            }
            if (a->birthday_month < b->birthday_month) {
                return false;
            }
            if (a->birthday_day_of_the_month > b->birthday_day_of_the_month) {
                return true;
            }
            return false;
        }
    };

    std::priority_queue<Person*, std::vector<Person*>, Compare> fDays;
};


class AddArcGM : public skiagm::GM {
public:
    AddArcGM() {
        Person gp1("mgdad", 2, 2, nullptr, nullptr);
        Person p1 ("mom", 1, 1, &gp1, nullptr);
        Person p2 ("dad", 1, 2, nullptr, nullptr);
        Person me("me", 1, 3, &p1, &p2);

        Mgr mgr(&me);

        mgr.doit();
    }

protected:
    SkString getName() const override { return SkString("addarc1"); }

    SkISize getISize() override { return SkISize::Make(1040, 1040); }

    void onDraw(SkCanvas* canvas) override {

    }

    bool onAnimate(double nanos) override {
        return false;
    }

private:
    using INHERITED = skiagm::GM;
};
DEF_GM( return new AddArcGM; )

///////////////////////////////////////////////////

#define R   400

DEF_SIMPLE_GM(addarc_meas, canvas, 2*R + 40, 2*R + 40) {
        canvas->translate(R + 20, R + 20);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStroke(true);

        SkPaint measPaint;
        measPaint.setAntiAlias(true);
        measPaint.setColor(SK_ColorRED);

        const SkRect oval = SkRect::MakeLTRB(-R, -R, R, R);
        canvas->drawOval(oval, paint);

        for (SkScalar deg = 0; deg < 360; deg += 10) {
            const SkScalar rad = SkDegreesToRadians(deg);
            SkScalar rx = SkScalarCos(rad) * R;
            SkScalar ry = SkScalarSin(rad) * R;

            canvas->drawLine(0, 0, rx, ry, paint);

            SkPathMeasure meas(SkPathBuilder().addArc(oval, 0, deg).detach(), false);
            SkScalar arcLen = rad * R;
            SkPoint pos;
            if (meas.getPosTan(arcLen, &pos, nullptr)) {
                canvas->drawLine({0, 0}, pos, measPaint);
            }
        }
}

///////////////////////////////////////////////////

// Emphasize drawing a stroked oval (containing conics) and then scaling the results up,
// to ensure that we compute the stroke taking the CTM into account
//
class StrokeCircleGM : public skiagm::GM {
public:
    StrokeCircleGM() : fRotate(0) {}

protected:
    SkString getName() const override { return SkString("strokecircle"); }

    SkISize getISize() override { return SkISize::Make(520, 520); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(20, 20);
        canvas->translate(13, 13);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStroke(true);
        paint.setStrokeWidth(SK_Scalar1 / 2);

        const SkScalar delta = paint.getStrokeWidth() * 3 / 2;
        SkRect r = SkRect::MakeXYWH(-12, -12, 24, 24);
        SkRandom rand;

        SkScalar sign = 1;
        while (r.width() > paint.getStrokeWidth() * 2) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(fRotate * sign);

            paint.setColor(ToolUtils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(double nanos) override {
        fRotate = TimeUtils::Scaled(1e-9 * nanos, 60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    using INHERITED = skiagm::GM;
};
DEF_GM( return new StrokeCircleGM; )

//////////////////////

// Fill circles and rotate them to test our Analytic Anti-Aliasing.
// This test is based on StrokeCircleGM.
class FillCircleGM : public skiagm::GM {
public:
    FillCircleGM() : fRotate(0) {}

protected:
    SkString getName() const override { return SkString("fillcircle"); }

    SkISize getISize() override { return SkISize::Make(520, 520); }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(20, 20);
        canvas->translate(13, 13);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStroke(true);
        paint.setStrokeWidth(SK_Scalar1 / 2);

        const SkScalar strokeWidth = paint.getStrokeWidth();
        const SkScalar delta = strokeWidth * 3 / 2;
        SkRect r = SkRect::MakeXYWH(-12, -12, 24, 24);
        SkRandom rand;

        // Reset style to fill. We only need stroke stype for producing delta and strokeWidth
        paint.setStroke(false);

        SkScalar sign = 1;
        while (r.width() > strokeWidth * 2) {
            SkAutoCanvasRestore acr(canvas, true);
            canvas->rotate(fRotate * sign);
            paint.setColor(ToolUtils::color_to_565(rand.nextU() | (0xFF << 24)));
            canvas->drawOval(r, paint);
            r.inset(delta, delta);
            sign = -sign;
        }
    }

    bool onAnimate(double nanos) override {
        fRotate = TimeUtils::Scaled(1e-9 * nanos, 60, 360);
        return true;
    }

private:
    SkScalar fRotate;

    using INHERITED = skiagm::GM;
};
DEF_GM( return new FillCircleGM; )

//////////////////////

static void html_canvas_arc(SkPathBuilder* path, SkScalar x, SkScalar y, SkScalar r, SkScalar start,
                            SkScalar end, bool ccw, bool callArcTo) {
    SkRect bounds = { x - r, y - r, x + r, y + r };
    SkScalar sweep = ccw ? end - start : start - end;
    if (callArcTo)
        path->arcTo(bounds, start, sweep, false);
    else
        path->addArc(bounds, start, sweep);
}

// Lifted from canvas-arc-circumference-fill-diffs.html
DEF_SIMPLE_GM(manyarcs, canvas, 620, 330) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStroke(true);

        canvas->translate(10, 10);

        // 20 angles.
        SkScalar sweepAngles[] = {
                           -123.7f, -2.3f, -2, -1, -0.3f, -0.000001f, 0, 0.000001f, 0.3f, 0.7f,
                           1, 1.3f, 1.5f, 1.7f, 1.99999f, 2, 2.00001f, 2.3f, 4.3f, 3934723942837.3f
        };
        for (size_t i = 0; i < std::size(sweepAngles); ++i) {
            sweepAngles[i] *= 180;
        }

        SkScalar startAngles[] = { -1, -0.5f, 0, 0.5f };
        for (size_t i = 0; i < std::size(startAngles); ++i) {
            startAngles[i] *= 180;
        }

        bool anticlockwise = false;
        SkScalar sign = 1;
        for (size_t i = 0; i < std::size(startAngles) * 2; ++i) {
            if (i == std::size(startAngles)) {
                anticlockwise = true;
                sign = -1;
            }
            SkScalar startAngle = startAngles[i % std::size(startAngles)] * sign;
            canvas->save();
            for (size_t j = 0; j < std::size(sweepAngles); ++j) {
                SkPathBuilder path;
                path.moveTo(0, 2);
                html_canvas_arc(&path, 18, 15, 10, startAngle, startAngle + (sweepAngles[j] * sign),
                                anticlockwise, true);
                path.lineTo(0, 28);
                canvas->drawPath(path.detach().setIsVolatile(true), paint);
                canvas->translate(30, 0);
            }
            canvas->restore();
            canvas->translate(0, 40);
        }
}

// Lifted from https://bugs.chromium.org/p/chromium/issues/detail?id=640031
DEF_SIMPLE_GM(tinyanglearcs, canvas, 620, 330) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStroke(true);

        canvas->translate(50, 50);

        SkScalar outerRadius = 100000.0f;
        SkScalar innerRadius = outerRadius - 20.0f;
        SkScalar centerX = 50;
        SkScalar centerY = outerRadius;
        SkScalar startAngles[] = { 1.5f * SK_ScalarPI , 1.501f * SK_ScalarPI  };
        SkScalar sweepAngle = 10.0f / outerRadius;

        for (size_t i = 0; i < std::size(startAngles); ++i) {
            SkPathBuilder path;
            SkScalar endAngle = startAngles[i] + sweepAngle;
            path.moveTo(centerX + innerRadius * std::cos(startAngles[i]),
                        centerY + innerRadius * std::sin(startAngles[i]));
            path.lineTo(centerX + outerRadius * std::cos(startAngles[i]),
                        centerY + outerRadius * std::sin(startAngles[i]));
            // A combination of tiny sweepAngle + large radius, we should draw a line.
            html_canvas_arc(&path, centerX, outerRadius, outerRadius,
                            startAngles[i] * 180 / SK_ScalarPI, endAngle * 180 / SK_ScalarPI,
                            true, true);
            path.lineTo(centerX + innerRadius * std::cos(endAngle),
                        centerY + innerRadius * std::sin(endAngle));
            html_canvas_arc(&path, centerX, outerRadius, innerRadius,
                            endAngle * 180 / SK_ScalarPI, startAngles[i] * 180 / SK_ScalarPI,
                            true, false);
            canvas->drawPath(path.detach(), paint);
            canvas->translate(20, 0);
        }
}
