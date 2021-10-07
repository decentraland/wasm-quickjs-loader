#ifndef DECENTRALAND_INTERFACE_H
#define DECENTRALAND_INTERFACE_H

#include "quickjs/quickjs.h"

class DecentralandInterface
{
public:
    DecentralandInterface(JSContext *ctx);

private:
    JSContext *ctx;
    std::vector<JSValue *> onEventFunctions;

    // Functions definitions
    JSValue onEvent(JSContext *ctx, JSValueConst this_val,
                    int argc, JSValueConst *argv);

    JSValue log(JSContext *ctx, JSValueConst this_val,
                    int argc, JSValueConst *argv);
};

#endif