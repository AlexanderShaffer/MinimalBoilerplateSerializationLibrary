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
class state {
public:
  explicit state(const std::string_view config) : m_config{config} {}

  std::string_view endianness_{"little"};

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

  [[nodiscard]] bool has_current_token() const { return !m_token.empty(); }
  [[nodiscard]] bool has_eof_error_message() const { return !m_eof_error_message.empty(); }
  [[nodiscard]] std::string_view get_current_token() const { return m_token; }
  [[nodiscard]] std::string_view get_eof_error_message() const { return m_eof_error_message; }

private:
  std::string_view m_config{};
  std::string_view m_token{};
  std::string_view m_eof_error_message{};
};

bool parse_struct(state& state) {
  writer::struct_block struct_block{state.get_next_token(), state.endianness_};

  if (state.get_next_token() != "{") {
    std::println("Error: a struct definition must begin with \"{{\"");
    return false;
  }

  while (state.get_next_token() != "}") {
    if (!state.has_current_token()) {
      std::println("Error: all structs must be terminated by a \"}}\"");
      return false;
    }

    struct_block.add_member(state.get_current_token(), state.get_next_token());
  }

  return true;
}

bool parse_endianness(state& state) {
  if (state.get_next_token() != "=") {
    std::println("Error: expected the endianness to be defined using the \"=\" operator");
    return false;
  }

  state.endianness_ = state.get_next_token();
  return true;
}
} // namespace

bool parse_config(const std::string_view path, const std::string_view data) {
  state state{data};
  bool success{true};

  while (success) {
    static const std::unordered_map<std::string_view, std::function<bool(parser::state&)>> PARSERS{{"endianness", parse_endianness},
                                                                                                   {"struct", parse_struct}};

    if (const auto iterator{PARSERS.find(state.get_next_token())}; iterator != PARSERS.end()) {
      const auto& [_, parse]{*iterator};
      success = parse(state);
      continue;
    }

    if (state.has_current_token() || state.has_eof_error_message()) {
      std::println("Error: {}",
                   state.has_current_token() ? std::format("unexpected token \"{}\"", state.get_current_token()) : state.get_eof_error_message());
      success = false;
    }

    break;
  }

  std::println("{} \"{}\"{}", success ? "Generated source code from" : "Error:", path, success ? "" : " is malformed");
  return success;
}
} // namespace parser
