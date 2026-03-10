// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class AddWallLayerCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            std::size_t wall_index;
            std::size_t insert_index;
            geom::WallLayer layer;

        public:
            AddWallLayerCommand(geom::GeometryEngine& e, std::size_t wi, std::size_t idx, const geom::WallLayer& l) : engine(e), wall_index(wi), insert_index(idx), layer(l) {}

            void execute() override { // override derived
                auto& layers = engine.get_walls_mutable()[wall_index].layers;
                layers.insert(layers.begin() + insert_index, layer);
            }

            void undo() override { // override derived
                auto& layers = engine.get_walls_mutable()[wall_index].layers;
                layers.erase(layers.begin() + insert_index);
            }
    };

} // end namespace ui