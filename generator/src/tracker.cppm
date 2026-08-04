export module tracker;
import std;

export struct package_tracker {
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

  static bool add(std::string_view group_name, std::string_view package_name, std::string_view endianness, std::vector<member>&& members);
  [[nodiscard]] static const group_map& get_group_map() { return g_group_map; }

private:
  static inline group_map g_group_map;
};

export struct module_import_tracker {
  static void add(std::string_view module_name);
  [[nodiscard]] static std::string_view get_import_declarations() { return g_import_declarations; }

private:
  static inline std::string g_import_declarations;
};
