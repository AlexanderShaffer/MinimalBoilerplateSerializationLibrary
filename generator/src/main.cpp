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
  const std::span args{argv, static_cast<std::size_t>(argc)};
  auto config_paths{args | std::views::drop(1) | std::ranges::to<std::vector<std::filesystem::path>>()};

  std::ranges::sort(config_paths);

  for (const auto& config_path : config_paths) {
    if (!std::filesystem::exists(config_path) || std::filesystem::is_directory(config_path)) {
      std::println("Ignoring \"{}\" because it is not a file that exists", config_path.native());
    } else if (!parser::parse_config(config_path)) {
      return 1;
    }
  }

  writer::write_library();
  return 0;
}
