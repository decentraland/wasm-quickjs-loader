
#ifndef CHANNELIOMODULE_H
#define CHANNELIOMODULE_H

#include "IIOModule.h"
#include <vector>
#include <map>
#include <string>

class FDChannel;

struct ChannelParam
{
    int fdWrite = -1, fdRead = -1;
};

class ChannelIOModule : public IIOModule
{
public:
    ChannelIOModule(std::map<std::string, ChannelParam> channels);
    ~ChannelIOModule();

    IChannel *getChannelByKey(const std::string key) const;

    void poll();

private:
    std::map<std::string, std::unique_ptr<FDChannel>> channels;
};

#endif
