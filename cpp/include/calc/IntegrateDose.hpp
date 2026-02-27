// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "EvaluateSingleRay.hpp"
#include "CalcScene.hpp"
#include "isotopes/IsotopeRegistry.hpp"



namespace calc {

    // Time integrated dose contribution from a single ray
    struct IntegratedDoseResult {
        double integrated_dose_uSv;     // per patient, over uptake time
        double average_rate_uSv_h;      // average during uptake
        double decay_factor;            // 1.0 if decay disabled
        double integration_time_h;      // uptake time
        double occupancy;               // user input
        double effective_dose_uSv;      // calculated
    };

    // Helper to integrate the single ray
    IntegratedDoseResult integrate_single_ray(
        const SingleRayDoseResult& rate,
        const CalcSource& source,
        const isotope::IsotopeDef& isotope
    );

} // end namespace calc