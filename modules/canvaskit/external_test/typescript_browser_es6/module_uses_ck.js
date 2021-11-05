var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
import { CanvasKitInit } from "./node_modules/canvaskit-wasm/bin/canvaskit.js";
function init() {
    return __awaiter(this, void 0, void 0, function* () {
        const CK = yield CanvasKitInit({
            locateFile: (file) => __dirname + "FIXME/bin/" + file
        });
        const color = CK.Color(1, 2, 3, 4);
        console.log(color);
    });
}
init();
