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

export module library_template;

export namespace library_template {
constexpr auto* DATA{
R"(/*
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

export module mbsl;
import std;

namespace {
template<typename T, template<typename> class Requirement>
concept Number = (std::is_arithmetic_v<T> || std::is_enum_v<T>) && Requirement<T>::VALUE;

template<typename T>
concept StdArray = requires (T t) {
  requires std::same_as<T, std::array<typename T::value_type, t.size()>>;
  requires !t.empty();
};

template<typename T, template<typename> class Requirement>
concept EndiannessReaction = Number<T, Requirement> || (StdArray<T> && Number<typename T::value_type, Requirement>);

template<typename T>
struct is_size_one {
  static constexpr bool VALUE{sizeof(T) == 1};
};

template<typename T>
concept EndiannessResistant = EndiannessReaction<T, is_size_one>;

template<typename T>
struct not_size_one {
  static constexpr bool VALUE{sizeof(T) != 1};
};

template<typename T>
concept EndiannessSusceptible = EndiannessReaction<T, not_size_one>;

template<typename T>
concept Noncontiguous = false; // TODO: Implement this concept

template<typename T>
consteval void assert_supported() {
  static_assert(EndiannessResistant<T> || EndiannessSusceptible<T> || Noncontiguous<T>, "Unsupported member type");
}

template<typename T>
consteval bool is_valid_member_order() {
  assert_supported<T>();
  return true;
}

template<typename CurrentT, typename NextT, typename... Ts>
consteval bool is_valid_member_order() {
  assert_supported<CurrentT>();

  if constexpr (EndiannessSusceptible<CurrentT>) {
    static_assert(!EndiannessResistant<NextT>, "Endianness resistant members must precede endianness susceptible members");
  } else if constexpr (Noncontiguous<CurrentT>) {
    static_assert(!EndiannessResistant<NextT>, "Endianness resistant members must precede noncontiguous members");
    static_assert(!EndiannessSusceptible<NextT>, "Endianness susceptible members must precede noncontiguous members");
  }

  return is_valid_member_order<NextT, Ts...>();
}

template<typename MemberType, size_t MEMBER_OFFSET>
struct member {
  using type = MemberType;
  static constexpr auto OFFSET{MEMBER_OFFSET};
};

template<typename T>
concept Member = std::same_as<T, member<typename T::type, T::OFFSET>>;

template<template<typename> class IsBeforeEndPos, Member CurrentMember, Member... Members>
requires requires { IsBeforeEndPos<typename CurrentMember::type>::VALUE; }
consteval std::size_t get_region_end_pos() {
  if constexpr (sizeof...(Members) > 0) {
    return IsBeforeEndPos<typename CurrentMember::type>::VALUE ? get_region_end_pos<IsBeforeEndPos, Members...>() : CurrentMember::OFFSET;
  } else {
    return CurrentMember::OFFSET;
  }
}

template<typename T>
struct is_before_endianness_susceptible_region {
  static constexpr bool VALUE{EndiannessResistant<T>};
};

template<Member... Members>
consteval std::size_t get_endianness_resistant_region_end_pos() {
  return get_region_end_pos<is_before_endianness_susceptible_region, Members...>();
}

template<typename T>
struct is_before_noncontiguous_region {
  static constexpr bool VALUE{!Noncontiguous<T>};
};

template<Member... Members>
consteval std::size_t get_endianness_susceptible_region_end_pos() {
  return get_region_end_pos<is_before_noncontiguous_region, Members...>();
}
} // namespace
)"};
} // namespace library_template
