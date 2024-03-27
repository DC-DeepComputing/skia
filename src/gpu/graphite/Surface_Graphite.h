/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_Surface_Graphite_DEFINED
#define skgpu_graphite_Surface_Graphite_DEFINED

#include "src/image/SkSurface_Base.h"

#include "src/gpu/graphite/TextureProxyView.h"

namespace skgpu::graphite {

class Context;
class Device;
class Image;
class Recorder;
class TextureProxy;

class Surface final : public SkSurface_Base {
public:
    static sk_sp<SkSurface> MakeGraphite(Recorder* recorder,
                                         const SkImageInfo& info,
                                         skgpu::Budgeted budgeted,
                                         Mipmapped = Mipmapped::kNo,
                                         const SkSurfaceProps* props = nullptr);

    // Make a surface that is not registered with the provided recorder. This surface should be
    // short-lived and it must be flushed manually for its draw commands to be recorded.
    static sk_sp<SkSurface> MakeGraphiteScratch(Recorder* recorder,
                                                const SkImageInfo& info,
                                                Mipmapped = Mipmapped::kNo,
                                                const SkSurfaceProps* props = nullptr);

    Surface(sk_sp<Device>);
    ~Surface() override;

    // From SkSurface.h
    SkImageInfo imageInfo() const override;

    // From SkSurface_Base.h
    SkSurface_Base::Type type() const override { return SkSurface_Base::Type::kGraphite; }

    Recorder* onGetRecorder() const override;
    SkCanvas* onNewCanvas() override;
    sk_sp<SkSurface> onNewSurface(const SkImageInfo&) override;
    sk_sp<SkImage> onNewImageSnapshot(const SkIRect* subset) override;
    void onWritePixels(const SkPixmap&, int x, int y) override;
    void onAsyncRescaleAndReadPixels(const SkImageInfo& info,
                                     SkIRect srcRect,
                                     RescaleGamma rescaleGamma,
                                     RescaleMode rescaleMode,
                                     ReadPixelsCallback callback,
                                     ReadPixelsContext context) override;
    void onAsyncRescaleAndReadPixelsYUV420(SkYUVColorSpace yuvColorSpace,
                                           bool readAlpha,
                                           sk_sp<SkColorSpace> dstColorSpace,
                                           SkIRect srcRect,
                                           SkISize dstSize,
                                           RescaleGamma rescaleGamma,
                                           RescaleMode,
                                           ReadPixelsCallback callback,
                                           ReadPixelsContext context) override;
    bool onCopyOnWrite(ContentChangeMode) override;
    sk_sp<const SkCapabilities> onCapabilities() override;

    TextureProxyView readSurfaceView() const;
    sk_sp<SkImage> asImage() const;
    sk_sp<SkImage> makeImageCopy(const SkIRect* subset, Mipmapped) const;
    TextureProxy* backingTextureProxy() const;

private:
    sk_sp<Device> fDevice;
    sk_sp<Image>  fImageView; // the image object returned by asImage()

