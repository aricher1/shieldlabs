#pragma once 
#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"


class AddWallLayerCommand : public Command {

    private:
        GeometryEngine& engine;
        std::size_t wall_index;
        std::size_t insert_index;
        WallLayer layer;

    public:
        AddWallLayerCommand(GeometryEngine& e, std::size_t wi, std::size_t idx, const WallLayer& l) : engine(e), wall_index(wi), insert_index(idx), layer(l) {}

        void execute() override { // override derived
            auto& layers = engine.get_walls_mutable()[wall_index].layers;
            layers.insert(layers.begin() + insert_index, layer);
        }

        void undo() override { // override derived
            auto& layers = engine.get_walls_mutable()[wall_index].layers;
            layers.erase(layers.begin() + insert_index);
        }
};