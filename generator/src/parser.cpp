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
  explicit state(const std::filesystem::path& config_path) : m_config{config_path} {}

  std::endian endianness_{};

  std::string_view get_next_token() {
    if (read() != "/*") {
      return m_token;
    }

    while (read() != "*/") {
      if (!has_current_token()) {
        m_eof_error_message = "all comments must be terminated by a \"*/\"";
        return {};
      }
    }

    return get_next_token();
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
  std::ifstream m_config{};
  std::string m_token{};
  const declaration_parsers* m_declaration_parsers{};
  std::string_view m_eof_error_message{};

  std::string_view read() { return m_config >> m_token ? m_token : m_token.erase(); }
};

template<typename Key, typename Value>
std::optional<std::reference_wrapper<const Value>> find(const std::unordered_map<Key, Value>& map, state& state) {
  if (const auto iterator{map.find(state.get_next_token())}; iterator != map.end()) {
    return iterator->second;
  }

  return std::nullopt;
}

bool parse_struct_declaration(state& state) {
  const writer::struct_block struct_block{state.get_next_token()};

  if (state.get_next_token() != "{") {
    std::println("Error: a struct definition must begin with \"{{\"");
    return false;
  }

  while (state.get_next_token() != "}") {
    if (!state.has_current_token()) {
      std::println("Error: all structs must be terminated by a \"}}\"");
      return false;
    }

    struct_block.write_type(state.get_current_token());
    struct_block.write_value(state.get_next_token());
  }

  return true;
}

bool parse_endianness_declaration(state& state) {
  if (state.get_next_token() != "=") {
    std::println("Error: expected the endianness to be defined using the \"=\" operator");
    return false;
  }

  static const std::unordered_map<std::string_view, std::endian> ENDIANNESS_OPTIONS{
    {"little", std::endian::little}, {"big", std::endian::big}, {"native", std::endian::native}};

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

namespace parser {
bool parse_config(const std::filesystem::path& config_path) {
  state state{config_path};
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

  std::println("{} \"{}\"{}", success ? "Generated source code from" : "Error:", config_path.native(), success ? "" : " is malformed");
  return success;
}
} // namespace parser
