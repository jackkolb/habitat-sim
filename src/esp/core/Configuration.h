// Copyright (c) Meta Platforms, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#ifndef ESP_CORE_CONFIGURATION_H_
#define ESP_CORE_CONFIGURATION_H_

#include <Corrade/Utility/Configuration.h>
#include <Corrade/Utility/Debug.h>
#include <Corrade/Utility/FormatStl.h>
#include <Magnum/Magnum.h>
#include <string>
#include <unordered_map>

#include "esp/core/Check.h"
#include "esp/core/Esp.h"
#include "esp/io/Json.h"

namespace Cr = Corrade;
namespace Mn = Magnum;

namespace esp {
namespace core {

namespace config {

constexpr int CONFIG_VAL_SIZE = 8;  // 2 * 4

/**
 * @brief This enum lists every type of value that can be currently stored
 * directly in an @ref esp::core::config::Configuration.  All supported types
 * should have entries in this enum class. All pointer-backed types (i.e. data
 * larger than ConfigValue::_data array sizee) should have their enums placed
 * after @p _storedAsAPointer tag. All non-trivial types should have their enums
 * placed below @p _nonTrivialTypes tag. Any small, trivially copyable types
 * should be placed before @p _storedAsAPointer tag.
 */
enum class ConfigValType {
  /**
   * @brief Unknown type
   */
  Unknown = ID_UNDEFINED,
  /**
   * @brief boolean type
   */
  Boolean,
  /**
   * @brief integer type
   */
  Integer,
  /**
   * @brief Magnum::Rad angle type
   */
  MagnumRad,
  /**
   * @brief double type
   */
  Double,

  /**
   * @brief Magnum::Vector2 type
   */
  MagnumVec2,

  // Types stored as a pointer.  All non-trivial types must also be placed after
  // this marker.
  _storedAsAPointer,
  /**
   * @brief Magnum::Vector3 type. All types of size greater than _data size must
   * be placed after this marker, either before or after String, depending on if
   * they are trivially copyable or not.
   */
  MagnumVec3 = _storedAsAPointer,
  /**
   * @brief Magnum::Vector4 type
   */
  MagnumVec4,
  /**
   * @brief Magnum::Quaternion type
   */
  MagnumQuat,
  /**
   * @brief Magnum::Matrix3 (3x3) type
   */
  MagnumMat3,
  /**
   * @brief Magnum::Matrix4 (4x4) type
   */
  MagnumMat4,

  // These types are not trivially copyable. All non-trivial types are by
  // default stored as pointers in ConfigValue::_data array.
  _nonTrivialTypes,
  /**
   * @brief All enum values of nontrivial types must be added after @p String .
   */
  String = _nonTrivialTypes,
};  // enum class ConfigValType

/**
 * @brief Retrieve a string description of the passed @ref ConfigValType enum
 * value.
 */
std::string getNameForStoredType(const ConfigValType& value);

/**
 * @brief Quick check to see if type is stored as a pointer in the data or not
 * (i.e. the type is trivially copyable or not)
 */
constexpr bool isConfigValTypePointerBased(ConfigValType type) {
  return static_cast<int>(type) >=
         static_cast<int>(ConfigValType::_storedAsAPointer);
}
/**
 * @brief Quick check to see if type is trivially copyable or not.
 */
constexpr bool isConfigValTypeNonTrivial(ConfigValType type) {
  return static_cast<int>(type) >=
         static_cast<int>(ConfigValType::_nonTrivialTypes);
}
/**
 * @brief Function template to return type enum for specified type. All
 * supported types should have a specialization of this function handling their
 * type to @ref ConfigValType enum tags mapping.
 */
template <typename T>
constexpr ConfigValType configValTypeFor() {
  static_assert(sizeof(T) == 0, "unsupported type ");
  return {};
}

/**
 * @brief Returns @ref ConfigValType::Boolean type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<bool>() {
  return ConfigValType::Boolean;
}
/**
 * @brief Returns @ref ConfigValType::Integer type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<int>() {
  return ConfigValType::Integer;
}
/**
 * @brief Returns @ref ConfigValType::Double type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<double>() {
  return ConfigValType::Double;
}
/**
 * @brief Returns @ref ConfigValType::String type enum for specified type
 */

template <>
constexpr ConfigValType configValTypeFor<std::string>() {
  return ConfigValType::String;
}
/**
 * @brief Returns @ref ConfigValType::MagnumVec2 type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Vector2>() {
  return ConfigValType::MagnumVec2;
}
/**
 * @brief Returns @ref ConfigValType::MagnumVec3 type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Vector3>() {
  return ConfigValType::MagnumVec3;
}

/**
 * @brief Returns @ref ConfigValType::MagnumVec3 type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Color3>() {
  return ConfigValType::MagnumVec3;
}

/**
 * @brief Returns @ref ConfigValType::MagnumVec4 type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Vector4>() {
  return ConfigValType::MagnumVec4;
}

/**
 * @brief Returns @ref ConfigValType::MagnumVec4 type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Color4>() {
  return ConfigValType::MagnumVec4;
}

/**
 * @brief Returns @ref ConfigValType::MagnumMat3 type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Matrix3>() {
  return ConfigValType::MagnumMat3;
}

/**
 * @brief Returns @ref ConfigValType::MagnumMat4 type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Matrix4>() {
  return ConfigValType::MagnumMat4;
}

/**
 * @brief Returns @ref ConfigValType::MagnumQuat type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Quaternion>() {
  return ConfigValType::MagnumQuat;
}

/**
 * @brief Returns @ref ConfigValType::MagnumRad type enum for specified type
 */
template <>
constexpr ConfigValType configValTypeFor<Mn::Rad>() {
  return ConfigValType::MagnumRad;
}

/**
 * @brief Stream operator to support display of @ref ConfigValType enum tags
 */
MAGNUM_EXPORT Mn::Debug& operator<<(Mn::Debug& debug,
                                    const ConfigValType& value);

/**
 * @brief This class uses an anonymous tagged union to store values of different
 * types, as well as providing access to the values in a type safe manner.
 */
class ConfigValue {
 private:
  /**
   * @brief This is the type of the data represented in this ConfigValue.
   */
  ConfigValType _type{ConfigValType::Unknown};

