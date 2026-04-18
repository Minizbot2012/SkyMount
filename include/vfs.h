#pragma once
#include <filesystem>
#include <string>
#include <vector>
struct VFS {
  std::vector<std::string> modpaths;
  std::string gameroot;
  std::string overwrite;
  std::string work;
  //void from_modlist(std::filesystem::path modlist);
  //void casefold_mods();
  void mount();
};
/*
static constexpr const char replacements[] = {
#embed "replacements.json"
    , 0};

std::string GetReplacement(std::string str);
*/
