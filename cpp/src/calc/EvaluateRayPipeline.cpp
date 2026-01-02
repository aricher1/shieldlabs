#include "calc/EvaluateRayPipeline.hpp"
#include "calc/TransportRay.hpp"
#include "calc/EvaluateSingleRay.hpp"
#include "calc/IntegrateDose.hpp"
#include "materials/MaterialRegistry.hpp"
#include <stdexcept>

extern MaterialRegistry material_registry;

namespace calc {

CompilerRayOutput evaluate_ray_pipeline(
    const CalcScene& scene,
    int source_index,
    int dose_index,
    const IsotopeDef& isotope
) {
    // -----------------------------------------
    // Safety checks
    // -----------------------------------------
    if (source_index < 0 || source_index >= static_cast<int>(scene.sources.size())) {
        throw std::runtime_error("Invalid source index");
    }

    if (dose_index < 0 || dose_index >= static_cast<int>(scene.dose_points.size())) {
        throw std::runtime_error("Invalid dose index");
    }

    const CalcSource& src = scene.sources[source_index];

    // -----------------------------------------
    // 1. Geometry: build transport ray
    // -----------------------------------------
    TransportRay ray = build_transport_ray(scene, source_index, dose_index);

    // -----------------------------------------
    // 2. Instantaneous dose rate
    // -----------------------------------------
    SingleRayDoseResult dose =
        evaluate_single_ray(
            ray,
            src,
            isotope,
            material_registry,
            src.activity_per_patient_MBq
        );

    // -----------------------------------------
    // 3. Time integration (decay)
    // -----------------------------------------
    IntegratedDoseResult integrated =
        integrate_single_ray(
            dose,
            src,
            isotope
        );

    // -----------------------------------------
    // 4. Package result
    // -----------------------------------------
    CompilerRayOutput out;
    out.ray = ray;
    out.dose = dose;
    out.integrated = integrated;
    out.isotope_key = isotope.key;

    return out;
}

} // end namespace calc
