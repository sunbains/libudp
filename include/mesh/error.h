#pragma once

#include <stdexcept>
#include <string>

namespace mesh {

struct Node_error : public std::runtime_error {
  explicit Node_error(const std::string& msg) : std::runtime_error(msg) {}
};

struct Timeout_error : public Node_error {
  explicit Timeout_error(const std::string& msg) : Node_error(msg) {}
};

struct Operation_error : public Node_error {
  explicit Operation_error(const std::string& msg) : Node_error(msg) {}
};

} // namespace mesh

