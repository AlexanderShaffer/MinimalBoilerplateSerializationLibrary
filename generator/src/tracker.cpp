module tracker;

bool package_tracker::add(const std::string_view group_name, const std::string_view package_name, const std::string_view endianness,
                          std::vector<member>&& members) {
  package_map& package_map{g_group_map.try_emplace(group_name).first->second};
  const bool unique{package_map.try_emplace(package_name, endianness, std::move(members)).second};

  return unique;
}

void module_import_tracker::add(const std::string_view module_name) { g_import_declarations.append("import ").append(module_name).append(";\n"); }
