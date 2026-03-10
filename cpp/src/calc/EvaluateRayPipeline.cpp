// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <stdexcept>

#include "calc/EvaluateRayPipeline.hpp"
#include "calc/TransportRay.hpp"
#include "calc/EvaluateSingleRay.hpp"
#include "calc/IntegrateDose.hpp"
#include "materials/MaterialRegistry.hpp"


using namespace isotope;
using namespace material;
using namespace calc;

extern MaterialRegistry material_registry;


namespace calc {

CompilerRayOutput evaluate_ray_pipeline(const CalcScene& scene, int source_index, int dose_index, const IsotopeDef& isotope) {
    if (source_index < 0 || source_index >= static_cast<int>(scene.sources.size())) {
        throw std::runtime_error("Invalid source index");
    }

    if (dose_index < 0 || dose_index >= static_cast<int>(scene.dose_points.size())) {
        throw std::runtime_error("Invalid dose index");
    }

    const CalcSource& src = scene.sources[source_index];
    TransportRay ray = build_transport_ray(scene, source_index, dose_index);

    // instantaneous dose rate
    SingleRayDoseResult dose = evaluate_single_ray(ray, src, isotope, material_registry, src.activity_per_patient_MBq);

    // time integration (radioactive decay)
    IntegratedDoseResult integrated = integrate_single_ray(dose, src, isotope);

    CompilerRayOutput out;
    out.source_index = source_index;
    out.dose_index = dose_index;
    out.ray = ray;
    out.dose = dose;
    out.integrated = integrated;

    return out;
}

} // end namespace calc