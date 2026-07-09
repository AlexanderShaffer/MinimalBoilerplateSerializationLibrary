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
struct conduit {
  std::string version_{};
  std::flat_map<std::string, std::string> structs_{};
};

std::unordered_map<std::string, conduit> g_conduits{};
} // namespace

struct_block struct_block::create(const std::string_view struct_name, const properties& properties) { return struct_block{struct_name}; }

struct_block::struct_block(const std::string_view name) : m_name{name} {}

struct_block::~struct_block() {}

void struct_block::add_member(const std::string_view type, const std::string_view name) {}

void write_library() {
  std::ofstream out{"mbsl.cppm"};
  std::print(out, library_template::FORMAT_STRING, "", "");
}
} // namespace writer
