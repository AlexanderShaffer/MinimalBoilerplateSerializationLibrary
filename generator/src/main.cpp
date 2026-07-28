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
  const std::ranges::drop_view args{std::span{argv, static_cast<std::size_t>(argc)}, 1};
  std::size_t buffer_size{};

  for (const char* const arg : args) {
    std::filesystem::path config_path{arg};

    if (!std::filesystem::exists(config_path) || std::filesystem::is_directory(config_path)) {
      std::println(std::cerr, "Error: \"{}\" is not a file that exists", arg);
      return 1;
    }

    buffer_size += std::filesystem::file_size(config_path);
  }

  const std::unique_ptr buffer{std::make_unique_for_overwrite<char[]>(buffer_size)};

  for (std::span buffer_span{buffer.get(), buffer_size}; const char* const config_path : args) {
    std::ifstream config_file{config_path, std::ios::binary};

    if (!config_file) {
      std::println(std::cerr, "Error: failed to open \"{}\"", config_path);
      return 1;
    }

    config_file.read(buffer_span.data(), buffer_span.size());

    const std::size_t config_size{static_cast<std::size_t>(config_file.gcount())};

    if (!parser::parse(std::string_view{buffer_span.data(), config_size})) {
      std::println(std::cerr, "Error: \"{}\" is malformed", config_path);
      return 1;
    }

    buffer_span = buffer_span.subspan(config_size);
    std::println("Successfully parsed \"{}\"", config_path);
  }

  writer::write_library();
  return 0;
}
