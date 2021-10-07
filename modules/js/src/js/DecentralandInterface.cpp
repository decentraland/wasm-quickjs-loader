#include "quickjs/quickjs.h"
#include "DecentralandInterface.h"
#include "helper/Function.h"
#include "io/IChannel.h"

uint64_t DecentralandInterface::Promise::counter = 0;

#define BIND_JSC_METHOD(uniqueId, ctx, obj, classMethod, functionName)                                                                              \
    {                                                                                                                                               \
        std::function<JSCFunction> f = std::bind(classMethod,                                                                                       \
                                                 this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4); \
        JS_SetPropertyStr(ctx, obj, functionName,                                                                                                   \
                          JS_NewCFunction(ctx, get_fn_ptr<uniqueId>(f), functionName, 1));                                                          \
    }

DecentralandInterface::DecentralandInterface(JSContext *context, IChannel *kernelChannel)
    : ctx(context), kernelChannel(kernelChannel)
{

    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue dcl = JS_NewObject(ctx);

    JS_SetPropertyStr(ctx, dcl, "DEBUG", JS_NewBool(ctx, 0));

    BIND_JSC_METHOD(0, ctx, dcl, &DecentralandInterface::log, "log")
    BIND_JSC_METHOD(1, ctx, dcl, &DecentralandInterface::onUpdate, "onUpdate")
    BIND_JSC_METHOD(2, ctx, dcl, &DecentralandInterface::loadModule, "loadModule")
    BIND_JSC_METHOD(3, ctx, dcl, &DecentralandInterface::callRpc, "callRpc")

    JS_SetPropertyStr(ctx, global_obj, "dcl", dcl);
}

JSValue DecentralandInterface::onUpdate(JSContext *ctx, JSValueConst this_val,
                                        int argc, JSValueConst *argv)
{
    if (argc == 1)
    {
        auto funct = argv[0];
        if (JS_IsFunction(ctx, funct))
        {
            JSValue copy = JS_DupValue(ctx, funct);
            onUpdateFunction.push_back(copy);
            return JS_UNDEFINED;
        }
    }

    // Unhappy case
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::log(JSContext *ctx, JSValueConst this_val,
                                   int argc, JSValueConst *argv)
{
    int i;
    const char *str;
    size_t len;

    for (i = 0; i < argc; i++)
    {
        if (i != 0)
            putchar(' ');
        str = JS_ToCStringLen(ctx, &len, argv[i]);
        if (!str)
            return JS_EXCEPTION;
        fwrite(str, 1, len, stdout);
        JS_FreeCString(ctx, str);
    }
    putchar('\n');
    return JS_UNDEFINED;
}

void DecentralandInterface::emitUpdate(float dt)
{
    JSValue jsDtValue = JS_NewFloat64(ctx, dt);

    for (const JSValue cb : onUpdateFunction)
    {
        if (JS_IsFunction(ctx, cb))
        {
            JSValue ret = JS_Call(ctx, cb, JS_UNDEFINED, 1, (JSValueConst *)&jsDtValue);
            if (JS_IsException(ret)){
                JSValue exception_val = JS_GetException(ctx);
                const char* str = JS_ToCString(ctx, exception_val);
                printf("Exception on eval -> %s ", str);
                JS_FreeCString(ctx, str);

                JSValue val = JS_GetPropertyStr(ctx, exception_val, "stack");
                if (!JS_IsUndefined(val)) {
                    const char* str2 = JS_ToCString(ctx, val);
                    printf("stack on eval -> %s ", str2);
                    JS_FreeCString(ctx, str2);
                }
            }
        }
    }

    JS_FreeValue(ctx, jsDtValue);
}

JSValue DecentralandInterface::callRpc(JSContext *ctx, JSValueConst this_val,
                                       int argc, JSValueConst *argv)
{

    auto newPromise = std::make_unique<Promise>();
    JSValue promise = JS_NewPromiseCapability(ctx, newPromise->resolving_functions);
    newPromise->promise = promise;
    onCallRpc.push_back(std::move(newPromise));
    
    return promise;
}

JSValue DecentralandInterface::loadModule(JSContext *ctx, JSValueConst this_val,
                                          int argc, JSValueConst *argv)
{

    return JS_UNDEFINED;
}