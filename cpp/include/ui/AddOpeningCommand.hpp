#pragma once
#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"




class AddOpeningCommand : public Command {

    private:
        GeometryEngine& engine;
        std::size_t wall_index;
        std::size_t opening_index{};
        WallOpening opening;

    public: 
        AddOpeningCommand(GeometryEngine& engine, std::size_t wall_index, std::size_t opening_index, const WallOpening& opening) : engine(engine), wall_index(wall_index), opening_index(opening_index), opening(opening) {}

        void execute() override { // override derived
            
            auto& openings = engine.get_walls_mutable()[wall_index].openings;
            openings.insert(openings.begin() + opening_index, opening);

        }

        void undo () override { // override derive
            
            auto& openings = engine.get_walls_mutable()[wall_index].openings;
            openings.erase(openings.begin() + opening_index);

        }

};