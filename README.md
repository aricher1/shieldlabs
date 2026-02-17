<p align="center">
  <img src="cpp/assets/logos/ShieldLabsTitleLogoTransparent.png" alt="ShieldLabs logo" width="420">
</p>

## Overview

ShieldLabs is a deterministic, ray-based radiation shielding and dose calculation application written in C++. It models gamma radiation transport through multi-layer materials using calibrated 2D floorplans to compute spatial dose distributions with engineering-level precision.

The system integrates geometric intersection logic, material attenuation modeling, isotope-specific emission data, and constrained shielding optimization into a single analysis workflow. All calculations are deterministic and reproducible, making results suitable for technical validation, regulatory comparison, and scenario-based engineering studies.

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

## Usage

Terminal based application. 
A packaged desktop executable is planned.


## Supported Isotopes

The current version (v1.0) supports the following isotopes:
- Carbon-11 (C-11)
- Fluorine-18 (F-18)
- Gallium-68 (Ga-68)
- Technetium-99m (Tc-99m)
- Iodine-131 (I-131)
- Lutetium-177 (Lu-177)
- Radium-226 (Ra-226)
- Actinium-225 (Ac-225)


## Supported Materials

- Concrete
- Lead
- Steel


## Optimization

ShieldLabs includes a nonlinear constrained optimization module that computes the minimum required lead shielding across all walls in a scene while satisfying specified dose limits at defined evaluation points. Each wall is assigned an independent lead thickness variable, and the objective is to minimize total lead usage subject to annual dose constraints at each dose location. At every iteration, the optimizer applies the candidate thickness configuration to the scene and deterministically recomputes the full dose distribution, since dose response is nonlinear with respect to shielding thickness. The system uses the derivative-free COBYLA algorithm from NLopt, which handles inequality constraints directly and does not require analytical gradients. The result is an updated scene configuration containing the minimum-lead shielding solution that satisfies all achievable dose constraints within defined physical bounds. ShieldLabs allows the user to visually inspect the optimization results directly within the scene. Walls modified by the optimizer are displayed with their applied lead shielding thickness, clearly indicating where material was added. Dose evaluation points update to reflect their computed annual dose relative to their specified dose limits, making it immediately visible which constraints are satisfied.

### Mathematical Formulation

Let  

- $x_i$ = lead thickness (cm) applied to wall $i$  
- $D_j(x)$ = computed annual dose at dose point $j$  
- $L_j$ = dose limit at dose point $j$

The optimization problem is:

$$
\min_{x} \sum_i x_i
$$

subject to

$$
D_j(x) \le L_j \quad \text{for all dose points with } L_j > 0
$$

$$
0 \le x_i \le 20 \quad \text{for all walls } i
$$

Dose $D_j(x)$ is evaluated deterministically at each iteration using the ray-based transport model.
