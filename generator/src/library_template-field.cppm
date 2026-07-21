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
export struct field_arg_holder;

template<std::size_t SIZE>
struct field {
  std::array<char, SIZE - 1> string_{};

  consteval field(const char (&string)[SIZE]) { std::copy_n(string, string_.size(), string_.begin()); }

  [[nodiscard]] std::string_view resolve([[maybe_unused]] const field_arg_holder& field_arg_holder) const { return std::string_view{string_}; }
};

using replaceable_field = field<0>;

template<>
struct field<0> {
  std::string_view field_arg_holder::* field_param_{};

  [[nodiscard]] std::string_view resolve(const field_arg_holder& field_arg_holder) const { return field_arg_holder.*field_param_; }
};
} // namespace library_template
