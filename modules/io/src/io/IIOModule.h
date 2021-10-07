
#ifndef IIOMODULE_H
#define IIOMODULE_H

#include "IChannel.h"

class IIOModule
{
public:
    /** 
     * @returns nullptr or IChannel instanced for renderer communications
     */
    virtual IChannel *getRendererChannel() const = 0;

    /** 
     * @returns nullptr or IChannel instanced for scene communications
     */
    virtual IChannel *getScene0Channel() const = 0;

    /** 
     * @returns nullptr or IChannel instanced for scene debugger communications
     */
    virtual IChannel *getScene0DebuggerChannel() const = 0;
};

#endif
