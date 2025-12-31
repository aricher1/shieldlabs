#include "calc/CalcScene.hpp"
#include "calc/CompilerOutput.hpp"
#include "calc/TransportRay.hpp"



namespace calc {

CompilerOutput build_compiler_output(const CalcScene& scene) {
    CompilerOutput out;

    // for now: 1 source -> 1 dose point
    // later: loops over all of them
    if (!scene.sources.empty() && !scene.dose_points.empty()) {
        out.rays.push_back(build_transport_ray(scene, 0, 0));
    }

    return out;
}

} // end namespace calc