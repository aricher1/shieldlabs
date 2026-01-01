#pragma once
#include "EvaluateSingleRay.hpp"
#include "CalcScene.hpp"
#include "isotopes/IsotopeRegistry.hpp"


namespace calc {

    struct IntegratedDoseResult {
        double integrated_dose_uSv;     // per patient, over uptake time
        double average_rate_uSv_h;      // average during uptake
        double decay_factor;            // 1.0 if decay disabled
        double integration_time_h;      // uptake time
        double occupancy;
        double effective_dose_uSv;
    };

    IntegratedDoseResult integrate_single_ray(
        const SingleRayDoseResult& rate,
        const CalcSource& source,
        const IsotopeDef& isotope
    );

} // end namespace calc