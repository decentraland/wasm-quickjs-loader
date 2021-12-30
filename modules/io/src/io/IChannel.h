#ifndef ICHANNEL_H
#define ICHANNEL_H

#include <memory>
#include <functional>

typedef std::function<void(const void *, int)> DataArrivalCallback;

class IChannel
{
public:
    virtual ~IChannel(){};
    virtual int writeMessage(const char *buffer, uint32_t bufferLength, bool direct = false) = 0;
    virtual void setOnDataArrival(DataArrivalCallback f) = 0;
};

#endif
