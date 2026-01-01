#pragma once
#include "TransportRay.hpp"
#include "calc/EvaluateSingleRay.hpp"
#include <vector>
#include <string>


namespace calc {

    struct CompilerRayOutput {
        TransportRay ray;
        SingleRayDoseResult dose;
        std::string isotope_key;
    };

    struct CompilerOutput {
        std::vector<CompilerRayOutput> rays;
    };
        
    CompilerOutput build_compiler_output(const CalcScene& scene);

} // end namespace calc