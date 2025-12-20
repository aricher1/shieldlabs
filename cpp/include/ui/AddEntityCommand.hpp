#pragma once
#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



class AddEntityCommand : public Command {

    private: 
        GeometryEngine& engine;
        PointEntity entity;

    public:
        AddEntityCommand(GeometryEngine& engine, const PointEntity& entity) : engine(engine), entity(entity) {}

        void execute() override { // override derived

            engine.add_entitiy_direct(entity);

        }

        void undo() override { // override derived

            engine.remove_entity_at(engine.get_entities().size() - 1);
        
        }

};