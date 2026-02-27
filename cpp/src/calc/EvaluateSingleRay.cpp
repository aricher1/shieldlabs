// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <cmath>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>

#include "calc/EvaluateSingleRay.hpp"


using namespace isotope;
using namespace material;

namespace calc {

    // Shielding attenuation is applied once per material using total path length along the ray (HVL/TVL models are non-linear).
    SingleRayDoseResult evaluate_single_ray(const TransportRay& ray, const CalcSource& source, const IsotopeDef& isotope, const MaterialRegistry& material_registry, double activity_per_patient_MBq) {
        SingleRayDoseResult out{};

        // distance + inverse-square law
        out.distance_cm = ray.geometric_distance_cm;

        const double distance_m = out.distance_cm / 100.0;
        if (distance_m <= 0.0) {
            throw std::runtime_error("Invalid source–dose distance");
        }

        // inverse-square falloff (point source assumption)
        out.inverse_square = 1.0 / (distance_m * distance_m);

        // total source activity
        out.activity_MBq = activity_per_patient_MBq * source.num_patients;

        // patient attenuation
        double patient_transmission = 1.0;

        if (source.apply_patient_attenuation) {
            // user-selected + isotope-defined
            patient_transmission = 1.0 - std::clamp(source.patient_attenuation_percent, 0.0f, 1.0f);
        }

        out.patient_transmission = patient_transmission;

        // shielding attenuation (per material, integrated)
        double transmission_total = 1.0;

        // accumulate total thickness per material
        std::unordered_map<int, double> material_thickness_cm;
        for (const auto& seg : ray.segments) {
            if (seg.material_id < 0) {
                continue;
            }

            material_thickness_cm[seg.material_id] += seg.path_length_cm;
        }

        // apply attenuation once per material
        for (const auto& [material_id, thickness_cm] : material_thickness_cm) {
            const MaterialDef* mat = material_registry.get(material_id);
            if (!mat) {
                throw std::runtime_error("Unknown material ID in TransportRay.\n");
            }

            auto it = isotope.materials.find(mat->key);
            if (it == isotope.materials.end()) {
                throw std::runtime_error("No shielding data for material '" + mat->key + "' for isotope '" + isotope.key + ".'\n");
            }

            const ShieldingData& sd = it->second;
            // convert cm to mm
            const double thickness_mm = thickness_cm * 10.0;
            double transmission = 1.0;
            
            /*
            Three-region shielding model (ICRP-107 / NCRP):
            - Scenario 1: t < HVL1
            - Scenario 2: HVL1 <= t < TVL1
            - Scenario 3: t >= TVL1
            */
            
            if (thickness_mm < sd.hvl1_mm) {
                transmission = std::pow(0.5, thickness_mm / sd.hvl1_mm);
            } else if (thickness_mm < sd.tvl1_mm) {
                const double remaining_mm = thickness_mm - sd.hvl1_mm;
                transmission = 0.5 * std::pow(0.5, remaining_mm / sd.hvl2_mm);
            } else { // t >= TVL1
                const double remaining_mm = thickness_mm - sd.tvl1_mm;
                transmission = 0.1 * std::pow(0.1, remaining_mm / sd.tvl2_mm);
            }

            transmission_total *= transmission;
        }

        out.transmission_total = transmission_total;

        // final dose rate
        out.dose_uSv_per_h = isotope.gamma_constant_uSv_m2_per_MBq_h * out.activity_MBq * out.inverse_square * patient_transmission * transmission_total;

        return out;
    }

} // end namespace calc