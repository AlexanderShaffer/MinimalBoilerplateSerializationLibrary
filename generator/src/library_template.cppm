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
class dynamic_serializer;

template<typename T>
concept instance_of_dynamic_serializer = requires (T t) { requires std::same_as<T, decltype(dynamic_serializer{t})>; };

template<typename DynamicSerializer>
struct explicit_serializer;

template<typename Arg, typename Result>
using same_constness_of = std::conditional_t<std::is_const_v<Arg>, std::add_const_t<Result>, std::remove_const_t<Result>>;

template<typename T>
using as_std_byte = same_constness_of<T, std::byte>;

template<typename T>
[[nodiscard]] auto to_span(T& t) {
  return std::span<as_std_byte<T>, sizeof(T)>{reinterpret_cast<as_std_byte<T>*>(&t), sizeof(T)};
}

template<typename T>
[[nodiscard]] T& from_span(const std::span<as_std_byte<T>> span, const std::size_t offset) {
  return reinterpret_cast<T&>(span.subspan(offset, sizeof(T)).front());
}

inline void copy(const std::span<std::byte> dest, const std::span<const std::byte> src) { std::ranges::copy(src, dest.first(src.size()).begin()); }

template<bool ENDIANNESS_MISMATCH, instance_of<member>... Members>
struct member_serializer {
private:
  using dynamic_serializer = dynamic_serializer<ENDIANNESS_MISMATCH>;
  using explicit_serializer = explicit_serializer<dynamic_serializer>;

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

  template<instance_of<member> Member>
  static constexpr bool EXPLICITLY_SERIALIZABLE{requires (dynamic_serializer d, const Member::type t) {
    explicit_serializer::serialize(d, t);
    requires std::same_as<typename Member::type, decltype(explicit_serializer::template deserialize<typename Member::type>(std::span<std::byte>{}))>;
  } && !implicitly_serializable<Member>};

  static constexpr std::size_t ENDIANNESS_SUSCEPTIBLE_REGION_START{
    find_region_offset([]<typename Member> { return EXPLICITLY_SERIALIZABLE<Member>; })};

  static constexpr std::size_t ENDIANNESS_RESISTANT_REGION_END{find_region_offset([]<typename /* Member */> { return true; })};

public:
  static_assert(std::endian::native == std::endian::little || std::endian::native == std::endian::big || (!endianness_susceptible<Members> && ...),
                "Endianness-susceptible members on mixed-endian systems are unsupported");

  static constexpr bool VALID_MEMBERS{((EXPLICITLY_SERIALIZABLE<Members> || implicitly_serializable<Members>) && ...)};
  static constexpr bool ONLY_IMPLICITLY_SERIALIZABLE_MEMBERS{(implicitly_serializable<Members> && ...)};
  static constexpr std::size_t IMPLICITLY_SERIALIZABLE_REGION_SIZE{ENDIANNESS_RESISTANT_REGION_END - ENDIANNESS_SUSCEPTIBLE_REGION_START};

  static void serialize_endianness_susceptible_members_to(const std::span<std::byte> dest, const std::span<const std::byte> src) {
    (serialize_to_if_endianness_susceptible<Members>(dest, src), ...);
  }

  static void serialize_implicitly_serializable_members_to(const std::span<std::byte> dest, const std::span<const std::byte> src) {
    if constexpr (ENDIANNESS_MISMATCH) {
      serialize_endianness_susceptible_members_to(dest, src);

      static constexpr std::size_t ENDIANNESS_RESISTANT_REGION_START{
        find_region_offset([]<typename Member> { return !endianness_resistant<Member>; })};
      copy(dest, src, ENDIANNESS_RESISTANT_REGION_START, ENDIANNESS_RESISTANT_REGION_END);
    } else {
      copy(dest, src, ENDIANNESS_SUSCEPTIBLE_REGION_START, ENDIANNESS_RESISTANT_REGION_END);
    }
  }

