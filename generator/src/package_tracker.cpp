module package_tracker;

namespace package_tracker {
namespace {
group_map g_group_map{};
} // namespace

bool add(const std::string_view group_name, const std::string_view package_name, const std::string_view endianness, std::vector<member>&& members) {
  package_map& package_map{g_group_map.try_emplace(group_name).first->second};
  const bool unique{package_map.try_emplace(package_name, endianness, std::move(members)).second};

  return unique;
}

const group_map& get_group_map() { return g_group_map; }
} // namespace package_tracker
