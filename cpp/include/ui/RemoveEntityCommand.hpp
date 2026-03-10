// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class RemoveEntityCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            std::size_t index;
            geom::PointEntity saved;

        public:
            RemoveEntityCommand(geom::GeometryEngine& engine, std::size_t index) : engine(engine), index(index), saved(engine.get_entities()[index]) {} 

            void execute() override { // override derived
                engine.remove_entity_at(index);
            }

            void undo() override { // override derived
                engine.add_entity_direct(saved);
            }

    };

} // end namespace ui