#pragma once 
#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"


class AddWallLayerCommand : public Command {

    private:
        GeometryEngine& engine;
        std::size_t wall_index;
        WallLayer layer;

    public:
        AddWallLayerCommand(GeometryEngine& e, std::size_t wi, WallLayer l) : engine(e), wall_index(wi), layer(l) {}

        void execute() override { // override derived
            engine.get_walls_mutable()[wall_index].layers.push_back(layer);
        }

        void undo() override { // override derived
            engine.get_walls_mutable()[wall_index].layers.pop_back();
        }
};