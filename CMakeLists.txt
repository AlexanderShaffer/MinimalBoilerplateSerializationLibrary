#[[
This file is part of MinimalBoilerplateSerializationLibrary.
Copyright (C) 2026 Alexander Shaffer <alexander.shaffer.623@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
]]

cmake_minimum_required(VERSION 4.3.2)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD 451f2fe2-a8a2-47c3-bc32-94786d8fc91b)
set(CMAKE_CXX_MODULE_STD ON)
project(MinimalBoilerplateSerializationLibrary)

if (NOT MBSL_CONFIG_DIR)
  message(FATAL_ERROR "The MBSL_CONFIG_DIR variable must be set to a directory containing .mbsl files.")
endif ()

add_executable(Generator generator/src/main.cpp)

file(
  GLOB CONFIG_FILES
  LIST_DIRECTORIES false
  CONFIGURE_DEPENDS "${MBSL_CONFIG_DIR}/*.mbsl"
)

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/mbsl.cpp" "${CMAKE_CURRENT_BINARY_DIR}/mbsl.cppm"
  COMMAND Generator ${CONFIG_FILES}
  DEPENDS ${CONFIG_FILES}
  COMMENT "Generating MinimalBoilerplateSerializationLibrary"
  VERBATIM CODEGEN
)

add_library(MinimalBoilerplateSerializationLibrary STATIC)

target_sources(
  MinimalBoilerplateSerializationLibrary
  PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/mbsl.cpp"
  PUBLIC FILE_SET CXX_MODULES BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}" FILES "${CMAKE_CURRENT_BINARY_DIR}/mbsl.cppm"
)
