// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <sstream>
#include <iomanip>

#include "output/PrintCompilerOutputUI.hpp"
#include "output/BuildDosePointReports.hpp"
#include "isotopes/IsotopeRegistry.hpp" 


using namespace isotope;
using namespace calc;
using namespace ui;

extern IsotopeRegistry isotope_registry;


namespace {

    std::string fmt(double v, int p = 4) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(p) << v;
        return ss.str();
    }

} // end anonymous namespace



namespace output {

    void print_to_ui(const CompilerOutput& out, UiLog& log) {
        
        if (!out.isotope_key.empty()) {
            if (const IsotopeDef* iso = isotope_registry.get_by_key(out.isotope_key)) {
                log.push("[Isotope " + iso->name + "]");
                log.push("- Key: " + iso->key);
                log.push("- Gamma constant: " + fmt(iso->gamma_constant_uSv_m2_per_MBq_h, 8));
                log.push("- Half-life (hrs): " + fmt(iso->half_life_hours, 2));
                log.separator();
            }
        }
        
        const auto reports = build_dose_point_reports(out);

        if (reports.empty()) {
            log.push("No dose points.");
            return;
        }

        for (const auto& report : reports) {
            log.push("[Dose Point: " + report.dose_label + "]");

            for (const auto& row : report.rows) {
                log.push("[Source Point: " + row.source_label + "]");
                log.push("- Dist (cm): " + fmt(row.distance_cm, 2));
                log.push("- Integration (h): " + fmt(row.integration_time_h, 2));
                log.push("- Occ: " + fmt(row.occupancy, 2));
                log.push("- Lead (cm): " + fmt(row.lead_cm, 2));
                log.push("- Conc (cm): " + fmt(row.concrete_cm, 2));
                log.push("- Steel (cm): " + fmt(row.steel_cm, 2));
                log.push("- Atten: " + fmt(row.wall_attenuation, 4));
                log.push("- Dose (uSv): " + fmt(row.effective_dose_uSv, 6));
                log.push("- Annual dose (uSv/y): " + fmt(row.annual_dose_uSv, 6));
            }

            log.push("[Total Dose]");
            log.push("- Dose Limit (uSv): " + fmt(report.dose_limit_uSv, 6));
            log.push("- Effective (uSv): " + fmt(report.total_effective_dose_uSv, 6));
            log.push("- Annual (uSv/y): " + fmt(report.total_annual_dose_uSv, 6));
            log.separator();
        }
        log.push("Press [Optimize]");
    }

} // end namespace output