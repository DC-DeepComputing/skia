#!/usr/bin/env python3

# Copyright 2025 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from http.server import ThreadingHTTPServer, SimpleHTTPRequestHandler

HTML_FILENAME = "dm.html"
WORKER_FILENAME = "dm_worker.js"

HTML = """<!doctype html>
<meta charset="utf-8" />
<button id="go">Go</button>
<button id="dl" disabled="true">Download images.zip</button>
<div id="status">Idle</div>
<pre id="out"></pre>

<script type="module">
  const out = document.getElementById("out");
  const dl = document.getElementById("dl");
  const status = document.getElementById("status");

  function onZipAvailable(zipBuffer) {
    dl.disabled = false;
    dl.onclick = () => {
      const url = URL.createObjectURL(new Blob([zipBuffer], { type: "application/zip" }));
      const a = document.createElement("a");
      a.href = url;
      a.download = "images.zip";
      a.click();
      a.remove();
      setTimeout(() => URL.revokeObjectURL(url), 0); // allow gc of zip buffer
    };
  }

  function run(argv) {
    const w = new Worker("./dm_worker.js", { type: "module" });

    w.onmessage = (ev) => {
      const m = ev.data || {};
      if (m.type === "status") status.textContent = m.value;
      if (m.type === "out") out.textContent += m.text + "\\n";
      if (m.type === "done") w.terminate();
      if (m.type === "zip") onZipAvailable(m.zip);
    };

    w.onerror = (e) => {
      status.textContent = "Worker error";
      w.terminate();
    };

    w.postMessage({ argv });
  }

  document.getElementById("go").onclick = () => {
    out.textContent = "";
    status.textContent = "Starting...";
    dl.disabled = true;
    run(["-w", "/out", "-t", "/tmp", "-m", "complexclip_bw_invert"]);
  };
</script>
"""

WORKER = """import createModule from "./dm.js";

function vfsDirToFileMap(FS, dir) {
  const join = (a, b) => (a.endsWith("/") ? a + b : `${a}/${b}`);
  const files = Object.create(null);

  function walk(vfsPath, relPrefix) {
    const pfx = relPrefix.endsWith("/") ? relPrefix : `${relPrefix}/`;
    for (const name of FS.readdir(vfsPath)) {
      if (name === "." || name === "..") continue;

      const child = join(vfsPath, name);
      const st = FS.stat(child);

      if (FS.isDir(st.mode)) {
        walk(child, `${pfx}${name}/`);
      } else if (FS.isFile(st.mode)) {
        files[`${pfx}${name}`] = FS.readFile(child); // Uint8Array
      }
    }
  }

  walk(dir, dir);
  return files;
}

function printDir(Module, path) {
  const FS = Module.FS;
  if (!FS) {
    console.error("Module.FS is undefined (filesystem likely not enabled/exported).");
    return;
  }

  const filemap = vfsDirToFileMap(FS, path);
  console.log(filemap);
}

async function zipDir(Module, path) {
  const { zipSync } = await import("https://cdn.jsdelivr.net/npm/fflate@0.8.2/esm/browser.js");
  const FS = Module.FS;
  const filemap = vfsDirToFileMap(FS, path);
  const compressed = zipSync(filemap); // Uint8Array
  const ab = compressed.buffer.slice(
    compressed.byteOffset,
    compressed.byteOffset + compressed.byteLength
  );
  return ab; // ArrayBuffer
}

onmessage = async (ev) => {
  const argv = (ev.data && ev.data.argv) ? ev.data.argv : [];

  postMessage({ type: "status", value: "Loading..." });

  const Module = await createModule({
    noInitialRun: true,
    print: (s) => postMessage({ type: "out", text: String(s) }),
    printErr: (s) => postMessage({ type: "out", text: String(s) }),
    onAbort: (r) => {
      postMessage({ type: "status", value: "Abort: " + String(r) });
      postMessage({ type: "done" });
    },
  });

  Module.onExit = async () => {
    const imgOutPath = "/out";
    postMessage({ type: "status", value: `Collecting image results...` });

    const zipBuff = await zipDir(Module, imgOutPath);
    postMessage({ type: "zip", zip: zipBuff }, [zipBuff]);
    postMessage({ type: "status", value: "Done" });
    postMessage({ type: "done" });
  };

  postMessage({ type: "status", value: "Running..." });
  Module.callMain(argv);
};
"""

class H(SimpleHTTPRequestHandler):
    def end_headers(self):
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        super().end_headers()

def main():
    with open(HTML_FILENAME, "w", encoding="utf-8") as f:
        f.write(HTML)
    with open(WORKER_FILENAME, "w", encoding="utf-8") as f:
        f.write(WORKER)

    print(f"Visit: http://localhost:8000/{HTML_FILENAME}")
    ThreadingHTTPServer(("localhost", 8000), H).serve_forever()

if __name__ == "__main__":
    main()
