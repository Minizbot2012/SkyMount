#include <nsutils.h>
#include <rfl/json/load.hpp>
#include <vfs.h>
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <VFS JSON file>" << std::endl;
    return 1;
  }
  uid_t uid = getuid();
  gid_t gid = getgid();
  if (argc > 2 && getuid() != 0 && getgid() != 0) {
    NSUtils::create_namespace();
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
  // vfs.from_modlist("/home/deck/Games/Bethesda/Data/SSE/");
  // vfs.casefold_mods();
  vfs.mount();
  std::filesystem::path pth(argv[argc - 1]);
  if (pth.filename() == "SkyrimSELauncher.exe") {
    pth.replace_filename("skse64_loader.exe");
    argv[argc - 1] = const_cast<char *>(pth.c_str());
  }
  if (argc > 2) {
    execv(argv[2], &argv[2]);
    exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
  return 0;
}
