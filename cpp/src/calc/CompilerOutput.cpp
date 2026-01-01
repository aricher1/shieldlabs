#include "calc/CalcScene.hpp"
#include "calc/CompilerOutput.hpp"
#include "calc/TransportRay.hpp"
#include "calc/EvaluateSingleRay.hpp"
#include "isotopes/IsotopeRegistry.hpp"
#include "materials/MaterialRegistry.hpp"
#include <stdexcept>

extern IsotopeRegistry isotope_registry;
extern MaterialRegistry material_registry;

namespace calc {

CompilerOutput build_compiler_output(const CalcScene& scene) {
    CompilerOutput out;

    if (!scene.sources.empty() && !scene.dose_points.empty()) {

        TransportRay ray =
            build_transport_ray(scene, 0, 0);

        const IsotopeDef* isotope =
            isotope_registry.get_by_key("f18");
        if (!isotope) {
            throw std::runtime_error("Isotope not found");
        }

        const auto& src = scene.sources[0];
        double activity_per_patient_MBq = src.activity_per_patient_MBq;

        SingleRayDoseResult dose =
            evaluate_single_ray(
                ray,
                src,
                *isotope,
                material_registry,
                activity_per_patient_MBq
            );
        
        IntegratedDoseResult integrated = 
            integrate_single_ray(
                dose,
                src,
                *isotope
            );

        const double occupancy = scene.dose_points[0].occupancy;
        integrated.occupancy = occupancy;
        integrated.effective_dose_uSv = integrated.integrated_dose_uSv * occupancy;
            
        CompilerRayOutput entry;
        entry.ray = ray;
        entry.dose = dose;
        entry.integrated = integrated;
        entry.isotope_key = isotope->key;
        
        out.rays.push_back(entry);
    }

    return out;
}


} // namespace calc