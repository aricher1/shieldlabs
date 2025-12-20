#pragma once
#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



class DeleteWallCommand : public Command {

    private:
        GeometryEngine& engine;
        std::size_t index;
        Wall removed_wall;

    public:
        DeleteWallCommand(GeometryEngine& engine, std::size_t index) : engine(engine), index(index) {}

        void execute() override {   // override from derived class

            removed_wall = engine.get_walls()[index];
            engine.remove_wall_at(index);

        }

        void undo() override {   // override from derived class

            engine.add_wall_direct(removed_wall);

        }
};