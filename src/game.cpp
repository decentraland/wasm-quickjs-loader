#define export __attribute__((visibility("default")))

#include "scene/Scene.h"

// c++ std libraries
#include <vector>
#include <string>
#include <map>

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

// IO Stuffs
int instance_init_flag = 0;
int renderer_fd = -1;
int debugger_input_fd = -1;
int debugger_output_fd = -1;
std::map<std::string, int *> file_descriptors;

std::unique_ptr<Scene> scene;
extern int quickjs_test(std::string address = "", int fdDebuggerInput = 0, int fdDebuggerOutput = 0);

void scene_init()
{
  // SCENE
  scene = std::make_unique<Scene>();

  printf("initializing game scene! %s \n", strerror(errno));

  // set standard input non blocking
  fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

  // init file_descriptor
  file_descriptors["RENDERER"] = &renderer_fd;
  file_descriptors["DEBUG_IN"] = &debugger_input_fd;
  file_descriptors["DEBUG_OUT"] = &debugger_output_fd;
  renderer_fd = -1;
}

std::vector<std::string> split_string(std::string s, std::string delimiter)
{
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  std::string token;
  std::vector<std::string> res;

  while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
  {
    token = s.substr(pos_start, pos_end - pos_start);
    pos_start = pos_end + delim_len;
    res.push_back(token);
  }

  res.push_back(s.substr(pos_start));
  return res;
}

void processStdIn()
{
  std::vector<std::string> commands;
  std::string parse_input;
  char buf[128];

  do
  {
    char c;
    int ret = read(0, &c, 1);
    if (ret == 1) {
      if (c == 0 || c == '\n'){
        commands.push_back(parse_input);
        parse_input = "";
      }else{
        parse_input += c;
      }
    }else{
      commands.push_back(parse_input);
      break;
    }
  } while (1);

  for (const std::string& input: commands){
    if (!input.empty())
    {
      printf("processing input '%s' \n", input.c_str());

      std::size_t first_space = input.find_first_of(" ");
      if (first_space == std::string::npos)
      {
        return;
      }

      std::string command = input.substr(0, first_space);
      std::string raw_params = input.substr(first_space + 1, input.length() - first_space);
      std::vector<std::string> params = split_string(raw_params, " ");

      printf("processing command '%s' with %d params - rawparams '%s' \n",
            command.c_str(), static_cast<int>(params.size()), raw_params.c_str());

      if (command == "set_fd" && params.size() == 2)
      {
        std::string fd_name = params[0];
        int value = atoi(params[1].c_str());

        if (file_descriptors.find(fd_name) != file_descriptors.end())
        {
          *(file_descriptors[fd_name]) = value;
          printf("setting a fileDescriptor '%s' to %d OK\n", fd_name.c_str(), value);
        }
        else
        {
          printf("setting a fileDescriptor '%s' to %d failed: not exists\n", fd_name.c_str(), value);
        }
      }
    }
  }
}

int _update(float dt)
{
  if (instance_init_flag == 0)
  {
    instance_init_flag = 1;
    scene_init();
  }

  // debugger_input_fd => 

  quickjs_test("0.0.0.0:7666", debugger_output_fd, debugger_input_fd);

  processStdIn();
  // printf("update(%f)\n", dt);

  scene->update(dt);
  scene->sendUpdates(renderer_fd);

  return 0;
}

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
}