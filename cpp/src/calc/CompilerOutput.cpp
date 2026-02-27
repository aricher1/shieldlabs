// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <stdexcept>

#include "calc/CalcScene.hpp"
#include "calc/CompilerOutput.hpp"
#include "calc/TransportRay.hpp"
#include "calc/EvaluateSingleRay.hpp"
#include "calc/EvaluateRayPipeline.hpp"
#include "isotopes/IsotopeRegistry.hpp"
#include "materials/MaterialRegistry.hpp"

constexpr double WEEKS_PER_YEAR = 52.0;

using namespace isotope;
using namespace material;
using namespace calc;

extern IsotopeRegistry isotope_registry;
extern MaterialRegistry material_registry;



namespace calc {

CompilerOutput build_compiler_output(const CalcScene& scene) {
    CompilerOutput out;
    out.isotope_key = scene.isotope_key;

    if (scene.sources.empty() || scene.dose_points.empty()) { return out; }

    const IsotopeDef* isotope = isotope_registry.get_by_key(scene.isotope_key);
    if (!isotope) {
        throw std::runtime_error("Isotope not found: " + scene.isotope_key);
    }

    // loop over dose points
    for (size_t d = 0; d < scene.dose_points.size(); ++d) {
        DosePointTotal dose_total;
        dose_total.dose_index = static_cast<int>(d);
        dose_total.dose_limit_uSv = scene.dose_points[d].dose_limit_uSv;
        dose_total.dose_name = scene.dose_points[d].label;
        dose_total.occupancy = scene.dose_points[d].occupancy;

        double total_integrated_dose = 0.0;
        double integration_time_h = 0.0;

        // loop over sources
        for (size_t s = 0; s < scene.sources.size(); ++s) {
            CompilerRayOutput ray_out = evaluate_ray_pipeline(scene, static_cast<int>(s), static_cast<int>(d), *isotope);
            ray_out.source_label = scene.sources[s].label;
            ray_out.dose_label = scene.dose_points[d].label;
            out.rays.push_back(ray_out);
            total_integrated_dose += ray_out.integrated.integrated_dose_uSv;
            // all sources must share integration time
            integration_time_h = ray_out.integrated.integration_time_h;
        }

        dose_total.integrated_dose_uSv = total_integrated_dose;
        if (integration_time_h > 0.0) {
            dose_total.average_rate_uSv_h = total_integrated_dose / integration_time_h;
        }

        dose_total.effective_dose_uSv = dose_total.integrated_dose_uSv * dose_total.occupancy;
        // Annual dose assumes weekly repition of workload (52 weeks/year)
        dose_total.annual_dose_uSv = dose_total.effective_dose_uSv * WEEKS_PER_YEAR;

        out.dose_totals.push_back(dose_total);
    }
    
    return out;
}

} // end namespace calc