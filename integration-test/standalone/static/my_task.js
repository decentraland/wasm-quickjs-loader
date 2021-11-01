
postMessage("Message from Debugger Worker");

var arrayBuffer = null;
var heap8 = null;
var heap32 = null;
const debuggerWs = new WebSocket("ws://localhost:7666")

onmessage = function (oEvent) {
  if (oEvent.data instanceof SharedArrayBuffer) {
    postMessage("Receiveing the ArrayBuffer");
    arrayBuffer = oEvent.data;
    heap8 = new Uint8Array(arrayBuffer);
    heap32 = new Int32Array(arrayBuffer);  
    heap32[0] = 0
    heap32[1] = 0
  }else{
    postMessage("Receiveing the ArrayBuffer");
    console.log(`{debug} sending "${oEvent.data}"`)
    debuggerWs.send(oEvent.data)
  }
};


let vsCodeMsgs = []

debuggerWs.onopen = () => console.log(`{debug} onopen`)
debuggerWs.onclose = () => console.log(`{debug} onclose`)
debuggerWs.onerror = (e) => console.log(`{debug} onerror ${e}`)
debuggerWs.onmessage = async (msg) => {
  const arr = await msg.data.arrayBuffer()
  const buffer = new Uint8Array(arr)
  vsCodeMsgs.push(buffer)
}


async function tick() {
  if (arrayBuffer && vsCodeMsgs.length > 0){
    for (var i = 0; i < vsCodeMsgs.length; i++){
      console.log("{debug-ws} from vs-code", new TextDecoder().decode(vsCodeMsgs[i]), heap32[1])
      
      while (heap32[1] !==0){
        await new Promise(resolve => setTimeout(resolve, 0))
      }

      heap32[0] = 1
      heap8.set(vsCodeMsgs[i], heap32[1] + 8)
      heap32[1] += vsCodeMsgs[i].length
      heap32[0] = 0
    }
  }
  setTimeout(tick, 100);
}

tick();