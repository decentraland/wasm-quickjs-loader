#include "ChannelIOModule.h"
#include "FDChannel.h"

ChannelIOModule::ChannelIOModule(int rendererFdWrite, int rendererFdRead,
                                 int scene0FdWrite, int scene0FdRead,
                                 int scene0DebuggerFdWrite, int scene0DebuggerFdRead)

{
    scene0Channel = std::make_unique<FDChannel>(scene0FdRead, scene0FdWrite);
    scene0DebuggerChannel = std::make_unique<FDChannel>(scene0DebuggerFdRead, scene0DebuggerFdWrite);
    rendererChannel = std::make_unique<FDChannel>(rendererFdRead, rendererFdWrite);

    channels.push_back(scene0Channel.get());
    channels.push_back(scene0DebuggerChannel.get());
    channels.push_back(rendererChannel.get());
}

ChannelIOModule::~ChannelIOModule()
{
}

void ChannelIOModule::poll()
{
    for (IChannel *channel : channels)
    {
        FDChannel *fdChannel = dynamic_cast<FDChannel *>(channel);
        if (fdChannel != nullptr)
        {
            fdChannel->poll();
        }
    }
}

IChannel *ChannelIOModule::getRendererChannel() const
{
    return rendererChannel.get();
}

IChannel *ChannelIOModule::getScene0Channel() const
{
    return scene0Channel.get();
}

IChannel *ChannelIOModule::getScene0DebuggerChannel() const
{
    return scene0DebuggerChannel.get();
}