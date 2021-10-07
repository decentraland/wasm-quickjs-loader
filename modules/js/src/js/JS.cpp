
#include <string>
#include <vector>

#include "quickjs/quickjs.h"

#include "io/IIOModule.h"
#include "JS.h"
#include "DecentralandInterface.h"

JSRuntime *runtime;
JSContext *ctx;

std::vector<JSValue *> onEventFunctions;

static JSValue log(JSContext *ctx, JSValueConst this_val,
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

static JSValue onUpdate(JSContext *ctx, JSValueConst this_val,
                        int argc, JSValueConst *argv)
{
    if (argc == 1)
    {
        auto funct = argv[0];
        if (JS_IsFunction(ctx, funct))
        {
            JSValue copy = JS_DupValue(ctx, funct);
            onEventFunctions.push_back(&copy);
            return JS_UNDEFINED;
        }
    }

    // Unhappy case
    return JS_UNDEFINED;
}

JS::JS(IIOModule *value)
{
    ioModule = value;

    ioModule->getRendererChannel()->setOnDataArrival([&](const void *data, int dataLength)
                                                     { ioModule->getRendererChannel()->writeMessage("hello kernel", 12); });

    runtime = JS_NewRuntime();
    ctx = JS_NewContext(runtime);

    // Creating global object context
    JSValue global_obj = JS_GetGlobalObject(ctx);
    JSValue dcl = JS_NewObject(ctx);
    JS_SetPropertyStr(ctx, dcl, "DEBUG", JS_NewBool(ctx, 0));
    JS_SetPropertyStr(ctx, dcl, "log", JS_NewCFunction(ctx, log, "log", 1));
    JS_SetPropertyStr(ctx, dcl, "onUpdate", JS_NewCFunction(ctx, onUpdate, "onUpdate", 1));

    JS_SetPropertyStr(ctx, global_obj, "dcl", dcl);

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

        dcl.onUpdate('asd')
    )";

    JS_Eval(ctx, code.c_str(), code.length(), "index.js", 0);

    DecentralandInterface dclInterface(ctx);
    
}

void JS::loop(double dt)
{
    JSValue jsDtValue = JS_NewFloat64(ctx, 0.033);

    for (const JSValue *cb : onEventFunctions)
    {
        if (JS_IsFunction(ctx, *cb))
        {
            JS_Call(ctx, *cb, JS_UNDEFINED, 1, (JSValueConst *)&jsDtValue);
        }
    }

    JS_FreeValue(ctx, jsDtValue);
}