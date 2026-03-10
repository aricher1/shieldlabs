// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once


namespace app {

    enum class AppMode {
        ProjectPicker,          // new/open project on screen
        NewProjectSetup,        // pdf upload and scale calibration
        Editing,                // full editing state
        OpeningProject          // loading a saved project
    };

} // end namespace app