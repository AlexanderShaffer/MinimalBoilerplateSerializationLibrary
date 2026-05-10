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

export module writer;
import std;

export namespace writer {
class block {
public:
  block(std::string_view type, std::string_view name, bool semicolon_end);
  ~block();
  block(const block& other) = delete;
  block(block&& other) = delete;
  void operator=(const block& other) = delete;
  void operator=(block&& other) = delete;

private:
  std::string_view m_type{};
  std::string_view m_name{};
  bool m_semicolon_end{};
};

class namespace_block : public block {
public:
  namespace_block(const bool exported, const std::string_view name) : block{exported ? "export namespace" : "namespace", name, false} {}
};

class struct_block : public block {
public:
  static void write_type(std::string_view type);
  static void write_value(std::string_view value);

  explicit struct_block(const std::string_view name) : block{"struct", name, true} {}
};

void write_module_declaration();
} // namespace writer
