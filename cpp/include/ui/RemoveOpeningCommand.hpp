#pragma once

#include "Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class RemoveOpeningCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            std::size_t wall_index;
            std::size_t opening_index;
            geom::WallOpening removed;

        public:
            RemoveOpeningCommand(geom::GeometryEngine& engine, std::size_t wall_index, std::size_t opening_index) : engine(engine), wall_index(wall_index), opening_index(opening_index) {}

            void execute() override { // override derived
                auto& openings = engine.get_walls_mutable()[wall_index].openings;
                removed = openings[opening_index];
                openings.erase(openings.begin() + opening_index);
            }

            void undo() override { // override derived
                auto& openings = engine.get_walls_mutable()[wall_index].openings;
                openings.insert(openings.begin() + opening_index, removed);
            }

    };

} // end namespace ui