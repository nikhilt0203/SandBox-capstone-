#ifndef SANDBOX_SD_CARD_HPP_
#define SANDBOX_SD_CARD_HPP_

#include <nst/fixed_string.hpp>
#include <nst/fixed_vector.hpp>
#include <SD.h>
#include <string_view>
#include <vector>
#include <filesystem>

namespace nst::teensy {

inline constexpr std::size_t maxLineLength = 75;
inline constexpr std::size_t maxFileLines = 128;

[[nodiscard]] inline bool init() { return SD.begin(BUILTIN_SDCARD); }

inline bool remove(std::filesystem::path path) {
  const auto file_path = path.c_str();
  if (!SD.exists(file_path)) {
    return false;
  }
  return SD.remove(file_path);
}

inline bool writeLine(std::string_view line, std::filesystem::path path = "/") {
  const auto file_path = path.c_str();
  File file_out = SD.open(file_path, FILE_WRITE);

  if (!file_out) {
    Serial.printf("Failed to open file: %s\n", file_path);
    return false;
  }

  if (line.empty()) {
    file_out.close();
    return false;
  }

  file_out.println(line.data());
  file_out.flush();
  file_out.close();
  return true;
}

template<std::size_t MaxLineLength, std::size_t MaxLines>
using FileLines = nst::fixed_vector<nst::string<MaxLineLength>, MaxLines>;

[[nodiscard]] inline const FileLines<maxLineLength, maxFileLines> fileContents(std::string_view path) {
  const auto file_path = path.data();
  FileLines<maxLineLength, maxFileLines> contents;

  if (!SD.exists(file_path)) {
    Serial.printf("File does not exist: %s\n", file_path);
    return contents;
  }

  File file_in = SD.open(file_path, FILE_READ);
  if (!file_in) {
    Serial.printf("Failed to open file: %s\n", file_path);
    return contents;
  }

  while (file_in.available()) {
    auto line = file_in.readStringUntil('\n');
    line.replace("\r", "");
    line.trim();

    if (line.length() == 0) {
      continue;
    }
    contents.emplace_back(line.c_str());
  }

  file_in.close();
  return contents;
}

// [[nodiscard]] const auto directoryContents(std::filesystem::path path) ->
// nst::vector_128U<nst::string16_t>
// {
//   auto directoryPath = path.c_str();
//   nst::vector_128U<nst::string16_t> contents;

//   if (!SD.exists(directoryPath))
//   {
//     Serial.print("File does not exist: ");
//     LOG(directoryPath);
//     return contents;
//   }

//   File directory = SD.open(directoryPath, FILE_READ);
//   File file = directory.openNextFile();

//   while (file)
//   {
//     contents.emplace_back(file.name());
//     file = directory.openNextFile();
//   }

//   return contents;
// }
} // namespace sndbx::sdcard

#endif