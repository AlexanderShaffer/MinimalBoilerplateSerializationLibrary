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
import :field;

export namespace library_template {
struct field_arg_holder {
  std::string_view comma_{};
  std::string_view group_name_{};
  std::string_view packet_name_{};
  std::string_view packet_endianness_{};
  std::string_view member_type_{};
  std::string_view member_name_{};
};

template<field... FIELDS>
struct field_collection {
  static std::array<std::string_view, sizeof...(FIELDS)> resolve(const field_arg_holder& field_arg_holder) {
    return {FIELDS.resolve(field_arg_holder)...};
  }
};

constexpr replaceable_field COMMA{.field_param_ = &field_arg_holder::comma_};
constexpr replaceable_field GROUP_NAME{.field_param_ = &field_arg_holder::group_name_};
constexpr replaceable_field PACKET_NAME{.field_param_ = &field_arg_holder::packet_name_};
constexpr replaceable_field PACKET_ENDIANNESS{.field_param_ = &field_arg_holder::packet_endianness_};
constexpr replaceable_field MEMBER_TYPE{.field_param_ = &field_arg_holder::member_type_};
constexpr replaceable_field MEMBER_NAME{.field_param_ = &field_arg_holder::member_name_};

constexpr std::string_view TEMPLATE_START{R"(/*
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

export namespace mbsl {)"};

struct exported_definitions_template {
  using group_start = field_collection<"\nnamespace ", GROUP_NAME, " {">;
  using packet_start = field_collection<"\nstruct ", PACKET_NAME, " {\n">;
  using member = field_collection<"  ", MEMBER_TYPE, " ", MEMBER_NAME, ";\n">;
  using packet_end = field_collection<"};\n">;
  using group_end = field_collection<"} // namespace ", GROUP_NAME, "\n">;
};

constexpr std::string_view TEMPLATE_BODY{R"(} // namespace mbsl

namespace mbsl {
template<typename T>
concept explicitly_serializable = false; // TODO: Implement this concept

template<typename T, template<typename, std::size_t> class Template>
concept instance_of = requires (T t) { Template(t); };

template<typename T>
consteval bool is_numerical() {
  if constexpr (instance_of<T, std::array>) {
    return is_numerical<typename T::value_type>();
  } else {
    return std::is_arithmetic_v<T> || std::is_enum_v<T>;
  }
}

template<typename T>
concept endianness_susceptible = alignof(T) > 1 && is_numerical<T>();

template<typename T>
concept endianness_resistant = alignof(T) == 1 && is_numerical<T>();

template<typename T, std::size_t OFFSET_>
requires explicitly_serializable<T> || endianness_susceptible<T> || endianness_resistant<T>
struct member : std::type_identity<T> {
  static constexpr std::size_t OFFSET{OFFSET_};
};

namespace {
template<class PacketStruct, std::endian ENDIANNESS, instance_of<member>... Members>
struct packet : std::type_identity<PacketStruct> {
private:
  static constexpr bool NO_EXPLICITLY_SERIALIZABLE_MEMBERS{(!explicitly_serializable<typename Members::type> && ...)};

public:
  static constexpr std::initializer_list<std::size_t> REFLECTION_VALUES{Members::OFFSET..., sizeof(typename Members::type)..., sizeof(PacketStruct)};
  static constexpr std::size_t REFLECTION_VALUES_SIZE_BYTES{REFLECTION_VALUES.size() * sizeof(typename decltype(REFLECTION_VALUES)::value_type)};

  template<typename ReinterpretAs = void>
  requires NO_EXPLICITLY_SERIALIZABLE_MEMBERS
  static constexpr auto serialize_in_place(PacketStruct& src_dest) {
    if constexpr (ENDIANNESS_MISMATCH) {
      (serialize_to_if_endianness_susceptible<Members>(src_dest, src_dest), ...);
    }

    if constexpr (!std::is_void_v<ReinterpretAs>) {
      return reinterpret_cast<ReinterpretAs*>(&src_dest);
    }
  }

  template<instance_of<std::array> Dest>
  requires (NO_EXPLICITLY_SERIALIZABLE_MEMBERS && sizeof(Dest) >= sizeof(PacketStruct))
  static constexpr void serialize_to(const PacketStruct& src, Dest& dest) {
    static constexpr std::size_t ENDIANNESS_RESISTANT_REGION_END{find_region_offset([]<typename /* T */> { return true; })};

    if constexpr (ENDIANNESS_MISMATCH) {
      (serialize_to_if_endianness_susceptible<Members>(src, dest), ...);

      static constexpr std::size_t ENDIANNESS_RESISTANT_REGION_START{find_region_offset([]<typename T> { return !endianness_resistant<T>; })};
      copy<ENDIANNESS_RESISTANT_REGION_START, ENDIANNESS_RESISTANT_REGION_END>(src, dest);
    } else {
      static constexpr std::size_t ENDIANNESS_SUSCEPTIBLE_REGION_START{find_region_offset([]<typename T> { return !explicitly_serializable<T>; })};
      copy<ENDIANNESS_SUSCEPTIBLE_REGION_START, ENDIANNESS_RESISTANT_REGION_END>(src, dest);
    }
  }

private:
  static constexpr bool ENDIANNESS_MISMATCH{std::endian::native != ENDIANNESS};

  template<instance_of<member> Member, std::integral CurrentIntegral, std::integral... Integrals>
  static consteval auto to_integral() {
    if constexpr (sizeof(typename Member::type) == sizeof(CurrentIntegral)) {
      return CurrentIntegral{};
    } else if constexpr (sizeof...(Integrals) > 0) {
      return to_integral<Member, Integrals...>();
    }
  }

  template<typename ReinterpretAs, std::size_t OFFSET, typename Arg>
  requires (std::is_pointer_v<ReinterpretAs> || std::is_reference_v<ReinterpretAs>)
  static ReinterpretAs reinterpret(Arg& arg) {
    using byte_ptr = std::conditional_t<std::is_const_v<Arg>, const std::byte*, std::byte*>;
    auto& data_at_offset{reinterpret_cast<byte_ptr>(&arg)[OFFSET]};

    if constexpr (std::is_pointer_v<ReinterpretAs>) {
      return reinterpret_cast<ReinterpretAs>(&data_at_offset);
    } else {
      return reinterpret_cast<ReinterpretAs>(data_at_offset);
    }
  }

  template<instance_of<member> Member>
  requires (endianness_susceptible<typename Member::type> && !instance_of<typename Member::type, std::array>)
  static void serialize_to_if_endianness_susceptible(const PacketStruct& src, auto& dest) {
    using integral = decltype(to_integral<Member, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>());
    static_assert(!std::is_void_v<integral>, "Member type sizes must be powers of 2 and at most 8 bytes");

    reinterpret<integral&, Member::OFFSET>(dest) = std::byteswap(reinterpret<const integral&, Member::OFFSET>(src));
  }

  template<instance_of<member> ArrayMember, std::size_t INDEX = 0>
  requires (endianness_susceptible<typename ArrayMember::type> && instance_of<typename ArrayMember::type, std::array>)
  static constexpr void serialize_to_if_endianness_susceptible(const PacketStruct& src, auto& dest) {
    if constexpr (INDEX < std::tuple_size_v<typename ArrayMember::type>) {
      static constexpr std::size_t ELEMENT_OFFSET{ArrayMember::OFFSET + (INDEX * sizeof(typename ArrayMember::type::value_type))};

      serialize_to<member<typename ArrayMember::type::value_type, ELEMENT_OFFSET>>(src, dest);
      serialize_to<ArrayMember, INDEX + 1>(src, dest);
    }
  }

  template<instance_of<member> Member>
  requires (!endianness_susceptible<typename Member::type>)
  static constexpr void serialize_to_if_endianness_susceptible(const PacketStruct& /* src */, auto& /* dest */) {}

  static consteval bool has_valid_member_order() {
    bool inside_endianness_susceptible_region{};
    bool inside_endianness_resistant_region{};

    return ([&] {
      const bool valid_endianness_susceptible_region{!inside_endianness_susceptible_region || !explicitly_serializable<typename Members::type>};
      const bool valid_endianness_resistant_region{!inside_endianness_resistant_region || endianness_resistant<typename Members::type>};

      inside_endianness_susceptible_region = endianness_susceptible<typename Members::type>;
      inside_endianness_resistant_region = endianness_resistant<typename Members::type>;
      return valid_endianness_susceptible_region && valid_endianness_resistant_region;
    }() && ...);
  }

  static consteval std::size_t find_region_offset(const auto is_before_offset) {
    static_assert(has_valid_member_order(),
                  "Expected packet members to follow the order: explicitly serializable, endianness susceptible, and endianness resistant");

    std::size_t offset{};
    std::size_t size{};

    const bool last_member_is_before_offset{([&] {
      offset = Members::OFFSET;
      size = sizeof(typename Members::type);
      return is_before_offset.template operator()<typename Members::type>();
    }() && ...)};

    return last_member_is_before_offset ? offset + size : offset;
  }

  template<std::size_t START_OFFSET, std::size_t END_OFFSET>
  static void copy(const PacketStruct& src, auto& dest) {
    std::memcpy(reinterpret<void*, START_OFFSET>(dest), reinterpret<const void*, START_OFFSET>(src), END_OFFSET - START_OFFSET);
  }
};

template<typename Vendor, typename PacketStruct>
concept contains_packet = requires { typename Vendor::template get<PacketStruct>; };

template<class Item = void, class... Items>
struct packet_vendor {
private:
  template<class PacketStruct>
  static consteval auto find_packet_linked_to() {
    if constexpr (std::derived_from<Item, std::type_identity<std::remove_cv_t<PacketStruct>>>) {
      return Item{};
    } else if constexpr (contains_packet<Item, PacketStruct>) {
      return typename Item::template get<PacketStruct>{};
    } else if constexpr (sizeof...(Items) > 0) {
      return packet_vendor<Items...>::template find_packet_linked_to<PacketStruct>();
    }
  }

public:
  template<class PacketStruct>
  requires (!std::is_void_v<decltype(find_packet_linked_to<PacketStruct>())>)
  using get = decltype(find_packet_linked_to<PacketStruct>());
};

template<class... Packets>
struct group : packet_vendor<Packets...> {
  static consteval auto get_reflection() {
    std::array<std::uint8_t, (Packets::REFLECTION_VALUES_SIZE_BYTES + ... + 0)> reflection{};
    std::ranges::subrange subrange{reflection};

    for (const std::initializer_list<std::size_t> values : {Packets::REFLECTION_VALUES...}) {
      for (const std::size_t value : values) {
        const std::size_t little_endian_value{std::endian::native == std::endian::little ? value : std::byteswap(value)};
        const std::array serialized_value{std::bit_cast<std::array<std::uint8_t, sizeof(little_endian_value)>>(little_endian_value)};

        std::ranges::copy(serialized_value, subrange.begin());
        subrange.advance(serialized_value.size());
      }
    }

    return reflection;
  }
};

using registry = packet_vendor<)"};

struct registry_template {
  using group_start = field_collection<COMMA, "\n  group<">;
  using packet_start = field_collection<COMMA, "\n    packet<", GROUP_NAME, "::", PACKET_NAME, ", std::endian::", PACKET_ENDIANNESS>;
  using member = field_collection<",\n      member<", MEMBER_TYPE, ", offsetof(", GROUP_NAME, "::", PACKET_NAME, ", ", MEMBER_NAME, ")>">;
  using packet_end = field_collection<"\n    >">;
  using group_end = field_collection<"\n  >">;
};

constexpr std::string_view TEMPLATE_END{R"(
>;

template<typename PacketStruct>
concept packet_exists = contains_packet<registry, PacketStruct>;
} // namespace
} // namespace mbsl

export namespace mbsl {
template<typename ReinterpretAs = void, packet_exists PacketStruct>
constexpr auto serialize_in_place(PacketStruct& src_dest) {
  return registry::get<PacketStruct>::template serialize_in_place<ReinterpretAs>(src_dest);
}

template<packet_exists PacketStruct, instance_of<std::array> Dest>
constexpr void serialize_to(const PacketStruct& src, Dest& dest) {
  registry::get<PacketStruct>::serialize_to(src, dest);
}
} // namespace mbsl
)"};
} // namespace library_template
