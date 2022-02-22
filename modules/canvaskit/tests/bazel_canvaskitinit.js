// The increased timeout is especially needed with larger binaries
// like in the debug/gpu build
jasmine.DEFAULT_TIMEOUT_INTERVAL = 60000;

let CanvasKit = null;
const _LoadCanvasKit = new Promise((resolve, reject) => {
    console.log('canvaskit loading', new Date());
    CanvasKitInit({
        locateFile: (file) => '/base/skia/modules/canvaskit/canvaskit_wasm/'+file,
    }).then((loaded) => {
        console.log('canvaskit loaded', new Date());
        CanvasKit = loaded;
        resolve();
    }).catch((e) => {
        console.error('canvaskit failed to load', new Date(), e);
        reject();
    });
});

let ReportServerOnline = false;
const _TestReportServer = new Promise((resolve, reject) => {
    fetch("/gold_rpc/healthz").then((resp) => {
        if (resp.status === 200) {
            ReportServerOnline = true;
        }
        resolve();
    }).catch((e) => {
        console.log('Server for reporting results was not up', e)
        reject();
    });
});

const EverythingLoaded = Promise.all([_LoadCanvasKit, _TestReportServer]);