#ifndef SANDBOX_SD_CARD_HPP_
#define SANDBOX_SD_CARD_HPP_

#include "logging.hpp"
#include <SD.h>
#include <nst/inplace_string.hpp>
#include <nst/inplace_vector.hpp>
#include <string_view>
#include <vector>

namespace sndbx::sdcard {

inline constexpr std::size_t maxLineLength = 75;
inline constexpr std::size_t maxFileLines = 128;
inline void init() {
  if (!SD.begin(BUILTIN_SDCARD)) {
    LOG("SD card failed.");
    while (true) {
    }
  }
}

inline bool remove(std::string_view path) {
  const auto filePath = path.data();
  if (!SD.exists(filePath)) {
    Serial.print("File does not exist: ");
    LOG(filePath);
    return false;
  }
  return SD.remove(filePath);
}

inline bool writeLine(std::string_view line, std::string_view path = "/") {
  const auto filePath = path.data();
  File fileOut = SD.open(filePath, FILE_WRITE);

  if (!fileOut) {
    Serial.print("Failed to open file: ");
    LOG(filePath);
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

template <std::size_t MaxLineLength, std::size_t MaxLines>
using FileLines =
    nst::inplace_vector<nst::inplace_string<MaxLineLength>, MaxLines>;

[[nodiscard]] inline const FileLines<maxLineLength, maxFileLines>
fileContents(std::string_view path) {
  const auto filePath = path.data();
  FileLines<maxLineLength, maxFileLines> contents;

  if (!SD.exists(filePath)) {
    Serial.print("File does not exist: ");
    LOG(filePath);
    return contents;
  }

  File fileIn = SD.open(filePath, FILE_READ);
  if (!fileIn) {
    Serial.print("Failed to open file: ");
    LOG(filePath);
    return contents;
  }

  while (fileIn.available()) {
    auto line = fileIn.readStringUntil('\n');
    line.replace("\r", "");
    line.trim();

    if (line.length() == 0) {
      continue;
    }
    contents.emplace_back(line.c_str());
  }

  fileIn.close();
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