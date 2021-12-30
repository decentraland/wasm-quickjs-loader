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

    pollingState.dataLength = 8192;
    this->pollingState.data = (char *)malloc(pollingState.dataLength);

    outgoingData.dataOffset = 0;
    outgoingData.dataLength = 48 * 1024;
    outgoingData.buffer = new char[outgoingData.dataLength];

    tmpbuffer = new char[8192];

    pollingState.dataLength = 8192;
    this->pollingState.data = (char *)malloc(pollingState.dataLength);
}

FDChannel::~FDChannel()
{
    free(this->pollingState.data);
}

int FDChannel::writeMessage(const char *buffer, uint32_t bufferLength, bool direct)
{
    if (direct)
    {
        memcpy(tmpbuffer, &bufferLength, 4);
        memcpy(&tmpbuffer[4], buffer, bufferLength);
        lseek(this->fdWrite, 0, SEEK_SET);
        ::write(this->fdWrite, tmpbuffer, bufferLength + 4);
    }
    else
    {
        if ((bufferLength+4) > outgoingData.dataLength){
            // Handle this exception
            return 0;
        }
        
        if (outgoingData.dataOffset + bufferLength + 4 > outgoingData.dataLength){
            flush();
        }

        memcpy(&outgoingData.buffer[outgoingData.dataOffset], &bufferLength, 4);
        memcpy(&outgoingData.buffer[outgoingData.dataOffset + 4], buffer, bufferLength);
        outgoingData.dataOffset += bufferLength + 4;
    }
    return 0;
}

void FDChannel::flush() {
    if (outgoingData.dataOffset > 0){
        lseek(this->fdWrite, 0, SEEK_SET);
        ::write(this->fdWrite, outgoingData.buffer, outgoingData.dataOffset);
        outgoingData.dataOffset = 0;
    }
}

void FDChannel::setOnDataArrival(DataArrivalCallback f)
{
    this->onDataArrival = f;
}

void FDChannel::poll()
{
    int ret;
    uint32_t bufferLength;
    uint32_t fileLength = lseek(this->fdRead, 0, SEEK_END);

    lseek(this->fdRead, 0, SEEK_SET);
    do
    {
        if (this->pollingState.state == WatingDataLength)
        {
            ret = read(this->fdRead, &bufferLength, 4);
            if (ret > 0)
            {
                if (bufferLength > fileLength)
                {
                    assert(0);
                    break;
                }

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
            // the second condition shouldn't happen ever
            if (ret > 0 || (ret == 0 && remaining == 0))
            {
                remaining -= ret;
                if (remaining == 0)
                {
                    this->pollingState.state = WatingDataLength;
                    if (onDataArrival && pollingState.dataLength > 0)
                    {
                        onDataArrival(pollingState.data, pollingState.dataLength);
                    }
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