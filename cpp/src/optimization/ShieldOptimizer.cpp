// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


// ShieldOptimizer implements a nonlinear constrained optimization of lead shielding thickness across all walls in the scene.
// 
// Decision variables:
//      x_i = lead thickness (cm) for wall i
//
// Objective:
//      Minimize total lead used: sum_i x_i
//
// Constraints:
//      For each dose point j:
//          AnnualDose_j(x) <= DoseLimit_j
//
// The optimizer used NLopt's COBYLA algorithm (derivative-free), which is suitable for nonlinear dose response functions and constraint-based feasibility problems starting from an infeasible state.


#include <iostream>
#include <memory>

#include <nlopt.hpp>

#include "optimization/ShieldOptimizer.hpp"
#include "calc/CompilerOutput.hpp"
#include "materials/MaterialRegistry.hpp"


extern material::MaterialRegistry material_registry;

namespace optimization {

// ConstraintData is passed to NLopt for each dose constraint.
// It stores which dose index the constraint corresponds to, allowing the shared callback to evaluate the correct dose point.
struct ConstraintData {
    ShieldOptimizer* optimizer;
    int dose_index;
};


// The optimizer operates directly on a working copy of the scene (created by the user).
// The number of decision variables equals the number of walls, since each wall may receive an independent lead thickness.
ShieldOptimizer::ShieldOptimizer(const calc::CalcScene& original_scene) : working_scene(original_scene) {
    num_walls = working_scene.walls.size();
}


// Recomputes the full dose distribution for the current scene state.
// THis is called repeatedly during optimization to evaluate constraints.
calc::CompilerOutput ShieldOptimizer::computeOutput() {
    return calc::build_compiler_output(working_scene);
}


// Extracts the current lead thickness (cm) for each wall and builds the optimization variable vector x.
// If a wall has no lead layer, it is treated as zero thickness.
std::vector<double> ShieldOptimizer::extractInitialThicknessVector() {
    std::vector<double> x(num_walls, 0.0);

    for (size_t i = 0; i < num_walls; ++i) {
        for (const auto& layer : working_scene.walls[i].layers) {
            const auto* mat = material_registry.get(layer.material_id);
            if (mat && mat->key == "lead") {
                x[i] = layer.thickness_cm;
                break;
            }
        }
    }

    return x;
}


// Applies the optimization variable vector x to the scence.
// For each wall, the lead layer is either updated or created if missing.
// This directly modifies working_scene prior to dose recomputation.
void ShieldOptimizer::applyThicknessVector(const std::vector<double>& x) {

    for (size_t i = 0; i < num_walls; ++i) {

        auto& wall = working_scene.walls[i];
        int lead_layer_index = -1;

        for (size_t j = 0; j < wall.layers.size(); ++j) {
            const auto* mat = material_registry.get(wall.layers[j].material_id);
            if (mat && mat->key == "lead") {
                lead_layer_index = static_cast<int>(j);
                break;
            }
        }

        if (lead_layer_index < 0) {
            int lead_id = material_registry.get_id_by_key("lead");
            wall.layers.push_back({ lead_id, 0.0 });
            lead_layer_index = static_cast<int>(wall.layers.size() - 1);
        }

        wall.layers[lead_layer_index].thickness_cm = x[i];
    }
}


// Objective function: minimize total lead thickness.
//
// f(x) = sum_i x_i
//
// The gradient is constant (1 for each variable), although COBYLA does not require gradients. Providing it is harmless.
double ShieldOptimizer::objectiveCallback(const std::vector<double>& x, std::vector<double>& grad, [[maybe_unused]] void* data) {

    if (!grad.empty()) {
        for (size_t i = 0; i < x.size(); ++i) {
            grad[i] = 1.0;
        }
    }

    double sum = 0.0;
    
    for (double t : x) {
        sum += t;   
    }
    
    return sum;
}


// Constraint function for a single dose point.
//
// For dose point j:
// 
//      g_j(x) = AnnualDose_j(x) - DoseLimit_j
//
// The constraint is satisfied when g_j(x) <= 0.
// The full dose field is recomputed after applying x, since dose is a nonlinear function of shielding thickness.
double ShieldOptimizer::constraintCallback(const std::vector<double>& x, [[maybe_unused]] std::vector<double>& grad, void* data) {

    ConstraintData* cd = static_cast<ConstraintData*>(data);
    ShieldOptimizer* optimizer = cd->optimizer;
    int dose_index = cd->dose_index;

    optimizer->applyThicknessVector(x);

    auto out = optimizer->computeOutput();

    double dose = out.dose_totals[dose_index].annual_dose_uSv;
    double limit = out.dose_totals[dose_index].dose_limit_uSv;

    return dose - limit;  // must be <= 0
}


// Solves the constrained nonlinear optimization problem.
//
// Variables:
//     x_i ∈ [0, 20] cm
//
// Objective:
//     minimize sum_i x_i
//
// Constraints:
//     AnnualDose_j(x) <= DoseLimit_j  for each dose point j
//
// COBYLA is used because:
//     - Dose response is nonlinear
//     - Analytical gradients are not supplied
//     - The initial configuration may be infeasible
//
// One inequality constraint is registered per dose point.
// ConstraintData objects are heap-allocated and stored in a vector to ensure stable memory for NLopt callbacks.
calc::CalcScene ShieldOptimizer::optimize() {

    nlopt::opt opt(nlopt::LN_COBYLA, num_walls);

    opt.set_min_objective(objectiveCallback, this);

    // Physical bounds on shielding thickness.
    // Lower bound enforces non-negative thickness.
    // Upper bound limits unrealistic shielding growth.
    std::vector<double> lb(num_walls, 0.0);
    std::vector<double> ub(num_walls, 20.0);  // 20 cm max lead

    opt.set_lower_bounds(lb);
    opt.set_upper_bounds(ub);

    // add one constraint per dose point
    auto initial_out = computeOutput();

    std::vector<std::unique_ptr<ConstraintData>> constraint_data;

    // Register one inequality constraint per dose point.
    // Only dose points with positive limits are enforced.
    // Each constraint evaluates a different dose index.
    for (size_t i = 0; i < initial_out.dose_totals.size(); ++i) {

        if (initial_out.dose_totals[i].dose_limit_uSv <= 0.0) {
            continue;
        }

        auto cd = std::make_unique<ConstraintData>();
        cd->optimizer = this;
        cd->dose_index = static_cast<int>(i);

        opt.add_inequality_constraint(constraintCallback, cd.get(), 1e-6);

        constraint_data.push_back(std::move(cd));
    }

    // Initialize decision vector with current lead configuration.
    // NLopt modifies x in-place during optimization.
    opt.set_xtol_rel(1e-4);
    opt.set_maxeval(2000);

    std::vector<double> x = extractInitialThicknessVector();

    double minf;

    try {
        opt.optimize(x, minf);
        // apply final optimized thickness vector to the scene
        applyThicknessVector(x);
        // uncomment for terminal debugging
        // std::cout << "Optimization complete. Total lead = " << minf << " cm\n";
    }
    catch (std::exception& e) {
        // uncomment for terminal debugging
        // std::cerr << "NLopt failed: " << e.what() << "\n";
    }

    // After optimization, working_scene contains the minimum-lead configuration satisfying all achievable dose constraints within the specified bounds.
    return working_scene;
}

} // end namespace optimization