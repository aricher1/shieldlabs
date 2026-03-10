// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class RemoveWallLayerCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            std::size_t wall_index;
            std::size_t layer_index;
            geom::WallLayer removed;

        public:
            RemoveWallLayerCommand(geom::GeometryEngine& e, std::size_t wi, std::size_t li) : engine(e), wall_index(wi), layer_index(li) {}

            void execute() override { // override derived
                auto& layers = engine.get_walls_mutable()[wall_index].layers;
                removed = layers[layer_index];
                layers.erase(layers.begin() + layer_index);
            }

            void undo() override { // override derived
                auto& layers = engine.get_walls_mutable()[wall_index].layers;
                layers.insert(layers.begin() + layer_index, removed);
            }
            
    };

} // end namespace ui