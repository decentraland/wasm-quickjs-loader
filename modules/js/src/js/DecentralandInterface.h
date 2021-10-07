#ifndef DECENTRALAND_INTERFACE_H
#define DECENTRALAND_INTERFACE_H

#include "quickjs/quickjs.h"
#include <vector>

class IChannel;

class DecentralandInterface
{
public:
    DecentralandInterface(JSContext *ctx, IChannel *kernelChannel);

    void emitUpdate(float dt);

private:
    JSContext *ctx;
    IChannel *kernelChannel;

    struct Promise
    {
        Promise() : id(counter++) {}
        static uint64_t counter;
        uint64_t id;
        JSValue resolving_functions[2];
        JSValue promise;
    };

    std::vector<std::unique_ptr<Promise>> onCallRpc;
    std::vector<std::unique_ptr<Promise>> onLoadModule;
    std::vector<JSValue> onUpdateFunction;

    // Functions definitions
    JSValue onUpdate(JSContext *ctx, JSValueConst this_val,
                     int argc, JSValueConst *argv);

    JSValue log(JSContext *ctx, JSValueConst this_val,
                int argc, JSValueConst *argv);

    JSValue loadModule(JSContext *ctx, JSValueConst this_val,
                       int argc, JSValueConst *argv);

    JSValue callRpc(JSContext *ctx, JSValueConst this_val,
                    int argc, JSValueConst *argv);
};

#endif