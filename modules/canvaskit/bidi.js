(function(CanvasKit){
  CanvasKit._extraInitializations = CanvasKit._extraInitializations || [];
  CanvasKit._extraInitializations.push(function() {

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

    CanvasKit.Bidi.prototype.getBidiRegions = function(text, textDirection) {
      var dir = textDirection == CanvasKit.TextDirection.LTR ? 1 : 0;
      /**
      * @type {Int32Array}
      */
      var int32Array = this._getBidiRegions(text, dir);
      return Int32ArrayToBidiRegions(int32Array);
    }

    CanvasKit.Bidi.prototype.reorderVisual = function(visualRuns) {
      /**
      * @type {Uint8Array}
      */
      var vPtr = copy1dArray(visualRuns, 'HEAPU8');
      /**
       * @type {Int32Array}
       */
      var int32Array = this._reorderVisual(vPtr, visualRuns && visualRuns.length || 0);
      freeArraysThatAreNotMallocedByUsers(vPtr, visualRuns);
      return int32Array;
    }

    CanvasKit.CodeUnits.prototype.compute = function(text) {
      /**
       * @type {Uint16Array}
       */
      var uint16Array = this._compute(text);
      return uint16Array;
    }
});
}(Module)); // When this file is loaded in, the high level object is "Module";
