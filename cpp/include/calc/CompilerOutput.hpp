#pragma once
#include "TransportRay.hpp"
#include "EvaluateSingleRay.hpp"
#include "IntegrateDose.hpp"
#include <vector>
#include <string>


namespace calc {

    struct CompilerRayOutput {
        TransportRay ray;
        SingleRayDoseResult dose;
        IntegratedDoseResult integrated;
        std::string isotope_key;
    };

    struct DosePointTotal {
        int dose_index;
        double integrated_dose_uSv = 0.0;
        double average_rate_uSv_h = 0.0;
        double occupancy = 1.0;
        double effective_dose_uSv = 0.0;
    };

    struct CompilerOutput {
        std::vector<CompilerRayOutput> rays;
        std::vector<DosePointTotal> dose_totals;
    };
        
    CompilerOutput build_compiler_output(const CalcScene& scene);

} // end namespace calc