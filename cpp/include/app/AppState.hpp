#pragma once
#include "AppMode.hpp"


namespace app {

    struct AppState {
        AppMode mode = AppMode::ProjectPicker;
    };

} // end namespace app