#include "calc/EvaluateSingleRay.hpp"
#include <cmath>
#include <stdexcept>

/*
    Evaluates dose contribution for a SINGLE transport ray:
    - one source
    - one dose point
    - one straight-line path

    This function is intentionally explicit and step-by-step
    so every physical factor can be inspected during debugging.
*/

SingleRayDoseResult evaluate_single_ray(
    const calc::TransportRay& ray,
    const calc::CalcSource& source, 
    const IsotopeDef& isotope,
    const MaterialRegistry& material_registry,
    double activity_per_patient_MBq
) {
    SingleRayDoseResult out{};

    // -------------------------------------------------
    // 1. Distance + inverse-square law
    // -------------------------------------------------
    out.distance_cm = ray.geometric_distance_cm;

    const double distance_m = out.distance_cm / 100.0;
    if (distance_m <= 0.0) {
        throw std::runtime_error("Invalid source–dose distance");
    }

    // Inverse-square falloff (point source assumption)
    out.inverse_square = 1.0 / (distance_m * distance_m);

    // -------------------------------------------------
    // 2. Total source activity
    // -------------------------------------------------
    out.activity_MBq =
        activity_per_patient_MBq * source.num_patients;

    // -------------------------------------------------
    // 3. Patient attenuation
    // -------------------------------------------------
    double patient_transmission = 1.0;

    if (source.apply_patient_attenuation &&
        isotope.patient_attenuation > 0.0) {

        // user-selected + isotope-defined
        patient_transmission = 1.0 - isotope.patient_attenuation;
    }

    out.patient_transmission = patient_transmission;

    // -------------------------------------------------
    // 4. Shielding attenuation
    // -------------------------------------------------
    double transmission_total = 1.0;

    for (const auto& seg : ray.segments) {

        // Air contributes no attenuation
        if (seg.material_id < 0)
            continue;

        const MaterialDef* mat = material_registry.get(seg.material_id);
        if (!mat) {
            throw std::runtime_error("Unknown material ID in TransportRay");
        }

        auto it = isotope.materials.find(mat->key);
        if (it == isotope.materials.end()) {
            throw std::runtime_error(
                "No shielding data for material '" + mat->key +
                "' for isotope '" + isotope.key + "'"
            );
        }

        const ShieldingData& sd = it->second;

        // convert cm → mm
        const double thickness_mm = seg.path_length_cm * 10.0;
        double transmission = 1.0;

        /*
            HVL model:
            - first HVL may differ (build-up region)
            - equilibrium HVL applies thereafter
        */
        if (thickness_mm <= sd.hvl1_mm) {
            transmission = std::pow(0.5, thickness_mm / sd.hvl1_mm);
        } else {
            const double remaining_mm = thickness_mm - sd.hvl1_mm;
            transmission =
                0.5 * std::pow(0.5, remaining_mm / sd.hvl2_mm);
        }

        transmission_total *= transmission;
    }

    out.transmission_total = transmission_total;

    // -------------------------------------------------
    // 5. Final dose rate
    // -------------------------------------------------
    out.dose_uSv_per_h =
        isotope.gamma_constant_uSv_m2_per_MBq_h *
        out.activity_MBq *
        out.inverse_square *
        patient_transmission *
        transmission_total;

    return out;
}