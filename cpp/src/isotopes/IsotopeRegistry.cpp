// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <yaml-cpp/yaml.h>
#include <iostream>

#include "isotopes/IsotopeRegistry.hpp"


namespace isotope {

    bool IsotopeRegistry::load_from_file(const std::string& path) {
        isotopes.clear();
        key_to_id.clear();

        YAML::Node root;
        try {
            root = YAML::LoadFile(path);
        } catch (const std::exception& e) {
            std::cerr << "Failed to load isotopes.yml: " << e.what() << "\n";
            return false;
        }

        if (!root["isotopes"] || !root["isotopes"].IsSequence()) {
            std::cerr << "isotopes.yml missing 'isotopes' list.\n";
            return false;
        }

        for (const auto& node : root["isotopes"]) {
            IsotopeDef iso;

            iso.id = node["id"].as<int>();
            iso.key = node["key"].as<std::string>();
            iso.name = node["name"].as<std::string>();
            iso.gamma_constant_uSv_m2_per_MBq_h = node["gamma_constant_uSv_m2_per_MBq_h"].as<double>();
            iso.half_life_hours = node["half_life_hours"].as<double>();

            if (!node["materials"]) {
                std::cerr << "Isotope missing materials block.\n";
                return false;
            }

            for (const auto& it : node["materials"]) {
                const std::string mat_key = it.first.as<std::string>();
                const auto& m = it.second;

                ShieldingData sd;
                sd.hvl1_mm = m["hvl1_mm"].as<double>();
                sd.hvl2_mm = m["hvl2_mm"].as<double>();
                sd.tvl1_mm = m["tvl1_mm"].as<double>();
                sd.tvl2_mm = m["tvl2_mm"].as<double>();

                iso.materials.emplace(mat_key, sd);
            }

            isotopes.emplace(iso.id, iso);
            key_to_id.emplace(iso.key, iso.id);
        }

        return true;
    }


    const IsotopeDef* IsotopeRegistry::get(int id) const {
        auto it = isotopes.find(id);
        if (it == isotopes.end()) { return nullptr; }
        return &it->second;
    }


    const IsotopeDef* IsotopeRegistry::get_by_key(const std::string& key) const {
        auto it = key_to_id.find(key);
        if (it == key_to_id.end()) { return nullptr; }
        return get(it->second);
    }


    const std::vector<std::string>& IsotopeRegistry::get_all_keys() const {
        static std::vector<std::string> keys;
        keys.clear();

        for (const auto& [key, _] : key_to_id) {
            keys.push_back(key);
        }
        return keys;
    }

} // end namespace isotope