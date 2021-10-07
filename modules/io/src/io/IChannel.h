#ifndef ICHANNEL_H
#define ICHANNEL_H

#include <memory>
#include <functional>

typedef std::function<void(const void *, int)> DataArrivalCallback;

class IChannel
{
public:
    virtual ~IChannel(){};
    virtual int writeMessage(char *buffer, uint32_t bufferLength) = 0;
    virtual void setOnDataArrival(DataArrivalCallback f) = 0;
};

#endif
