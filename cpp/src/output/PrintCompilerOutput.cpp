#include "output/PrintCompilerOutput.hpp"
#include "materials/MaterialRegistry.hpp"
#include <tabulate/table.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

constexpr double WEEKS_PER_YEAR = 52.0;

extern MaterialRegistry material_registry;

namespace output {

static std::string fmt(double v, int precision = 6) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << v;
    return ss.str();
}

void print(const calc::CompilerOutput& out) {
    std::cout << "\n====================== COMPILER OUTPUT ======================\n";

    // Per-ray detailed output (debug)
    for (size_t i = 0; i < out.rays.size(); ++i) {
        const auto& entry = out.rays[i];
        const auto& ray   = entry.ray;
        const auto& dose  = entry.dose;

        std::cout << "\nRay " << i << ":\n";
        std::cout << "  Source index: " << ray.source_index << "\n";
        std::cout << "  Dose index:   " << ray.dose_index << "\n";
        std::cout << "  Distance from Source to Dose: " << std::fixed << std::setprecision(4) << ray.geometric_distance_cm << " cm\n";

        double cumulative = 0.0;
        for (const auto& seg : ray.segments) {
            cumulative += seg.path_length_cm;
            std::cout << "    " << (seg.material_id < 0 ? "Air" : "Material") << " | " << seg.path_length_cm << " cm | cumulative: " << cumulative << " cm\n";
        }

        std::cout << "  Isotope: " << entry.isotope_key << "\n";
    }

    if (out.dose_totals.empty()) {
        std::cout << "No dose points.\n";
        return;
    }

    for (const auto& dose_total : out.dose_totals) {
        const int d = dose_total.dose_index;
        std::cout << "\n======================= DOSE POINT " << d << " ========================\n\n";
        tabulate::Table table;
        table.add_row({"Src","t(hrs)","Dist(cm)","Occ","Lead(cm)","Conc(cm)","Steel(cm)","Atten","d(uSv)","Ad(uSv/y)"});

        double total_effective = 0.0;
        double total_annual    = 0.0;

        for (const auto& entry : out.rays) {
            if (entry.ray.dose_index != d) {
                continue;
            }

            double lead_cm = 0.0;
            double concrete_cm = 0.0;
            double steel_cm = 0.0;

            for (const auto& seg : entry.ray.segments) {
                if (seg.material_id < 0) {
                    continue;
                }

                const MaterialDef* mat = material_registry.get(seg.material_id);
                if (!mat) {
                    continue;
                }

                if (mat->key == "lead") {
                    lead_cm += seg.path_length_cm;
                } else if (mat->key == "concrete") {
                    concrete_cm += seg.path_length_cm;
                } else if (mat->key == "steel") {
                    steel_cm += seg.path_length_cm;
                }
            }

            const double effective_dose = entry.integrated.integrated_dose_uSv * dose_total.occupancy;
            const double annual_dose = effective_dose * WEEKS_PER_YEAR;

            total_effective += effective_dose;
            total_annual += annual_dose;

            table.add_row({
                std::to_string(entry.ray.source_index),
                fmt(entry.integrated.integration_time_h, 2),
                fmt(entry.ray.geometric_distance_cm, 2),
                fmt(dose_total.occupancy, 2),
                fmt(lead_cm, 4),
                fmt(concrete_cm, 4),
                fmt(steel_cm, 4),
                fmt(entry.dose.transmission_total, 4),
                fmt(effective_dose, 6),
                fmt(annual_dose, 6)
            });
        }

        table.add_row({"Total","-","-","-","-","-","-","-",fmt(total_effective, 6),fmt(total_annual, 6)});
        table.format().font_align(tabulate::FontAlign::right);

        for (size_t i = 0; i < table[0].size(); ++i) {
            table[0][i].format().font_align(tabulate::FontAlign::center);
        }

        std::cout << table << "\n";
    }

    std::cout << "=============================================================\n";
}

} // end namespace output
