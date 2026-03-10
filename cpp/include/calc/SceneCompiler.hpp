// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <nlohmann/json.hpp>

#include "CalcScene.hpp"



namespace calc {

    class SceneCompiler {

        public:
            static CalcScene compile(const nlohmann::json& j); // Throws on parse error or missing fields

    };

} // end namespace calc