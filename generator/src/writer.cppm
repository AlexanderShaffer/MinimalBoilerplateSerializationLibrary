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
  std::string_view name_{"conduit"};
  std::string_view version_{"0"};
  std::string_view endianness_{"little"};
};

class struct_block {
public:
  static struct_block create(std::string_view struct_name, const properties& properties);

  ~struct_block();
  struct_block(const struct_block& other) = delete;
  struct_block(struct_block&& other) = delete;
  void operator=(const struct_block& other) = delete;
  void operator=(struct_block&& other) = delete;

  void add_member(std::string_view type, std::string_view name);

private:
  std::string_view m_name{};

  explicit struct_block(std::string_view name);
};

void write_library();
} // namespace writer