  /**
   * @brief The data this ConfigValue holds.
   * Aligns to individual 8-byte bounds. The _type is 4 bytes, 4 bytes of
   * padding (on 64 bit machines) and 36 bytes for data.
   */
  alignas(8) char _data[CONFIG_VAL_SIZE] = {0};

  /**
   * @brief Copy the passed @p val into this ConfigValue.  If this @ref
   * ConfigValue's type is not trivial, this will call the appropriate copy
   * handler for the type.
   * @param val source val to copy into this config
   */
  void copyValueFrom(const ConfigValue& val);

  /**
   * @brief Move the passed @p val into this ConfigVal. If this @ref
   * ConfigValue's type is not trivial, this will call the appropriate move
   * handler for the type.
   * @param val source val to copy into this config
   */
  void moveValueFrom(ConfigValue&& val);

  /**
   * @brief Delete the current value. If this @ref
   * ConfigValue's type is not trivial, this will call the appropriate
   * destructor handler for the type.
   */
  void deleteCurrentValue();

 public:
  /**
   * @brief Constructor
   */
  ConfigValue() = default;
  /**
   * @brief Copy Constructor
   */
  ConfigValue(const ConfigValue& otr);
  /**
   * @brief Move Constructor
   */
  ConfigValue(ConfigValue&& otr) noexcept;
  ~ConfigValue();

  /**
   * @brief Copy assignment
   */
  ConfigValue& operator=(const ConfigValue& otr);

  /**
   * @brief Move assignment
   */
  ConfigValue& operator=(ConfigValue&& otr) noexcept;

  /**
   * @brief Whether this @ref ConfigValue is valid.
   * @return Whether or not the specified type of this @ref ConfigValue is known.
   */
  bool isValid() const { return _type != ConfigValType::Unknown; }

  /**
   * @brief Write this ConfigValue to an appropriately configured json object.
   */
  io::JsonGenericValue writeToJsonObject(io::JsonAllocator& allocator) const;

  /**
   * @brief Set the passed @p value as the data for this @ref ConfigValue, while also setting the appropriate type.
   * @tparam The type of the @p value being set. Must be a handled type as specified by @ref ConfigValType.
   * @param value The value to store in this @ref ConfigValue
   */
  template <typename T>
  void set(const T& value) {
    deleteCurrentValue();
    // This will blow up at compile time if given type is not supported
    _type = configValTypeFor<T>();
    // These asserts are checking the integrity of the support for T's type, and
    // will fire if conditions are not met.

    // This fails if we added a new type into @ref ConfigValType enum improperly
    // (trivial type added after entry ConfigValType::_nonTrivialTypes, or
    // vice-versa)
    static_assert(isConfigValTypeNonTrivial(configValTypeFor<T>()) !=
                      std::is_trivially_copyable<T>::value,
                  "Something's incorrect about enum placement for added type "
                  "(type is not trivially copyable, or vice-versa)");

    // This verifies that any values that are too large to be stored directly
    // (or are already specified as non-trivial) are pointer based, while those
    // that are trivial and small are stored directly,
    //
    static_assert(
        ((sizeof(T) >= sizeof(_data)) ||
         isConfigValTypeNonTrivial(configValTypeFor<T>()) ==
             (isConfigValTypePointerBased(configValTypeFor<T>()))),
        "ConfigValue's internal storage is too small for added type!");
    // This fails if a new type was added whose alignment does not match
    // internal storage alignment
    static_assert(
        alignof(T) <= alignof(ConfigValue),
        "ConfigValue's internal storage improperly aligned for added type!");

    //_data should be destructed at this point, construct a new value
    setInternal(value);
  }

