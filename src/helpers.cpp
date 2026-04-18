#include <helpers.h>
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

void create_namespace() {
    uid_t uid = getuid();
    gid_t gid = getgid();
    if (unshare(CLONE_NEWNS | CLONE_NEWUSER) != 0) {
      std::cout << "Error creating child NS" << std::endl;
      exit(EXIT_FAILURE);
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
