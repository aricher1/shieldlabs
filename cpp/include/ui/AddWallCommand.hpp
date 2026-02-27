// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class AddWallCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            geom::Wall wall;

        public:
            AddWallCommand(geom::GeometryEngine& engine, const geom::Wall& wall) : engine(engine), wall(wall) {} 

            void execute() override { // override derived
                engine.add_wall_direct(wall);
            }

            void undo() override { // override derived
                engine.remove_last_wall();
            }

    };

} // end namespace ui