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
std::ofstream g_interface{"mbsl.cppm"};
std::ofstream g_implementation{"mbsl.cpp"};
} // namespace

namespace writer {
block::block(const bool interface, const std::string_view type, const std::string_view name, const bool semicolon_end)
  : m_ofstream{interface ? &g_interface : &g_implementation}, m_type{type}, m_name{name}, m_semicolon_end{semicolon_end} {
  std::println(*m_ofstream, "{} {} {{", m_type, m_name);
}

block::~block() { std::println(*m_ofstream, "}}{}", m_semicolon_end ? ";" : ""); }

void struct_block::write_member(const std::string_view type, const std::string_view identifier) const {
  std::println(*m_ofstream, "  {} {}{{}};", type, identifier);
}

void write_module_declarations() {
  std::println(g_interface, "export module mbsl;\nimport std;");
  std::println(g_implementation, "module mbsl;");
}
} // namespace writer
