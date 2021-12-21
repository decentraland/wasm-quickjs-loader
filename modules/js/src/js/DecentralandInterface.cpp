
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

    kernelChannel->setOnDataArrival([&](const void *data, int dataLength)
                                            { 

        auto json = JS_ParseJSON(ctx, static_cast<const char*>(data), dataLength, "<input>");
        if (JS_IsException(json)){
            JS_FreeValue(ctx, json);
            log("Couldn't process received data from kernel");
            log((const char*)data);
            return;
        }

        if (JS_IsObject(json)){
            JSValue promiseIdValue = JS_GetPropertyStr(ctx, json, "promiseId");
            if (JS_IsUndefined(promiseIdValue)){
                JS_FreeValue(ctx, promiseIdValue);
                JSValue eventValue = JS_GetPropertyStr(ctx, json, "event");

                if (!JS_IsUndefined(eventValue)){
                    for (const JSValue cb : onEventFunction)
                    {
                        if (JS_IsFunction(ctx, cb))
                        {
                            JSValue ret = JS_Call(ctx, cb, JS_UNDEFINED, 1, (JSValueConst *)&eventValue);
                            logIfError(ret);
                        }
                    }
                    
                }

                JS_FreeValue(ctx, json);
                JS_FreeValue(ctx, eventValue);
                return;
            }else{
                JSValue dataValue = JS_GetPropertyStr(ctx, json, "data");
                JSValue resolvedValue = JS_GetPropertyStr(ctx, json, "resolved");
                int promiseId;

                if (JS_ToInt32(ctx, &promiseId, promiseIdValue)){
                    goto finish;
                }

                if (JS_IsUndefined(dataValue)){ 
                    goto finish;
                }
                
                if (JS_IsUndefined(resolvedValue)){ 
                    goto finish;
                }

                for (auto &promise : promises)
                {
                    if (promise){
                        if (promise->id == promiseIdValue){
                            int index = JS_ToBool(ctx, resolvedValue) ? 0 : 1;
                            auto result = JS_Call(ctx, promise->resolving_functions[index], JS_NULL, 1, &dataValue);
                            this->logIfError(result);
                            promise.reset();
                            break;
                        }
                    }
                }

                finish:
                    JS_FreeValue(ctx, json);
                    JS_FreeValue(ctx, promiseIdValue);
                    JS_FreeValue(ctx, dataValue);
                    JS_FreeValue(ctx, promiseIdValue);
                    JS_FreeValue(ctx, resolvedValue);
            }
        }
    });

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
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    auto promise = ptr->createPromise();
    ptr->sendToRuntime("log", ctx, this_val, argc, argv, promise->id);

    // int i;
    // const char *str;
    // size_t len;

    // for (i = 0; i < argc; i++)
    // {
    //     if (i != 0)
    //         putchar(' ');
    //     str = JS_ToCStringLen(ctx, &len, argv[i]);
    //     if (!str)
    //         return JS_EXCEPTION;
    //     fwrite(str, 1, len, stdout);
    //     JS_FreeCString(ctx, str);
    // }
    // putchar('\n');

    // str = JS_ToCStringLen(ctx, &len, JS_JSONStringify(ctx, this_val, JS_UNDEFINED, 2));
    // if (!str)
    //     return JS_EXCEPTION;
    // fwrite(str, 1, len, stdout);
    // JS_FreeCString(ctx, str);

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
            logIfError(ret);
        }
    }

    JS_FreeValue(ctx, jsDtValue);

    // static double counter = 0.0;
    // counter += dt;

    // if (counter > 10.0)
    // {
    //     for (const auto &promise : promises)
    //     {
    //         JS_Call(ctx, promise->resolving_functions[0], JS_UNDEFINED, 0, NULL);
    //     }
    // }
}

DecentralandInterface::Promise *DecentralandInterface::createPromise()
{
    std::unique_ptr<Promise> newPromise = std::make_unique<Promise>();
    Promise* rawPtr = newPromise.get();
    JSValue promise = JS_NewPromiseCapability(ctx, newPromise->resolving_functions);
    newPromise->promise = promise;
    promises.push_back(std::move(newPromise));
    return rawPtr;
}

void DecentralandInterface::sendToRuntime(std::string methodName, JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, uint64_t promiseId)
{
    std::stringstream ss;
    bool needComma = false;

    ss << "{\"method\":\"" << methodName << "\",\"params\":[";
    for (int i = 0; i < argc; i++)
    {
        JSValue json = JS_JSONStringify(ctx, argv[i], JS_UNDEFINED, JS_UNDEFINED);

        if (JS_IsUndefined(json)){
            if (needComma){
                ss << ",\"undefined\"";
            }else{
                ss << "\"undefined\"";
            }
        }else{
            const char *str = JS_ToCString(ctx, json);
            std::string stringToAdd = std::string(str);
            if (stringToAdd.length() == 0){
                if (needComma){
                    ss << ",\"undefined2\"";
                }else{
                    ss << "\"undefined2\"";
                }
            }else{
                if (needComma){
                    ss << "," << str;
                }else{
                    ss << str;
                }
            }
            JS_FreeCString(ctx, str);
        }

        needComma = true;
        JS_FreeValue(ctx, json);
    }
    ss << "],\"promiseId\":" << promiseId << "}";

    std::string buf = ss.str();

    kernelChannel->writeMessage(buf.c_str(), buf.length());
}

bool DecentralandInterface::logIfError(JSValue value)
{
    if (JS_IsException(value))
    {
        JSValue errorVal = JS_NewString(ctx, "logIfErrorCall");
        JSValue exception_val = JS_GetException(ctx);
        std::vector<JSValueConst> values;
        values.push_back(errorVal);
        values.push_back(exception_val);

        JSValue val = JS_GetPropertyStr(ctx, exception_val, "stack");
        if (!JS_IsUndefined(val))
        {
            values.push_back(val);
        }

        sendToRuntime("log", ctx, JS_NULL, values.size(), &values[0]);

        JS_FreeValue(ctx, exception_val);
        JS_FreeValue(ctx, val);
        JS_FreeValue(ctx, errorVal);
        return true;
    }
    return false;
}
    
void DecentralandInterface::log(const char* value)
{
    JSValue str = JS_NewString(ctx, value);
    sendToRuntime("log", ctx, JS_NULL, 1, &str);
    JS_FreeValue(ctx, str);
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
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    if (argc == 1)
    {
        auto funct = argv[0];
        if (JS_IsFunction(ctx, funct))
        {
            JSValue copy = JS_DupValue(ctx, funct);
            ptr->onEventFunction.push_back(copy);
            return JS_UNDEFINED;
        }
    }

    return JS_UNDEFINED;
}

JSValue DecentralandInterface::onStart(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSValue this_ptr = JS_GetPropertyStr(ctx, this_val, "__ptr");
    DecentralandInterface *ptr = reinterpret_cast<DecentralandInterface *>(this_ptr);

    if (argc == 1)
    {
        auto funct = argv[0];
        if (JS_IsFunction(ctx, funct))
        {
            JSValue copy = JS_DupValue(ctx, funct);
            ptr->onStartFunction.push_back(copy);
            return JS_UNDEFINED;
        }
    }
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
