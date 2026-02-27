// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class DeleteWallCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            std::size_t index;
            geom::Wall removed_wall;

        public:
            DeleteWallCommand(geom::GeometryEngine& engine, std::size_t index) : engine(engine), index(index) {}

            void execute() override {   // override from derived class
                removed_wall = engine.get_walls()[index];
                engine.remove_wall_at(index);
            }

            void undo() override {   // override from derived class
                engine.add_wall_direct(removed_wall);
            }
    };

} // end namespace ui