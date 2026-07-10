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
constexpr std::string_view FORMAT_STRING{
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

module;
#include <cstddef>
export module mbsl;
import std;

static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Mixed endianness is unsupported");

export namespace mbsl {{
template<class>
struct reflection_vendor {{
  static consteval auto get_reflection();
}};
{}}} // namespace mbsl

namespace mbsl {{
namespace {{
template<typename T>
concept noncontiguous = false; // TODO: Implement this concept

template<typename T>
concept endianness_susceptible = std::is_trivially_copyable_v<T> && !std::is_pointer_v<T> && !std::is_member_pointer_v<T> && alignof(T) > 1;

template<typename T>
concept endianness_resistant = std::is_trivially_copyable_v<T> && alignof(T) == 1;

template<typename T, size_t MEMBER_OFFSET>
requires noncontiguous<T> || endianness_susceptible<T> || endianness_resistant<T>
struct member : std::type_identity<T> {{
  static constexpr auto OFFSET{{MEMBER_OFFSET}};
}};

template<typename T, template<typename, std::size_t> class Template>
concept instance_of = requires (T t) {{ Template(t); }};

template<std::size_t... VALUES>
consteval void compress_losslessly_and_push(const std::optional<std::span<std::uint8_t>> reflection, std::size_t& size) {{
  for (const auto value : {{VALUES...}}) {{
    const auto big_endian_value{{std::endian::native == std::endian::little ? std::byteswap(value) : value}};
    const auto bytes{{std::bit_cast<std::array<std::uint8_t, sizeof(big_endian_value)>>(big_endian_value)}};
    std::span<const std::uint8_t> remaining_bytes{{bytes}};

    while (!remaining_bytes.empty() && remaining_bytes.front() == 0) {{
      remaining_bytes = remaining_bytes.subspan(1);
    }}

    if (reflection) {{
      (*reflection)[size] = remaining_bytes.size();
      std::ranges::copy(remaining_bytes, std::next(reflection->begin(), size + 1));
    }}

    size += 1 + remaining_bytes.size();
  }}
}}

template<class Struct, std::endian ENDIANNESS, instance_of<member>... Members>
struct struct_register : std::type_identity<Struct> {{
  static consteval void build_reflection(const std::optional<std::span<std::uint8_t>> reflection, std::size_t& size) {{
    compress_losslessly_and_push<Members::OFFSET..., sizeof(typename Members::type)..., sizeof(Struct)>(reflection, size);
  }}

private:
  static consteval bool is_valid_member_order() {{
    bool inside_endianness_susceptible_region{{}};
    bool inside_endianness_resistant_region{{}};

    return ([&] {{
      const bool valid_endianness_susceptible_region{{!inside_endianness_susceptible_region || !noncontiguous<typename Members::type>}};
      const bool valid_endianness_resistant_region{{!inside_endianness_resistant_region || endianness_resistant<typename Members::type>}};

      inside_endianness_susceptible_region = endianness_susceptible<typename Members::type>;
      inside_endianness_resistant_region = endianness_resistant<typename Members::type>;
      return valid_endianness_susceptible_region && valid_endianness_resistant_region;
    }}() && ...);
  }}

  static consteval std::size_t find_region_offset(const auto is_before_offset) {{
    static_assert(is_valid_member_order(), "Struct members must follow the order: noncontiguous, endianness susceptible, and endianness resistant");
    std::size_t offset{{}};
    std::size_t size{{}};

    const bool last_member_is_before_offset{{([&] {{
      offset = Members::OFFSET;
      size = sizeof(typename Members::type);
      return is_before_offset.template operator()<typename Members::type>();
    }}() && ...)}};

    return last_member_is_before_offset ? offset + size : offset;
  }}
}};

template<class T = void, class... Ts>
struct vendor {{
  template<class Identifier>
  static consteval auto find_type_linked_to() {{
    if constexpr (std::derived_from<T, std::type_identity<Identifier>>) {{
      return T{{}};
    }} else if constexpr (requires {{ typename T::template get<Identifier>; }}) {{
      return typename T::template get<Identifier>{{}};
    }} else if constexpr (sizeof...(Ts) > 0) {{
      return vendor<Ts...>::template find_type_linked_to<Identifier>();
    }}
  }}

  template<class Identifier>
  requires (!std::is_void_v<decltype(find_type_linked_to<Identifier>())>)
  using get = decltype(find_type_linked_to<Identifier>());
}};

template<class Conduit, std::uint16_t VERSION, class... StructRegisters>
struct conduit_register : std::type_identity<Conduit>, vendor<StructRegisters...> {{
  static consteval auto get_reflection() {{
    std::array<std::uint8_t, build_reflection()> reflection{{}};
    build_reflection(reflection);
    return reflection;
  }}

private:
  static consteval std::size_t build_reflection(const std::optional<std::span<std::uint8_t>> reflection = {{}}) {{
    std::size_t size{{}};
    compress_losslessly_and_push<VERSION>(reflection, size);
    (StructRegisters::build_reflection(reflection, size), ...);
    return size;
  }}
}};

using registry = vendor<{}
>;

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

// TODO: disallow endianness susceptible class types and allow multidimensional arrays
template<instance_of<member> Member>
void swap_bytes_if_endianness_susceptible(const auto& in, auto& out) {{
  if constexpr (instance_of<typename Member::type, std::array> && endianness_susceptible<typename Member::type>) {{
    for (size_t i{{}}; i < sizeof(typename Member::type); i += sizeof(typename Member::type::value_type)) {{
      swap_bytes<typename Member::type::value_type>(Member::OFFSET + i, in, out);
    }}
  }} else if constexpr (endianness_susceptible<typename Member::type>) {{
    swap_bytes<typename Member::type>(Member::OFFSET, in, out);
  }}
}}
}} // namespace

template<class Conduit>
consteval auto reflection_vendor<Conduit>::get_reflection() {{
  return registry::get<Conduit>::get_reflection();
}}
}} // namespace mbsl
)"};
} // namespace library_template
