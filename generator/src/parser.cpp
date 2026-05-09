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

namespace {
enum class Declaration : std::uint8_t { ENDIANNESS, TYPE };

class State {
public:
  explicit State(const std::filesystem::path& config_path) : m_input{config_path} {}

  Declaration declaration_{Declaration::ENDIANNESS};
  std::endian endianness_{};

  std::string_view get_next_token() {
    m_input >> m_token;
    return m_input ? m_token : m_token.erase();
  }

  [[nodiscard]] bool has_next_token() const { return !m_token.empty(); }
  [[nodiscard]] std::string_view get_current_token() const { return m_token; }

private:
  std::ifstream m_input{};
  std::string m_token{};
};

using ParseDeclaration = std::function<bool(State&)>;

bool parse_comment(State& state) {
  std::string_view current_token{};

  while ((current_token = state.get_next_token()) != "*/") {
    if (current_token.empty()) {
      std::println("Error: unterminated comment");
      return false;
    }
  }

  return true;
}

ParseDeclaration assign_declaration_parser(const Declaration assigned_declaration, ParseDeclaration&& parse_declaration) {
  return [=, parse_declaration = std::move(parse_declaration)](State& state) {
    if (state.declaration_ == assigned_declaration) {
      return parse_declaration(state);
    }

    std::println("Error: unexpected token \"{}\"", state.get_current_token());
    return false;
  };
}

template<typename Key, typename Value>
std::optional<std::reference_wrapper<const Value>> find(const std::unordered_map<Key, Value>& map, State& state) {
  if (const auto iterator{map.find(state.get_next_token())}; iterator != map.end()) {
    return iterator->second;
  }

  return std::nullopt;
}

bool parse_endianness_declaration(State& state) {
  if (state.get_next_token() != "=") {
    std::println("Error: expected the endianness to be defined using the \"=\" operator");
    return false;
  }

  static const std::unordered_map<std::string_view, std::endian> ENDIANNESS_MAP{
    {"little", std::endian::little}, {"big", std::endian::big}, {"native", std::endian::native}};

  if (const auto endianness{find(ENDIANNESS_MAP, state)}) {
    state.endianness_ = *endianness;
    state.declaration_ = Declaration::TYPE;
    return true;
  }

  std::println("Error: invalid endianness \"{}\"", state.get_current_token());
  return false;
}

bool parse_struct_declaration(const State& state) {
  return true; // TODO: Implement this function
}
} // namespace

namespace parser {
bool parse_config(const std::filesystem::path& config_path) {
  State state{config_path};
  bool success{true};

  while (success) {
    static const std::unordered_map<std::string_view, ParseDeclaration> DECLARATION_PARSERS{
      {"/*", parse_comment},
      {"endianness", assign_declaration_parser(Declaration::ENDIANNESS, parse_endianness_declaration)},
      {"struct", assign_declaration_parser(Declaration::TYPE, parse_struct_declaration)}};

    if (const auto parse_declaration{find(DECLARATION_PARSERS, state)}) {
      success = (*parse_declaration)(state);
      continue;
    }

    if (state.has_next_token()) {
      std::println("Error: unrecognized token \"{}\"", state.get_current_token());
      success = false;
    }

    break;
  }

  if (success) {
    std::println("Generated source code from \"{}\"", config_path.native());
  } else {
    std::println("Error: \"{}\" is malformed", config_path.native());
  }

  return success;
}
} // namespace parser
