/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkImage.h"
#include "include/core/SkM44.h"
#include "samplecode/Sample.h"

struct Ray {
    SkV3    origin;
    SkV3    direction;

    SkV3 eval(float t) const {
        return origin + t * direction;
    }
};

struct HitResult {
    SkV3    point;
    SkV3    normal;
    SkV3    color;
};

class Light {
public:
    Light() {}
    virtual ~Light() {}

    virtual SkV3 contribute(SkV3 position, SkV3 normal) = 0;
};

class PointLight : public Light {
    SkV3    fPos;
    SkV3    fColor;
public:
    PointLight(SkV3 pos, SkV3 color) : fPos(pos), fColor(color) {}

    SkV3 contribute(SkV3 pos, SkV3 norm) override {
        return std::max((fPos - pos).normalize().dot(norm), 0.0f) * fColor;
    }
};

class Shape {
public:
    Shape() {}
    virtual ~Shape() {}

    virtual bool raytest(const Ray&, HitResult*) const = 0;
};

class Plane : public Shape {
    const SkV3 fLoc, fNormal, fColor;

public:
    Plane(SkV3 loc, SkV3 normal, SkV3 color)
        : fLoc(loc)
        , fNormal(normal.normalize())
        , fColor(color)
    {}
    
    bool raytest(const Ray& ray, HitResult* result) const override {
        float t = (fLoc - ray.origin).dot(fNormal) / ray.direction.dot(fNormal);
        
        if (t > 0) {
            result->point = ray.eval(t);
            result->normal = fNormal;
            result->color = fColor;
            return true;
        }
        return false;
    }
};

class Sphere : public Shape {
public:
    SkV3    fCenter;
    float   fRadius;
    SkV3    fColor;

    Sphere(SkV3 center, float radius, SkV3 color)
        : fCenter(center), fRadius(radius), fColor(color)
    {}

    bool raytest(const Ray& ray, HitResult* result) const override {
        SkV3 A = ray.origin - fCenter;
        // at^2 + bt + c = 0
        //
        // float a = 1;    since ray.direction is a unit vector
        float b = 2 * A.dot(ray.direction);
        float c = A.dot(A) - fRadius * fRadius;

        float discrim = b*b - 4*c;
        if (discrim <= 0) {
            return false;
        }
        float t0 = (-b + sqrt(discrim)) * 0.5f;
        float t1 = (-b - sqrt(discrim)) * 0.5f;
        SkV3 p = ray.eval(std::min(t0, t1));
        *result = {p, (p - fCenter).normalize(), fColor};
        return true;
    }
};

#include "include/core/SkColorPriv.h"
#include "include/utils/SkRandom.h"

class SampleRT : public Sample {
    SkV3 fAmbient;
    Light* fLights[2];
    
    struct Rec {
        Sphere  sphere;
        SkV3    velocity;
    };
    std::vector<Rec> fRecs;
    
    std::vector<std::unique_ptr<Shape>> fShapes;
    
    SkV3    fEye{0, 0, -20};
    SkV3    fMin = {-12, -12,  1},
            fMax = { 12,  12, 24};
    float   fPrevS = 0;

public:
    SampleRT() {
        const float a = 0.5;
        fAmbient = {a, a, a};
        fLights[0] = new PointLight({-10, -10, -10}, {1,1,1});
        fLights[1] = new PointLight({0, -50, -20}, {0,0,0});

        SkRandom rand;
        for (int i = 0; i < 10; ++i) {
            SkV3 center{rand.nextRangeF(-5, 5), rand.nextRangeF(-5, 5), rand.nextRangeF(2, 23)};
            float rad = rand.nextRangeF(1, 3);
            SkV3 color = {rand.nextF(), rand.nextF(), rand.nextF()};
            const float speed = 1;
            SkV3 vel = {rand.nextF() * speed, rand.nextF() * speed, rand.nextF() * speed};
            fRecs.push_back({{center, rad, color}, vel});
        }
        
        fShapes.push_back(std::unique_ptr<Shape>(new Plane({0, 0, fMax.z},
                                                           {0, 0, -1},
                                                           {0.5, 0.25, 0.25})));
    }

protected:
    static unsigned pin255(float x) {
        return sk_float_round2int(std::max(std::min(x, 1.0f), 0.0f) * 255);
    }