  /**
   * @brief Retrieve an appropriately cast copy of the data stored in this @ref ConfigValue
   * @tparam The type the data should be cast as.
   */
  template <typename T>
  const T& get() const {
    ESP_CHECK(_type == configValTypeFor<T>(),
              "Attempting to access ConfigValue of" << _type << "with type"
                                                    << configValTypeFor<T>());
    return getInternal<T>();
  }

  /**
   * @brief Returns the current type of this @ref ConfigValue
   */
  ConfigValType getType() const { return _type; }

  /**
   * @brief Retrieve a string representation of the data held in this @ref
   * ConfigValue
   */
  std::string getAsString() const;

  /**
   * @brief Copy this @ref ConfigValue into the passed @ref Corrade::Utility::ConfigurationGroup
   */
  bool putValueInConfigGroup(const std::string& key,
                             Cr::Utility::ConfigurationGroup& cfg) const;

 private:
  template <typename T>
  EnableIf<isConfigValTypePointerBased(configValTypeFor<T>()), void>
  setInternal(const T& value) {
    T** tmpDst = reinterpret_cast<T**>(_data);
    *tmpDst = new T(value);
  }

  template <typename T>
  EnableIf<!isConfigValTypePointerBased(configValTypeFor<T>()), void>
  setInternal(const T& value) {
    new (_data) T(value);
  }

  template <typename T>
  EnableIf<isConfigValTypePointerBased(configValTypeFor<T>()), const T&>
  getInternal() const {
    auto val = [&]() {
      return *reinterpret_cast<const T* const*>(this->_data);
    };
    return *val();
  }

  template <typename T>
  EnableIf<!isConfigValTypePointerBased(configValTypeFor<T>()), const T&>
  getInternal() const {
    auto val = [&]() { return reinterpret_cast<const T*>(this->_data); };
    return *val();
  }

 public:
  /**
   * @brief Comparison
   */
  friend bool operator==(const ConfigValue& a, const ConfigValue& b);
  /**
   * @brief Inequality Comparison
   */
  friend bool operator!=(const ConfigValue& a, const ConfigValue& b);

  ESP_SMART_POINTERS(ConfigValue)
};  // namespace config

/**
 * @brief provide debug stream support for @ref ConfigValue
 */
MAGNUM_EXPORT Mn::Debug& operator<<(Mn::Debug& debug, const ConfigValue& value);

/**
 * @brief This class holds configuration data in a map of ConfigValues, and
 * also supports nested configurations via a map of smart pointers to this
 * type.
 */
class Configuration {
 public:
  /**
   * @brief Convenience typedef for the value map
   */
  typedef std::unordered_map<std::string, ConfigValue> ValueMapType;
  /**
   * @brief Convenience typedef for the subconfiguration map
   */
  typedef std::map<std::string, std::shared_ptr<Configuration>> ConfigMapType;

  /**
   * @brief Constructor
   */
  Configuration() = default;

  /**
   * @brief Copy Constructor
   */
  Configuration(const Configuration& otr)
      : configMap_(), valueMap_(otr.valueMap_) {
    for (const auto& entry : otr.configMap_) {
      configMap_[entry.first] = std::make_shared<Configuration>(*entry.second);
    }
  }  // copy ctor

  /**
   * @brief Move Constructor
   */
  Configuration(Configuration&& otr) noexcept
      : configMap_(std::move(otr.configMap_)),
        valueMap_(std::move(otr.valueMap_)) {}  // move ctor

  // virtual destructor set to that pybind11 recognizes attributes inheritance
  // from configuration to be polymorphic
  virtual ~Configuration() = default;

  /**
   * @brief Copy Assignment.
   */
  Configuration& operator=(const Configuration& otr);

  /**
   * @brief Move Assignment.
   */
  Configuration& operator=(Configuration&& otr) noexcept = default;

