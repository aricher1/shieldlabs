// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <string>
#include <unordered_map>
#include <vector>



namespace isotope {
    
    struct ShieldingData {
        double hvl1_mm;
        double hvl2_mm;
        double tvl1_mm;
        double tvl2_mm;
    };


    struct IsotopeDef {
        int id;
        std::string key;
        std::string name;
        double gamma_constant_uSv_m2_per_MBq_h;
        double half_life_hours;

        std::unordered_map<std::string, ShieldingData> materials;
    };


    class IsotopeRegistry {

        private:
            std::unordered_map<int, IsotopeDef> isotopes;
            std::unordered_map<std::string, int> key_to_id;

        public:
            bool load_from_file(const std::string& path);

            const IsotopeDef* get(int id) const;
            const IsotopeDef* get_by_key(const std::string& key) const;
            
            const std::vector<std::string>& get_all_keys() const;
    };

} // end namespace isotope