  static void serialize_explicitly_serializable_members_to(dynamic_serializer& dest, const std::span<const std::byte> src) {
    (serialize_to_if_explicitly_serializable<Members>(dest, src), ...);
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

      integral& integral_dest{from_span<integral>(dest, Member::OFFSET - ENDIANNESS_SUSCEPTIBLE_REGION_START)};
      const integral& integral_src{from_span<const integral>(src, Member::OFFSET)};
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

  static void copy(const std::span<std::byte> dest, const std::span<const std::byte> src, const std::size_t start_offset,
                   const std::size_t end_offset) {
    mbsl::copy(dest.subspan(start_offset - ENDIANNESS_SUSCEPTIBLE_REGION_START), src.subspan(start_offset, end_offset - start_offset));
  }

  template<instance_of<member> Member>
  static void serialize_to_if_explicitly_serializable(dynamic_serializer& dest, const std::span<const std::byte> src) {
    if constexpr (EXPLICITLY_SERIALIZABLE<Member>) {
      explicit_serializer::serialize(dest, from_span<const typename Member::type>(src, Member::OFFSET));
    }
  }
};

template<bool ENDIANNESS_MISMATCH>
class dynamic_serializer : std::allocator<std::byte> {
  template<typename T>
  using member_serializer = member_serializer<ENDIANNESS_MISMATCH, member<T>>;

  template<typename T>
  static constexpr bool SERIALIZABLE{member_serializer<T>::VALID_MEMBERS};

public:
  dynamic_serializer() = default;

  ~dynamic_serializer() {
    if (m_span.data()) {
      deallocate(m_span.data(), m_span.size());
    }
  }

  dynamic_serializer(const dynamic_serializer& other) : m_size{other.m_size} {
    reserve_at_least(other.m_size);
    copy(m_span, other.m_span.first(other.m_size));
  }

  dynamic_serializer(dynamic_serializer&& other) noexcept { swap(other); }

  dynamic_serializer& operator=(dynamic_serializer other) {
    swap(other);
    return *this;
  }

  std::size_t preallocate(const std::size_t size) {
    const std::size_t min_capacity{m_size + size};
    reserve_at_least(min_capacity);
    return min_capacity;
  }

  template<typename Serializable>
  requires SERIALIZABLE<Serializable>
  void serialize(const Serializable& serializable) {
    if constexpr (implicitly_serializable<member<Serializable>>) {
      const std::size_t new_size{preallocate(sizeof(Serializable))};
      serialize_implicitly(serializable);
      m_size = new_size;
    } else {
      explicit_serializer<dynamic_serializer>::serialize(*this, serializable);
    }
  }

  template<typename Range>
  requires (std::ranges::range<Range> && SERIALIZABLE<std::ranges::range_value_t<Range>>)
  void serialize_range(const Range& range) {
    using range_value = std::remove_const_t<std::ranges::range_value_t<Range>>;
    using range_value_member = member<range_value>;

    static constexpr bool CAN_PREALLOCATE{std::ranges::sized_range<Range> && implicitly_serializable<range_value_member>};
    std::size_t byte_count{};
    std::size_t new_size{};

    if constexpr (CAN_PREALLOCATE) {
      byte_count = std::ranges::size(range) * sizeof(range_value);
      new_size = preallocate(byte_count);
    }

    if constexpr (CAN_PREALLOCATE && (!ENDIANNESS_MISMATCH || endianness_resistant<range_value_member>)) {
      std::ranges::copy(range, reinterpret_cast<range_value*>(get_unoccupied_space().first(byte_count).data()));
      m_size = new_size;
    } else {
      for (const range_value& element : range) {
        if constexpr (CAN_PREALLOCATE) {
          serialize_implicitly(element);
          m_size += sizeof(element);
        } else {
          serialize(element);
        }
      }
    }
  }

  std::span<std::byte> append(const std::size_t size) {
    const std::size_t new_size{preallocate(size)};
    const std::span appended_data{m_span.subspan(m_size, size)};
    m_size = new_size;
    return appended_data;
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
      copy(new_span, m_span.first(m_size));
      deallocate(m_span.data(), m_span.size());
    }

    m_span = new_span;
  }

  template<typename T>
  void serialize_implicitly(const T& t) {
    member_serializer<T>::serialize_implicitly_serializable_members_to(get_unoccupied_space(), to_span(t));
  }

