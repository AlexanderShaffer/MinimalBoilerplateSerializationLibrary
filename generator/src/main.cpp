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

import std;
import parser;
import writer;

int main(const int argc, const char* const* const argv) {
  const auto config_paths{std::span{argv, static_cast<std::size_t>(argc)} | std::views::drop(1)};
  std::size_t max_config_size{};

  for (const auto* const path : config_paths) {
    std::filesystem::path config_path{path};

    if (!std::filesystem::exists(config_path) || std::filesystem::is_directory(config_path)) {
      std::println("Aborting because \"{}\" is not a file that exists", config_path.native());
      return 1;
    }

    max_config_size = std::max(max_config_size, std::filesystem::file_size(config_path));
  }

  std::string config(max_config_size, '\0');

  for (const auto* const config_path : config_paths) {
    std::ifstream config_file{config_path, std::ios::binary};

    config_file.read(config.data(), config.size());
    // TODO: Redesign the parser module to parse a std::string
  }

  writer::write_library();
  return 0;
}
