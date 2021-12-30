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
    bool logIfError(JSValue value);
    void log(const char *value);
    
private:
    JSContext *ctx;
    IChannel *kernelChannel;

    struct Promise
    {
        // asign the id=counter and then increment, it starts with the value '1'
        Promise() : id(counter++) {}
        static uint64_t counter;
        uint64_t id;
        JSValue resolving_functions[2];
        JSValue promise;
    };

    std::vector<std::unique_ptr<Promise>> promises;
    std::vector<JSValue> onUpdateFunction;
    std::vector<JSValue> onEventFunction;
    std::vector<JSValue> onStartFunction;

    Promise* createPromise();
    void sendToRuntime(std::string methodName, JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv, uint64_t promiseId = 0);


    // Functions definitions
    static JSValue onUpdate(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue log(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue loadModule(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue callRpc(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue updateEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue error(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue openExternalUrl(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue openNFTDialog(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue onStart(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue addEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue removeEntity(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue updateEntityComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue attachEntityComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue removeEntityComponent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue setParent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue query(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue componentCreated(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue componentDisposed(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue componentUpdated(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue onEvent(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue subscribe(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
    static JSValue unsubscribe(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv);
};

#endif