    static SkPMColor to_skcolor(SkV3 c) {
        return SkPackARGB32(0xFF, pin255(c.x), pin255(c.y), pin255(c.z));
    }

    SkString name() override { return SkString("raytrace"); }

    bool onChar(SkUnichar c) override {
        switch (c) {
            default: return this->INHERITED::onChar(c);

            case 'z': fEye.z -= 1; SkDebugf("%g\n", fEye.z); break;
            case 'Z': fEye.z += 1; SkDebugf("%g\n", fEye.z); break;
        }
        return true;
    }
    
    bool onAnimate(double nanos) override {
        float secs = float(nanos * 1e-9);
        if (fPrevS == 0) {
            fPrevS = secs;
        }
        float d = (secs - fPrevS) * 3;
        fPrevS = secs;
        
        auto reflect = [d](float p, float* v, float min, float max) {
            float newp = p + *v * d;
            if (*v < 0 && newp < min) {
                *v = - *v;
                newp = min;
            } else if (*v > 0 && newp > max) {
                *v = - *v;
                newp = max;
            }
            return newp;
        };
        
        for (auto& r : fRecs) {
            for (int j = 0; j < 3; ++j) {
                r.sphere.fCenter[j] = reflect(r.sphere.fCenter[j], &r.velocity[j],
                                              fMin[j] + r.sphere.fRadius,
                                              fMax[j] - r.sphere.fRadius);
            }
        }
        return true;
    }

    void onDrawContent(SkCanvas* canvas) override {
        const float max_dist_to_eye = 99999999;

        const int W = 512, H = 512;
        SkImageInfo info = SkImageInfo::MakeN32Premul(W, H);
        SkBitmap bm;
        bm.allocPixels(info);

        SkRect screen = { -10, -10, 10, 10 };
        float screenZ = 0;

        const float dy = screen.height() / (H - 1);
        const float dx = screen.width() / (W - 1);
        float fy = screen.fTop + dy * 0.5f;
        for (int y = 0; y < H; ++y) {
            float fx = screen.fLeft + dx * 0.5;
            for (int x = 0; x < W; ++x) {
                const Ray ray{fEye, (SkV3{fx, fy, screenZ} - fEye).normalize()};

                HitResult closest;
                float distToEye = max_dist_to_eye;

                // spheres
                for (auto& r : fRecs) {
                    const auto& sp = r.sphere;
                    HitResult result;
                    if (sp.raytest(ray, &result)) {
                        float len = (result.point - fEye).length();
                        if (len < distToEye) {
                            distToEye = len;
                            closest = result;
                        }
                    }
                }
                // shapes
                for (const auto& s : fShapes) {
                    HitResult result;
                    if (s->raytest(ray, &result)) {
                        float len = (result.point - fEye).length();
                        if (len < distToEye) {
                            distToEye = len;
                            closest = result;
                        }
                    }
                }

                if (distToEye < max_dist_to_eye) {
                    SkV3 c = {0,0,0};
                    c = fAmbient;
                    for (auto l : fLights) {
                        SkV3 lc = l->contribute(closest.point, closest.normal);
                        c += lc * closest.color;
                    }
                    *bm.getAddr32(x, y) = to_skcolor(c);
                }

                fx += dy;
            }
            fy += dy;
        }

        canvas->drawImage(bm.asImage(), 0, 0, SkSamplingOptions(), nullptr);
    }

#if 0
    Sample::Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey modi) override {
        SkPoint pt = { x, y };
        int index = -1;
        int count = fFB.getTriangle() ? 3 : 2;
        SkScalar tol = 12;

        for (int i = 0; i < count; ++i) {
            if (SkPointPriv::EqualsWithinTolerance(fPts[i], pt, tol)) {
                index = i;
                break;
            }
        }
        return new IndexClick(index);
    }

    bool onClick(Click* click) override {
        int index = IndexClick::GetIndex(click);
        if (index >= 0 && index <= 2) {
            fPts[index] = click->fCurr;
        } else {
            SkScalar dx = click->fCurr.fX - click->fPrev.fX;
            SkScalar dy = click->fCurr.fY - click->fPrev.fY;
            fPts[0].offset(dx, dy);
            fPts[1].offset(dx, dy);
            fPts[2].offset(dx, dy);
        }
        return true;
    }
#endif

private:

    using INHERITED = Sample;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new SampleRT(); )
