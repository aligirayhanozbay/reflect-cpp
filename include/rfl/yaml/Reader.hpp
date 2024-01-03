#ifndef RFL_YAML_READER_HPP_
#define RFL_YAML_READER_HPP_

#include <yaml-cpp/yaml.h>

#include <array>
#include <exception>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "../Result.hpp"
#include "../always_false.hpp"

namespace rfl {
namespace yaml {

struct Reader {
  struct YAMLInputArray {
    YAMLInputArray(const YAML::Node& _node) : node_(_node) {}
    YAML::Node node_;
  };

  struct YAMLInputObject {
    YAMLInputObject(const YAML::Node& _node) : node_(_node) {}
    YAML::Node node_;
  };

  struct YAMLInputVar {
    YAMLInputVar(const YAML::Node& _node) : node_(_node) {}
    YAML::Node node_;
  };

  using InputArrayType = YAMLInputArray;
  using InputObjectType = YAMLInputObject;
  using InputVarType = YAMLInputVar;

  template <class T, class = void>
  struct has_from_json_obj : std::false_type {};

  template <class T>
  static constexpr bool has_custom_constructor = (requires(InputVarType var) {
    T::from_yaml_obj(var);
  });

  rfl::Result<InputVarType> get_field(
      const std::string& _name, const InputObjectType& _obj) const noexcept {
    auto var = InputVarType(_obj.node_[_name]);
    if (!var.node_) {
      return rfl::Error("Object contains no field named '" + _name + "'.");
    }
    return var;
  }

  bool is_empty(const InputVarType& _var) const noexcept {
    return !_var.node_ && true;
  }

  template <class T>
  rfl::Result<T> to_basic_type(const InputVarType& _var) const noexcept {
    try {
      if constexpr (std::is_same<std::decay_t<T>, std::string>() ||
                    std::is_same<std::decay_t<T>, bool>() ||
                    std::is_floating_point<std::decay_t<T>>() ||
                    std::is_integral<std::decay_t<T>>()) {
        return _var.node_.as<std::decay_t<T>>();
      } else {
        static_assert(rfl::always_false_v<T>, "Unsupported type.");
      }
    } catch (std::exception& e) {
      return rfl::Error(e.what());
    }
  }

  rfl::Result<InputArrayType> to_array(
      const InputVarType& _var) const noexcept {
    if (!_var.node_.IsSequence()) {
      return rfl::Error("Could not cast to sequence!");
    }
    return InputArrayType(_var.node_);
  }

  template <size_t size, class FunctionType>
  std::array<std::optional<InputVarType>, size> to_fields_array(
      const FunctionType& _fct, const InputObjectType& _obj) const noexcept {
    std::array<std::optional<InputVarType>, size> f_arr;
    for (const auto& p : _obj.node_) {
      try {
        const auto k = p.first.as<std::string>();
        const auto ix = _fct(std::string_view(k));
        if (ix != -1) {
          f_arr[ix] = InputVarType(p.second);
        }
      } catch (std::exception& e) {
        continue;
      }
    }
    return f_arr;
  }

  std::vector<std::pair<std::string, InputVarType>> to_map(
      const InputObjectType& _obj) const noexcept {
    std::vector<std::pair<std::string, InputVarType>> m;
    for (const auto& p : _obj.node_) {
      try {
        auto k = p.first.as<std::string>();
        m.emplace_back(std::make_pair(k, p.second));
      } catch (std::exception& e) {
        continue;
      }
    }
    return m;
  }

  rfl::Result<InputObjectType> to_object(
      const InputVarType& _var) const noexcept {
    if (!_var.node_.IsMap()) {
      return rfl::Error("Could not cast to map!");
    }
    return InputObjectType(_var.node_);
  }

  std::vector<InputVarType> to_vec(const InputArrayType& _arr) const noexcept {
    std::vector<InputVarType> vec;
    for (size_t i = 0; i < _arr.node_.size(); ++i) {
      vec.push_back(InputVarType(_arr.node_[i]));
    }
    return vec;
  }

  template <class T>
  rfl::Result<T> use_custom_constructor(
      const InputVarType _var) const noexcept {
    try {
      return T::from_yaml_obj(_var);
    } catch (std::exception& e) {
      return rfl::Error(e.what());
    }
  }
};

}  // namespace yaml
}  // namespace rfl

#endif
