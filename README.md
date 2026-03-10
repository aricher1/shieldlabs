<p align="center">
  <img src="cpp/assets/logos/ShieldLabsTitleLogoTransparent.png"
       alt="ShieldLabs logo"
       width="320">
</p>

## Overview

ShieldLabs is a deterministic, ray-based radiation shielding and dose calculation application written in C++20. It models gamma radiation transport through multi-layer materials using calibrated 2D floorplans to compute spatial dose distributions with engineering-level precision.

The system integrates geometric intersection logic, material attenuation modeling, isotope-specific emission data, and constrained shielding optimization into a single analysis workflow. All calculations are deterministic and reproducible, making results suitable for technical validation and scenario-based studies.

ShieldLabs is designed for evaluating shielding effectiveness, layout modifications, source placement, and minimum-lead optimization, with a strict separation between computational core, data registries, and user interface.


## Repository Structure

```text
.
├── README.md
├── .gitignore
├── .gitmodules
└── cpp/
    ├── CMakeLists.txt
    ├── assets/
    │   ├── fonts/          # Font used in UI
    │   ├── isotopes/       # Isotope YAML file (gamma constants, attenutatio coefficients)
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
    │   ├── optimization/   # NLopt's COBYLA optimization algorithm
    │   ├── output/         # Reports, exports, result formatting
    │   ├── ui/             # UI interfaces (GridRenderer, main)
    │   └── utils/          # Pdf to Png conversion
    │
    └── src/                # Implentation files (.cpp)
        ├── calc/
        ├── geometry/
        ├── isotopes/
        ├── materials/
        ├── optimization/
        ├── output/
        ├── ui/
        └── utils/

```

## Dependencies

ShieldLabs is built and tested against the following versions:

| Library | Version |
|----------|----------|
| SFML | commit 6b23a47 (based on `sfml-stable`) |
| Dear ImGui | 1.92.6 |
| ImGui-SFML | 3.0.0 |
| ImGuiFileDialog | 0.6.9 |
| NLopt | 2.7.1 |
| yaml-cpp | 0.8.0 |
| nlohmann/json | 3.11.3 |
| tabulate | 1.5.0 |

All third-party dependencies are vendored locally within the repository to provide deterministic cross-platform builds across Linux, macOS, and Windows.

## Usage

Builds on Linux and MacOS currently

### Toolbar Guide

| Button | Description |
|--------|-------------|
| `Selection` | Select and inspect geometry. |
| `Editing` | Switch to placement and editing mode. |
| `Add Wall` | Place a wall segment. |
| `Add Source` | Place a radiation source. |
| `Add Dose` | Place a dose point. |
| `Delete` | Remove the selected item. |
| `Undo` | Reverse the last action. |
| `Redo` | Restore the last undone action. |
| `Isotopes` | Show the list of supported isotopes. |
| `Materials` | Show the list of supported materials. |
| `Run Calc` | Calculate dose and lock geometry. |
| `Unlock Geometry` | Unlock geometry so you can edit again. |
| `Optimize` | Optimize shielding. |
| `Results` | Show optimization results. |
| `Info` | Toggle wall layer contents plus source and dose annotations. |
| `Edit Scale` | Recalibrate the floorplan scale. |
| `Help` | Open the in-application help window. |
| `Save` | Save the project or export results. |

### Typical Workflow

1. Upload the PDF floorplan.
2. Calibrate the real-world scale.
3. Add walls, sources, and dose points.
4. Adjust wall materials and thicknesses.
5. Enter the source and dose-point data.
6. Review the supported isotopes and materials.
7. Select the isotope to use throughout the calculation.
8. Run the initial calculation.
9. Optimize and review the results if needed.


## Supported Isotopes

| Isotope         | Symbol |
|-----------------|--------|
| Carbon-11       | C-11   |
| Fluorine-18     | F-18   |
| Gallium-68      | Ga-68  |
| Technetium-99m  | Tc-99m |
| Iodine-131      | I-131  |
| Lutetium-177    | Lu-177 |
| Radium-226      | Ra-226 |
| Actinium-225    | Ac-225 |

## Supported Materials

- Concrete, Steel, Lead

## References

1. Canadian Nuclear Safety Commission (CNSC).  
   **REGDOC-2.5.6 — Design of Rooms Where Unsealed Nuclear Substances Are Used.**  
   May 2023.

2. Canadian Nuclear Safety Commission (CNSC).  
   **Radionuclide Information Booklet.**  
   July 2025 Edition.

3. AAPM Task Group 108.  
   Marsden, M. et al.  
   **PET and PET/CT Shielding Requirements.**  
   Medical Physics, 2006.

## Author
Developed by Aidan Richer <richer2@uwindsor.ca | richer.a@outlook.com>