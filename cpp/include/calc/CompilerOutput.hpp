#pragma once
#include "TransportRay.hpp"
#include <vector>


namespace calc {

    struct CompilerOutput {
        std::vector<TransportRay> rays;
    };

    CompilerOutput build_compiler_output(const CalcScene& scene);

} // end namespace calc