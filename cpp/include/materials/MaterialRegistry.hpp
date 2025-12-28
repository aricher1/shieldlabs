#pragma once
#include <string>
#include <unordered_map>


struct MaterialDef {
    int id;
    std::string key;
    std::string name;
};


class MaterialRegistry {

    private:
        std::unordered_map<int, MaterialDef> materials;

    public:
        bool load_from_file(const std::string& path);
        const MaterialDef* get(int id) const;
};