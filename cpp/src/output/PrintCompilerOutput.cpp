#include "output/PrintCompilerOutput.hpp"
#include <iostream>
#include <iomanip>

namespace output {

void print(const calc::CompilerOutput& out) {
    std::cout << "\n====================== COMPILER OUTPUT ======================\n";

    for (size_t i = 0; i < out.rays.size(); ++i) {
        const auto& entry = out.rays[i];
        const auto& ray   = entry.ray;
        const auto& dose  = entry.dose;

        std::cout << "\nRay " << i << ":\n";
        std::cout << "  Distance from Source to Dose: "
                  << std::fixed << std::setprecision(2)
                  << ray.geometric_distance_cm << " cm\n";

        double cum = 0.0;
        for (const auto& seg : ray.segments) {
            cum += seg.path_length_cm;

            std::cout << "    "
                      << (seg.material_id < 0 ? "Air" : "Material")
                      << " | "
                      << seg.path_length_cm
                      << " cm | cumulative dist: "
                      << cum << " cm\n";
        }

        std::cout << "  ========== Dose Calculation ==========\n";
        std::cout << "  Isotope: " << entry.isotope_key << "\n";
        std::cout << "  Wall Transmission: " << dose.transmission_total << "\n";
        std::cout << "  Patient Transmission: " << dose.patient_transmission << "\n";
        std::cout << "  Dose (uSv/h): " << dose.dose_uSv_per_h << "\n";
    }

    std::cout << "=============================================================\n";
}

} // namespace output