  // ****************** Getters ******************
  /**
   * @brief Get @ref ConfigValue specified by key, or empty ConfigValue if DNE.
   *
   * @param key The key of the value desired to be retrieved.
   * @return ConfigValue specified by key. If none exists, will be empty
   * ConfigValue, with type @ref ConfigValType::Unknown
   */
  ConfigValue get(const std::string& key) const {
    ValueMapType::const_iterator mapIter = valueMap_.find(key);
    if (mapIter != valueMap_.end()) {
      return mapIter->second;
    }
    ESP_WARNING() << "Key :" << key << "not present in configuration";
    return {};
  }

  /**
   * @brief Get value specified by @p key and expected to be type @p T and
   * return it if it exists and is appropriate type.  Otherwise throw a
   * warning and return a default value.
   * @tparam The type of the value desired
   * @param key The key of the value desired to be retrieved.
   * @return The value held at @p key, expected to be type @p T .  If not
   * found, or not of expected type, gives an error message and returns a
   * default value.
   */
  template <typename T>
  T get(const std::string& key) const {
    ValueMapType::const_iterator mapIter = valueMap_.find(key);
    const ConfigValType desiredType = configValTypeFor<T>();
    if (mapIter != valueMap_.end() &&
        (mapIter->second.getType() == desiredType)) {
      return mapIter->second.get<T>();
    }
    ESP_ERROR() << "Key :" << key << "not present in configuration as"
                << getNameForStoredType(desiredType);
    return {};
  }

  /**
   * @brief Return the @ref ConfigValType enum representing the type of the
   * value referenced by the passed @p key or @ref ConfigValType::Unknown
   * if unknown/unspecified.
   */
  ConfigValType getType(const std::string& key) const {
    ValueMapType::const_iterator mapIter = valueMap_.find(key);
    if (mapIter != valueMap_.end()) {
      return mapIter->second.getType();
    }
    ESP_ERROR() << "Key :" << key << "not present in configuration.";
    return ConfigValType::Unknown;
  }

  // ****************** String Conversion ******************

  /**
   * @brief This method will look for the provided key, and return a string
   * holding the object, if it is found in one of this Configuration's maps
   */
  std::string getAsString(const std::string& key) const {
    ValueMapType::const_iterator mapIter = valueMap_.find(key);
    if (mapIter != valueMap_.end()) {
      return mapIter->second.getAsString();
    }
    std::string retVal = Cr::Utility::formatString(
        "Key {} does not represent a valid value in this Configuration.", key);
    ESP_WARNING() << retVal;
    return retVal;
  }

  // ****************** Key List Retrieval ******************

  /**
   * @brief Retrieve list of keys present in this @ref Configuration's
   * valueMap_.  Subconfigs are not included.
   */
  std::vector<std::string> getKeys() const {
    std::vector<std::string> keys;
    keys.reserve(valueMap_.size());
    for (const auto& entry : valueMap_) {
      keys.push_back(entry.first);
    }
    return keys;
  }

  /**
   * @brief This function returns this @ref Configuration's subconfig keys.
   */
  std::vector<std::string> getSubconfigKeys() const {
    std::vector<std::string> keys;
    keys.reserve(configMap_.size());
    for (const auto& entry : configMap_) {
      keys.push_back(entry.first);
    }
    return keys;
  }

  /**
   * @brief Retrieve a list of all the keys in this @ref Configuration pointing
  to values of passed type @p storedType.
   * @param storedType The desired type of value whose key should be returned.
   * @return vector of string keys pointing to values of desired @p storedType
   */
  std::vector<std::string> getStoredKeys(ConfigValType storedType) const {
    std::vector<std::string> keys;
    // reserve space for all keys
    keys.reserve(valueMap_.size());
    unsigned int count = 0;
    for (const auto& entry : valueMap_) {
      if (entry.second.getType() == storedType) {
        ++count;
        keys.push_back(entry.first);
      }
    }
    return keys;
  }

  // ****************** Setters ******************

  /**
   * @brief Save the passed @p value using specified @p key
   * @tparam The type of the value to be saved.
   * @param key The key to assign to the passed value.
   * @param value The value to save at given @p key
   */
  template <typename T>
  void set(const std::string& key, const T& value) {
    valueMap_[key].set<T>(value);
  }
  /**
   * @brief Save the passed @p value char* as a string to the configuration at
   * the passed @p key.
   * @param key The key to assign to the passed value.
   * @param value The char* to save at given @p key as a string.
   */
  void set(const std::string& key, const char* value) {
    valueMap_[key].set<std::string>(std::string(value));
  }

