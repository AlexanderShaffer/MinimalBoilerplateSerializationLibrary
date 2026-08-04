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
import package_tracker;
import library_template;

namespace writer {
namespace {
template<typename T>
concept instance_of_field_collection = requires (T t) { requires std::same_as<T, decltype(library_template::field_collection{t})>; };

class library_writer {
public:
  void write_string(const std::string_view string_view) { m_ofstream << string_view; }

  template<typename FieldTemplate>
  void write_field_template() {
    for (bool first_group{true}; const auto& [group_name, package_map] : package_tracker::get_group_map()) {
      m_field_arg_holder.group_name_ = group_name;
      replace_comma_field(first_group);
      write_field_collection<typename FieldTemplate::group_start>();

      for (bool first_struct{true}; const auto& [package_name, package] : package_map) {
        m_field_arg_holder.package_name_ = package_name;
        m_field_arg_holder.package_endianness_ = package.endianness_;
        replace_comma_field(first_struct);
        write_field_collection<typename FieldTemplate::package_start>();

        for (const auto& [member_type, member_name] : package.members_) {
          m_field_arg_holder.member_type_ = member_type;
          m_field_arg_holder.member_name_ = member_name;
          write_field_collection<typename FieldTemplate::member>();
        }

        write_field_collection<typename FieldTemplate::package_end>();
      }

      write_field_collection<typename FieldTemplate::group_end>();
    }
  }

private:
  std::ofstream m_ofstream{"mbsl.cppm"};
  library_template::field_arg_holder m_field_arg_holder;

  void replace_comma_field(bool& ignore) {
    m_field_arg_holder.comma_ = ignore ? "" : ",\n";
    ignore = false;
  }

  template<instance_of_field_collection FieldCollection>
  void write_field_collection() {
    std::ranges::for_each(FieldCollection::resolve(m_field_arg_holder), std::bind_front(&library_writer::write_string, this));
  }
};
} // namespace

void write_library() {
  library_writer library_writer;

  library_writer.write_string(library_template::TEMPLATE_START);
  library_writer.write_field_template<library_template::exported_definitions_template>();
  library_writer.write_string(library_template::TEMPLATE_BODY);
  library_writer.write_field_template<library_template::registry_template>();
  library_writer.write_string(library_template::TEMPLATE_END);
}
} // namespace writer
