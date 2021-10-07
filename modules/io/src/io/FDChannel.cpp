#include "FDChannel.h"
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

FDChannel::FDChannel(int fdRead, int fdWrite)
{
    // set non blocking to try to read each frame
    fcntl(fdRead, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
    this->pollingState.state = WatingDataLength;
    this->fdRead = fdRead;
    this->fdWrite = fdWrite;
}

FDChannel::~FDChannel()
{
}

int FDChannel::writeMessage(char *buffer, uint32_t bufferLength)
{
    lseek(this->fdWrite, 0, SEEK_SET);
    ::write(this->fdWrite, buffer, bufferLength);
    return 0;
}

void FDChannel::setOnDataArrival(DataArrivalCallback f)
{
    this->onDataArrival = f;
}

void FDChannel::poll()
{
    int ret;
    uint32_t bufferLength;

    lseek(this->fdRead, 0, SEEK_SET);
    do
    {
        if (this->pollingState.state == WatingDataLength)
        {
            ret = read(this->fdRead, &bufferLength, 4);
            if (ret > 0)
            {
                this->pollingState.state = ReadingData;
                this->pollingState.dataOffset = 0;
                this->pollingState.dataLength = bufferLength;
                if (bufferLength > this->pollingState.dataLength)
                {
                    this->pollingState.data = (char *)realloc(this->pollingState.data, this->pollingState.dataLength);
                }
            }
        }
        else if (this->pollingState.state == ReadingData)
        {
            uint32_t remaining = this->pollingState.dataLength - this->pollingState.dataOffset;
            ret = read(this->fdRead, &this->pollingState.data[pollingState.dataOffset], remaining);
            if (ret > 0)
            {
                remaining -= ret;
                if (remaining == 0)
                {
                    if (onDataArrival)
                    {
                        onDataArrival(pollingState.data, pollingState.dataLength);
                    }
                    this->pollingState.state = WatingDataLength;
                }
                else
                {
                    this->pollingState.dataOffset += ret;
                }
            }
        }
        else
        {
            assert(0);
        }
    } while (ret > 0);

    ftruncate(this->fdRead, 0);
}