  /**
   * @brief Save the passed float @p value as a double using the specified @p
   * key .
   * @param key The key to assign to the passed value.
   * @param value The float value to save at given @p key as a double.
   */
  void set(const std::string& key, float value) {
    valueMap_[key].set<double>(static_cast<double>(value));
  }

  // ****************** Value removal ******************

  /**
   * @brief Remove value specified by @p key and return it if it exists.
   * Otherwise throw a warning and return a default value.
   * @param key The key of the value desired to be retrieved/removed.
   * @return The erased value, held at @p key if found.  If not found, or not
   * of expected type, gives a warning and returns a default value.
   */

  ConfigValue remove(const std::string& key) {
    ValueMapType::const_iterator mapIter = valueMap_.find(key);
    if (mapIter != valueMap_.end()) {
      valueMap_.erase(mapIter);
      return mapIter->second;
    }
    ESP_WARNING() << "Key :" << key << "not present in configuration";
    return {};
  }

  /**
   * @brief Remove value specified by @p key and expected to be type @p T and
   * return it if it exists and is appropriate type.  Otherwise throw a
   * warning and return a default value.
   * @tparam The type of the value desired
   * @param key The key of the value desired to be retrieved/removed.
   * @return The erased value, held at @p key and expected to be type @p T ,
   * if found.  If not found, or not of expected type, gives a warning and
   * returns a default value.
   */
  template <typename T>
  T remove(const std::string& key) {
    ValueMapType::const_iterator mapIter = valueMap_.find(key);
    const ConfigValType desiredType = configValTypeFor<T>();
    if (mapIter != valueMap_.end() &&
        (mapIter->second.getType() == desiredType)) {
      valueMap_.erase(mapIter);
      return mapIter->second.get<T>();
    }
    ESP_WARNING() << "Key :" << key << "not present in configuration as"
                  << getNameForStoredType(desiredType);
    return {};
  }

  /**
   * @brief Return number of value and subconfig entries in this
   * Configuration. This only counts each subconfiguration entry as a single
   * entry.
   */
  int getNumEntries() const { return configMap_.size() + valueMap_.size(); }

  /**
   * @brief Return total number of value and subconfig entries held by this
   * Configuration and all its subconfigs.
   */
  int getConfigTreeNumEntries() const {
    int num = getNumEntries();
    for (const auto& subConfig : configMap_) {
      num += subConfig.second->getConfigTreeNumEntries();
    }
    return num;
  }
  /**
   * @brief Return number of subconfig entries in this Configuration. This
   * only counts each subconfiguration entry as a single entry.
   */
  int getNumSubconfigs() const { return configMap_.size(); }

  /**
   * @brief Return size of entire subconfig tree (i.e. total number of
   * subconfigs nested under this Configuration.)
   */
  int getConfigTreeNumSubconfigs() const {
    int num = configMap_.size();
    for (const auto& subConfig : configMap_) {
      num += subConfig.second->getConfigTreeNumSubconfigs();
    }
    return num;
  }

  /**
   * @brief returns number of values in this Configuration.
   */
  int getNumValues() const { return valueMap_.size(); }

  /**
   * @brief Return total number of values held by this Configuration and all
   * its subconfigs.
   */
  int getConfigTreeNumValues() const {
    int num = valueMap_.size();
    for (const auto& subConfig : configMap_) {
      num += subConfig.second->getConfigTreeNumValues();
    }
    return num;
  }

  /**
   * @brief Returns whether this @ref Configuration has the passed @p key as a
   * non-configuration value. Does not check subconfigurations.
   */
  bool hasValue(const std::string& key) const {
    return valueMap_.count(key) > 0;
  }

  /**
   * @brief Whether passed @p key references a @ref ConfigValue of passed @ref ConfigValType @p desiredType
   * @param key The key to check the type of.
   * @param desiredType the @ref ConfigValType to compare the value's type to
   * @return Whether @p key references a value that is of @p desiredType.
   */
  bool hasKeyOfType(const std::string& key, ConfigValType desiredType) {
    ValueMapType::const_iterator mapIter = valueMap_.find(key);
    return (mapIter != valueMap_.end() &&
            (mapIter->second.getType() == desiredType));
  }

  /**
   * @brief Checks if passed @p key is contained in this Configuration.
   * Returns a list of nested subconfiguration keys, in order, to the
   * configuration where the key was found, ending in the requested @p key.
   * If list is empty, @p key was not found.
   * @param key The key to look for
   * @return A breadcrumb list to where the value referenced by @p key
   * resides. An empty list means the value was not found.
   */
  std::vector<std::string> findValue(const std::string& key) const;

