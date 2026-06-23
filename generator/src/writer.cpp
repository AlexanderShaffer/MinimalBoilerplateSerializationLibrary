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
std::string g_struct_definitions{};
std::string g_struct_registers{};
bool g_first_struct{true};
} // namespace

struct_block::struct_block(const std::string_view name, const std::string_view endianness) : m_name{name} {
  g_struct_definitions += std::format("\nstruct {} {{\n", name);
  g_struct_registers += std::format("{}  struct_register<{}, {}", g_first_struct ? "\n" : ",\n", name, endianness);
}

struct_block::~struct_block() {
  g_struct_definitions += "};\n";
  g_struct_registers += ">";
  g_first_struct = false;
}

void struct_block::add_member(const std::string_view type, const std::string_view name) {
  g_struct_definitions += std::format("  {} {}{{}};\n", type, name);
  g_struct_registers += std::format(", member<{}, offsetof({}, {})>", type, m_name, name);
}

void write_library() {
  std::ofstream out{"mbsl.cppm"};
  std::print(out, library_template::FORMAT_STRING, g_struct_definitions, g_struct_registers);
}
} // namespace writer
