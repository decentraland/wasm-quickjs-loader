#include "ChannelIOModule.h"
#include "FDChannel.h"
#include <memory>

ChannelIOModule::ChannelIOModule(std::map<std::string, ChannelParam> fdChannels)
{
    for (const auto &entry : fdChannels)
    {
        this->channels[entry.first] = std::make_unique<FDChannel>(entry.second.fdRead, entry.second.fdWrite);
    }
}

ChannelIOModule::~ChannelIOModule()
{
}

void ChannelIOModule::poll()
{
    for (const auto &entry : channels)
    {
        entry.second->poll();
    }
}


void ChannelIOModule::flush()
{
    for (const auto &entry : channels)
    {
        entry.second->flush();
    }
}

IChannel *ChannelIOModule::getChannelByKey(std::string key) const
{
    auto it = channels.find(key);
    if (it != channels.end())
    {
        return it->second.get();
    }
    else
    {
        return nullptr;
    }
}