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
struct packet {
  std::string_view endianness_{};
  std::vector<member> members_{};
};

using packet_map = std::flat_map<std::string_view, packet>;
std::flat_map<std::string_view, packet_map> g_group_map{};

class library_writer {
public:
  void write_string(const std::string_view string_view) { m_ofstream << string_view; }

  template<typename FieldTemplate>
  void write_field_template() {
    for (bool first_group{true}; const auto& [group_name, packet_map] : g_group_map) {
      m_replaceable_field_holder.group_name_ = group_name;
      calculate_comma_field(first_group);
      write_field_collection<typename FieldTemplate::group_start>();

      for (bool first_struct{true}; const auto& [packet_name, packet] : packet_map) {
        m_replaceable_field_holder.packet_name_ = packet_name;
        m_replaceable_field_holder.packet_endianness_ = packet.endianness_;
        calculate_comma_field(first_struct);
        write_field_collection<typename FieldTemplate::packet_start>();

        for (const auto& [member_type, member_name] : packet.members_) {
          m_replaceable_field_holder.member_type_ = member_type;
          m_replaceable_field_holder.member_name_ = member_name;
          write_field_collection<typename FieldTemplate::member>();
        }

        write_field_collection<typename FieldTemplate::packet_end>();
      }

      write_field_collection<typename FieldTemplate::group_end>();
    }
  }

private:
  std::ofstream m_ofstream{"mbsl.cppm"};
  library_template::replaceable_field_holder m_replaceable_field_holder{};

  void calculate_comma_field(bool& first_list_item) {
    m_replaceable_field_holder.comma_ = first_list_item ? "" : ",\n";
    first_list_item = false;
  }

  template<typename FieldCollection>
  void write_field_collection() {
    std::ranges::for_each(FieldCollection::resolve(m_replaceable_field_holder), std::bind_front(&library_writer::write_string, this));
  }
};
} // namespace

bool add_packet(const std::string_view group_name, const std::string_view packet_name, const std::string_view endianness,
                std::vector<member>&& members) {
  packet_map& packet_map{g_group_map.try_emplace(group_name).first->second};
  const bool unique{packet_map.try_emplace(packet_name, endianness, std::move(members)).second};

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
