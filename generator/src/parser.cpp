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

module parser;
import writer;

namespace parser {
namespace {
class state;
using parse_declaration = std::function<bool(state&)>;
using declaration_parsers = std::unordered_map<std::string_view, parse_declaration>;

struct declaration_type {
  declaration_parsers parsers_{};
  std::string_view eof_error_message_{};
};

class state {
public:
  explicit state(const std::string_view config) : m_config{config} {}

  std::string_view endianness_{};

  std::string_view get_next_token() {
    constexpr std::string_view WHITESPACE_OR_COMMENT{"/ \r\n\t"};
    constexpr std::string_view WHITESPACE{WHITESPACE_OR_COMMENT.substr(1)};

    m_config.remove_prefix(std::min(m_config.find_first_not_of(WHITESPACE), m_config.size()));

    if (constexpr std::string_view MULTILINE_COMMENT_START{"/*"}; m_config.starts_with(MULTILINE_COMMENT_START)) {
      constexpr std::string_view MULTILINE_COMMENT_END{"*/"};
      const auto end_pos{m_config.find(MULTILINE_COMMENT_END)};

      if (end_pos == std::string_view::npos) {
        m_eof_error_message = "all comments must be terminated by a \"*/\"";
        m_token = {};
        return {};
      }

      m_config.remove_prefix(end_pos + MULTILINE_COMMENT_END.size());
      return get_next_token();
    }

    const std::string_view next_token{m_config.begin(), std::min(m_config.find_first_of(WHITESPACE_OR_COMMENT), m_config.size())};
    m_config.remove_prefix(next_token.size());
    m_token = next_token;
    return next_token;
  }

  void target_declaration_type(const declaration_type& declaration_type) {
    m_declaration_parsers = &declaration_type.parsers_;
    m_eof_error_message = declaration_type.eof_error_message_;
  }

  [[nodiscard]] bool has_current_token() const { return !m_token.empty(); }
  [[nodiscard]] bool has_eof_error_message() const { return !m_eof_error_message.empty(); }
  [[nodiscard]] std::string_view get_current_token() const { return m_token; }
  [[nodiscard]] const declaration_parsers& get_declaration_parsers() const { return *m_declaration_parsers; }
  [[nodiscard]] std::string_view get_eof_error_message() const { return m_eof_error_message; }

private:
  std::string_view m_config{};
  std::string_view m_token{};
  const declaration_parsers* m_declaration_parsers{};
  std::string_view m_eof_error_message{};
};

template<typename Key, typename Value>
std::optional<std::reference_wrapper<const Value>> find(const std::unordered_map<Key, Value>& map, state& state) {
  if (const auto iterator{map.find(state.get_next_token())}; iterator != map.end()) {
    return iterator->second;
  }

  return std::nullopt;
}

bool parse_struct_declaration(state& state) {
  const std::string struct_name{state.get_next_token()};

  if (state.get_next_token() != "{") {
    std::println("Error: a struct definition must begin with \"{{\"");
    return false;
  }

  writer::struct_block struct_block{struct_name, state.endianness_};

  while (state.get_next_token() != "}") {
    if (!state.has_current_token()) {
      std::println("Error: all structs must be terminated by a \"}}\"");
      return false;
    }

    struct_block.add_member(state.get_current_token(), state.get_next_token());
  }
  return true;
}

bool parse_endianness_declaration(state& state) {
  if (state.get_next_token() != "=") {
    std::println("Error: expected the endianness to be defined using the \"=\" operator");
    return false;
  }

  static const std::unordered_map<std::string_view, std::string_view> ENDIANNESS_OPTIONS{
    {"little", "std::endian::little"}, {"big", "std::endian::big"}, {"native", "std::endian::native"}};

  if (const auto endianness{find(ENDIANNESS_OPTIONS, state)}) {
    state.endianness_ = *endianness;

    static const declaration_type TYPE{.parsers_{{"struct", parse_struct_declaration}}};
    state.target_declaration_type(TYPE);
    return true;
  }

  std::println("Error: invalid endianness \"{}\"", state.get_current_token());
  return false;
}
} // namespace

bool parse(const std::string_view config_path, const std::string_view config) {
  state state{config};
  bool success{true};

  {
    static const declaration_type ENDIANNESS{.parsers_{{"endianness", parse_endianness_declaration}},
                                             .eof_error_message_{"the required first declaration \"endianness = <little|big|native>\" is missing"}};
    state.target_declaration_type(ENDIANNESS);
  }

  while (success) {
    if (const auto parse_declaration{find(state.get_declaration_parsers(), state)}) {
      success = (*parse_declaration)(state);
      continue;
    }

    if (state.has_current_token() || state.has_eof_error_message()) {
      std::println("Error: {}",
                   state.has_current_token() ? std::format("unexpected token \"{}\"", state.get_current_token()) : state.get_eof_error_message());
      success = false;
    }

    break;
  }

  std::println("{} \"{}\"{}", success ? "Generated source code from" : "Error:", config_path, success ? "" : " is malformed");
  return success;
}
} // namespace parser
