// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class EditWallLayerCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            std::size_t wall_index;
            std::size_t layer_index;
            geom::WallLayer old_layer;
            geom::WallLayer new_layer;

        public:
            EditWallLayerCommand(geom::GeometryEngine& e, std::size_t wi, std::size_t li, geom::WallLayer old_l, geom::WallLayer new_l) : engine(e), wall_index(wi), layer_index(li), old_layer(old_l), new_layer(new_l) {}

            void execute() override { // override derived
                engine.get_walls_mutable()[wall_index].layers[layer_index] = new_layer;
            }

            void undo() override { // override derived
                engine.get_walls_mutable()[wall_index].layers[layer_index] = old_layer;
            }
    };

} // end namespace ui