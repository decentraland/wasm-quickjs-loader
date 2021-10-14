
#include <string>
#include <vector>
#include <memory>

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

    IChannel *rendererChannel;

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

    pimpl->rendererChannel = pimpl->ioModule->getChannelByKey("RENDERER");

    pimpl->rendererChannel->setOnDataArrival([&](const void *data, int dataLength)
                                             { pimpl->rendererChannel->writeMessage("hello from cpp callback", 23); });

    pimpl->runtime = JS_NewRuntime();
    pimpl->ctx = JS_NewContext(pimpl->runtime);

    pimpl->dcl = std::make_unique<DecentralandInterface>(pimpl->ctx, pimpl->rendererChannel);
}

void JS::loop(double dt)
{

    JSContext *ctx1;
    int err;

    /* execute the pending jobs */
    for (;;)
    {
        err = JS_ExecutePendingJob(pimpl->runtime, &ctx1);
        if (err <= 0)
        {
            if (err < 0)
            {
                // js_std_dump_error(ctx1);
            }
            break;
        }
    }

    pimpl->dcl->emitUpdate(dt);
}

void JS::eval(const char *buf, size_t bufSize, std::string file)
{

    JSValue eval = JS_Eval(pimpl->ctx, buf, bufSize, file.c_str(), 0);
    if (JS_IsException(eval))
    {
        JSValue exception_val = JS_GetException(pimpl->ctx);
        const char *str = JS_ToCString(pimpl->ctx, exception_val);
        printf("Exception on eval -> %s ", str);
        JS_FreeCString(pimpl->ctx, str);

        JSValue val = JS_GetPropertyStr(pimpl->ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
        {
            const char *str2 = JS_ToCString(pimpl->ctx, val);
            printf("stack on eval -> %s ", str2);
            JS_FreeCString(pimpl->ctx, str2);
        }
    }
    else
    {
        const char *str = JS_ToCString(pimpl->ctx, eval);
        printf("Eval result -> %s ", str);
    }
}