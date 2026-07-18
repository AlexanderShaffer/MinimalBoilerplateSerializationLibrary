/*
 * This file is part of MinimalBoilerplateSerializationLibrary.
 * Copyright (C) 2026 Alexander Shaffer <alexander.shaffer.623@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

module writer;
import library_template;

namespace writer {
namespace {
struct struct_info {
  std::string_view endianness_{};
  std::vector<member> members_{};
};

using struct_map = std::flat_map<std::string_view, struct_info>;
std::flat_map<std::string_view, struct_map> g_group_map{};

class library_writer {
public:
  void write_string(const std::string_view string_view) { m_ofstream << string_view; }

  template<typename FieldTemplate>
  void write_field_template() {
    for (const auto& [group_name, struct_map] : g_group_map) {
      m_replaceable_field_holder.group_name_ = group_name;
      write_field_collection<typename FieldTemplate::group_start>();

      for (const auto& [struct_name, struct_info] : struct_map) {
        m_replaceable_field_holder.struct_name_ = struct_name;
        m_replaceable_field_holder.struct_endianness_ = struct_info.endianness_;
        write_field_collection<typename FieldTemplate::struct_start>();

        for (const auto& [type, member_name] : struct_info.members_) {
          m_replaceable_field_holder.member_type_ = type;
          m_replaceable_field_holder.member_name_ = member_name;
          write_field_collection<typename FieldTemplate::member>();
        }

        write_field_collection<typename FieldTemplate::struct_end>();
      }

      write_field_collection<typename FieldTemplate::group_end>();
    }
  }

private:
  std::ofstream m_ofstream{"mbsl.cppm"};
  library_template::replaceable_field_holder m_replaceable_field_holder{};

  template<typename FieldCollection>
  void write_field_collection() {
    std::ranges::for_each(FieldCollection::resolve(m_replaceable_field_holder), std::bind_front(&library_writer::write_string, this));
  }
};
} // namespace

bool add_struct(const std::string_view group_name, const std::string_view struct_name, const std::string_view endianness,
                std::vector<member>&& members) {
  struct_map& struct_map{g_group_map.try_emplace(group_name).first->second};
  const bool unique{struct_map.try_emplace(struct_name, endianness, std::move(members)).second};

  return unique;
}

void write_library() {
  library_writer library_writer{};

  library_writer.write_string(library_template::TEMPLATE_START);
  library_writer.write_field_template<library_template::exported_definitions_template>();
  library_writer.write_string(library_template::TEMPLATE_BODY);
  library_writer.write_field_template<library_template::registry_template>();
  library_writer.write_string(library_template::TEMPLATE_END);
}
} // namespace writer
