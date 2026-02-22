#ifndef serialization_hpp_
#define serialization_hpp_

#include <string>
#include <string_view>
#include <memory>
#include "grid.hpp"
#include <vector>

class Serializable
{
public:
  virtual ~Serializable() = default;

  [[nodiscard]] virtual std::string toString() const = 0;
  virtual void fromString(std::string_view s) {}
};

namespace sndbx::serialization
{
  [[nodiscard]] std::vector<std::string> split(std::string s, std::string delimiter) 
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

  [[nodiscard]] std::string formatModule(std::string_view name, std::vector<std::string> parameterValues)
  {
    std::string formatted{name};
    formatted.reserve(50);
    formatted += "(";

    const auto numValues = parameterValues.size();
    for (std::size_t i{}; i < numValues; i++)
    {
      formatted += parameterValues[i];
      if (i < numValues - 1) { formatted += ", "; }
    }

    formatted += ")";
    return formatted;
  }

  [[nodiscard]] std::string serialize(Serializable* module, sndbx::grid::Position pos)
  {
    std::string formatted = module->toString();
    formatted += "<";
    formatted += std::to_string(pos.index());
    formatted += ">";
    return formatted;
  }
};

#endif