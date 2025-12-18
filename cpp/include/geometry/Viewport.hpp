#pragma once

#include "Point.hpp"



class Viewport {

    public: 
        double pixels_per_cm = 10.0;
        double origin_x_px = 0.0;
        double origin_y_px = 0.0;

        Point pixel_to_world(double px, double py) const;

};