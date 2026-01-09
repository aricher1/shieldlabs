#pragma once
#include "AppMode.hpp"


struct AppState {
    AppMode mode = AppMode::Editing;        // default = our current behaviour
};