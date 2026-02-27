// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "AppMode.hpp"


namespace app {

    struct AppState {
        AppMode mode = AppMode::ProjectPicker;
    };

} // end namespace app