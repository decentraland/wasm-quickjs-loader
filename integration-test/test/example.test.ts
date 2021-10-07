import { readFileSync } from "fs";
import { resolve } from "path";
import { run } from "@dcl/wasm-runtime";

async function loadStartAndUpdateOnce(filePath: string) {
  // Fetch our Wasm File
  const file = readFileSync(resolve(filePath));

  // Instantiate the WebAssembly file
  const wasmBytes = new Uint8Array(file);
  const result = await run({ wasmBytes });

  await result.start();

  // result.metaverseWrapper.Renderer0FDChannel.writeMessage(
  //   new TextEncoder().encode("hello loader")
  // );

  // result.metaverseWrapper.Renderer0FDChannel.setDataArriveCallback((data) => {
  //   console.log(
  //     `RendererDataArrival => "${Buffer.from(data).toString("utf-8")}"`
  //   );
  // });

  for (let i = 0; i < 5; i++) {
    await result.update(1 / 30);
  }

  console.log(`stdout: '${await result.wasmFs.getStdOut()}'`);
}

async function main() {
  console.log("Testing C++ module...");
  await loadStartAndUpdateOnce("../dist/loader.wasm");
}

main().catch((err) => {
  console.error(err);
  process.exit(1);
});
