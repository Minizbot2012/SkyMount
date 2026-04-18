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

  auto res = rfl::json::load<VFS>(argv[1]);
  if (!res) {
    std::cout << "Failed to load VFS: " << res.error().what() << std::endl;
    return 1;
  }
  auto vfs = res.value();
  mount_vfs(vfs);
  if (argc > 2) {
    execv(argv[2], &argv[2]);
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
  return 0;
}
