// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <vector>

#include "calc/CalcScene.hpp"
#include "calc/CompilerOutput.hpp"


namespace optimization {

class ShieldOptimizer {

    private:
        calc::CalcScene working_scene;

        size_t num_walls;
        
        calc::CompilerOutput computeOutput();

        void applyThicknessVector(const std::vector<double>& x);

        std::vector<double> extractInitialThicknessVector();

        // NLopt callbacks
        static double objectiveCallback(const std::vector<double>& x, std::vector<double>& grad, void* data);

        static double constraintCallback(const std::vector<double>& x, std::vector<double>& grad, void* data);

    public:
        explicit ShieldOptimizer(const calc::CalcScene& original_scene);

        calc::CalcScene optimize();
};

} // end namespace optimization