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
struct properties {
  std::string_view conduit_name_{"conduit"};
  std::string_view struct_endianness_{"little"};
};

class struct_code_generator {
public:
  static struct_code_generator* create(std::string_view struct_name, const properties& properties);

  explicit struct_code_generator(std::string_view name);

  void add_member(std::string_view type, std::string_view name);

private:
  std::string m_name{};
  std::string m_definition{};
  std::string m_register{};
};

void write_library();
} // namespace writer
