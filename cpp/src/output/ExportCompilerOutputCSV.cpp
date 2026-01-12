#include "output/ExportCompilerOutputCSV.hpp"
#include "output/BuildDosePointReports.hpp"
#include <fstream>
#include <iomanip>

namespace output {

bool export_compiler_output_csv(const calc::CompilerOutput& out, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    const auto reports = build_dose_point_reports(out);

    file <<
        "Dose Point,Dose Label,Source,"
        "t(hrs),dist(cm),T,"
        "Lead(cm),Conc(cm),Steel(cm),"
        "B,d(uSv),Ad(uSv/y)\n";

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
            << "\n";
    }

    return true;
}

} // end namespace output