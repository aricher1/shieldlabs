// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <cmath>

#include "calc/IntegrateDose.hpp"


using namespace isotope;


namespace calc {

    IntegratedDoseResult integrate_single_ray(const SingleRayDoseResult& rate, const CalcSource& source, const IsotopeDef& isotope) {
        IntegratedDoseResult out{};

        const double T = source.uptake_time_hours;
        out.integration_time_h = T;

        // if no exposure time then no dose
        if (T <= 0.0) {
            out.integrated_dose_uSv = 0.0;
            out.average_rate_uSv_h = 0.0;
            out.decay_factor = 1.0;
            return out;
        }

        // case where radioactive decay is disabled
        if (!source.apply_radioactive_decay) {
            out.decay_factor = 1.0;
            out.integrated_dose_uSv = rate.dose_uSv_per_h * T;
            out.average_rate_uSv_h = rate.dose_uSv_per_h;
            return out;
        }

        // case where radioactive decay is enabled
        const double lambda = std::log(2.0) / isotope.half_life_hours;
        const double decay_integral = (1.0 - std::exp(-lambda * T)) / lambda;
        out.integrated_dose_uSv = rate.dose_uSv_per_h * decay_integral;
        out.average_rate_uSv_h = out.integrated_dose_uSv / T;

        // debugging
        out.decay_factor = decay_integral / T;

        return out;
    }

} // end namespace calc
