
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <memory>
#include <string>
#include <vector>

#include "Scene.h"

Scene *mainScene = nullptr;

std::vector<std::string> get_params_by_lines(int fileDescriptor)
{
  ssize_t ret;
  std::vector<std::string> lines;
  uint32_t fsize = lseek(fileDescriptor, 0, SEEK_END);
  std::stringstream ss;
  std::stringbuf sbuf;

  char *buffer = new char[fsize + 1];
  buffer[fsize + 1] = 0;
  lseek(fileDescriptor, 0, SEEK_SET);
  read(fileDescriptor, buffer, fsize);

  std::istringstream f(buffer);
  std::string line;
  while (std::getline(f, line))
  {
    lines.push_back(line);
  }

  return std::move(lines);
}

int _init(int paramsFd)
{
  std::vector<std::string> lines = get_params_by_lines(paramsFd);
  printf("cpp_wasm init with %d params.\n", static_cast<int>(lines.size()));

  mainScene = new Scene();
  mainScene->setup(lines);
  return reinterpret_cast<int>(mainScene);
}

int _update(float dt)
{
  mainScene->loop(dt);
  return 0;
}

int _update(int ptr, float dt)
{
  Scene *scene = reinterpret_cast<Scene *>(ptr);
  if (scene)
  {
    scene->loop(dt);
  }
  return 0;
}

//
// Export section to WebAssembly
//

#define export __attribute__((visibility("default")))

extern "C"
{
  export int skipStartWASI()
  {
    return 0;
  }

  export int update(float dt)
  {
    return _update(dt);
  }

  export int updateScene(int ptr, float dt)
  {
    return _update(ptr, dt);
  }

  export int main(int argn, char *argv[])
  {
    return 0;
  }

  export int init(int paramsFd)
  {
    return _init(paramsFd);
  }
}