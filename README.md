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
    │   ├── floorplans/     # 2D layouts used for ray tracing
    │   ├── fonts/          # UI fonts
    │   ├── isotopes/       # Isotope data (energies, constants, metadata)
    │   ├── logos/          # Application branding
    │   └── materials/      # Material attenuation / density data
    │
    ├── external/           # Git submodules (third-party libraries)
    │   ├── ImGuiFileDialog
    │   ├── json
    │   └── tabulate
    │
    ├── include/            # Public headers (interfaces)
    │   ├── app/            # Application lifecycle / orchestration
    │   ├── calc/           # Dose, ray, and shielding calculations
    │   ├── geometry/       # Geometric primitives and spatial logic
    │   ├── isotopes/       # Isotope models and interfaces
    │   ├── materials/      # Material models and interfaces
    │   ├── output/         # Reports, exports, result formatting
    │   ├── ui/             # UI interfaces
    │   └── utils/          # Shared utilities
    │
    └── src/                # Implementations
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