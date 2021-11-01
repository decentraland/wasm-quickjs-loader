
#include <string>
#include <vector>
#include <memory>

#include "quickjs/quickjs.h"

#include "io/IIOModule.h"
#include "JS.h"
#include "DecentralandInterface.h"
#include "io/FDChannel.h"

#include <queue>

class JS::JSPimpl
{
public:
    IIOModule *ioModule;

    JSRuntime *runtime;
    JSContext *ctx;

    IChannel *rendererChannel;
    IChannel *debuggerChannel;

    std::unique_ptr<DecentralandInterface> dcl;
    std::queue<char> debuggerReadbuffer;
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
    pimpl->debuggerChannel = pimpl->ioModule->getChannelByKey("DEBUGGER");

    pimpl->debuggerChannel->setOnDataArrival([&](const void *data, int dataLength)
                                             {
                                                 for (int i = 0; i < dataLength; i++)
                                                 {
                                                     pimpl->debuggerReadbuffer.push(static_cast<const char *>(data)[i]);
                                                 }
                                             });

    pimpl->rendererChannel->setOnDataArrival([&](const void *data, int dataLength)
                                             { pimpl->rendererChannel->writeMessage("hello from cpp callback", 23); });

    pimpl->runtime = JS_NewRuntime();
    pimpl->ctx = JS_NewContext(pimpl->runtime);

    pimpl->dcl = std::make_unique<DecentralandInterface>(pimpl->ctx, pimpl->rendererChannel);

    int this_ptr = reinterpret_cast<int>(this);
    std::string this_number = std::to_string(this_ptr);

    setenv("QUICKJS_DEBUG_ADDRESS", this_number.c_str(), 1);
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

int JS::debugger_read(void *buffer, size_t length)
{
    char *b = static_cast<char *>(buffer);
    int l = length < pimpl->debuggerReadbuffer.size() ? length : pimpl->debuggerReadbuffer.size();
    for (int i = 0; i < l; i++)
    {
        b[i] = pimpl->debuggerReadbuffer.front();
        pimpl->debuggerReadbuffer.pop();
    }
    return l;
}

int JS::debugger_write(const void *buffer, size_t length)
{
    pimpl->debuggerChannel->writeMessage(static_cast<const char *>(buffer), length);
    return length;
}

int JS::debugger_peek()
{
    return pimpl->debuggerReadbuffer.size();
}

int JS::debugger_poll()
{
    auto channel = reinterpret_cast<FDChannel *>(pimpl->debuggerChannel);

    if (channel)
    {
        channel->poll();
        return 0;
    }
    return -1;
}