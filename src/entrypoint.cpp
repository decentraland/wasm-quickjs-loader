
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <sstream>
#include <string.h>
#include <memory>
#include <string>
#include "io/ChannelIOModule.h"
#include "js/JS.h"

std::unique_ptr<ChannelIOModule> ioModule;
std::unique_ptr<JS> js;

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

  int rendererFdWrite = -1, rendererFdRead = -1,
      scene0FdWrite = -1, scene0FdRead = -1,
      scene0DebuggerFdWrite = -1, scene0DebuggerFdRead = -1;

  for (int i = 0; i < lines.size(); i++)
  {
    if (lines[i] == "--set_fd" && (i + 2) < lines.size())
    {
      const std::string &key = lines[i + 1];
      int number = atoi(lines[i + 2].c_str());
      if (!key.empty() && number > 0)
      {

        if (key == "FD_RENDERER_READ")
        {
          rendererFdRead = number;
        }
        else if (key == "FD_RENDERER_WRITE")
        {
          rendererFdWrite = number;
        }
        else if (key == "FD_SCENES0_READ")
        {
          scene0FdRead = number;
        }
        else if (key == "FD_SCENES0_WRITE")
        {
          scene0FdWrite = number;
        }
        else if (key == "FD_SCENES0DEBUGGER_READ")
        {
          scene0DebuggerFdRead = number;
        }
        else if (key == "FD_SCENES0DEBUGGER_WRITE")
        {
          scene0DebuggerFdWrite = number;
        }
      }
      i += 2;
    }
  }

  ioModule = std::make_unique<ChannelIOModule>(rendererFdWrite, rendererFdRead,
                                               scene0FdWrite, scene0FdRead,
                                               scene0DebuggerFdWrite, scene0DebuggerFdRead);

  js = std::make_unique<JS>(ioModule.get());

  printf("[CppWasm] Init scene called with %d params.\n", lines.size());
  return 0;
}

int _update(float dt)
{
  ioModule->poll();
  js->loop(dt);

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

  export int main(int argn, char *argv[])
  {
    return 0;
  }

  export int init(int paramsFd)
  {
    return _init(paramsFd);
  }
}