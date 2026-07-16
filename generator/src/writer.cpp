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
std::flat_map<std::string_view, struct_map> g_groups{};
} // namespace

bool add_struct(const std::string_view group_name, const std::string_view struct_name, const std::string_view endianness,
                std::vector<member>&& members) {
  struct_map& structs{g_groups.try_emplace(group_name).first->second};
  const bool unique{structs.try_emplace(struct_name, endianness, std::move(members)).second};

  return unique;
}

void write_library() {
  std::ofstream out{"mbsl.cppm"};
  std::print(out, "{}{}{}", library_template::BEGINNING, library_template::MIDDLE, library_template::END);
}
} // namespace writer
