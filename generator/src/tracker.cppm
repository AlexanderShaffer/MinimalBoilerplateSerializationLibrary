export module tracker;
import std;

export namespace package_tracker {
struct member {
  std::string_view type_;
  std::string_view name_;
};

struct package {
  std::string_view endianness_;
  std::vector<member> members_;
};

using package_map = std::flat_map<std::string_view, package>;
using group_map = std::flat_map<std::string_view, package_map>;

bool add(std::string_view group_name, std::string_view package_name, std::string_view endianness, std::vector<member>&& members);
[[nodiscard]] const group_map& get_group_map();
} // namespace package_tracker

export namespace module_import_tracker {
void add(std::string_view module_name);
[[nodiscard]] std::string_view get_import_declarations();
} // namespace module_import_tracker
