#include "calc/IntegrateDose.hpp"
#include <cmath>

namespace calc {

IntegratedDoseResult integrate_single_ray(
    const SingleRayDoseResult& rate,
    const CalcSource& source,
    const IsotopeDef& isotope
) {
    IntegratedDoseResult out{};

    const double T = source.uptake_time_hours;
    out.integration_time_h = T;

    // No exposure time → no dose
    if (T <= 0.0) {
        out.integrated_dose_uSv = 0.0;
        out.average_rate_uSv_h = 0.0;
        out.decay_factor = 1.0;
        return out;
    }

    // -------------------------------
    // Case 1: decay disabled
    // -------------------------------
    if (!source.apply_radioactive_decay) {
        out.decay_factor = 1.0;
        out.integrated_dose_uSv = rate.dose_uSv_per_h * T;
        out.average_rate_uSv_h = rate.dose_uSv_per_h;
        return out;
    }

    // -------------------------------
    // Case 2: decay enabled
    // -------------------------------
    const double lambda = std::log(2.0) / isotope.half_life_hours;

    // Exact analytical integration
    const double decay_integral = (1.0 - std::exp(-lambda * T)) / lambda;

    out.integrated_dose_uSv = rate.dose_uSv_per_h * decay_integral;
    out.average_rate_uSv_h = out.integrated_dose_uSv / T;

    // For debugging / transparency
    out.decay_factor = decay_integral / T;

    return out;
}

} // namespace calc
