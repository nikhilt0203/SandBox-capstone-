#ifndef SANDBOX_SERIALIZATION_HPP_
#define SANDBOX_SERIALIZATION_HPP_

#include <string>
#include <string_view>
#include <memory>
#include "grid.hpp"
#include "modules/dep/module_interfaces.hpp"
#include <vector>
#include <charconv>

namespace sndbx::serialization
{
  [[nodiscard]] inline sndbx::vector_32U<sndbx::string16_t> split(std::string_view s, std::string_view delimiter) 
  {
    std::size_t startIndex = 0;
    std::size_t endIndex = 0;

    const auto delimiterLen = delimiter.length();
    sndbx::vector_32U<sndbx::string16_t> tokens;

    while ((endIndex = s.find(delimiter, startIndex)) != std::string::npos) 
    {
      tokens.emplace_back(s.substr(startIndex, endIndex - startIndex));
      startIndex = endIndex + delimiterLen;
    }

    return tokens;
  }

  [[nodiscard]] inline std::string formatModule(std::uint32_t id, sndbx::vector_4U<float> normalizedParameterVals)
  {
    std::string formatted{std::to_string(id)};
    formatted.reserve(50);
    formatted += "(";

    std::size_t i{};
    for (const auto& value : normalizedParameterVals) 
    { 
      formatted += std::to_string(value); 
      if (i < normalizedParameterVals.size() - 1) { formatted += ", "; }
      i++;
    }

    formatted += ")";
    return formatted;
  }

  [[nodiscard]] inline std::string serializeModule(Serializable* module, 
                                                   sndbx::grid::Position pos,   
                                                   std::size_t typeIndex, 
                                                   std::uint32_t id)
  {
    std::string formatted = module->toString(); //ID(w, x, y, z)
    formatted += "<";
    formatted += std::to_string(typeIndex); //ID(w, x, y, z)<Type#>
    formatted += ">";
    formatted += "[";
    formatted += std::to_string(pos.index()); //ID(w, x, y, z)<Type#>[index]
    formatted += "]";
    return formatted;
  }

  struct ModuleFields
  {
    std::uint32_t id;
    sndbx::grid::Position pos;
    std::size_t typeIndex;
    sndbx::vector_4U<float> values;
  };

  [[nodiscard]] inline ModuleFields getFields(const char* formatted)
  { 
    ModuleFields result{};
    const char* str = formatted;

    // ID
    while (*str && (*str < '0' || *str > '9')) ++str;
    parseNumber(str, result.id);

    // (
    while (*str && *str != '(') ++str;
    if (*str != '(') { return result; }
    ++str;

    // values inside parenthesis
    for (std::size_t i = 0; i < 4 && *str && *str != ')'; ++i)
    {
      while (*str == ' ' || *str == ',') { ++str; }
      if (*str == ')') { break; }
      parseFloat(str, result.values[i]);
    }

    // )
    while (*str && *str != ')') ++str;
    if (*str != ')') { return result; }
    ++str;

    // <>
    while (*str && *str != '<') ++str;
    if (*str != '<') { return result; }
    ++str;
    parseNumber(str, result.typeIndex);
    while (*str && *str != '>') ++str;
    if (*str != '>') { return result; }
    ++str;

    // []
    while (*str && *str != '[') ++str;
    if (*str != '[') { return result; }
    ++str;

    std::size_t posIndex = 0;
    parseNumber(str, posIndex);
    if (*str != ']') { return result; }
    result.pos = sndbx::grid::toPosition(posIndex);

    return result;
  }

  struct PatchFields
  {
    std::uint32_t srcID;
    std::uint8_t srcPort; 
    std::uint32_t destID;
    std::uint8_t destPort;
  };

  [[nodiscard]] inline std::string serializePatch(std::uint32_t srcID, 
                                                  std::uint8_t srcPort, 
                                                  std::uint32_t destID,
                                                  std::uint8_t destPort)
  {
    std::string formatted = "P";
    formatted += "[";
    formatted += std::to_string(srcID);
    formatted += ",";
    formatted += std::to_string(destID);
    formatted += ",";
    formatted += std::to_string(srcPort);
    formatted += ",";
    formatted += std::to_string(destPort);
    formatted += "]";
    return formatted;
  }
};

#endif