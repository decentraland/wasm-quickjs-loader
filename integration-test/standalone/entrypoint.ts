declare const globalThis: { start: Function };

import { run } from "@dcl/wasm-runtime";
const loaderWasmStr = require("!!binary-loader!./../../dist/loader.bin");

export async function startModule() {
  // if necessary, convert the imported string to Uint8Array:
  let wasmBytes = new Uint8Array(loaderWasmStr.length);
  for (let i = 0; i < loaderWasmStr.length; i++) {
    wasmBytes[i] = loaderWasmStr.charCodeAt(i);
  }

  return await run({ wasmBytes });
}

globalThis.start = () => {
  startModule().then(async (result) => {
    await result.start();

    result.metaverseWrapper.Renderer0FDChannel.setDataArriveCallback(
      (data: Uint8Array) => {
        console.log(Buffer.from(data).toString("utf-8"));
      }
    );
    async function update() {
      result.metaverseWrapper.Renderer0FDChannel.writeMessage(
        new TextEncoder().encode("hello scene!")
      );

      const resultOut = await result.metaverseWrapper.wasmFs.getStdOut()
      if (resultOut) {
        console.log(resultOut)
        await result.metaverseWrapper.wasmFs.fs.writeFileSync('/dev/stdout', '')
      }

      result.update(0.5);
      setTimeout(update, 500);
    }

    setTimeout(update, 500);
  });
};
