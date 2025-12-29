#pragma once
#include "CalcScene.hpp"
#include <nlohmann/json.hpp>


namespace calc {

    class SceneCompiler {

        public:
            static CalcScene compile(const nlohmann::json& j); // throws on parse error or missing fields

    };

} // end namespace calc