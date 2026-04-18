#include "vfs.h"
#include <cstdlib>
#include <iostream>
#include <ranges>
#include <sys/mount.h>
void mount_vfs(VFS filesys) {
  int fsfd = fsopen("overlay", 0);
  if (fsfd < 0) {
    std::cout << "Failed to open VFS: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  for (const auto &modpath : filesys.modpaths | std::views::reverse) {
    if (fsconfig(fsfd, FSCONFIG_SET_STRING, "lowerdir+", modpath.c_str(), 0) <
        0) {
      std::cout << "Failed to add VFS lowerdir: " << fsfd << std::endl;
      exit(EXIT_FAILURE);
    }
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "lowerdir+", filesys.gameroot.c_str(),
               0) < 0) {
    std::cout << "Failed to set VFS lowerdir: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "upperdir", filesys.overwrite.c_str(),
               0) < 0) {
    std::cout << "Failed to set VFS upperdir: " << fsfd << std::endl;
    exit(EXIT_FAILURE);
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "workdir", filesys.work.c_str(), 0) <
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
  move_mount(mntfd, "", AT_FDCWD, filesys.gameroot.c_str(),
             MOVE_MOUNT_F_EMPTY_PATH);
}
