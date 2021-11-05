import {CanvasKitInit as CKInit} from "canvaskit-wasm";

declare const CanvasKitInit: typeof CKInit;

async function init() {
    const CK = await CanvasKitInit({
        locateFile: (file: string) => "node_modules/canvaskit-wasm/bin/" + file
    });
    const color = CK.Color(1,2,3,4);
    console.log(color);
}
init();