  /**
   * @brief Builds and returns @ref Corrade::Utility::ConfigurationGroup
   * holding the values in this esp::core::config::Configuration.
   *
   * @return a reference to a configuration group for this Configuration
   * object.
   */
  Cr::Utility::ConfigurationGroup getConfigGroup() const {
    Cr::Utility::ConfigurationGroup cfg{};
    putAllValuesInConfigGroup(cfg);
    return cfg;
  }

  /**
   * @brief This method will build a map of the keys of all the config values
   * this Configuration holds and the types of each of these values.
   */
  std::unordered_map<std::string, ConfigValType> getValueTypes() const {
    std::unordered_map<std::string, ConfigValType> res{};
    res.reserve(valueMap_.size());
    for (const auto& elem : valueMap_) {
      res[elem.first] = elem.second.getType();
    }
    return res;
  }

  // ****************** Subconfiguration accessors ******************

  /**
   * @brief return if passed key corresponds to a subconfig in this
   * configuration
   */
  bool hasSubconfig(const std::string& key) const {
    ConfigMapType::const_iterator mapIter = configMap_.find(key);
    return (mapIter != configMap_.end());
  }

  /**
   * @brief Templated subconfig copy getter. Retrieves a shared pointer to a
   * copy of the subConfig @ref esp::core::config::Configuration that has the
   * passed @p name .
   *
   * @tparam Type to return. Must inherit from @ref
   * esp::core::config::Configuration
   * @param name The name of the configuration to retrieve.
   * @return A pointer to a copy of the configuration having the requested
   * name, cast to the appropriate type, or nullptr if not found.
   */

  template <typename T>
  std::shared_ptr<T> getSubconfigCopy(const std::string& name) const {
    static_assert(std::is_base_of<Configuration, T>::value,
                  "Configuration : Desired subconfig must be derived from "
                  "core::config::Configuration");
    auto configIter = configMap_.find(name);
    if (configIter != configMap_.end()) {
      // if exists return copy, so that consumers can modify it freely
      return std::make_shared<T>(
          *std::static_pointer_cast<T>(configIter->second));
    }
    return nullptr;
  }

  /**
   * @brief return pointer to read-only sub-configuration of given @p name.
   * Will fail if configuration with given name dne.
   * @param name The name of the desired configuration.
   */
  std::shared_ptr<const Configuration> getSubconfigView(
      const std::string& name) const {
    auto configIter = configMap_.find(name);
    CORRADE_ASSERT(
        configIter != configMap_.end(),
        "Subconfiguration with name " << name << " not found in Configuration.",
        nullptr);
    // if exists return actual object
    return configIter->second;
  }

  /**
   * @brief Templated Version. Retrieves the stored shared pointer to the
   * subConfig @ref esp::core::config::Configuration that has the passed @p name
   * , cast to the specified type. This will create a shared pointer to a new
   * sub-configuration if none exists and return it, cast to specified type.
   *
   * Use this function when you wish to modify this Configuration's
   * subgroup, possibly creating it in the process.
   * @tparam The type to cast the @ref esp::core::config::Configuration to. Type
   * is checked to verify that it inherits from Configuration.
   * @param name The name of the configuration to edit.
   * @return The actual pointer to the configuration having the requested
   * name, cast to the specified type.
   */
  template <typename T>
  std::shared_ptr<T> editSubconfig(const std::string& name) {
    static_assert(std::is_base_of<Configuration, T>::value,
                  "Configuration : Desired subconfig must be derived from "
                  "core::config::Configuration");
    // retrieve existing (or create new) subgroup, with passed name
    return std::static_pointer_cast<T>(addSubgroup(name));
  }

  /**
   * @brief move specified subgroup config into configMap at desired name.
   * Will replace any subconfiguration at given name without warning if
   * present.
   * @param name The name of the subconfiguration to add
   * @param configPtr A pointer to a subconfiguration to add.
   */
  template <typename T>
  void setSubconfigPtr(const std::string& name, std::shared_ptr<T>& configPtr) {
    static_assert(std::is_base_of<Configuration, T>::value,
                  "Configuration : Desired subconfig must be derived from "
                  "core::config::Configuration");

    configMap_[name] = std::move(configPtr);
  }  // setSubconfigPtr

  /**
   * @brief Removes and returns the named subconfig. If not found, returns an
   * empty subconfig with a warning.
   * @param name The name of the subconfiguration to delete
   * @return a shared pointer to the removed subconfiguration.
   */
  std::shared_ptr<Configuration> removeSubconfig(const std::string& name) {
    ConfigMapType::const_iterator mapIter = configMap_.find(name);
    if (mapIter != configMap_.end()) {
      configMap_.erase(mapIter);
      return mapIter->second;
    }
    ESP_WARNING() << "Name :" << name
                  << "not present in map of subconfigurations.";
    return {};
  }

