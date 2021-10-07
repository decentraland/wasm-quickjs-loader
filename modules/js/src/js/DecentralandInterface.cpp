#include "quickjs/quickjs.h"
#include "DecentralandInterface.h"
#include <functional>

DecentralandInterface::DecentralandInterface(JSContext *context) : ctx(context)
{
    // Creating global object context
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue dcl = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, dcl, "DEBUG", JS_NewBool(ctx, 0));
    //JS_SetPropertyStr(ctx, dcl, "log", JS_NewCFunction(ctx, log, "log", 1));
    JS_SetPropertyStr(ctx, dcl, "onUpdate", JS_NewCFunction(ctx, std::bind(&DecentralandInterface::onEvent, this, std::placeholders::_4), "onUpdate", 1));

    JS_SetPropertyStr(ctx, global_obj, "dcl", dcl);
}

JSValue DecentralandInterface::onEvent(JSContext *ctx, JSValueConst this_val,
                                       int argc, JSValueConst *argv)
{
}