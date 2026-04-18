#pragma once
#include <fcntl.h>
#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>
namespace NSUtils {
void update_map(const char *mapping, const char *map_file);
void create_namespace();
} // namespace NSUtils
