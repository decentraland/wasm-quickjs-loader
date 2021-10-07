
#ifndef CHANNELIOMODULE_H
#define CHANNELIOMODULE_H

#include "IIOModule.h"
#include <vector>

class ChannelIOModule : public IIOModule
{
public:
    ChannelIOModule(int rendererFdWrite = -1, int rendererFdRead = -1,
                    int scene0FdWrite = -1, int scene0FdRead = -1,
                    int scene0DebuggerFdWrite = -1, int scene0DebuggerFdRead = -1);
    ~ChannelIOModule();

    IChannel *getRendererChannel() const;
    IChannel *getScene0Channel() const;
    IChannel *getScene0DebuggerChannel() const;

    void poll();

private:
    std::vector<IChannel *> channels;
    std::unique_ptr<IChannel> rendererChannel;
    std::unique_ptr<IChannel> scene0Channel;
    std::unique_ptr<IChannel> scene0DebuggerChannel;
};

#endif
