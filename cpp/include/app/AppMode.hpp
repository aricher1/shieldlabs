#pragma once


namespace app {

    enum class AppMode {
        ProjectPicker,          // new/open project on screen
        NewProjectSetup,        // pdf upload and scale calibration
        Editing,                // full editing state
        OpeningProject          // loading a saved project
    };

} // end namespace app