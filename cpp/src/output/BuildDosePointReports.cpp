// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include "output/BuildDosePointReports.hpp"
#include "materials/MaterialRegistry.hpp"


using namespace material;
using namespace calc;

extern MaterialRegistry material_registry;


namespace output {

    std::vector<DosePointReport> build_dose_point_reports(const CompilerOutput& out) {
        std::vector<DosePointReport> reports;
        for (const auto& dose_total : out.dose_totals) {
            DosePointReport report{};
            report.dose_index = dose_total.dose_index;
            report.dose_limit_uSv = dose_total.dose_limit_uSv;
            report.dose_label = "";
            report.total_effective_dose_uSv = 0.0;
            report.total_annual_dose_uSv = 0.0;

            for (const auto& entry : out.rays) {
                if (entry.ray.dose_index != dose_total.dose_index) {
                    continue;
                }

                SourceDoseRow row{};
                row.source_index = entry.ray.source_index;
                row.source_label = entry.source_label;
                row.integration_time_h = entry.integrated.integration_time_h;
                row.distance_cm = entry.ray.geometric_distance_cm;
                row.occupancy = dose_total.occupancy;
                row.wall_attenuation = entry.dose.transmission_total;

                row.lead_cm = 0.0;
                row.concrete_cm = 0.0;
                row.steel_cm = 0.0;

                for (const auto& seg : entry.ray.segments) {
                    if (seg.material_id < 0) {
                        continue;
                    }

                    const MaterialDef* mat = material_registry.get(seg.material_id);
                    if (!mat) {
                        continue;
                    }

                    if (mat->key == "lead") {
                        row.lead_cm += seg.path_length_cm;
                    } else if (mat->key == "concrete") {
                        row.concrete_cm += seg.path_length_cm;
                    } else if (mat->key == "steel") {
                        row.steel_cm += seg.path_length_cm;
                    }
                }

                row.effective_dose_uSv = entry.integrated.integrated_dose_uSv * dose_total.occupancy;
                row.annual_dose_uSv = row.effective_dose_uSv * 52.0; // weeks per year
                report.total_effective_dose_uSv += row.effective_dose_uSv;
                report.total_annual_dose_uSv += row.annual_dose_uSv;
                report.dose_label = entry.dose_label;

                report.rows.push_back(row);
            }

            reports.push_back(report);
        }

        return reports;
    }

} // end namespace output