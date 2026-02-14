#include "optimization/ShieldOptimizer.hpp"
#include "calc/CompilerOutput.hpp"


namespace optimization {
    

ShieldOptimizer::ShieldOptimizer(const calc::CalcScene& original_scene) : working_scene(original_scene) { }


calc::CompilerOutput ShieldOptimizer::computeOutput() {
    // compute full dose map for current working scene
    return calc::build_compiler_output(working_scene);
}


int ShieldOptimizer::findWorstDosePoint(const calc::CompilerOutput& out, double& worst_violation) {
    // find dose point with the highest violation ratio
    // violation = annual_dose/dose_limit
    worst_violation = 0.0;
    int worst_index = -1;

    for (size_t i = 0; i < out.dose_totals.size(); ++i) {
        double annual = out.dose_totals[i].annual_dose_uSv;
        double limit = out.dose_totals[i].dose_limit_uSv;

        if (limit <= 0.0) { continue; }

        double violation = annual/limit;

        if (violation > worst_violation) {
            worst_violation = violation;
            worst_index = static_cast<int>(i);
        }
    }
    
    return worst_index;
}


bool ShieldOptimizer::step() {
    // perform one optimization iteration
    auto out = computeOutput();
    double worst_violation = 0.0;
    int worst_dose = findWorstDosePoint(out, worst_violation);

    const double tolerance = 1.0001;

    // all constraints satisfied
    if (worst_dose < 0 || worst_violation <= tolerance) {
        return false;
    }

    // find dominant source contributing to this worst dose
    double max_source_contribution = 0.0;
    int dominant_source = -1;
    
    const double occupancy = out.dose_totals[worst_dose].occupancy;

    for (const auto& ray : out.rays) {
        if (ray.dose_index != worst_dose) { continue; }

        double integrated = ray.integrated.integrated_dose_uSv;
        double annual = integrated * occupancy * 52.0; // 52 weeks per year

        if (annual > max_source_contribution) {
            max_source_contribution = annual;
            dominant_source = ray.source_index;
        }
    }

    // no adjustable source found
    if (dominant_source < 0) { return false; }

    /*
    At this stage we know the worst dose and the dominant source mapping to that dose.
    
    Next step (not implemented yet):
        - exract corresponding TransportRay
        - extract candidate walls
        - compute sensitivity
        - apply thickness change
    */

    return false; // thickness modification not implemented yet
}


calc::CalcScene ShieldOptimizer::optimize() {
    // run full optimization loop
    const int max_iterations = 200; // safety brake
    const double tolerance = 1.0001;

    for (int i = 0; i < max_iterations; ++i) {
        auto out = computeOutput();
        double worst_violation = 0.0;
        int worst_dose = findWorstDosePoint(out, worst_violation);

        if (worst_dose < 0 || worst_violation <= tolerance) { break; }
        if (!step()) { break; }
    }

    return working_scene;
}


} // end namespace optimization