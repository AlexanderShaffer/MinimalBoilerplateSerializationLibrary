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
using name = std::string;
using conduit = std::flat_map<name, struct_code_generator>;

std::unordered_map<name, conduit> g_conduits{};

template<typename... ValueConstructorArgs>
auto try_emplace(auto& map, const std::string_view key_constructor_arg, ValueConstructorArgs&&... value_constructor_args) {
  const auto [iterator, success]{map.try_emplace(std::string{key_constructor_arg}, std::forward<ValueConstructorArgs...>(value_constructor_args...))};
  auto& value{iterator->second};
  return std::pair{&value, success};
}
} // namespace

struct_code_generator* struct_code_generator::create(const std::string_view struct_name, const properties& properties) {
  conduit* const conduit{try_emplace(g_conduits, properties.conduit_name_).first};
  const auto [struct_code_generator, success]{try_emplace(*conduit, struct_name, struct_name)};

  if (!success) {
    std::println(std::cerr, "Error: all structs within a conduit must have a unique name");
    return nullptr;
  }

  return struct_code_generator;
}

struct_code_generator::struct_code_generator(const std::string_view name) : m_name{name}, m_definition{std::format("struct {} {{\n", name)} {}

void struct_code_generator::add_member(const std::string_view type, const std::string_view name) {}

void write_library() {
  std::ofstream out{"mbsl.cppm"};
  std::print(out, library_template::FORMAT_STRING, "", "");
}
} // namespace writer
