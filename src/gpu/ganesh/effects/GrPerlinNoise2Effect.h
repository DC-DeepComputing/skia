#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/KeyBuilder.h"
#include "src/gpu/ganesh/GrFPArgs.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"
#include "src/gpu/ganesh/GrShaderCaps.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrMatrixEffect.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ganesh/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

class GrPerlinNoise2Effect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(
            SkPerlinNoiseShaderImpl::Type type,
            int numOctaves,
            bool stitchTiles,
            std::unique_ptr<SkPerlinNoiseShaderImpl::PaintingData> paintingData,
            GrSurfaceProxyView permutationsView,
            GrSurfaceProxyView noiseView,
            const GrCaps& caps) {
        static constexpr GrSamplerState kRepeatXSampler = {GrSamplerState::WrapMode::kRepeat,
                                                           GrSamplerState::WrapMode::kClamp,
                                                           GrSamplerState::Filter::kNearest};
        auto permutationsFP =
                GrTextureEffect::Make(std::move(permutationsView), kPremul_SkAlphaType,
                                      SkMatrix::I(), kRepeatXSampler, caps);
        auto noiseFP = GrTextureEffect::Make(std::move(noiseView), kPremul_SkAlphaType,
                                             SkMatrix::I(), kRepeatXSampler, caps);

        return std::unique_ptr<GrFragmentProcessor>(
                new GrPerlinNoise2Effect(type,
                                         numOctaves,
                                         stitchTiles,
                                         std::move(paintingData),
                                         std::move(permutationsFP),
                                         std::move(noiseFP)));
    }

    const char* name() const override { return "PerlinNoise"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new GrPerlinNoise2Effect(*this));
    }

    const SkPerlinNoiseShaderImpl::StitchData& stitchData() const { return fPaintingData->fStitchDataInit; }

    SkPerlinNoiseShaderImpl::Type type() const { return fType; }
    bool stitchTiles() const { return fStitchTiles; }
    const SkVector& baseFrequency() const { return fPaintingData->fBaseFrequency; }
    int numOctaves() const { return fNumOctaves; }

private:
    class Impl : public ProgramImpl {
    public:
        SkString emitHelper(EmitArgs& args);
        void emitCode(EmitArgs&) override;

    private:
        void onSetData(const GrGLSLProgramDataManager&, const GrFragmentProcessor&) override;

        GrGLSLProgramDataManager::UniformHandle fStitchDataUni;
        GrGLSLProgramDataManager::UniformHandle fBaseFrequencyUni;
    };

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override {
        return std::make_unique<Impl>();
    }

    void onAddToKey(const GrShaderCaps& caps, skgpu::KeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const GrPerlinNoise2Effect& s = sBase.cast<GrPerlinNoise2Effect>();
        return fType == s.fType &&
               fPaintingData->fBaseFrequency == s.fPaintingData->fBaseFrequency &&
               fNumOctaves == s.fNumOctaves &&
               fStitchTiles == s.fStitchTiles &&
               fPaintingData->fStitchDataInit == s.fPaintingData->fStitchDataInit;
    }

    GrPerlinNoise2Effect(SkPerlinNoiseShaderImpl::Type type,
                         int numOctaves,
                         bool stitchTiles,
                         std::unique_ptr<SkPerlinNoiseShaderImpl::PaintingData> paintingData,
                         std::unique_ptr<GrFragmentProcessor> permutationsFP,
                         std::unique_ptr<GrFragmentProcessor> noiseFP)
            : INHERITED(kGrPerlinNoise2Effect_ClassID, kNone_OptimizationFlags)
            , fType(type)
            , fNumOctaves(numOctaves)
            , fStitchTiles(stitchTiles)
            , fPaintingData(std::move(paintingData)) {
        this->registerChild(std::move(permutationsFP), SkSL::SampleUsage::Explicit());
        this->registerChild(std::move(noiseFP), SkSL::SampleUsage::Explicit());
        this->setUsesSampleCoordsDirectly();
    }

    GrPerlinNoise2Effect(const GrPerlinNoise2Effect& that)
            : INHERITED(that)
            , fType(that.fType)
            , fNumOctaves(that.fNumOctaves)
            , fStitchTiles(that.fStitchTiles)
            , fPaintingData(new SkPerlinNoiseShaderImpl::PaintingData(*that.fPaintingData)) {}

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkPerlinNoiseShaderImpl::Type       fType;
    int                                 fNumOctaves;
    bool                                fStitchTiles;

    std::unique_ptr<SkPerlinNoiseShaderImpl::PaintingData> fPaintingData;
};
