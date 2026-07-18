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

export module library_template:field;
import std;

namespace library_template {
export struct replaceable_field_holder;
using replaceable_field = std::string_view replaceable_field_holder::*;

template<std::size_t SIZE = 0>
struct field {
  static constexpr bool REPLACEABLE{SIZE == 0};

  std::array<char, SIZE == 0 ? 0 : SIZE - 1> string_{};
  replaceable_field replaceable_field_{};

  consteval field(const char (&string)[SIZE]) requires (!REPLACEABLE) { std::copy_n(string, string_.size(), string_.begin()); }
  consteval field(const replaceable_field replaceable_field) : replaceable_field_{replaceable_field} {}

  [[nodiscard]] std::string_view resolve(const replaceable_field_holder& replaceable_field_holder) const {
    if constexpr (REPLACEABLE) {
      return replaceable_field_holder.*replaceable_field_;
    } else {
      return std::string_view{string_};
    }
  }
};
} // namespace library_template
