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
  std::string conduit_name_{};
  std::string struct_name_{};
  std::string endianness_{};
  std::vector<member> members_{};
};

std::strong_ordering operator<=>(const struct_info& lhs, const struct_info& rhs) {
  const std::strong_ordering conduit_ordering{lhs.conduit_name_ <=> rhs.conduit_name_};
  return conduit_ordering == std::strong_ordering::equal ? lhs.struct_name_ <=> rhs.struct_name_ : conduit_ordering;
}

std::flat_set<struct_info> g_structs{};
} // namespace

bool add_struct(const std::string_view conduit_name, const std::string_view struct_name, const std::string_view endianness,
                std::vector<member>&& members) {
  const bool unique{g_structs.emplace(std::string{conduit_name}, std::string{struct_name}, std::string{endianness}, std::move(members)).second};
  return unique;
}

void write_library() {
  std::ofstream out{"mbsl.cppm"};
  std::print(out, library_template::FORMAT_STRING, "", "");
}
} // namespace writer
