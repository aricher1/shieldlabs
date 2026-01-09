#pragma once
#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



class RemoveEntityCommand : public Command {

    private:
        GeometryEngine& engine;
        std::size_t index;
        PointEntity saved;

    public:
        RemoveEntityCommand(GeometryEngine& engine, std::size_t index) : engine(engine), index(index), saved(engine.get_entities()[index]) {} 

        void execute() override { // override derived

            engine.remove_entity_at(index);

        }

        void undo() override { // override derived

            engine.add_entity_direct(saved);

        }

};