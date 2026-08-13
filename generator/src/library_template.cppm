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
  std::string_view comma_;
  std::string_view group_name_;
  std::string_view package_name_;
  std::string_view package_endianness_;
  std::string_view member_type_;
  std::string_view member_name_;
};

template<field... FIELDS>
struct field_collection {
  static std::array<std::string_view, sizeof...(FIELDS)> resolve(const field_arg_holder& field_arg_holder) {
    return {FIELDS.resolve(field_arg_holder)...};
  }
};

constexpr replaceable_field COMMA{&field_arg_holder::comma_};
constexpr replaceable_field GROUP_NAME{&field_arg_holder::group_name_};
constexpr replaceable_field PACKAGE_NAME{&field_arg_holder::package_name_};
constexpr replaceable_field PACKAGE_ENDIANNESS{&field_arg_holder::package_endianness_};
constexpr replaceable_field MEMBER_TYPE{&field_arg_holder::member_type_};
constexpr replaceable_field MEMBER_NAME{&field_arg_holder::member_name_};

constexpr std::string_view SECTION_1{R"(/*
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
)"};

constexpr std::string_view SECTION_2{R"(
static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big, "Mixed endianness is unsupported");

namespace mbsl {
template<typename T, template<typename, auto> class Template>
concept instance_of = requires (T t) { requires std::same_as<T, decltype(Template(t))>; };

template<typename T>
[[nodiscard]] consteval bool is_numerical() {
  if constexpr (instance_of<T, std::array>) {
    return is_numerical<typename T::value_type>();
  } else {
    return std::is_arithmetic_v<T> || std::is_enum_v<T>;
  }
}

template<typename T>
concept numerical = is_numerical<T>();

template<typename Member>
concept endianness_susceptible = alignof(typename Member::type) > 1 && numerical<typename Member::type>;

template<typename Member>
concept endianness_resistant = alignof(typename Member::type) == 1 && numerical<typename Member::type>;

template<typename Member>
concept implicitly_serializable = endianness_susceptible<Member> || endianness_resistant<Member>;

template<typename T, std::size_t OFFSET_ = 0>
struct member : std::type_identity<T> {
  static constexpr std::size_t OFFSET{OFFSET_};
};
} // namespace mbsl

export namespace mbsl {
template<bool /* ENDIANNESS_MISMATCH */>
class dynamic_serializer : std::allocator<std::byte> {
public:
  dynamic_serializer() = default;

  ~dynamic_serializer() {
    if (m_span.data()) {
      deallocate(m_span.data(), m_span.size());
    }
  }

  dynamic_serializer(const dynamic_serializer& other) : m_size{other.m_size} {
    reserve_at_least(other.m_size);
    std::memcpy(m_span.data(), other.m_span.data(), other.m_size);
  }

  dynamic_serializer(dynamic_serializer&& other) noexcept { swap(other); }

  dynamic_serializer& operator=(dynamic_serializer other) {
    swap(other);
    return *this;
  }

  void preallocate(const std::size_t size) { reserve_at_least(m_size + size); }

  template<numerical Numerical>
  void serialize_without_bounds_checking(const Numerical& numerical) {
    serialize(numerical);
    m_size += sizeof(Numerical);
  }

  template<numerical Numerical>
  void serialize_with_bounds_checking(const Numerical& numerical) {
    const std::size_t new_size{m_size + sizeof(Numerical)};

    reserve_at_least(new_size);
    serialize(numerical);
    m_size = new_size;
  }

  [[nodiscard]] const std::byte* data() const noexcept { return m_span.data(); }
  [[nodiscard]] std::size_t size() const noexcept { return m_size; }

private:
  std::span<std::byte> m_span;
  std::size_t m_size{};

  void swap(dynamic_serializer& other) noexcept {
    std::swap(m_span, other.m_span);
    std::swap(m_size, other.m_size);
  }

  void reserve_at_least(const std::size_t min_capacity) {
    if (min_capacity <= m_span.size()) {
      return;
    }

    const std::size_t new_capacity{min_capacity * 2};
    const std::span new_span{allocate(new_capacity), new_capacity};

    if (m_span.data()) {
      std::memcpy(new_span.data(), m_span.data(), m_size);
      deallocate(m_span.data(), m_span.size());
    }

    m_span = new_span;
  }

