<p align="center">
  <img src="cpp/assets/logos/ShieldLabsTitleLogoTransparent.png" alt="ShieldLabs logo" width="420">
</p>

## ShieldLabs

ShieldLabs is a deterministic, ray-based radiation shielding and dose calculation application. 
It models radiation transport through materials using calibrated 2D floorplans to estimate real-world exposure with spatial accuracy.

The application combines geometry, material attenuation, and isotope-specific emission data to compute dose at defined points and regions. Calculations are fully deterministic, making results reproducible and suitable for engineering analysis, validation, and reporting.

ShieldLabs is designed for scenario-based evaluation of shielding effectiveness, layout changes, and source placement, with a clear separation between computational logic, assets, and the UI.


## Repository Structure

```text
.
├── README.md
├── .gitignore
├── .gitmodules
└── cpp/
    ├── CMakeLists.txt
    ├── assets/
    │   ├── floorplans/     # Floorplans used for testing
    │   ├── fonts/          # Font used in UI
    │   ├── isotopes/       # Isotope YAML file (gamma constants, attenutation coefficients)
    │   ├── logos/          # ShieldLabs Logos
    │   └── materials/      # Material YAML file (concrete, steel, lead)
    │
    ├── external/           # Git submodules
    │   ├── ImGuiFileDialog # File Dialog
    │   ├── json            # nlohmann/json 
    │   └── tabulate        # Terminal debugging
    │
    ├── include/            # Header files (.hpp)
    │   ├── app/            # Application mode status files
    │   ├── calc/           # Computational math + physics logic (ray-based)
    │   ├── geometry/       # Geometric entities + bounds logic
    │   ├── isotopes/       # Isotope YAML file registry
    │   ├── materials/      # Material YAML file registry
    │   ├── output/         # Reports, exports, result formatting
    │   ├── ui/             # UI interfaces (GridRenderer, main)
    │   └── utils/          # Pdf to Png conversion
    │
    └── src/                # Implentation files (.cpp)
        ├── calc/
        ├── geometry/
        ├── isotopes/
        ├── materials/
        ├── output/
        ├── ui/
        └── utils/

```

## Usage

Terminal based application. 
A packaged desktop executable is planned.