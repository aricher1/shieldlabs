// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "ui/Command.hpp"
#include "geometry/GeometryEngine.hpp"



namespace ui {

    class ShiftGeometryCommand : public Command {

        private:
            geom::GeometryEngine& engine;
            double dx;
            double dy;

        public: 
            ShiftGeometryCommand(geom::GeometryEngine& engine, double dx_cm, double dy_cm) : engine(engine), dx(dx_cm), dy(dy_cm) {}
            
            void execute() override { // override derived
                // walls
                for (auto& w : engine.get_walls_mutable()) {
                    w.a.x_cm += dx;
                    w.a.y_cm += dy;
                    w.b.x_cm += dx;
                    w.b.y_cm += dy;
                }    

                // entities
                for (auto& e : engine.get_entities_mutable()) {
                    e.position.x_cm += dx;
                    e.position.y_cm += dy;
                }
            }

            void undo() override {
                // reverse shift
                for (auto& w : engine.get_walls_mutable()) {
                    w.a.x_cm -= dx;
                    w.a.y_cm -= dy;
                    w.b.x_cm -= dx;
                    w.b.y_cm -= dy;
                }

                for (auto& e : engine.get_entities_mutable()) {
                    e.position.x_cm -= dx;
                    e.position.y_cm -= dy;
                }
            }

    };

} // end namespace ui