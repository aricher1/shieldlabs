// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "Point.hpp"



namespace geom {

    class Viewport {

        public: 
            double pixels_per_cm = 10.0;
            double origin_x_px = 0.0;
            double origin_y_px = 0.0;

            Point pixel_to_world(double px, double py) const;

    };

} // end namespace geom