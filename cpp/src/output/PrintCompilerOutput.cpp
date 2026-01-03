#include "output/PrintCompilerOutput.hpp"
#include "output/BuildDosePointReports.hpp"
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
    const auto reports = build_dose_point_reports(out);

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

    if (reports.empty()) {
        std::cout << "No dose points.\n";
        return;
    }

    for (const auto& report : reports) {
        std::cout
            << "\n======================= DOSE POINT " << report.dose_index << " ========================\n\n";
        tabulate::Table table;

        // ---- header ----
        table.add_row({"s","t(hrs)","d(cm)","T","Lead(cm)","Conc(cm)","Steel(cm)","B","d(uSv)","Ad(uSv/y)"});

        // ---- per-source rows ----
        for (const auto& row : report.rows) {
            table.add_row({
                std::to_string(row.source_index),
                fmt(row.integration_time_h, 3),
                fmt(row.distance_cm, 2),
                fmt(row.occupancy, 2),
                fmt(row.lead_cm, 2),
                fmt(row.concrete_cm, 2),
                fmt(row.steel_cm, 2),
                fmt(row.wall_attenuation, 4),
                fmt(row.effective_dose_uSv, 6),
                fmt(row.annual_dose_uSv, 6)
            });
        }

        // ---- totals row ----
        table.add_row({"Total","-","-","-","-","-","-","-",fmt(report.total_effective_dose_uSv, 6),fmt(report.total_annual_dose_uSv, 6)});

        // ---- formatting ----
        table.format().font_align(tabulate::FontAlign::right);

        for (size_t c = 0; c < table[0].size(); ++c) {
            table[0][c].format().font_align(tabulate::FontAlign::center);
        }

        std::cout << table << "\n";
    }

    std::cout << "=============================================================\n";
}

} // end namespace output