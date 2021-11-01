#include "Scene.h"
#include <map>
#include "io/ChannelIOModule.h"
#include "js/JS.h"
#include "utils/utils.h"
#include <unistd.h>
Scene::Scene()
{
}

Scene::~Scene()
{
}

void Scene::setup(std::vector<std::string> params)
{

  std::map<std::string, ChannelParam> channels;
  scene0GamejsFd = -1;
  for (int i = 0; i < params.size(); i++)
  {
    if (params[i] == "--set_fd" && (i + 2) < params.size())
    {
      const std::string &key = params[i + 1];
      int number = atoi(params[i + 2].c_str());
      if (!key.empty() && number > 0)
      {
        auto param = utils::split(key, "_");
        if (param.size() == 3 && param[0] == "FD")
        {
          auto it = channels.find(param[1]);
          if (it == channels.end())
          {
            channels[param[1]] = ChannelParam();
          }
          if (param[2] == "WRITE")
          {
            channels[param[1]].fdWrite = number;
          }
          else
          {
            channels[param[1]].fdRead = number;
          }
        }
        else if (key == "FD_GAMEJS")
        {
          scene0GamejsFd = number;
        }
      }
      i += 2;
    }
  }

  ioModule = std::make_unique<ChannelIOModule>(channels);
  js = std::make_unique<JS>(ioModule.get());
}

extern size_t js_transport_peek(void *udata);

void Scene::loop(float dt)
{
  ioModule->poll();

  if (startedJs)
    js->loop(dt);
  else
  {
    if (js_transport_peek(nullptr) > 50){
      auto gameJsSize = lseek(scene0GamejsFd, 0, SEEK_END);
      char *gameJsBuf = (char *)malloc(gameJsSize);
      lseek(scene0GamejsFd, 0, SEEK_SET);
      read(scene0GamejsFd, gameJsBuf, gameJsSize);
      js->eval(gameJsBuf, gameJsSize);
      startedJs = true;
    }
  }
}
