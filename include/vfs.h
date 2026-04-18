#pragma once
#include <string>
#include <vector>
struct VFS {
  std::vector<std::string> modpaths;
  std::string gameroot;
  std::string overwrite;
  std::string work;
};

void mount_vfs(VFS filesys);
