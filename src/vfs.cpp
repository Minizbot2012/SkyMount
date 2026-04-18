#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <linux/mount.h>
#include <regex>
#include <rfl/json.hpp>
#include <string>
#include <sys/mount.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>
#include <vfs.h>

void VFS::mount() {
  int fsfd = fsopen("overlay", FSOPEN_CLOEXEC);
  if (fsfd < 0) {
    std::cout << "Failed to open VFS: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  int used_limit = 0;
  std::vector<int> mnts;
  for (const auto &modpath : this->modpaths) {
    if (fsconfig(fsfd, FSCONFIG_SET_STRING, "lowerdir+", modpath.c_str(), 0) <
        0) {
      std::cout << "Failed to add VFS lowerdir" << std::endl;
      exit(EXIT_FAILURE);
    }
    used_limit++;
    if (used_limit == 500) {
      used_limit = 0;
      if (fsconfig(fsfd, FSCONFIG_SET_FLAG, "userxattr", NULL, 0) < 0) {
        std::cout << "Failed to set VFS userxattr: " << fsfd << std::endl;
        exit(EXIT_FAILURE);
      }
      if (fsconfig(fsfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0) < 0) {
        std::cout << "Failed to create VFS: " << fsfd << std::endl;
        exit(EXIT_FAILURE);
      }
      int mntfd = fsmount(fsfd, 0, 0);
      if (mntfd < 0) {
        std::cout << "Failed to mount VFS: " << mntfd << std::endl;
        exit(EXIT_FAILURE);
      }
      mnts.push_back(mntfd);
      fsfd = fsopen("overlay", FSOPEN_CLOEXEC);
      if (fsfd < 0) {
        std::cout << "Failed to open VFS: " << fsfd << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }
  {
    if (fsconfig(fsfd, FSCONFIG_SET_FLAG, "userxattr", NULL, 0) < 0) {
      std::cout << "Failed to set VFS userxattr: " << fsfd << std::endl;
      exit(EXIT_FAILURE);
    }
    if (fsconfig(fsfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0) < 0) {
      std::cout << "Failed to create VFS: " << fsfd << std::endl;
      exit(EXIT_FAILURE);
    }
    int mntfd = fsmount(fsfd, 0, 0);
    if (mntfd < 0) {
      std::cout << "Failed to mount VFS: " << mntfd << std::endl;
      exit(EXIT_FAILURE);
    }
    mnts.push_back(mntfd);
    fsfd = fsopen("overlay", FSOPEN_CLOEXEC);
    if (fsfd < 0) {
      std::cout << "Failed to open VFS: " << fsfd << std::endl;
      exit(EXIT_FAILURE);
    }
    for (auto mnt : mnts) {
      if (fsconfig(fsfd, FSCONFIG_SET_FD, "lowerdir+", nullptr, mnt) < 0) {
        std::cout << "Error adding lower dir to overall mnt" << std::endl;
        exit(EXIT_FAILURE);
      }
    }
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "lowerdir+", this->gameroot.c_str(),
               0) < 0) {
    std::cout << "Failed to set VFS gamedir: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "upperdir", this->overwrite.c_str(),
               0) < 0) {
    std::cout << "Failed to set VFS upperdir: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  std::filesystem::create_directories(this->work);
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "workdir", this->work.c_str(), 0) <
      0) {
    std::cout << "Failed to set VFS workdir: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "xino", "off", 0) < 0) {
    std::cout << "Failed to set VFS xino" << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "index", "off", 0) < 0) {
    std::cout << "Failed to set VFS index" << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_SET_FLAG, "userxattr", NULL, 0) < 0) {
    std::cout << "Failed to set VFS userxattr: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_SET_FLAG, "rw", NULL, 0) < 0) {
    std::cout << "Failed to set VFS read-write: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0) < 0) {
    std::cout << "Failed to create VFS: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }

  int mntfd = fsmount(fsfd, 0, 0);
  if (mntfd < 0) {
    std::cout << "Failed to mount VFS: " << mntfd << std::endl;
    exit(EXIT_FAILURE);
  }
  move_mount(mntfd, "", AT_FDCWD, this->gameroot.c_str(),
             MOVE_MOUNT_F_EMPTY_PATH);
}

/*
 * Whomever cracks this in the future... I am glad to have at least written the framework of it -- Mini
void VFS::from_modlist(std::filesystem::path ml) {
  std::ifstream input(ml / "profiles" / "Default" / "modlist.txt");
  std::string line;
  std::regex ptrn1(R"(^\+)");
  std::regex ptrn2(R"(\r)");
  std::regex ptrn3(R"(\n)");
  while (std::getline(input, line)) {
    if (line.starts_with("+")) {
      line = std::regex_replace(line, ptrn1, "");
      line = std::regex_replace(line, ptrn2, "");
      line = std::regex_replace(line, ptrn3, "");
      auto path = ml / "mods" / line;
      if (std::filesystem::exists(path)) {
        this->modpaths.push_back(path);
      }
    }
  }
}

static std::unordered_map<std::string, std::string> flname_table;
std::string GetReplacement(std::string str) {
  if (flname_table.size() == 0) {
    auto loaded = rfl::json::read<std::unordered_map<std::string, std::string>>(
        std::string_view(replacements));
    if (loaded.has_value()) {
      flname_table = loaded.value();
    }
  }
  std::string orig = str;
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  if (flname_table.contains(str)) {
    return flname_table.at(str);
  } else {
    flname_table[str] = orig;
    return flname_table[str];
  }
};
static std::unordered_map<std::string, std::string> fname_table;
void casefold_path(std::filesystem::path root, std::filesystem::path folder) {
  for (auto pth : std::filesystem::directory_iterator(folder)) {
    if (pth.is_directory()) {
      auto path = GetReplacement(pth.path().filename().string());
      if (path != pth.path().filename().string()) {
        std::filesystem::rename(pth, pth.path().parent_path() / path);
        pth.replace_filename(path);
      }
      casefold_path(root, pth.path());
    } else if (pth.is_regular_file()) {
      auto dest = std::filesystem::relative(pth, root).string();
      std::transform(dest.begin(), dest.end(), dest.begin(), ::tolower);
      if (!fname_table.contains(dest)) {
        fname_table.emplace(dest,
                            std::filesystem::relative(pth, root).string());
      } else {
        std::filesystem::rename(pth, root / fname_table[dest]);
      }
    }
  }
}

void VFS::casefold_mods() {
  for (auto mod : this->modpaths) {
    casefold_path(mod, mod);
  }
  casefold_path(this->overwrite, this->overwrite);
}
*/