  /**
   * @brief Retrieve the number of entries held by the subconfig with the
   * given name
   * @param name The name of the subconfig to query. If not found, returns 0
   * with a warning.
   * @return The number of entries in the named subconfig
   */
  int getSubconfigNumEntries(const std::string& name) const {
    auto configIter = configMap_.find(name);
    if (configIter != configMap_.end()) {
      return configIter->second->getNumEntries();
    }
    ESP_WARNING() << "No Subconfig found named :" << name;
    return 0;
  }

  /**
   * @brief Retrieve the number of entries held by the subconfig with the
   * given name, recursing subordinate subconfigs
   * @param name The name of the subconfig to query. If not found, returns 0
   * with a warning.
   * @return The number of entries in the named subconfig, including all
   * subconfigs
   */
  int getSubconfigTreeNumEntries(const std::string& name) const {
    auto configIter = configMap_.find(name);
    if (configIter != configMap_.end()) {
      return configIter->second->getConfigTreeNumEntries();
    }
    ESP_WARNING() << "No Subconfig found named :" << name;
    return 0;
  }

  /**
   * @brief Merges configuration pointed to by @p config into this
   * configuration, including all subconfigs.  Passed config overwrites
   * existing data in this config.
   * @param src The source of configuration data we wish to merge into this
   * configuration.
   */
  void overwriteWithConfig(const std::shared_ptr<const Configuration>& src) {
    if (src->getNumEntries() == 0) {
      return;
    }
    // copy every element over from src
    for (const auto& elem : src->valueMap_) {
      valueMap_[elem.first] = elem.second;
    }
    // merge subconfigs
    for (const auto& subConfig : src->configMap_) {
      const auto name = subConfig.first;
      // make if DNE and merge src subconfig
      addSubgroup(name)->overwriteWithConfig(subConfig.second);
    }
  }

  /**
   * @brief Returns a const iterator across the map of values.
   */
  std::pair<ValueMapType::const_iterator, ValueMapType::const_iterator>
  getValuesIterator() const {
    return std::make_pair(valueMap_.cbegin(), valueMap_.cend());
  }

  /**
   * @brief Returns a const iterator across the map of subconfigurations.
   */
  std::pair<ConfigMapType::const_iterator, ConfigMapType::const_iterator>
  getSubconfigIterator() const {
    return std::make_pair(configMap_.cbegin(), configMap_.cend());
  }

  // ==================== load from and save to json =========================

  /**
   * @brief Load values into this Configuration from the passed @p jsonObj.
   * Will recurse for subconfigurations.
   * @param jsonObj The JSON object to read from for the data for this
   * configuration.
   * @return The number of fields successfully read and populated.
   */
  int loadFromJson(const io::JsonGenericValue& jsonObj);

  /**
   * @brief Build and return a json object holding the values and nested
   * objects holding the subconfigs of this Configuration.
   */
  io::JsonGenericValue writeToJsonObject(io::JsonAllocator& allocator) const;

  /**
   * @brief Populate a json object with all the first-level values held in
   * this configuration.  May be overridden to handle special cases for
   * root-level configuration of Attributes classes derived from
   * Configuration.
   */
  virtual void writeValuesToJson(io::JsonGenericValue& jsonObj,
                                 io::JsonAllocator& allocator) const;

  /**
   * @brief Populate a json object with all the data from the
   * subconfigurations, held in json sub-objects, for this Configuration.
   */
  virtual void writeSubconfigsToJson(io::JsonGenericValue& jsonObj,
                                     io::JsonAllocator& allocator) const;

  /**
   * @brief Take the passed @p key and query the config value for that key,
   * writing it to @p jsonName within the passed @p jsonObj.
   * @param key The key of the data in the configuration
   * @param jsonName The tag to use in the json file
   * @param jsonObj The json object to write to
   * @param allocator The json allocator to use to build the json object
   */
  void writeValueToJson(const char* key,
                        const char* jsonName,
                        io::JsonGenericValue& jsonObj,
                        io::JsonAllocator& allocator) const;

  /**
   * @brief Take the passed @p key and query the config value for that key,
   * writing it to tag with @p key as name within the passed @p jsonObj.
   * @param key The key of the data in the configuration
   * @param jsonObj The json object to write to
   * @param allocator The json allocator to use to build the json object
   */
  void writeValueToJson(const char* key,
                        io::JsonGenericValue& jsonObj,
                        io::JsonAllocator& allocator) const {
    writeValueToJson(key, key, jsonObj, allocator);
  }

