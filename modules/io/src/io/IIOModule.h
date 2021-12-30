
#ifndef IIOMODULE_H
#define IIOMODULE_H

#include "IChannel.h"

class IIOModule
{
public:
    /** 
     * @returns nullptr or IChannel instanced for communications
     */
    virtual IChannel *getChannelByKey(const std::string key) const = 0;
};

#endif