  template<numerical Numerical>
  void serialize(const Numerical& numerical);
};

template<typename T>
concept instance_of_dynamic_serializer = requires (T t) { requires std::same_as<T, decltype(dynamic_serializer{t})>; };

template<typename DynamicSerializer>
struct explicit_serializer;

enum class method : std::uint8_t { MUTABLY, IMMUTABLY, NEW };

[[nodiscard]] auto to_span(auto& data) {
  static constexpr bool CONST{std::is_const_v<std::remove_reference_t<decltype(data)>>};
  using byte_type = std::conditional_t<CONST, const std::byte, std::byte>;

  return std::span<byte_type, sizeof(data)>{reinterpret_cast<byte_type*>(&data), sizeof(data)};
}

template<bool ENDIANNESS_MISMATCH, instance_of<member>... Members>
struct member_serializer {
private:
  using dynamic_serializer = dynamic_serializer<ENDIANNESS_MISMATCH>;
  using explicit_serializer = explicit_serializer<dynamic_serializer>;

  template<instance_of<member> Member>
  static constexpr bool EXPLICITLY_SERIALIZABLE{requires (dynamic_serializer d, const Member::type t) {
    explicit_serializer::serialize(d, t);
    requires std::same_as<typename Member::type, decltype(explicit_serializer::template deserialize<typename Member::type>(std::span<std::byte>{}))>;
  } && !implicitly_serializable<Member>};

public:
  static constexpr bool VALID_MEMBERS{((EXPLICITLY_SERIALIZABLE<Members> || implicitly_serializable<Members>) && ...)};
  static constexpr bool HAS_EXPLICITLY_SERIALIZABLE_MEMBER{(EXPLICITLY_SERIALIZABLE<Members> || ...)};

  template<method GIVEN_METHOD, method PREFERRED_METHOD>
  static constexpr bool VALID_METHOD{VALID_MEMBERS && !HAS_EXPLICITLY_SERIALIZABLE_MEMBER && GIVEN_METHOD == PREFERRED_METHOD};

  static void serialize_endianness_susceptible_members_to(const std::span<std::byte> dest, const std::span<const std::byte> src) {
    (serialize_to_if_endianness_susceptible<Members>(dest, src), ...);
  }

  static void serialize_implicitly_serializable_members_to(const std::span<std::byte> dest, const std::span<const std::byte> src) {
    static constexpr std::size_t ENDIANNESS_RESISTANT_REGION_END{find_region_offset([]<typename /* Member */> { return true; })};

    if constexpr (ENDIANNESS_MISMATCH) {
      serialize_endianness_susceptible_members_to(dest, src);

      static constexpr std::size_t ENDIANNESS_RESISTANT_REGION_START{
        find_region_offset([]<typename Member> { return !endianness_resistant<Member>; })};
      copy<ENDIANNESS_RESISTANT_REGION_START, ENDIANNESS_RESISTANT_REGION_END>(dest, src);
    } else {
      static constexpr std::size_t ENDIANNESS_SUSCEPTIBLE_REGION_START{
        find_region_offset([]<typename Member> { return EXPLICITLY_SERIALIZABLE<Member>; })};
      copy<ENDIANNESS_SUSCEPTIBLE_REGION_START, ENDIANNESS_RESISTANT_REGION_END>(dest, src);
    }
  }

private:
  template<instance_of<member> Member, std::integral Integral, std::integral... Integrals>
  [[nodiscard]] static consteval auto to_integral() {
    if constexpr (sizeof(typename Member::type) == sizeof(Integral)) {
      return Integral{};
    } else if constexpr (sizeof...(Integrals) > 0) {
      return to_integral<Member, Integrals...>();
    }
  }

  template<instance_of<member> Member>
  requires (!instance_of<typename Member::type, std::array>)
  static void serialize_to_if_endianness_susceptible(const std::span<std::byte> dest, const std::span<const std::byte> src) {
    if constexpr (endianness_susceptible<Member>) {
      using integral = decltype(to_integral<Member, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>());
      static_assert(!std::is_void_v<integral>, "Member type sizes must be powers of 2 and at most 8 bytes");

      integral& integral_dest{reinterpret_cast<integral&>(dest[Member::OFFSET])};
      const integral& integral_src{reinterpret_cast<const integral&>(src[Member::OFFSET])};
      integral_dest = std::byteswap(integral_src);
    }
  }

