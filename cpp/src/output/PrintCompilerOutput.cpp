#include "output/PrintCompilerOutput.hpp"
#include <iostream>
#include <iomanip>

namespace output {

void print(const calc::CompilerOutput& out) {
    std::cout << "\n====================== COMPILER OUTPUT ======================\n";

    // ------------------------------------------------------------
    // Per-ray detailed output (debug / audit)
    // ------------------------------------------------------------
    for (size_t i = 0; i < out.rays.size(); ++i) {
        const auto& entry = out.rays[i];
        const auto& ray   = entry.ray;
        const auto& dose  = entry.dose;

        std::cout << "\nRay " << i << ":\n";
        std::cout << "  Source index: " << ray.source_index << "\n";
        std::cout << "  Dose index:   " << ray.dose_index << "\n";
        std::cout << "  Distance from Source to Dose: " << std::fixed << std::setprecision(4) << ray.geometric_distance_cm << " cm\n";

        double cumulative = 0.0;
        for (const auto& seg : ray.segments) {
            cumulative += seg.path_length_cm;

            std::cout << "    " << (seg.material_id < 0 ? "Air" : "Material") << " | " << seg.path_length_cm << " cm | cumulative: " << cumulative << " cm\n";
        }

        // --------------------------------------------------------
        // Instantaneous dose
        // --------------------------------------------------------
        std::cout << std::fixed << std::setprecision(8);
        std::cout << "  ========== Dose Calculation ==========\n";
        std::cout << "  Isotope: " << entry.isotope_key << "\n";
        std::cout << "  Wall Transmission: " << dose.transmission_total << "\n";
        std::cout << "  Patient Transmission: " << dose.patient_transmission << "\n";
        std::cout << "  Initial Dose Rate (uSv/h): " << dose.dose_uSv_per_h << "\n";

        // --------------------------------------------------------
        // Time integration
        // --------------------------------------------------------
        std::cout << "  ========== Time Integration ==========\n";
        std::cout << "  Integration time (h): " << entry.integrated.integration_time_h << "\n";
        std::cout << "  Decay factor: " << entry.integrated.decay_factor << "\n";
        std::cout << "  Integrated dose (uSv): " << entry.integrated.integrated_dose_uSv << "\n";
        std::cout << "  Average dose rate (uSv/h): " << entry.integrated.average_rate_uSv_h << "\n";
    }

    // ------------------------------------------------------------
    // Per-dose aggregated totals
    // ------------------------------------------------------------
    if (!out.dose_totals.empty()) {
        std::cout << "\n====================== DOSE TOTALS ======================\n";
        std::cout << std::fixed << std::setprecision(8);

        for (const auto& d : out.dose_totals) {
            std::cout << "\nDose Point " << d.dose_index << ":\n";
            std::cout << "  Integrated dose (uSv): " << d.integrated_dose_uSv << "\n";
            std::cout << "  Average dose rate (uSv/h): " << d.average_rate_uSv_h << "\n";
            std::cout << "  Occupancy factor: " << d.occupancy << "\n";
            std::cout << "  Effective dose (uSv): " << d.effective_dose_uSv << "\n";
        }
    }

    std::cout << "=============================================================\n";
}

} // namespace output
