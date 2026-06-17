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

namespace struct_block {
void begin(const std::string_view name, const std::string_view endianness) {
  g_struct_definitions += std::format("\nstruct {} {{\n", name);
  g_struct_registers += std::format("{}  struct_register<{}, {}", g_first_struct ? "\n" : ",\n", name, endianness);
}

void add_member_type(const std::string_view type) {
  g_struct_definitions += std::format("  {} ", type);
  g_struct_registers += std::format(", member<{}, ", type);
}

void add_member_name(const std::string_view struct_name, const std::string_view member_name) {
  g_struct_definitions += std::format("{}{{}};\n", member_name);
  g_struct_registers += std::format("offsetof({}, {})>", struct_name, member_name);
}

void end() {
  g_struct_definitions += "};\n";
  g_struct_registers += ">";
  g_first_struct = false;
}
} // namespace struct_block

void write_library() {
  std::ofstream out{"mbsl.cppm"};
  std::print(out, library_template::FORMAT_STRING, g_struct_definitions, g_struct_registers);
}
} // namespace writer
