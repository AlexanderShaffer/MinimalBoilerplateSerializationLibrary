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
  std::string_view conduit_{"conduit"};
  std::string_view version_{"0"};
  std::string_view endianness_{"little"};

  explicit state(const std::string_view config) : m_config{config} {}

  std::string_view get_next_token() {
    constexpr std::string_view WHITESPACE{" \n\r\t"};

    do {
      m_config.remove_prefix(std::min(m_config.find_first_not_of(WHITESPACE), m_config.size()));
    } while (ignore_comment("/*", "*/") || ignore_comment("//", "\n"));

    m_token = {m_config.substr(0, std::min(m_config.find_first_of(WHITESPACE), m_config.size()))};
    m_config.remove_prefix(m_token.size());
    return m_token;
  }

  [[nodiscard]] std::string_view get_current_token() const { return m_token; }

private:
  std::string_view m_config{};
  std::string_view m_token{};

  bool ignore_comment(const std::string_view start_delimiter, const std::string_view end_delimiter) {
    if (!m_config.starts_with(start_delimiter)) {
      return false;
    }

    const auto end_pos{m_config.find(end_delimiter)};
    m_config.remove_prefix(end_pos == std::string_view::npos ? m_config.size() : end_pos + end_delimiter.size());
    return true;
  }
};

using parser = std::function<bool(state&)>;

std::pair<std::string_view, parser> create_assignment_parser(const std::string_view name, std::string_view state::* const member) {
  return {name, [=](state& state) {
            state.*member = state.get_next_token();
            return true;
          }};
}

bool parse_struct(state& state) {
  writer::struct_block struct_block{state.get_next_token(), state.endianness_};

  if (constexpr std::string_view START{"{"}; state.get_next_token() != START) {
    std::println(std::cerr, "Error: a struct definition must begin with a \"{}\" surrounded by whitespace", START);
    return false;
  }

  constexpr std::string_view END{"}"};

  while (state.get_next_token() != END) {
    if (state.get_current_token().empty()) {
      std::println(std::cerr, "Error: all structs must end with a \"{}\" surrounded by whitespace", END);
      return false;
    }

    struct_block.add_member(state.get_current_token(), state.get_next_token());
  }

  return true;
}

bool parse_unrecognized_token(const state& state) {
  std::println(std::cerr, "Error: unrecognized token \"{}\"", state.get_current_token());
  return false;
}

const parser& get_parser(const std::string_view token) {
  static const std::unordered_map PARSERS{create_assignment_parser("conduit", &state::conduit_),
                                          create_assignment_parser("version", &state::version_),
                                          create_assignment_parser("endianness", &state::endianness_),
                                          {"struct", parse_struct}};

  if (const auto iterator{PARSERS.find(token)}; iterator != PARSERS.end()) {
    const auto& [_, parse]{*iterator};
    return parse;
  }

  static const parser PARSE_UNRECOGNIZED_TOKEN{parse_unrecognized_token};
  return PARSE_UNRECOGNIZED_TOKEN;
}
} // namespace

bool parse(const std::string_view config) {
  state state{config};

  while (!state.get_next_token().empty()) {
    if (const auto& parse{get_parser(state.get_current_token())}; !parse(state)) {
      return false;
    }
  }

  return true;
}
} // namespace parser
