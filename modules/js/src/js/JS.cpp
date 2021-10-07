
#include <string>
#include <vector>

#include "quickjs/quickjs.h"

#include "io/IIOModule.h"
#include "JS.h"
#include "DecentralandInterface.h"

class JS::JSPimpl
{
public:
    IIOModule *ioModule;

    JSRuntime *runtime;
    JSContext *ctx;

    std::unique_ptr<DecentralandInterface> dcl;
};

JS::~JS()
{
    JS_FreeContext(pimpl->ctx);
    JS_FreeRuntime(pimpl->runtime);
}

JS::JS(IIOModule *value) : pimpl(new JSPimpl)
{
    pimpl->ioModule = value;

    pimpl->ioModule->getRendererChannel()->setOnDataArrival([&](const void *data, int dataLength)
                                                            { pimpl->ioModule->getRendererChannel()->writeMessage("hello kernel", 12); });

    pimpl->runtime = JS_NewRuntime();
    pimpl->ctx = JS_NewContext(pimpl->runtime);

    pimpl->dcl = std::make_unique<DecentralandInterface>(pimpl->ctx, value->getRendererChannel());

    // Testing JS
    std::string code = R"(
        if (dcl.DEBUG) {
            dcl.log("hello world from quickjs! i'm in debug mode")
        } else {
            dcl.log("hello world from quickjs! i'm in non-debug mode")
        }

        dcl.onUpdate(dt => {
            dcl.log(`onUpdate called from quickjs ${dt}`)
        })

    
        dcl.callRpc().then(() => {
            dcl.log('promise resolve')
        }).catch(() => {
            dcl.log('promise reject'    )
        })
    )";

    JSValue eval = JS_Eval(pimpl->ctx, code.c_str(), code.length(), "index.js", 0);
    if (JS_IsException(eval)){
        JSValue exception_val = JS_GetException(pimpl->ctx);
        const char* str = JS_ToCString(pimpl->ctx, exception_val);
        printf("Exception on eval -> %s ", str);
        JS_FreeCString(pimpl->ctx, str);

        JSValue val = JS_GetPropertyStr(pimpl->ctx, exception_val, "stack");
        if (!JS_IsUndefined(val)) {
            const char* str2 = JS_ToCString(pimpl->ctx, val);
            printf("stack on eval -> %s ", str2);
            JS_FreeCString(pimpl->ctx, str2);
        }
    }else{
        const char* str = JS_ToCString(pimpl->ctx, eval);
        printf("Eval result -> %s ", str);
    }
}

void JS::loop(double dt)
{

    // JSContext *ctx1;
    // int err;

    // /* execute the pending jobs */
    // for (;;)
    // {
    //     err = JS_ExecutePendingJob(pimpl->runtime, &ctx1);
    //     if (err <= 0)
    //     {
    //         if (err < 0)
    //         {
    //             // js_std_dump_error(ctx1);
    //         }
    //         break;
    //     }
    // }

    pimpl->dcl->emitUpdate(dt);
}