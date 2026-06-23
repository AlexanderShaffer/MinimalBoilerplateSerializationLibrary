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
  const auto args{std::span{argv, static_cast<std::size_t>(argc)} | std::views::drop(1)};
  std::size_t max_config_size{};

  for (const auto* const arg : args) {
    std::filesystem::path config_path{arg};

    if (!std::filesystem::exists(config_path) || std::filesystem::is_directory(config_path)) {
      std::println("Error: \"{}\" is not a file that exists", arg);
      return 1;
    }

    max_config_size = std::max(max_config_size, std::filesystem::file_size(config_path));
  }

  for (std::string config(max_config_size, '\0'); const auto* const config_path : args) {
    std::ifstream config_file{config_path, std::ios::binary};

    if (!config_file) {
      std::println("Error: failed to open \"{}\"", config_path);
      return 1;
    }

    config_file.read(config.data(), config.size());
    parser::parse(config_path, config);
  }

  writer::write_library();
  return 0;
}