    /**
     *
     * so if we add sk_sp<Device> to Graphite's Image and assign it when creating the Image for
     * the surface view, that lets us flush the device tasks if it has anything pending.
     * When the surface is deleted, it can remove the Device from the Image to not trigger any more
     * flushes because nothing will be changing the image anymore.
     *
     * However, when we reset the Device, if it has pending work (e.g. a scratch surface or a client
     * surface that's being drawn once and goes out of scope), we need to account for those tasks.
     * The imagefilter/layer scratch use case would go through snapSpecial(), so I'd been thinking
     * that that would just take the DrawTask knowing that everything else was about to fall down.
     *  -> these also do not have an SkSurface wrapper that keeps them alive
     *
     * But that doesn't work for other scratch cases like color conversion or mipmap generation where
     * the surface goes out of scope. In these cases, though, we want the tasks to go to the root
     * task list, which does happen when the Device goes out of scope.
     *  so that's fine, I think this surface-imageview scratch pattern works out fine if we remove
     * device pointer from the image view when the surface is deleted, which lets the device
     * add tasks that were pending before the image itself was referenced. and they all go to the
     * root task list, which is what we want since these types of surface scratches are either
     * client created, or used directly in response to a client action.
     *
     *
     * Is there a way to unify the layer/filter case with this?
     * They create devices directly, then snapSpecial() is called to get a special image, this
     * would flush everything to a DrawTask and currently put it into the recorder.
     *
     * we could have the drawtask stay on the Device, e.g. last task?
     *  -> just internally flushing isn't necessarily enough because the first reference to the
     *     image would grab the DrawTask off the DrawContext and then where would we be?
     * -> we could have it remember the last drawtask?
     * -> or maybe it makes sense for Device to hold on to the last task, like it snaps a task
     *    and if it's a device that goes to the root recorder, it moves the task to the list,
     *    and if it's not, it holds on to the draw task
     *
     *
     * but how does all of this work with backdrop filters?
     *  - a regular surface's device (non-scratch) can have snapSpecial() called multiple times
     *  - a scratch layer device can have snapSpecial() called multiple times if there's a backdrop
     *    filter on a grand child layer.
     *     - naively this creates a cycle within the draw tasks? or not because it explicitly
     *       is moving tasks?
     *
     *
     * scratch case first:
     *    Surface records some pending draws (into task S1)
     *    Surface pushes a layer w/o backdrop filter so we get a scratch device A
     *        device A records some pending draws to give it backdrop content (into task A1)
     *        device A pushes a layer w/ a backdrop filter to initialize scratch device B
     *             device A snaps a special image, which creates a full DrawTask and sets it on
     *                  A (A's DC has a new task)
     *             scratch device C is created explicitly to store the output of the backdrop filter
     *                  A's snapped special image is drawn into C with a shader effect
     *                  A's last DrawTask (A1) is recorded into C's current DrawTask
     *             device C snaps a special image, storing its attached task C1 (referencing A1)
     *                  C's snapped image is drawn into B, recording C1 into B's current task
     *             device B records some additional draws on top of the backdrop filtered content
     *        device B is restored, which snaps a special image, and B's last task points to B1
     *             this image is drawn into A, which appends an operation to the new current task A2
     *                 as well as a reference to B1
     *    device A is restored, which snaps A2 into a special image and then draws it into S, which
     *         adds a reference between A2 and S1.
     *
     * We get the graph:
     *
     *     S1 (for S)
     *        |- A2 (for A),
     *           |- B1 (for B)
     *              |- C1 (for C)
     *                 |- A1 (for A)
     *                    |- any uploads required for draws that came before the backdrop filter layer
     *                    |- RP task to A that produces A1 implied content
     *                 |- RP task that evaluates shader effect on A (assuming A1 content)
     *              |- optional uploads for draws that came after backdrop initialization
     *              |- RP task to B including draws that sample C (assuming C1 content)
     *           |- optional uploads for A for draws that came after B was restored to it
     *           |- RP to A that includes the draw sampling from B (assuming B1 content)
     *        |- optional uploads for S
     *        |- RP task to S that includes a draw sampling A (assuming A2 content)
     *
     *
     * How does this work for pending read count tracking and lifetime determination?
     * Does this break down when the special image goes out of scope (e.g. makeImageSnapshot or
     * makeFilteredImage that up converts it to an image but passes it to the client instead of
     * consuming it, which is how other asImage() conversions get handled). )
     *    -> some issues with this that I realized -> pending read count would have to go to the
     *       texture proxy, not the draw task if a scratch texture were broken up like this.
     *    -> if you just do an internal flush when C needs to sample A, we don't have to link the
     *       draw task in C (actually we could in theory, it would have been instantiated already
     *       when we hit the first RP)
     *    -> that actually is nice because then we don't have to do any special consideration about
     *       handling the DrawTask on the image, we just reference the last one.
     *
     * The main thing that's left open is knowing how to upgrade some scratch operations to the
     * root task list. Do mipmaps or copies aply here? Maybe not if they use SkSurfaces::RT and AsImage.
     *
     * but MakeWithFilter() definitely runs into an issue.
     *  -> the image keeps the device alive
     *  -> normally a surface would remove the device when the surface goes out of scope, and then
     *     that removes the device from the image, the device then pushes any tasks to the recorder
     *     automatically
     *  -> for filter/layer scratch devices, the image also goes out of scope, deleting the device,
     *     but that happens after the draw task has been collected by dependent tasks.
     *  -> for MakeWithFilter, we don't have a surface and the image doesn't go out of scope, so
     *     we need to explicitly move the task into the recorder
     *
     *
     *
     * so can we say that for scratch devices, we only expect a single DrawTask?
     *
     * now in the short term, when devices are just being snapped as tasks and added to the
     * root recorder list, do we have everything in a state to work with backdrop filters?
     *
     * in this case, A would be snapped into a full task, then we'd get to C reading A,
     * then B reading C, and then come back to A with more drawing operations to make a second
     * task, and that's the point in time where A becomes reusable.
     *
     * But we don't have a read count or do we?
     *  -> we'd have to store it on the texture proxy
     *
     * -> in the short term, it looks like scratch "surfaces" for mipmap gen, rescaling are all
     * using instantiated surfaces
     *  -> but I think they dont' have to be, and the logic could be rewritten to just use
     *     Device::MakeScratch() which does not instantiate the texture at all.
     *
     *
     *
     * so what's the plan:
     *   - add device to Image, clean up when surface goes away, only for surface-created image views to begin with
     *   - detect images with devices when being sampled and flush tasks to root (if they are instantiated?, assert they are instantiated?)
     *   - add temporary pending read count to TextureProxy (to be moved to DrawTask)
     *   - add device to snapped special images
     *   - update pending read count when referencing images with devices that aren't instantiated
     *   - remember last draw task on device for devices that don't go to the root task list
     *   - add refDrawTasks when referencing the above images
     *   - and instantiation logic
     *   - update mipmap/etc internal draws to use uninstantiated scratch devices
     *        - in the mipmap case, we create 2 devices uninstantiated and then we ping-pong back and forth
     *        - having only one draw task per surface isn't actually possible because there's a cycle
     *               - A draws the source (presumably in the root? otherwise becomes a child draw task)
     *               - B draws A into itself, so A is flushed into an internal RP task containing S
     *                 and A's DT is added to B's task list.
     *               - A draws B into itself, so B is flushed into an internal RP task containg A1
     *                 and B's DT is added to A's task list.
     *
     *  and so on, but then when we go to evaluate the child tasks, we start skipping DTs that have
     *  already executed, which is inaccurate. The planned DT representation can support a child referencing
     *  a parent at one point in time (representing prior state of parent), and the parent can represent the child's ending state/time.
     *
     *  - now maybe this is worked out naturally because we also have to intermingle copy texture to texture tasks
     *    that we want to execute in a careful order.
     *     -> now we could collect all of these tasks into a single task list that is then aggregated into a gen-mipmap node
     *        but regardless, it does involve multiple draw tasks for each surface, and that's annoying
     *
     *     -> or do we reorg it to use new devices with compatible dimensions? that actually might work and could
     *        work into the equivalent texture reuse scenario
     *
     *       but how would we then prevent or convey the fact that these scratch devices can't be used that
     *       way?
     *
     *   - stop pushing scratch draw tasks to the root, but have a mechanism for forcing it (makeWithFilter)
     *   - move pending read count to DrawTask
     *   - remove refdrawtask and reference the drawtask directly in child lists
    */

    friend void Flush(SkSurface*);
};

// TODO: The long-term for the public API around surfaces and flushing/submitting will likely
// be replaced with explicit control over Recorders and submitting Recordings to the Context
// directly. For now, internal tools often rely on surface/canvas flushing to control what's
// being timed (nanobench or viewer's stats layer), so we flush any pending draws to a DrawPass.
// While this won't measure actual conversion of the task list to backend command buffers, that
// should be fairly negligible since most of the work is handled in DrawPass::Make().
// Additionally flushing pending work here ensures we don't batch across or clear prior recorded
// work when looping in a benchmark, as the controlling code expects.
void Flush(sk_sp<SkSurface> surface);
void Flush(SkSurface* surface);

} // namespace skgpu::graphite

#endif // skgpu_graphite_Surface_Graphite_DEFINED
