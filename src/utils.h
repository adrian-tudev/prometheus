#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <ctime>
#include <string>

#include "types.h"

#define LOG(level, msg) do { \
    std::time_t now = std::time(nullptr); \
    char buf[20]; \
    std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&now)); \
    std::cerr << "[" << buf << "] [" << level << "] " << msg << std::endl; \
} while(0)

std::string format(Square sq);

#endif
