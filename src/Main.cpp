#include <helpers.h>
#include <vfs.h>
#include <rfl/json/load.hpp>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>
#include <ranges>
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <VFS JSON file>" << std::endl;
    return 1;
  }
  uid_t uid = getuid();
  gid_t gid = getgid();
  if (argc > 2 && getuid() != 0 && getgid() != 0) {
      create_namespace();
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
