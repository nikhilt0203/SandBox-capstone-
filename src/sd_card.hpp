#ifndef SANDBOX_SD_CARD_HPP_
#define SANDBOX_SD_CARD_HPP_

#include "SD.h"
#include <string_view>
#include <filesystem>
#include "core/fixed_string.hpp"
#include "core/fixed_vector.hpp"


namespace sndbx::sdcard
{
  void init() 
  {
    if (!SD.begin(BUILTIN_SDCARD)) 
    {
      Serial.println("SD card failed.");
      while (true) {}
    }
  }

  bool writeLine(std::string_view line, std::filesystem::path path)
  {
    auto filePath = path.c_str();

    SD.remove(filePath);
    File fileOut = SD.open(filePath, FILE_WRITE);

    if (!fileOut) 
    {
      Serial.print("Failed to open file: ");
      Serial.println(filePath);
      return false;
    }

    if (line.empty()) 
    {
      fileOut.close();
      return false;
    }

    fileOut.println(line.data());

    fileOut.flush();
    fileOut.close();
    return true;
  }

  [[nodiscard]] const auto fileContents(std::filesystem::path path) -> sndbx::vector_128U<sndbx::string50_t>
  {
    auto filePath = path.c_str();
    sndbx::vector_128U<sndbx::string50_t> contents;

    if (!SD.exists(filePath))
    {
      Serial.print("File does not exist: ");
      Serial.println(filePath);
      return contents;
    }

    File fileIn = SD.open(filePath, FILE_READ);
    if (!fileIn) 
    {
      Serial.print("Failed to open file: ");
      Serial.println(filePath);
      return contents;
    }

    while (fileIn.available())
    {
      auto line = fileIn.readStringUntil('\n');
      line.replace("\r", "");
      line.trim();

      if (line.length() == 0) { continue; }
      contents.emplace_back(line.c_str());
    }

    fileIn.close();
    return contents;
  }

  [[nodiscard]] const auto directoryContents(std::filesystem::path path) -> sndbx::vector_128U<sndbx::string16_t>
  {
    auto directoryPath = path.c_str();
    sndbx::vector_128U<sndbx::string16_t> contents;

    if (!SD.exists(directoryPath))
    {
      Serial.print("File does not exist: ");
      Serial.println(directoryPath);
      return contents;
    }

    File directory = SD.open(directoryPath, FILE_READ);
    File file = directory.openNextFile();

    while (file) 
    {
      contents.emplace_back(file.name());
      file = directory.openNextFile();
    }

    return contents;
  }
}

#endif