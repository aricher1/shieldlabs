#pragma once
#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"


class EditWallLayerCommand : public Command {

    private:
        GeometryEngine& engine;
        std::size_t wall_index;
        std::size_t layer_index;
        WallLayer old_layer;
        WallLayer new_layer;

    public:
        EditWallLayerCommand(GeometryEngine& e, std::size_t wi, std::size_t li, WallLayer old_l, WallLayer new_l) : engine(e), wall_index(wi), layer_index(li), old_layer(old_l), new_layer(new_l) {}

        void execute() override { // override derived
            engine.get_walls_mutable()[wall_index].layers[layer_index] = new_layer;
        }

        void undo() override { // override derived
            engine.get_walls_mutable()[wall_index].layers[layer_index] = old_layer;
        }
};