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

namespace mbsl {{
export {{
template<typename>
struct reflection_vendor {{
  static consteval auto get_reflection();
}};
{}}}

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

template<class Struct, std::endian ENDIANNESS, instance_of<member>... Members>
struct struct_register : std::type_identity<Struct> {{
  static constexpr auto REFLECTION_SIZE{{(sizeof...(Members) * 2) + 1}};

  static consteval void create_reflection(const std::span<std::uint16_t> reflection, std::size_t& index) {{
    ((set<Members::OFFSET>(reflection, index), set<sizeof(typename Members::type)>(reflection, index)), ...);
    set<sizeof(Struct)>(reflection, index);
  }}

private:
  template<std::size_t VALUE>
  static consteval void set(const std::span<std::uint16_t> reflection, std::size_t& index) {{
    static_assert(VALUE <= std::numeric_limits<std::uint16_t>::max(),
                  "Registered structs must have member offsets and sizes that are at most the 16-bit unsigned integer limit");

    const auto value{{static_cast<std::uint16_t>(VALUE)}};
    reflection[index++] = std::endian::native == std::endian::little ? value : std::byteswap(value);
  }}

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

template<class... Ts>
struct vendor;

template<>
struct vendor<> {{
  template<class>
  static consteval void get() {{}}
}};

template<class T, class Identifier>
concept matching_identifier = std::derived_from<T, std::type_identity<Identifier>>;

template<class Vendor, class Identifier>
concept suitable_vendor = !std::same_as<decltype(Vendor::template get<Identifier>()), void>;

template<class T, class... Ts>
struct vendor<T, Ts...> {{
  template<class Identifier>
  static consteval auto get() {{
    if constexpr (matching_identifier<T, Identifier>) {{
      return T{{}};
    }} else if constexpr (suitable_vendor<T, Identifier>) {{
      return T::template get<Identifier>();
    }} else {{
      return vendor<Ts...>::template get<Identifier>();
    }}
  }}
}};

template<class Conduit, std::uint16_t VERSION, class... StructRegisters>
struct conduit_register : std::type_identity<Conduit>, vendor<StructRegisters...> {{
  static consteval auto create_reflection() {{
    std::array<std::uint16_t, (StructRegisters::REFLECTION_SIZE + ...)> reflection{{}};
    std::size_t index{{}};

    (StructRegisters::create_reflection(reflection, index), ...);
    return reflection;
  }}
}};

using registry = vendor<{}
>;

static_assert(requires {{ registry::get<void>(); }}, "Failed to initialize the registry");

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
  return registry::get<Conduit>().create_reflection();
}}
}} // namespace mbsl
)"};
} // namespace library_template