  [[nodiscard]] std::span<std::byte> get_unoccupied_space() const { return m_span.subspan(m_size); }
};

template<typename T>
concept byte = sizeof(T) == 1 && (std::integral<T> || std::is_enum_v<T>);

template<typename T>
concept byte_pointer = std::is_pointer_v<T> && byte<std::remove_pointer_t<T>>;

template<class Container>
requires requires (Container c) { requires byte_pointer<decltype(c.data())>; }
class depot : Container {
  template<class PackageSerializer>
  friend struct serialization_mode;

public:
  using Container::Container;
  using Container::size;

  template<byte Byte>
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
template<class PackageSerializer>
struct serialization_mode {
  template<std::size_t N>
  using allocation = depot<std::array<std::byte, N>>;

  template<byte Byte, std::size_t N>
  [[nodiscard]] static auto to_view(const std::span<Byte, N> src) {
    return depot<std::span<const Byte, N>>{src.data(), src.size()};
  }

  template<typename Container>
  [[nodiscard]] static Container& get_container(depot<Container>& depot) {
    return static_cast<Container&>(depot);
  }

  [[nodiscard]] static auto execute_dynamically(const std::span<const std::byte> src) { return PackageSerializer::serialize_dynamically(src); }
};

template<class PackageSerializer>
struct deserialization_mode {
  template<std::size_t /* N */>
  using allocation = PackageSerializer::package;

  template<byte Byte>
  [[nodiscard]] static decltype(auto) to_view(const std::span<Byte> src) {
    return reinterpret_cast<same_constness_of<Byte, typename PackageSerializer::package>&>(src.front());
  }

  [[nodiscard]] static auto& get_container(PackageSerializer::package& package) { return package; }

  [[nodiscard]] static auto execute_dynamically(const std::span<const std::byte> src) { return PackageSerializer::deserialize_dynamically(src); }
};

template<class Package, std::endian ENDIANNESS, instance_of<member>... Members>
struct package_serializer : std::type_identity<Package> {
private:
  static constexpr bool ENDIANNESS_MISMATCH{std::endian::native != ENDIANNESS};

  using member_serializer = member_serializer<ENDIANNESS_MISMATCH, Members...>;

public:
  using package = Package;

  static constexpr std::initializer_list<std::size_t> REFLECTION_VALUES{Members::OFFSET..., sizeof(typename Members::type)..., sizeof(Package)};
  static constexpr std::size_t REFLECTION_VALUES_SIZE_BYTES{REFLECTION_VALUES.size() * sizeof(typename decltype(REFLECTION_VALUES)::value_type)};

  [[nodiscard]] static auto serialize_dynamically(const std::span<const std::byte> src)
  requires member_serializer::VALID_MEMBERS {
    using dynamic_serializer = dynamic_serializer<ENDIANNESS_MISMATCH>;

    depot<dynamic_serializer> depot;
    dynamic_serializer& dest{serialization_mode<package_serializer>::get_container(depot)};
    const std::span appended_data{dest.append(member_serializer::IMPLICITLY_SERIALIZABLE_REGION_SIZE)};

    member_serializer::serialize_explicitly_serializable_members_to(dest, src);
    member_serializer::serialize_implicitly_serializable_members_to(appended_data, src);
    return depot;
  }

  static void deserialize_dynamically(const std::span<const std::byte> /* src */)
  requires member_serializer::VALID_MEMBERS {
    static_assert(false, "Operation currently unsupported");
  }

  [[nodiscard]] static consteval auto bind(const auto execute_statically) {
    if constexpr (member_serializer::ONLY_IMPLICITLY_SERIALIZABLE_MEMBERS) {
      return execute_statically;
    } else {
      return []<class Mode>(auto&& src) { return Mode::execute_dynamically(std::forward<decltype(src)>(src)); };
    }
  }

  static constexpr auto EXECUTE_MUTABLY{bind([]<class Mode, std::size_t N> [[nodiscard]] (const std::span<std::byte, N> src_dest) -> decltype(auto) {
    if constexpr (ENDIANNESS_MISMATCH) {
      member_serializer::serialize_endianness_susceptible_members_to(src_dest, src_dest);
    }

    return Mode::to_view(src_dest);
  })};

