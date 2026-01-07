#include "output/PrintCompilerOutputUI.hpp"
#include "output/BuildDosePointReports.hpp" 
#include <sstream>
#include <iomanip>

namespace {

std::string fmt(double v, int p = 4) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(p) << v;
    return ss.str();
}

} // end anonymous namespace

namespace output {

void print_to_ui(const calc::CompilerOutput& out, UiLog& log) {
    const auto reports = build_dose_point_reports(out);

    if (reports.empty()) {
        log.push("No dose points.");
        return;
    }

    for (const auto& report : reports) {
        log.separator();
        log.push("DOSE POINT: " + report.dose_label);

        for (const auto& row : report.rows) {
            log.push("Source Point: " + row.source_label);
            log.push("  Dist (cm): " + fmt(row.distance_cm, 2));
            log.push("  Integration (h): " + fmt(row.integration_time_h, 2));
            log.push("  Occ: " + fmt(row.occupancy, 2));
            log.push("  Lead (cm): " + fmt(row.lead_cm, 2));
            log.push("  Conc (cm): " + fmt(row.concrete_cm, 2));
            log.push("  Steel (cm): " + fmt(row.steel_cm, 2));
            log.push("  Atten: " + fmt(row.wall_attenuation, 4));
            log.push("  Dose (uSv): " + fmt(row.effective_dose_uSv, 6));
            log.push("  Annual dose (uSv/y): " + fmt(row.annual_dose_uSv, 6));
        }

        log.push("TOTAL DOSE");
        log.push("  Effective (uSv): " + fmt(report.total_effective_dose_uSv, 6));
        log.push("  Annual (uSv/y): " + fmt(report.total_annual_dose_uSv, 6));
    }
}

} // end namespace output