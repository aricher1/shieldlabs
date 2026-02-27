// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <yaml-cpp/yaml.h>
#include <iostream>

#include "materials/MaterialRegistry.hpp"


namespace material {

    bool MaterialRegistry::load_from_file(const std::string& path) {
        materials.clear();

        YAML::Node root;
        try {
            root = YAML::LoadFile(path);
        } catch (const std::exception& e) {
            std::cerr << "Failed to load materials.yml: " << e.what() << "\n";
            return false;
        }

        if (!root["materials"] || !root["materials"].IsSequence()) {
            std::cerr << "materials.yml missing 'materials' list.\n";
            return false;
        }

        for (const auto& node : root["materials"]) {
            if (!node["id"] || !node["key"] || !node["name"]) {
                std::cerr << "Invalid material entry in materials.yml.\n";
                return false;
            }

            MaterialDef m;
            m.id = node["id"].as<int>();
            m.key = node["key"].as<std::string>();
            m.name = node["name"].as<std::string>();

            if (materials.count(m.id)) {
                std::cerr << "Duplicate material id: " << m.id << ".\n";
                return false;
            }

            materials.emplace(m.id, m);
            material_order.push_back(m.id);
        }

        return true;
    }


    const MaterialDef* MaterialRegistry::get(int id) const {
        auto it = materials.find(id);
        if (it == materials.end()) { return nullptr; }
        return &it->second;
    }


    const std::vector<int>& MaterialRegistry::ordered_ids() const {
        return material_order;
    }


    int MaterialRegistry::next_id(int current_id) const {
        if (material_order.empty()) {
            return current_id;
        }

        auto it = std::find(material_order.begin(), material_order.end(), current_id);

        if (it == material_order.end() || ++it == material_order.end()) {
            return material_order.front();
        }

        return *it;
    }

    int MaterialRegistry::get_id_by_key(const std::string& key) const {
        for (const auto& [id, mat] : materials) {
            if (mat.key == key) {
                return id;
            }
        }
        
        return -1; // not found
    }

} // end namespace material