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
import package_tracker;

namespace parser {
namespace {
class state {
public:
  std::string_view group_name_{"group"};
  std::string_view package_endianness_{"little"};

  explicit state(const std::string_view config) : m_config{config} {}

  std::string_view get_next_token() {
    static constexpr std::string_view WHITESPACE{" \n\r\t"};

    do {
      m_config.remove_prefix(std::min(m_config.find_first_not_of(WHITESPACE), m_config.size()));
    } while (ignore_comment("/*", "*/") || ignore_comment("//", "\n"));

    std::size_t token_size{};

    do {
      token_size = m_config.find_first_not_of(WHITESPACE, token_size);
      token_size = m_config.find_first_of(WHITESPACE, token_size);
      m_token = m_config.substr(0, token_size);
    } while (m_token.ends_with(',') && m_token.size() != m_config.size());

    m_config.remove_prefix(m_token.size());
    return m_token;
  }

  [[nodiscard]] std::string_view get_current_token() const { return m_token; }

private:
  std::string_view m_config;
  std::string_view m_token;

  bool ignore_comment(const std::string_view start_delimiter, const std::string_view end_delimiter) {
    if (!m_config.starts_with(start_delimiter)) {
      return false;
    }

    const std::size_t end_pos{m_config.find(end_delimiter)};
    m_config.remove_prefix(end_pos == std::string_view::npos ? m_config.size() : end_pos + end_delimiter.size());
    return true;
  }
};

using token_parser = bool (*)(state&);

template<std::string_view state::* MEMBER>
std::pair<std::string_view, token_parser> create_assignment_parser(const std::string_view name) {
  return {name, [](state& state) {
            state.*MEMBER = state.get_next_token();
            return true;
          }};
}

bool parse_package_definition(state& state) {
  const std::string_view name{state.get_next_token()};

  if (static constexpr std::string_view START{"{"}; state.get_next_token() != START) {
    std::println(std::cerr, "Error: a package definition must begin with a \"{}\" surrounded by whitespace", START);
    return false;
  }

  static constexpr std::size_t INITIAL_MEMBER_CAPACITY{8};
  static constexpr std::string_view END{"}"};
  std::vector<package_tracker::member> members;

  members.reserve(INITIAL_MEMBER_CAPACITY);

  while (state.get_next_token() != END) {
    if (state.get_current_token().empty()) {
      std::println(std::cerr, "Error: a package definition must end with a \"{}\" surrounded by whitespace", END);
      return false;
    }

    members.emplace_back(state.get_current_token(), state.get_next_token());
  }

  const bool unique{package_tracker::add(state.group_name_, name, state.package_endianness_, std::move(members))};

  if (!unique) {
    std::println(std::cerr, "Error: all packages within a group must have a unique name");
  }

  return unique;
}

bool parse_unrecognized_token(state& state) {
  std::println(std::cerr, "Error: unrecognized token \"{}\"", state.get_current_token());
  return false;
}

token_parser get_token_parser(const std::string_view token) {
  static const std::unordered_map TOKEN_PARSERS{create_assignment_parser<&state::group_name_>("group"),
                                                create_assignment_parser<&state::package_endianness_>("endianness"),
                                                {"package", parse_package_definition}};

  if (const auto iterator{TOKEN_PARSERS.find(token)}; iterator != TOKEN_PARSERS.end()) {
    return iterator->second;
  }

  return parse_unrecognized_token;
}
} // namespace

bool parse(const std::string_view config) {
  state state{config};

  while (!state.get_next_token().empty()) {
    if (const token_parser& parse{get_token_parser(state.get_current_token())}; !parse(state)) {
      return false;
    }
  }

  return true;
}
} // namespace parser
