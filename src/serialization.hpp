#ifndef SANDBOX_SERIALIZATION_HPP_
#define SANDBOX_SERIALIZATION_HPP_

#include <string>
#include <string_view>
#include <memory>
#include "grid.hpp"
#include "modules/dep/module_interfaces.hpp"
#include <vector>

namespace sndbx::serialization
{
  [[nodiscard]] inline std::vector<std::string> split(std::string s, std::string delimiter) 
  {
    std::size_t startIndex = 0;
    std::size_t endIndex = 0;
    const std::size_t delimiterLen= delimiter.length();
    std::string token;
    std::vector<std::string> tokens;

    while ((endIndex = s.find(delimiter, startIndex)) != std::string::npos) 
    {
      tokens.push_back(s.substr(startIndex, endIndex - startIndex));
      startIndex = endIndex + delimiterLen;
      tokens.push_back(token);
    }

    tokens.push_back(s.substr(startIndex));
    return tokens;
  }

  [[nodiscard]] inline std::string formatModule(std::uint32_t id, std::initializer_list<std::string> parameterValues)
  {
    std::string formatted{std::to_string(id)};
    formatted.reserve(50);
    formatted += "(";

    std::size_t i{};
    for (const auto& value : parameterValues) 
    { 
      formatted += value; 
      if (i < parameterValues.size() - 1) { formatted += ", "; }
      i++;
    }

    formatted += ")";
    return formatted;
  }

  [[nodiscard]] inline std::string serialize(Serializable* module, std::size_t typeIndex, sndbx::grid::Position pos)
  {
    std::string formatted = module->toString();
    formatted += "<";
    formatted += std::to_string(typeIndex);
    formatted += ">";
    formatted += "[";
    formatted += std::to_string(pos.index());
    formatted += "]";
    return formatted;
  }
};

#endif