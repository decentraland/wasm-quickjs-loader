#ifndef FDCHANNEL_H
#define FDCHANNEL_H

#include "IChannel.h"

class FDChannel : public IChannel
{
public:
    FDChannel(int fdRead, int fdWrite);
    ~FDChannel() override;

    int writeMessage(const char *buffer, uint32_t bufferLength) override;
    void setOnDataArrival(DataArrivalCallback f) override;

    void poll();

private:
    int fdRead = -1, fdWrite = -1;
    DataArrivalCallback onDataArrival = nullptr;

    enum PollingState
    {
        WatingDataLength = 1,
        ReadingData = 2
    };

    struct
    {
        char *data = nullptr;
        uint32_t dataLength = 0;
        uint32_t dataOffset = 0;
        PollingState state;
    } pollingState;
};

#endif
