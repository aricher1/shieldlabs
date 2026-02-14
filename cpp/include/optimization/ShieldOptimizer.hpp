#pragma once

#include "calc/CalcScene.hpp"
#include "calc/CompilerOutput.hpp"


namespace optimization {

class ShieldOptimizer {

    private:
        calc::CalcScene working_scene;
        
        calc::CompilerOutput computeOutput();

        int findWorstDosePoint(const calc::CompilerOutput& out, double& worst_violation);

        bool step();

    public:
        explicit ShieldOptimizer(const calc::CalcScene& original_scene);

        calc::CalcScene optimize();
};


} // end namespace optimization