  /**
   * @brief Return all the values in this cfg in a formatted string.
   * Subconfigs will be displaced by a tab.
   * @param newLineStr The string to put at the end of each newline. As
   * subconfigs are called, add a tab to this.
   */
  std::string getAllValsAsString(const std::string& newLineStr = "\n") const;

 protected:
  /**
   * @brief Process passed json object into this Configuration, using passed
   * key.
   *
   * @param numVals number of values/configs loaded so far
   * @param key key to use to search @p jsonObj and also to set value or
   * subconfig within this Configuration.
   * @return the number of total fields successfully loaded after this
   * function executes.
   */
  int loadOneConfigFromJson(int numVals,
                            const std::string& key,
                            const io::JsonGenericValue& jsonObj);

  /**
   * @brief Friend function.  Checks if passed @p key is contained in @p
   * config. Returns the highest level where @p key was found
   * @param config The configuration to search for passed key
   * @param key The key to look for
   * @param parentLevel The parent level to the current iteration.  If
   * iteration finds @p key, it will return parentLevel+1
   * @param breadcrumb [out] List of keys to subconfigs to get to value.
   * Always should end with @p key.
   * @return The level @p key was found. 0 if not found (so can be treated
   * as bool)
   */
  static int findValueInternal(const Configuration& config,
                               const std::string& key,
                               int parentLevel,
                               std::vector<std::string>& breadcrumb);

  /**
   * @brief Populate the passed cfg with all the values this map holds, along
   * with the values any subgroups/sub-Configs it may hold
   */
  void putAllValuesInConfigGroup(Cr::Utility::ConfigurationGroup& cfg) const {
    // put ConfigVal values in map
    for (const auto& entry : valueMap_) {
      entry.second.putValueInConfigGroup(entry.first, cfg);
    }

    for (const auto& subConfig : configMap_) {
      const auto name = subConfig.first;
      auto* cfgSubGroup = cfg.addGroup(name);
      subConfig.second->putAllValuesInConfigGroup(*cfgSubGroup);
    }
  }

  /**
   * @brief if no subgroup with given name this will make one, otherwise does
   * nothing.
   * @param name Desired name of new subgroup.
   * @return whether a group was made or not
   */
  std::shared_ptr<Configuration> addSubgroup(const std::string& name) {
    // Attempt to insert an empty pointer
    auto result = configMap_.emplace(name, std::shared_ptr<Configuration>{});
    // If name not already present (insert succeeded) then add new
    // configuration
    if (result.second) {
      result.first->second = std::make_shared<Configuration>();
    }
    return result.first->second;
  }

  /**
   * @brief Map to hold configurations as subgroups
   */
  ConfigMapType configMap_{};

  /**
   * @brief Map that holds all config values
   */
  ValueMapType valueMap_{};

  ESP_SMART_POINTERS(Configuration)
};  // class Configuration

/**
 * @brief provide debug stream support for a @ref Configuration
 */
MAGNUM_EXPORT Mn::Debug& operator<<(Mn::Debug& debug,
                                    const Configuration& value);

/**
 * @brief Retrieves a shared pointer to a copy of the subConfig @ref
 * esp::core::config::Configuration that has the passed @p name . This will
 * create a pointer to a new sub-configuration if none exists already with
 * that name, but will not add this Configuration to this Configuration's
 * internal storage.
 *
 * @param name The name of the configuration to retrieve.
 * @return A pointer to a copy of the configuration having the requested
 * name, or a pointer to an empty configuration.
 */

template <>
std::shared_ptr<Configuration> Configuration::getSubconfigCopy<Configuration>(
    const std::string& name) const;
/**
 * @brief Retrieve a shared pointer to the actual subconfiguration given by @p
 * name, or a new subconfiguration with that name, if none exists.
 *
 * @param name The name of the desired subconfiguration
 * @return A pointer to the configuration having the requested
 * name, or a pointer to an empty configuration.
 */
template <>
std::shared_ptr<Configuration> Configuration::editSubconfig<Configuration>(
    const std::string& name);

/**
 * @brief Save the passed @ref Configuration pointed to by @p configPtr at location specified by @p name
 * @param name The name to save the subconfiguration by
 * @param configPtr A pointer to the @ref Configuration to save with the given @p name .
 */
template <>
void Configuration::setSubconfigPtr<Configuration>(
    const std::string& name,
    std::shared_ptr<Configuration>& configPtr);

}  // namespace config
}  // namespace core
}  // namespace esp

#endif  // ESP_CORE_CONFIGURATION_H_
