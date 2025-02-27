(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

    CanvasKit.Bidi.Make = function() {
      return CanvasKit.Bidi._Make();
    };

    CanvasKit.Bidi.prototype.getBidiRegions = function(text, textDirection) {
      /**
      * @type {Int32Array}
      */
      var int32Array = this._getBidiRegions(text, textDirection);
      return Int32ArrayToBidiRegions(int32Array);
    }

    CanvasKit.Bidi.prototype.reorderVisual = function(visualRuns) {
      /**
      * @type {Int32Array}
      */
      var vPtr = copy1dArray(visualRuns, 'HEAPU8');
      var bytesPerRun = 4;
      var lPtr = CanvasKit._malloc(visualRuns.length * bytesPerRun);
      this._reorderVisual(vPtr, visualRuns && visualRuns.length || 0, lPtr);
      var logicalRuns = new Int32Array(CanvasKit.HEAPU32.buffer, lPtr, visualRuns.length);
      freeArraysThatAreNotMallocedByUsers(vPtr, visualRuns);
      var result = Uint16Array.from(logicalRuns);
      CanvasKit._free(lPtr);
      return result;
    }

    function Int32ArrayToBidiRegions(int32Array) {
      if (!int32Array || !int32Array.length) {
        return [];
      }
      var ret = [];
      for (var i = 0; i < int32Array.length; i+=3) {
        var start = int32Array[i];
        var end = int32Array[i+1];
        var level = int32Array[i+2];
        ret.push({'start': start, 'end': end, 'level': level});
      }
      CanvasKit._free(int32Array.byteOffset);
      return ret;
    }
});
}(Module)); // When this file is loaded in, the high level object is "Module";
