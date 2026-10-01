#ifndef SANDBOX_SD_CARD_HPP_
#define SANDBOX_SD_CARD_HPP_

#include <SD.h>
#include <nst/inplace_string.hpp>
#include <nst/inplace_vector.hpp>
#include <string_view>
#include <vector>

#include "config/config.hpp"

// TODO: redo
namespace sndbx::sdcard {

using FileName = nst::inplace_string<limits::max_filename_len>;
using FileNames = nst::inplace_vector<FileName, limits::max_files_in_directory>;

using FileLine = nst::inplace_string<limits::max_file_line_len>;
using FileLines = nst::inplace_vector<FileLine, limits::max_lines_in_file>;

[[nodiscard]] inline bool init() {
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card failed.");
    return false;
  }
  return true;
}

inline bool remove(std::string_view path) {
  const auto file_path = path.data();
  if (!SD.exists(file_path)) {
    Serial.printf("File does not exist: %s", file_path);
    return false;
  }
  return SD.remove(file_path);
}

inline bool writeLine(std::string_view line, std::string_view path = "/") {
  const auto file_path = path.data();
  File fileOut = SD.open(file_path, FILE_WRITE);

  if (!fileOut) {
    Serial.printf("Failed to open file: %s", file_path);
    return false;
  }

  if (line.empty()) {
    fileOut.close();
    return false;
  }

  fileOut.println(line.data());
  fileOut.flush();
  fileOut.close();
  return true;
}

[[nodiscard]] inline FileLines read_file(std::string_view path) {
  const auto file_path = path.data();
  FileLines contents;

  if (!SD.exists(file_path)) {
    Serial.printf("File does not exist: %s", file_path);
    return contents;
  }

  File file = SD.open(file_path, FILE_READ);
  if (!file) {
    Serial.printf("Failed to open file: %s", file_path);
    return contents;
  }

  std::array<char, limits::max_file_line_len> buffer;
  while (file.available()) {
    const auto length = file.readBytesUntil('\n', buffer.data(), buffer.size());
    if (length > buffer.size() || length == 0) {
      continue;
    }
    buffer[length] = '\0';
    contents.emplace_back(buffer.data());
  }

  file.close();
  return contents;
}

[[nodiscard]] FileNames read_directory(std::string_view path) {
  auto directory_path = path.data();
  FileNames contents;

  if (!SD.exists(directory_path)) {
    Serial.printf("File does not exist: %s", directory_path);
    return contents;
  }

  File directory = SD.open(directory_path, FILE_READ);
  File file = directory.openNextFile();

  while (file) {
    contents.emplace_back(file.name());
    file = directory.openNextFile();
  }

  return contents;
}

} // namespace sndbx::sdcard

#endif