  template<instance_of<member> ArrayMember, std::size_t INDEX = 0>
  requires (instance_of<typename ArrayMember::type, std::array>)
  static void serialize_to_if_endianness_susceptible(const std::span<std::byte> dest, const std::span<const std::byte> src) {
    if constexpr (endianness_susceptible<ArrayMember> && INDEX < std::tuple_size_v<typename ArrayMember::type>) {
      static constexpr std::size_t ELEMENT_OFFSET{ArrayMember::OFFSET + (INDEX * sizeof(typename ArrayMember::type::value_type))};

      serialize_to_if_endianness_susceptible<member<typename ArrayMember::type::value_type, ELEMENT_OFFSET>>(dest, src);
      serialize_to_if_endianness_susceptible<ArrayMember, INDEX + 1>(dest, src);
    }
  }

  [[nodiscard]] static consteval bool has_valid_member_order() {
    bool inside_endianness_susceptible_region{};
    bool inside_endianness_resistant_region{};

    return ([&] {
      const bool valid_endianness_susceptible_region{!inside_endianness_susceptible_region || !EXPLICITLY_SERIALIZABLE<Members>};
      const bool valid_endianness_resistant_region{!inside_endianness_resistant_region || endianness_resistant<Members>};

      inside_endianness_susceptible_region = endianness_susceptible<Members>;
      inside_endianness_resistant_region = endianness_resistant<Members>;
      return valid_endianness_susceptible_region && valid_endianness_resistant_region;
    }() && ...);
  }

  [[nodiscard]] static consteval std::size_t find_region_offset(const auto is_before_offset) {
    static_assert(has_valid_member_order(),
                  "Expected package members to follow the order: explicitly serializable, endianness susceptible, and endianness resistant");

    std::size_t offset{};
    std::size_t size{};

    const bool last_member_is_before_offset{([&] {
      offset = Members::OFFSET;
      size = sizeof(typename Members::type);
      return is_before_offset.template operator()<Members>();
    }() && ...)};

    return last_member_is_before_offset ? offset + size : offset;
  }

  template<std::size_t START_OFFSET, std::size_t END_OFFSET>
  static void copy(const std::span<std::byte> dest, const std::span<const std::byte> src) {
    std::ranges::copy(src.subspan(START_OFFSET, END_OFFSET - START_OFFSET), dest.subspan(START_OFFSET).begin());
  }
};

template<bool ENDIANNESS_MISMATCH>
template<numerical Numerical>
void dynamic_serializer<ENDIANNESS_MISMATCH>::serialize(const Numerical& numerical) {
  member_serializer<ENDIANNESS_MISMATCH, member<Numerical>>::serialize_implicitly_serializable_members_to(m_span.subspan(m_size), to_span(numerical));
}

template<class Container>
class depot : Container {
public:
  using Container::Container;
  using Container::size;

  template<typename Byte>
  requires (sizeof(Byte) == 1 && (std::integral<Byte> || std::is_enum_v<Byte>))
  [[nodiscard]] const Byte* data() const noexcept(noexcept(Container::data())) {
    return reinterpret_cast<const Byte*>(Container::data());
  }
};
)"};

struct exported_definitions_template {
  using group_start = field_collection<"\nnamespace ", GROUP_NAME, " {">;
  using package_start = field_collection<"\nstruct ", PACKAGE_NAME, " {\n">;
  using member = field_collection<"  ", MEMBER_TYPE, " ", MEMBER_NAME, ";\n">;
  using package_end = field_collection<"};\n">;
  using group_end = field_collection<"} // namespace ", GROUP_NAME, "\n">;
};

