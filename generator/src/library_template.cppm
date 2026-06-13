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
import std;

export namespace library_template {
constexpr std::string_view START{
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

namespace {{
template<typename T, template<typename> class Requirement>
concept Number = (std::is_arithmetic_v<T> || std::is_enum_v<T>) && Requirement<T>::VALUE;

template<typename T>
concept StdArray = requires (T t) {{
  requires std::same_as<T, std::array<typename T::value_type, t.size()>>;
  requires !t.empty();
}};

template<typename T, template<typename> class Requirement>
concept EndiannessReaction = Number<T, Requirement> || (StdArray<T> && Number<typename T::value_type, Requirement>);

template<typename T>
struct is_size_one {{
  static constexpr bool VALUE{{sizeof(T) == 1}};
}};

template<typename T>
concept EndiannessResistant = EndiannessReaction<T, is_size_one>;

template<typename T>
struct not_size_one {{
  static constexpr bool VALUE{{sizeof(T) != 1}};
}};

template<typename T>
concept EndiannessSusceptible = EndiannessReaction<T, not_size_one>;

template<typename T>
concept Noncontiguous = false; // TODO: Implement this concept

template<typename T>
concept MemberType = EndiannessResistant<T> || EndiannessSusceptible<T> || Noncontiguous<T>;

template<MemberType MemberType, size_t MEMBER_OFFSET>
struct member {{
  using type = MemberType;
  static constexpr auto OFFSET{{MEMBER_OFFSET}};
}};

template<typename T>
concept Member = std::same_as<T, member<typename T::type, T::OFFSET>>;

template<Member CurrentMember, Member... Members>
struct region_parser;

template<Member Member>
struct region_parser<Member> {{
  template<template<typename> class IsBefore>
  static constexpr auto REGION_OFFSET{{Member::OFFSET + (IsBefore<typename Member::type>::VALUE ? sizeof(typename Member::type) : 0)}};
}};

template<Member CurrentMember, Member NextMember, Member... Members>
struct region_parser<CurrentMember, NextMember, Members...> : region_parser<NextMember, Members...> {{
  static_assert(!EndiannessSusceptible<typename CurrentMember::type> || !EndiannessResistant<typename NextMember::type>,
                "Endianness resistant members must precede endianness susceptible members");

  static_assert(!Noncontiguous<typename CurrentMember::type> || !EndiannessResistant<typename NextMember::type>,
                "Endianness resistant members must precede noncontiguous members");

  static_assert(!Noncontiguous<typename CurrentMember::type> || !EndiannessSusceptible<typename NextMember::type>,
                "Endianness susceptible members must precede noncontiguous members");

  template<template<typename> class IsBefore>
  static constexpr auto REGION_OFFSET{{
    IsBefore<typename CurrentMember::type>::VALUE ? region_parser<NextMember, Members...>::template REGION_OFFSET<IsBefore> : CurrentMember::OFFSET}};
}};

template<Member... Members>
struct member_serializer : region_parser<Members...> {{
  template<typename T>
  struct is_before_endianness_susceptible_region {{
    static constexpr bool VALUE{{EndiannessResistant<T>}};
  }};

  template<typename T>
  struct is_before_noncontiguous_region {{
    static constexpr bool VALUE{{!Noncontiguous<T>}};
  }};
}};

template<typename T>
struct serializer;

template<typename T, std::integral CurrentIntegral, std::integral... Integrals>
void swap_bytes(const auto& in, auto& out) {{
  if constexpr (sizeof(T) == sizeof(CurrentIntegral)) {{
    reinterpret_cast<CurrentIntegral&>(out) = std::byteswap(reinterpret_cast<const CurrentIntegral&>(in));
  }} else if constexpr (sizeof...(Integrals) > 0) {{
    swap_bytes<T, Integrals...>(in, out);
  }} else {{
    static_assert(false, "Cannot byte swap a type with an unsupported size");
  }}
}}

template<typename T>
void swap_bytes(const size_t offset, const auto& in, auto& out) {{
  const auto& in_pos{{reinterpret_cast<const std::byte*>(&in)[offset]}};
  auto& out_pos{{reinterpret_cast<std::byte*>(&out)[offset]}};

  swap_bytes<T, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>(in_pos, out_pos);
}}

template<typename Member>
void swap_bytes_if_endianness_susceptible(const auto& in, auto& out) {{
  if constexpr (StdArray<typename Member::type> && EndiannessSusceptible<typename Member::type>) {{
    for (size_t i{{}}; i < sizeof(typename Member::type); i += sizeof(typename Member::type::value_type)) {{
      swap_bytes<typename Member::type::value_type>(Member::OFFSET + i, in, out);
    }}
  }} else if constexpr (EndiannessSusceptible<typename Member::type>) {{
    swap_bytes<typename Member::type>(Member::OFFSET, in, out);
  }}
}}
}} // namespace

export namespace mbsl {{
)"};

constexpr std::string_view END{"}} // namespace mbsl\n"};
} // namespace library_template
