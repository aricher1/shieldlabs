#pragma once
#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



class AddWallCommand : public Command {

    private:
        GeometryEngine& engine;
        Wall wall;

    public:
        AddWallCommand(GeometryEngine& engine, const Wall& wall) : engine(engine), wall(wall) {} 

        void execute() override { 
            engine.add_wall_direct(wall);
        }

        void undo() override {
            engine.remove_last_wall();
        }

};