constexpr std::string_view SECTION_3{R"(} // namespace mbsl

namespace mbsl {
template<class Package, std::endian ENDIANNESS, instance_of<member>... Members>
struct package_serializer : std::type_identity<Package> {
private:
  static constexpr bool ENDIANNESS_MISMATCH{std::endian::native != ENDIANNESS};
  using member_serializer = member_serializer<ENDIANNESS_MISMATCH, Members...>;

public:
  static constexpr std::initializer_list<std::size_t> REFLECTION_VALUES{Members::OFFSET..., sizeof(typename Members::type)..., sizeof(Package)};
  static constexpr std::size_t REFLECTION_VALUES_SIZE_BYTES{REFLECTION_VALUES.size() * sizeof(typename decltype(REFLECTION_VALUES)::value_type)};

  template<method /* METHOD */>
  requires (member_serializer::VALID_MEMBERS && member_serializer::HAS_EXPLICITLY_SERIALIZABLE_MEMBER)
  static auto serialize(const std::span<const std::byte> package) {}

  template<method METHOD, size_t PACKAGE_SIZE>
  requires (member_serializer::template VALID_METHOD<METHOD, method::MUTABLY>)
  [[nodiscard]] static auto serialize(const std::span<std::byte, PACKAGE_SIZE> package) {
    if constexpr (ENDIANNESS_MISMATCH) {
      member_serializer::serialize_endianness_susceptible_members_to(package, package);
    }

    return to_depot_span(package);
  }

  template<method METHOD, size_t PACKAGE_SIZE>
  requires (member_serializer::template VALID_METHOD<METHOD, method::IMMUTABLY>)
  [[nodiscard]] static auto serialize(const std::span<const std::byte, PACKAGE_SIZE> package) {
    if constexpr (ENDIANNESS_MISMATCH) {
      return serialize<method::NEW>(package);
    } else {
      return to_depot_span(package);
    }
  }

  template<method METHOD, size_t PACKAGE_SIZE>
  requires (member_serializer::template VALID_METHOD<METHOD, method::NEW>)
  [[nodiscard]] static auto serialize(const std::span<const std::byte, PACKAGE_SIZE> package) {
    depot<std::array<std::byte, PACKAGE_SIZE>> depot;
    member_serializer::serialize_implicitly_serializable_members_to(to_span(depot), package);
    return depot;
  }

private:
  template<typename T, std::size_t PACKAGE_SIZE>
  [[nodiscard]] static auto to_depot_span(const std::span<T, PACKAGE_SIZE> package) {
    return depot<decltype(package)>{package.data(), package.size()};
  }
};

namespace {
template<class Item = void, class... Items>
struct vendor {
private:
  template<class Package>
  [[nodiscard]] static consteval auto find_package_serializer_linked_to() {
    if constexpr (std::derived_from<Item, std::type_identity<Package>>) {
      return Item{};
    } else if constexpr (requires { typename Item::template get<Package>; }) {
      return typename Item::template get<Package>{};
    } else if constexpr (sizeof...(Items) > 0) {
      return vendor<Items...>::template find_package_serializer_linked_to<Package>();
    }
  }

  template<class Package>
  static constexpr bool PACKAGE_EXISTS{!std::is_void_v<decltype(find_package_serializer_linked_to<Package>())>};

public:
  template<class Package>
  requires PACKAGE_EXISTS<Package>
  using get = decltype(find_package_serializer_linked_to<Package>());
};

template<instance_of<package_serializer>... Serializers>
struct group : vendor<Serializers...> {
  [[nodiscard]] static consteval auto get_reflection() {
    std::array<std::uint8_t, (Serializers::REFLECTION_VALUES_SIZE_BYTES + ... + 0)> reflection;
    std::ranges::subrange subrange{reflection};

    for (const std::initializer_list<std::size_t> values : {Serializers::REFLECTION_VALUES...}) {
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

using registry = vendor<)"};

struct registry_template {
  using group_start = field_collection<COMMA, "\n  group<">;
  using package_start = field_collection<COMMA, "\n    package_serializer<", GROUP_NAME, "::", PACKAGE_NAME, ", std::endian::", PACKAGE_ENDIANNESS>;
  using member = field_collection<",\n      member<", MEMBER_TYPE, ", offsetof(", GROUP_NAME, "::", PACKAGE_NAME, ", ", MEMBER_NAME, ")>">;
  using package_end = field_collection<"\n    >">;
  using group_end = field_collection<"\n  >">;
};

constexpr std::string_view SECTION_4{R"(
>;
} // namespace

template<method METHOD>
[[nodiscard]] auto serialize(auto& package) {
  using package_type = std::remove_cv_t<std::remove_reference_t<decltype(package)>>;
  return registry::get<package_type>::template serialize<METHOD>(to_span(package));
}
} // namespace mbsl

export namespace mbsl {
[[nodiscard]] auto serialize_mutably(auto& package)
requires (!std::is_const_v<std::remove_reference_t<decltype(package)>>) {
  return serialize<method::MUTABLY>(package);
}

[[nodiscard]] auto serialize_immutably(auto& package) { return serialize<method::IMMUTABLY>(std::as_const(package)); }
[[nodiscard]] auto serialize_new(const auto& package) { return serialize<method::NEW>(package); }
} // namespace mbsl
)"};
} // namespace library_template