  static constexpr auto EXECUTE_NEW{bind([]<class Mode, std::size_t N> [[nodiscard]] (const std::span<const std::byte, N> src) {
    typename Mode::template allocation<N> allocation;
    auto& container{Mode::get_container(allocation)};

    member_serializer::serialize_implicitly_serializable_members_to(to_span(container), src);
    return allocation;
  })};

  static constexpr auto EXECUTE_IMMUTABLY{
    bind([]<class Mode, std::size_t N> [[nodiscard]] (const std::span<const std::byte, N> src_dest) -> decltype(auto) {
      if constexpr (ENDIANNESS_MISMATCH && (endianness_susceptible<Members> || ...)) {
        return EXECUTE_NEW.template operator()<Mode>(src_dest);
      } else {
        return Mode::to_view(src_dest);
      }
    })};
};

namespace {
template<class Item = void, class... Items>
struct vendor {
private:
  template<class Package>
  [[nodiscard]] static consteval auto get_serializer_linked_to() {
    if constexpr (std::derived_from<Item, std::type_identity<std::remove_const_t<Package>>>) {
      return Item{};
    } else if constexpr (requires { typename Item::template get_serializer<Package>; }) {
      return typename Item::template get_serializer<Package>{};
    } else if constexpr (sizeof...(Items) > 0) {
      return vendor<Items...>::template get_serializer_linked_to<Package>();
    }
  }

  template<class Package>
  static constexpr bool PACKAGE_EXISTS{!std::is_void_v<decltype(get_serializer_linked_to<Package>())>};

public:
  template<class Package>
  requires PACKAGE_EXISTS<Package>
  using get_serializer = decltype(get_serializer_linked_to<Package>());
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

template<typename Package>
[[nodiscard]] auto serialize(const auto serialize, Package& package) {
  using serialization_mode = serialization_mode<registry::get_serializer<Package>>;
  return serialize.template operator()<serialization_mode>(to_span(package));
}

template<typename Package, byte Byte, std::size_t N>
[[nodiscard]] decltype(auto) deserialize(const auto deserialize, const std::span<Byte, N> src) {
  using std_byte = as_std_byte<Byte>;
  using deserialization_mode = deserialization_mode<registry::get_serializer<Package>>;

  const std::span<std_byte, N> std_byte_src{reinterpret_cast<std_byte*>(src.data()), N};
  return deserialize.template operator()<deserialization_mode>(std_byte_src);
}
} // namespace mbsl

export namespace mbsl {
template<typename Package>
requires (!std::is_const_v<Package>)
[[nodiscard]] auto serialize_mutably(Package& package) {
  return serialize(registry::get_serializer<Package>::EXECUTE_MUTABLY, package);
}

template<typename Package>
[[nodiscard]] auto serialize_new(const Package& package) {
  return serialize(registry::get_serializer<Package>::EXECUTE_NEW, package);
}

template<typename Package>
[[nodiscard]] auto serialize_immutably(Package& package) {
  return serialize(registry::get_serializer<Package>::EXECUTE_IMMUTABLY, std::as_const(package));
}

template<typename Package, byte Byte, std::size_t N>
requires (!std::is_const_v<Byte>)
[[nodiscard]] decltype(auto) deserialize_mutably(const std::span<Byte, N> src) {
  return deserialize<Package>(registry::get_serializer<Package>::EXECUTE_MUTABLY, src);
}

template<typename Package, byte Byte, std::size_t N>
[[nodiscard]] auto deserialize_new(const std::span<Byte, N> src) {
  return deserialize<Package, std::add_const_t<Byte>, N>(registry::get_serializer<Package>::EXECUTE_NEW, src);
}

template<typename Package, byte Byte, std::size_t N>
[[nodiscard]] decltype(auto) deserialize_immutably(const std::span<Byte, N> src) {
  return deserialize<Package, std::add_const_t<Byte>, N>(registry::get_serializer<Package>::EXECUTE_IMMUTABLY, src);
}
} // namespace mbsl
)"};
} // namespace library_template
