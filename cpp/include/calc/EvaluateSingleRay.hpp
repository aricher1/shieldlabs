#pragma once
#include "materials/MaterialRegistry.hpp"
#include "isotopes/IsotopeRegistry.hpp"
#include "TransportRay.hpp"


using namespace::calc;

struct SingleRayDoseResult {
    double distance_cm;
    double inverse_square;
    double activity_MBq;
    double transmission_total;
    double dose_uSv_per_h;
    double patient_transmission;
};


SingleRayDoseResult evaluate_single_ray(
    const TransportRay& ray,
    const CalcSource& source,
    const IsotopeDef& isotope,
    const MaterialRegistry& material_registry,
    double activity_per_patient_MBq
);