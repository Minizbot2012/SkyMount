#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <ranges>
#include <rfl/json.hpp>
#include <sched.h>
#include <string>
#include <sys/mount.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

struct VFS {
  std::vector<std::string> modpaths;
  std::string gameroot;
  std::string overwrite;
  std::string work;
};

void update_map(const char *mapping, const char *map_file) {
  int fd = open(map_file, O_WRONLY);
  if (fd < 0) {
    perror("open map file");
    exit(EXIT_FAILURE);
  }
  if (write(fd, mapping, strlen(mapping)) < 0) {
    perror("write map file");
    exit(EXIT_FAILURE);
  }
  close(fd);
}

int main(int argc, char *argv[]) {
  uid_t uid = getuid();
  gid_t gid = getgid();
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <VFS JSON file>" << std::endl;
    return 1;
  }
  if (argc > 2 && getuid() != 0 && getgid() != 0) {
    if (unshare(CLONE_NEWNS | CLONE_NEWUSER) != 0) {
      std::cout << "Error creating child NS" << std::endl;
      return 1;
    }
    if (fork() != 0) {
      wait(NULL);
      exit(EXIT_SUCCESS);
    } else {
      update_map("deny", "/proc/self/setgroups");
      char map_buf[100];
      snprintf(map_buf, sizeof(map_buf), "0 %d 1", uid);
      update_map(map_buf, "/proc/self/uid_map");

      snprintf(map_buf, sizeof(map_buf), "0 %d 1", gid);
      update_map(map_buf, "/proc/self/gid_map");
    }
  }
  if (getuid() != 0 || getgid() != 0) {
    std::cout << "No permission" << std::endl;
    return EPERM;
  }
  int fsfd = fsopen("overlay", 0);
  if (fsfd < 0) {
    std::cout << "Failed to open VFS: " << fsfd << std::endl;
    return 1;
  }

  auto res = rfl::json::load<VFS>(argv[1]);
  if (!res) {
    std::cout << "Failed to load VFS: " << res.error().what() << std::endl;
    return 1;
  }
  auto vfs = res.value();

  for (const auto &modpath : vfs.modpaths | std::views::reverse) {
    if (fsconfig(fsfd, FSCONFIG_SET_STRING, "lowerdir+", modpath.c_str(), 0) <
        0) {
      std::cout << "Failed to add VFS lowerdir: " << fsfd << std::endl;
      return 1;
    }
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "lowerdir+", vfs.gameroot.c_str(),
               0) < 0) {
    std::cout << "Failed to set VFS lowerdir: " << fsfd << std::endl;
    return 1;
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "upperdir", vfs.overwrite.c_str(),
               0) < 0) {
    std::cout << "Failed to set VFS upperdir: " << fsfd << std::endl;
    return 1;
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "workdir", vfs.work.c_str(), 0) < 0) {
    std::cout << "Failed to set VFS workdir: " << fsfd << std::endl;
    return 1;
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "xino", "off", 0) < 0) {
    std::cout << "Failed to set VFS xino" << fsfd << std::endl;
    return 1;
  }
  if (fsconfig(fsfd, FSCONFIG_SET_STRING, "index", "off", 0) < 0) {
    std::cout << "Failed to set VFS index" << fsfd << std::endl;
    return 1;
  }
  if (argc > 2) {
    if (fsconfig(fsfd, FSCONFIG_SET_FLAG, "userxattr", NULL, 0) < 0) {
      std::cout << "Failed to set VFS userxattr: " << fsfd << std::endl;
      return 1;
    }
  }
  if (fsconfig(fsfd, FSCONFIG_SET_FLAG, "rw", NULL, 0) < 0) {
    std::cout << "Failed to set VFS read-write: " << fsfd << std::endl;
    return 1;
  }
  if (fsconfig(fsfd, FSCONFIG_CMD_CREATE, NULL, NULL, 0) < 0) {
    std::cout << "Failed to create VFS: " << fsfd << std::endl;
    return 1;
  }

  int mntfd = fsmount(fsfd, 0, 0);
  if (mntfd < 0) {
    std::cout << "Failed to mount VFS: " << mntfd << std::endl;
    return 1;
  }
  move_mount(mntfd, "", AT_FDCWD, vfs.gameroot.c_str(),
             MOVE_MOUNT_F_EMPTY_PATH);
  if (argc > 2) {
    execv(argv[2], &argv[2]);
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
  return 0;
}
