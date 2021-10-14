declare const globalThis: { start: Function };

import { run } from "@dcl/wasm-runtime";
import { CHANNELS } from "@dcl/wasm-runtime/dist/io/fs";
import { IChannel } from "@dcl/wasm-runtime/dist/io/IChannel";

const loaderWasmStr = require("!!binary-loader!./../../dist/loader.bin");

export async function startModule() {
  // if necessary, convert the imported string to Uint8Array:
  let wasmBytes = new Uint8Array(loaderWasmStr.length);
  for (let i = 0; i < loaderWasmStr.length; i++) {
    wasmBytes[i] = loaderWasmStr.charCodeAt(i);
  }

  const gameJsSrc = await (await fetch("game.js")).text();
  const result = await run({ wasmBytes });

  result.metaverseWrapper.wasmFs.fs.writeFileSync("/game.js", gameJsSrc);

  return result;
}

globalThis.start = () => {
  startModule().then(async (result) => {
    if (!result.metaverseWrapper.channels.has(CHANNELS.RENDERER.KEY)) {
      throw new Error("Rendere channel is undefined");
    }

    const rendererChannel = result.metaverseWrapper.channels.get(
      CHANNELS.RENDERER.KEY
    ) as IChannel;

    rendererChannel.setDataArriveCallback((data: Uint8Array) => {
      console.log(`WASM->Kernel: '${Buffer.from(data).toString("utf-8")}`);
    });

    await result.start();

    async function update() {
      rendererChannel.writeMessage(new TextEncoder().encode("hello scene!"));

      const resultOut = await result.metaverseWrapper.wasmFs.getStdOut();
      if (resultOut) {
        console.log(resultOut);
        await result.metaverseWrapper.wasmFs.fs.writeFileSync(
          "/dev/stdout",
          ""
        );
      }

      result.update(0.5);
      setTimeout(update, 500);
    }

    setTimeout(update, 1000);
  });
};
