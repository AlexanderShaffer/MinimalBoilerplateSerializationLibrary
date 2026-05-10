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

namespace {
std::ofstream g_out{"mbsl.cppm"};
} // namespace

namespace writer {
block::block(const std::string_view type, const std::string_view name, const bool semicolon_end)
  : m_type{type}, m_name{name}, m_semicolon_end{semicolon_end} {
  std::println(g_out, "{} {} {{", m_type, m_name);
}

block::~block() { std::println(g_out, "}}{}", m_semicolon_end ? ";" : ""); }

void struct_block::write_type(std::string_view type) { std::print(g_out, "  {} ", type); }
void struct_block::write_value(std::string_view value) { std::println(g_out, "{};", value); }

void write_module_declaration() { std::println(g_out, "export module mbsl;\nimport std;"); }
} // namespace writer
