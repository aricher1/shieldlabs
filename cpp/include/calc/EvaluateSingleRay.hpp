// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "materials/MaterialRegistry.hpp"
#include "isotopes/IsotopeRegistry.hpp"
#include "TransportRay.hpp"



namespace calc {

    // Instantaneous dose contribution from a single transport ray
    struct SingleRayDoseResult {
        double distance_cm;
        double inverse_square;
        double activity_MBq;
        double transmission_total;
        double dose_uSv_per_h;
        double patient_transmission;
    };

    SingleRayDoseResult evaluate_single_ray(
        const TransportRay& ray,
        const CalcSource& source,
        const isotope::IsotopeDef& isotope,
        const material::MaterialRegistry& material_registry,
        double activity_per_patient_MBq
    );

} // end namespace calc