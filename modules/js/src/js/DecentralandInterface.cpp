
#include "DecentralandInterface.h"
#include "io/IChannel.h"

#include <sstream>
#include "nlohmann/json.hpp"

uint64_t DecentralandInterface::Promise::counter = 1;

DecentralandInterface::DecentralandInterface(JSContext *context, IChannel *kernelChannel)
    : ctx(context), kernelChannel(kernelChannel)
{

    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue dcl = JS_NewObject(ctx);
    JSValue this_ptr = JS_NewInt64(ctx, reinterpret_cast<int64_t>(this));

    JS_SetPropertyStr(ctx, dcl, "__ptr", this_ptr);
    JS_SetPropertyStr(ctx, dcl, "DEBUG", JS_NewBool(ctx, 1));

    JS_SetPropertyStr(ctx, dcl, "log", JS_NewCFunction(ctx, &DecentralandInterface::log, "log", 1));
    JS_SetPropertyStr(ctx, dcl, "onUpdate", JS_NewCFunction(ctx, &DecentralandInterface::onUpdate, "onUpdate", 1));
    JS_SetPropertyStr(ctx, dcl, "loadModule", JS_NewCFunction(ctx, &DecentralandInterface::loadModule, "loadModule", 1));
    JS_SetPropertyStr(ctx, dcl, "callRpc", JS_NewCFunction(ctx, &DecentralandInterface::callRpc, "callRpc", 1));
    JS_SetPropertyStr(ctx, dcl, "onEvent", JS_NewCFunction(ctx, &DecentralandInterface::onEvent, "onEvent", 1));
    JS_SetPropertyStr(ctx, dcl, "onStart", JS_NewCFunction(ctx, &DecentralandInterface::onStart, "onStart", 1));
    JS_SetPropertyStr(ctx, dcl, "updateEntity", JS_NewCFunction(ctx, &DecentralandInterface::updateEntity, "updateEntity", 1));
    JS_SetPropertyStr(ctx, dcl, "error", JS_NewCFunction(ctx, &DecentralandInterface::error, "error", 1));
    JS_SetPropertyStr(ctx, dcl, "openExternalUrl", JS_NewCFunction(ctx, &DecentralandInterface::openExternalUrl, "openExternalUrl", 1));
    JS_SetPropertyStr(ctx, dcl, "openNFTDialog", JS_NewCFunction(ctx, &DecentralandInterface::openNFTDialog, "openNFTDialog", 1));
    JS_SetPropertyStr(ctx, dcl, "onStart", JS_NewCFunction(ctx, &DecentralandInterface::onStart, "onStart", 1));
    JS_SetPropertyStr(ctx, dcl, "addEntity", JS_NewCFunction(ctx, &DecentralandInterface::addEntity, "addEntity", 1));
    JS_SetPropertyStr(ctx, dcl, "removeEntity", JS_NewCFunction(ctx, &DecentralandInterface::removeEntity, "removeEntity", 1));
    JS_SetPropertyStr(ctx, dcl, "updateEntityComponent", JS_NewCFunction(ctx, &DecentralandInterface::updateEntityComponent, "updateEntityComponent", 1));
    JS_SetPropertyStr(ctx, dcl, "attachEntityComponent", JS_NewCFunction(ctx, &DecentralandInterface::attachEntityComponent, "attachEntityComponent", 1));
    JS_SetPropertyStr(ctx, dcl, "removeEntityComponent", JS_NewCFunction(ctx, &DecentralandInterface::removeEntityComponent, "removeEntityComponent", 1));
    JS_SetPropertyStr(ctx, dcl, "setParent", JS_NewCFunction(ctx, &DecentralandInterface::setParent, "setParent", 1));
    JS_SetPropertyStr(ctx, dcl, "query", JS_NewCFunction(ctx, &DecentralandInterface::query, "query", 1));
    JS_SetPropertyStr(ctx, dcl, "componentCreated", JS_NewCFunction(ctx, &DecentralandInterface::componentCreated, "componentCreated", 1));
    JS_SetPropertyStr(ctx, dcl, "componentDisposed", JS_NewCFunction(ctx, &DecentralandInterface::componentDisposed, "componentDisposed", 1));
    JS_SetPropertyStr(ctx, dcl, "componentUpdated", JS_NewCFunction(ctx, &DecentralandInterface::componentUpdated, "componentUpdated", 1));
    JS_SetPropertyStr(ctx, dcl, "onEvent", JS_NewCFunction(ctx, &DecentralandInterface::onEvent, "onEvent", 1));
    JS_SetPropertyStr(ctx, dcl, "subscribe", JS_NewCFunction(ctx, &DecentralandInterface::subscribe, "subscribe", 1));
    JS_SetPropertyStr(ctx, dcl, "unsubscribe", JS_NewCFunction(ctx, &DecentralandInterface::unsubscribe, "unsubscribe", 1));

    JS_SetPropertyStr(ctx, global_obj, "dcl", dcl);
}

