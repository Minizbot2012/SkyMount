#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sched.h>
#include <sys/wait.h>
#include <unistd.h>
void update_map(const char *mapping, const char *map_file);
void create_namespace();
