#include "output/PrintCompilerOutput.hpp"
#include <iostream>
#include <iomanip>


namespace output {

void print(const calc::CompilerOutput& out) {
    std::cout << "\n=== COMPILER OUTPUT ===\n";

    for (size_t i = 0; i < out.rays.size(); ++i) {
        const auto& ray = out.rays[i];

        std::cout << "\nRay " << i << ":\n";
        std::cout << "  Distance: "
                  << std::fixed << std::setprecision(2)
                  << ray.geometric_distance_cm << " cm\n";

        double cum = 0.0;
        for (const auto& seg : ray.segments) {
            cum += seg.path_length_cm;

            std::cout << "    "
                      << (seg.material_id < 0 ? "Air" : "Material")
                      << " | "
                      << seg.path_length_cm
                      << " cm | cum "
                      << cum << " cm\n";
        }
    }

    std::cout << "=======================\n";
}

} // end namespace output