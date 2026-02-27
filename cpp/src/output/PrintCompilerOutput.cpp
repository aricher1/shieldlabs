// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <tabulate/table.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "output/PrintCompilerOutput.hpp"
#include "output/BuildDosePointReports.hpp"
#include "materials/MaterialRegistry.hpp"
#include "isotopes/IsotopeRegistry.hpp"

using namespace isotope;
using namespace material;


extern MaterialRegistry material_registry;
extern IsotopeRegistry isotope_registry;


namespace output {

    static std::string fmt(double v, int precision = 6) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(precision) << v;
        return ss.str();
    }

    void print(const calc::CompilerOutput& out) {
        std::cout << "\n====================== COMPILER OUTPUT DEBUG ======================\n";
        std::cout << "Isotope: " << out.isotope_key << "\n";          

        const auto reports = build_dose_point_reports(out);

        // Per-ray detailed output (debug)
        for (size_t i = 0; i < out.rays.size(); ++i) {
            const auto& entry = out.rays[i];
            const auto& ray = entry.ray;
            [[maybe_unused]] const auto& dose = entry.dose;
            std::cout << "\nRay " << i << ":\n";
            std::cout << "  Source Point: " << entry.source_label << "\n";
            std::cout << "  Dose Point: " << entry.dose_label << "\n";
            std::cout << "  Distance from Source to Dose: " << std::fixed << std::setprecision(4) << ray.geometric_distance_cm << " cm\n";
            double cumulative = 0.0;
            for (const auto& seg : ray.segments) {
                cumulative += seg.path_length_cm;
                std::cout << "    " << (seg.material_id < 0 ? "Air" : "Material") << " | " << seg.path_length_cm << " cm | cumulative: " << cumulative << " cm\n";
            }
        }

        if (reports.empty()) {
            std::cout << "No dose points.\n";
            return;
        }

        for (const auto& report : reports) {
            std::cout << "\n======================= DOSE POINT " << report.dose_label << " ========================\n\n";
            tabulate::Table table;

            table.add_row({"s","t(hrs)","d(cm)","T","Lead(cm)","Conc(cm)","Steel(cm)","B","d(uSv)","Ad(uSv/y)", "DoseLimit(uSv)"});
            for (const auto& row : report.rows) {
                table.add_row({
                    row.source_label,
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

            table.add_row({
                "Total","-","-","-","-","-","-","-",
                fmt(report.total_effective_dose_uSv, 6),
                fmt(report.total_annual_dose_uSv, 6),
                fmt(report.dose_limit_uSv, 6)
            });
            table.format().font_align(tabulate::FontAlign::right);

            for (size_t c = 0; c < table[0].size(); ++c) {
                table[0][c].format().font_align(tabulate::FontAlign::center);
            }

            std::cout << table << "\n";
        }

        std::cout << "=============================================================\n";
    }

} // end namespace output
