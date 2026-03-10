// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <string>
#include <unordered_map>
#include <vector>



namespace material {

    struct MaterialDef {
        int id;
        std::string key;
        std::string name;
    };


    class MaterialRegistry {

        private:
            std::unordered_map<int, MaterialDef> materials;
            std::vector<int> material_order;

        public:
            bool load_from_file(const std::string& path);

            const MaterialDef* get(int id) const;
            const std::vector<int>& ordered_ids() const;
            
            int next_id(int current_id) const;

            int get_id_by_key(const std::string& key) const;
    };

}