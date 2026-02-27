// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <fstream>
#include <iomanip>

#include "output/ExportCompilerOutputCSV.hpp"
#include "output/BuildDosePointReports.hpp"
#include "isotopes/IsotopeRegistry.hpp"


using namespace isotope;
using namespace calc;

extern IsotopeRegistry isotope_registry;


namespace output {

    bool export_compiler_output_csv(const CompilerOutput& out, const std::string& filepath) {
        std::ofstream file(filepath);
        if (!file.is_open()) {
            return false;
        }

        const IsotopeDef* iso = isotope_registry.get_by_key(out.isotope_key);
        if (iso) {
            file << "Isotope," << iso->key << "\n";
            file << "Isotope Name," << iso->name << "\n";
            file << "Gamma Constant (uSv_m2_per_MBq_h)," << iso->gamma_constant_uSv_m2_per_MBq_h << "\n";
            file << "Half-life (hours)," << iso->half_life_hours << "\n";
        }
        file << "\n";

        const auto reports = build_dose_point_reports(out);
        file <<"Dose Point,Dose Label,Source,""t(hrs),dist(cm),T,""Lead(cm),Conc(cm),Steel(cm),""B,d(uSv),Ad(uSv/y), DoseLimit(uSv)\n";

        for (size_t i = 0; i < reports.size(); ++i) {
            const auto& report = reports[i];

            for (const auto& row : report.rows) {
                file
                    << (i + 1) << ","
                    << report.dose_label << ","
                    << row.source_label << ","
                    << row.integration_time_h << ","
                    << row.distance_cm << ","
                    << row.occupancy << ","
                    << row.lead_cm << ","
                    << row.concrete_cm << ","
                    << row.steel_cm << ","
                    << row.wall_attenuation << ","
                    << row.effective_dose_uSv << ","
                    << row.annual_dose_uSv
                    << "\n";
            }

            file
                << (i + 1) << ","
                << report.dose_label << ",Total,"
                << ",,,,,,,"
                << report.total_effective_dose_uSv << ","
                << report.total_annual_dose_uSv
                << report.dose_limit_uSv
                << "\n";
        }

        return true;
    }

} // end namespace output