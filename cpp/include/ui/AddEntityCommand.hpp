// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class AddEntityCommand : public Command {

        private: 
            geom::GeometryEngine& engine;
            geom::PointEntity entity;

        public:
            AddEntityCommand(geom::GeometryEngine& engine, const geom::PointEntity& entity) : engine(engine), entity(entity) {}

            void execute() override { // override derived
                engine.add_entity_direct(entity);
            }

            void undo() override { // override derived
                engine.remove_entity_at(engine.get_entities().size() - 1);
            }

    };

} // end namespace ui