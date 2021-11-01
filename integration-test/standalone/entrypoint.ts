declare const globalThis: { start: Function };

import { run } from "@dcl/wasm-runtime";
import { CHANNELS } from "@dcl/wasm-runtime/dist/io/fs";
import { IChannel } from "@dcl/wasm-runtime/dist/io/IChannel";

const loaderWasmStr = require("!!binary-loader!./../../dist/loader.bin");

// const bigMemory = new WebAssembly.Memory({
//   initial: 10000,
//   maximum: 10000
// });
// console.log(bigMemory);

export async function startModule() {
  // if necessary, convert the imported string to Uint8Array:
  let wasmBytes = new Uint8Array(loaderWasmStr.length);
  for (let i = 0; i < loaderWasmStr.length; i++) {
    wasmBytes[i] = loaderWasmStr.charCodeAt(i);
  }

  const gameJsSrc = await (await fetch("index.js")).text();
  // const result = await run({ wasmBytes , customMemory: bigMemory });
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

    const scenePtr = await result.start();

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

      try{
        result.updateScene(scenePtr, 1.0);
        setTimeout(update, 1000);
      }catch(err){
        console.error(err)
      }
    }

    setTimeout(update, 1000);
  });
};
