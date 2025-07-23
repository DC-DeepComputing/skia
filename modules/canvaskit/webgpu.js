// Adds compile-time JS functions to augment the CanvasKit interface.
// Specifically, anything that should only be on the WebGL version of canvaskit.
// Functions in this file are supplemented by cpu.js.
(function(CanvasKit){
    CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
    CanvasKit._extraInitializations.push(function() {
      CanvasKit.MakeGPUContext = async function() {
        let ctx = { '_ctx': await this._MakeGPUContext() };
        return ctx;
      };

      CanvasKit.MakeGPUCanvasContext = function(gpuCtx, canvas) {
        var canvasCtx = this._MakeGPUCanvasContext(gpuCtx._ctx, canvas.id, canvas.width, canvas.height);
        var context = { '_inner': canvasCtx };
        context['requestAnimationFrame'] = function(callback) {
          requestAnimationFrame(function() {
            const surface = CanvasKit.MakeGPUCanvasSurface(context);
            if (!surface) {
              console.error('Failed to initialize Surface for current canvas swapchain texture');
              return;
            }
            callback(surface.getCanvas());
            surface.flush();
            surface.dispose();
          });
        };
        return context;
      };

      // CanvasKit.MakeGPUCanvasContext = function(devCtx, canvas, opts) {
      //   var canvasCtx = canvas.getContext('webgpu');
      //   if (!canvasCtx) {
      //     return null;
      //   }

      //   let format = (opts && opts.format) ? opts.format : navigator.gpu.getPreferredCanvasFormat();
      //   // GPUCanvasConfiguration
      //   canvasCtx.configure({
      //       device: devCtx._device,
      //       format: format,
      //       alphaMode: (opts && opts.alphaMode) ? opts.alphaMode : undefined,
      //   });

      //   var context = {
      //     '_inner': canvasCtx,
      //     '_deviceContext': devCtx,
      //     '_textureFormat': format,
      //   };
      //   context['requestAnimationFrame'] =  function(callback) {
      //     requestAnimationFrame(function() {
      //       const surface = CanvasKit.MakeGPUCanvasSurface(context);
      //       if (!surface) {
      //         console.error('Failed to initialize Surface for current canvas swapchain texture');
      //         return;
      //       }
      //       callback(surface.getCanvas());
      //       surface.flush();
      //       surface.dispose();
      //     });
      //   };
      //   return context;
      // };

      CanvasKit.MakeGPUCanvasSurface = function(canvasCtx) {
        let surface = this.MakeGPUTextureSurface(canvasCtx._inner);
        surface._canvasCtx = canvasCtx;
        return surface;
      };

      CanvasKit.MakeGPUTextureSurface = function (canvasCtx) {
          return this._MakeGPUTextureSurface(canvasCtx);
      };

      // CanvasKit.Surface.prototype.assignCurrentSwapChainTexture = function() {
      //   // This feature is only supported for a Surface that was created via MakeGPUCanvasSurface.
      //   if (!this._canvasContext) {
      //     console.log('Surface is not bound to a canvas context');
      //     return false;
      //   }
      //   let ctx = this._canvasContext._inner;
      //   return this._replaceBackendTexture(
      //       CanvasKit.JsValStore.add(ctx.getCurrentTexture()),
      //       CanvasKit.WebGPU.TextureFormat.indexOf(this._canvasContext._textureFormat),
      //       ctx.canvas.width, ctx.canvas.height);
      // };

      CanvasKit.Surface.prototype.getCanvas = function() {
        if (!this._canvasCtx) {
          var c = this._getCanvas();
          c._context = this._context;
          return c;
        }
        return CanvasKit._GetGPUCanvas(this._canvasCtx._inner);
      };

      CanvasKit.Surface.prototype.flush = function(dirtyRect) {
        if (!this._canvasCtx && this._canvas) {
          var pixels = new Uint8ClampedArray(CanvasKit.HEAPU8.buffer, this._pixelPtr, this._pixelLen);
          var imageData = new ImageData(pixels, this._width, this._height);

          if (!dirtyRect) {
            this._canvas.getContext('2d').putImageData(imageData, 0, 0);
          } else {
            this._canvas.getContext('2d').putImageData(imageData, 0, 0,
                                                       dirtyRect[0], dirtyRect[1],
                                                       dirtyRect[2] - dirtyRect[0],
                                                       dirtyRect[3] - dirtyRect[1]);
          }
          return;
        }
        return CanvasKit._Flush(this._canvasCtx._inner);
      }

      CanvasKit.Surface.prototype.requestAnimationFrame = function(callback, dirtyRect) {
        if (!this.reportBackendTypeIsGPU()) {
          return this._requestAnimationFrameInternal(callback, dirtyRect);
        }

        return requestAnimationFrame(function() {
          // Replace the render target of the Surface with the current swapchain surface if this is
          // bound to a canvas context.
          // if (this._canvasContext && !this.assignCurrentSwapChainTexture()) {
          //   console.log('failed to replace GPU backend texture');
          //   return;
          // }
          callback(this.getCanvas());
          this.flush(dirtyRect);
        }.bind(this));
      };

      CanvasKit.Surface.prototype.drawOnce = function(callback, dirtyRect) {
        if (!this.reportBackendTypeIsGPU()) {
          this._drawOnceInternal(callback, dirtyRect);
          return;
        }

        requestAnimationFrame(function() {
          // Replace the render target of the Surface with the current swapchain surface if this is
          // bound to a canvas context.
          // if (this._canvasContext && !this.assignCurrentSwapChainTexture()) {
          //   console.log('failed to replace GPU backend texture');
          //   return;
          // }
          callback(this.getCanvas());
          this.flush(dirtyRect);
          this.dispose();
        }.bind(this));
      };
    });
}(Module));  // When this file is loaded in, the high level object is "Module".