JSValue DecentralandInterface::onUpdate(JSContext *ctx, JSValueConst this_val,
                                        int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    if (argc == 1)
    {
        auto funct = argv[0];
        if (JS_IsFunction(ctx, funct))
        {
            JSValue copy = JS_DupValue(ctx, funct);
            ptr->onUpdateFunction.push_back(copy);
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

    str = JS_ToCStringLen(ctx, &len, JS_JSONStringify(ctx, this_val, JS_UNDEFINED, 2));
    if (!str)
        return JS_EXCEPTION;
    fwrite(str, 1, len, stdout);
    JS_FreeCString(ctx, str);

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
            if (JS_IsException(ret))
            {
                JSValue exception_val = JS_GetException(ctx);
                const char *str = JS_ToCString(ctx, exception_val);
                printf("Exception on eval -> %s ", str);
                JS_FreeCString(ctx, str);

                JSValue val = JS_GetPropertyStr(ctx, exception_val, "stack");
                if (!JS_IsUndefined(val))
                {
                    const char *str2 = JS_ToCString(ctx, val);
                    printf("stack on eval -> %s ", str2);
                    JS_FreeCString(ctx, str2);
                }
            }
        }
    }

    JS_FreeValue(ctx, jsDtValue);

    static double counter = 0.0;
    counter += dt;

    if (counter > 10.0)
    {
        for (const auto &promise : promises)
        {
            JS_Call(ctx, promise->resolving_functions[0], JS_UNDEFINED, 0, NULL);
        }
    }
}

DecentralandInterface::Promise *DecentralandInterface::createPromise()
{
    std::unique_ptr<Promise> newPromise = std::make_unique<Promise>();
    JSValue promise = JS_NewPromiseCapability(ctx, newPromise->resolving_functions);
    newPromise->promise = promise;
    promises.push_back(std::move(newPromise));
    return newPromise.get();
}

void DecentralandInterface::sendToRuntime(std::string methodName, JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, uint64_t promiseId)
{
    std::stringstream ss;
    ss << "{\"method\":\"" << methodName << "\",\"params\":[";
    for (int i = 0; i < argc; i++)
    {
        JSValue json = JS_JSONStringify(ctx, argv[i], JS_NULL, JS_UNDEFINED);
        const char *str = JS_ToCString(ctx, json);
        ss << str;
        if (i < argc - 1)
        {
            ss << ",";
        }
    }
    ss << "],\"promiseId\":" << promiseId << "}";

    std::string buf = ss.str();

    kernelChannel->writeMessage(buf.c_str(), buf.length());
}

JSValue DecentralandInterface::callRpc(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    auto promise = ptr->createPromise();
    ptr->sendToRuntime("callRpc", ctx, this_val, argc, argv, promise->id);
    return promise->promise;
}

JSValue DecentralandInterface::loadModule(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    auto promise = ptr->createPromise();
    ptr->sendToRuntime("loadModule", ctx, this_val, argc, argv, promise->id);
    return promise->promise;
}

JSValue DecentralandInterface::onEvent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    // callKernelRPC("onEvent", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::onStart(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    // callKernelRPC("onStart", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::updateEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("updateEntity", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("error", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::openExternalUrl(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("openExternalUrl", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::openNFTDialog(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("openNFTDialog", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::addEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("addEntity", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::removeEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("removeEntity", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::updateEntityComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("updateEntityComponent", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::attachEntityComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("attachEntityComponent", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::removeEntityComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("removeEntityComponent", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::setParent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("setParent", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::query(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("query", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::componentCreated(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("componentCreated", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::componentDisposed(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("componentDisposed", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::componentUpdated(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("componentUpdated", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::subscribe(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("subscribe", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}

JSValue DecentralandInterface::unsubscribe(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    ptr->sendToRuntime("unsubscribe", ctx, this_val, argc, argv);
    return JS_UNDEFINED;
}
