#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <string>
#include <vector>

class ChannelIOModule;
class JS;

class Scene
{
public:
  Scene();
  ~Scene();

  void setup(std::vector<std::string> params);
  void loop(float dt);

private:
  std::unique_ptr<ChannelIOModule> ioModule;
  std::unique_ptr<JS> js;
  bool startedJs = false;
  int scene0GamejsFd;
};

#endif