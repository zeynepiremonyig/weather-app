#pragma once

#include <string>

// This struct represents a logged-in user in memory.
// We store both ID and username so other classes can use one object
// instead of passing multiple separate values.
struct User {
    int id = -1;
    std::string username;
};