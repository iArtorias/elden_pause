//
// Created by X-ray on 5/18/2021.
//
#pragma once

#ifndef INIREADER_HPP
#define INIREADER_HPP
#include <string>
#include <regex>
#include <filesystem>
#include <fstream>
#include <cassert>
#include <memory>
#include <utility>
#include <sstream>
#include <unordered_map>
#include "conversion.hpp"

namespace ini {
  class Parser {
  public:
    /**
     * @param wipe_on_parse wipe the ini file root_ when parsing a new document
     */
    inline explicit Parser(bool wipe_on_parse = true) {
      wipe_on_parse_ = wipe_on_parse;
      root_ = std::make_unique<IniRoot>();
    }

    /**
     * @param file path/contents of ini file
     * @param is_path is the file a path or contents of a ini file
     */
    inline void Parse(const std::string& file, bool is_path) {
      if (is_path) {
        if (!std::filesystem::exists(file)) {
          assert(!std::filesystem::exists(file));
          throw std::runtime_error("File not found");
        } else {
          std::ifstream ini_file(file);
          Parse(ini_file);
        }
      } else {
        auto lines = Split(file, '\n');
        ImplParse(lines);
      }
    }

    /**
     * @param file path to a ini file
     */
    inline void Parse(std::filesystem::path& file) {
      if (!std::filesystem::exists(file)) {
        assert(!std::filesystem::exists(file));
        throw std::runtime_error("File not found");
      } else {
        std::ifstream ini_file(file);
        Parse(ini_file);
      }
    }

    /**
     * @param file a open stream of a ini file
     */
    inline void Parse(std::fstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    /**
     * @param file a open stream of a ini file
     */
    inline void Parse(std::ifstream& file) {
      auto lines = ReadFile(file);
      ImplParse(lines);
    }

    struct IniValue {
    public:
      /**
       * @tparam T return type of the value
       * @return get value as T
       */
      template<typename T>
      [[nodiscard]] inline T as() const {
        conversion::AsImpl<T> as;
        T res;
        if (as.is(value_)) {
          as.get(value_, res);
        } else {
          assert(as.is(value_));
        }
        return res;
      }

      /**
       * @tparam T type of the value
       * @return check if value is of type T
       */
      template<typename T>
      [[nodiscard]] inline bool is() const {
        conversion::AsImpl<T> as;
        return as.is(value_);
      }

      /**
       * @tparam T type of value to assign
       */
      template<typename T>
      inline IniValue& operator=(const T& value) {
        conversion::AsImpl<T> as;
        as.set(value, value_);
        return *this;
      }
    private:
      std::string value_;
    };

    struct IniSection {
      /**
       * @tparam T type of the value to add
       * @param key key of the value
       * @param value value to add
       */
      template<typename T>
      inline void Add(const std::string& key, const T& value) {
        conversion::AsImpl<T> as;
        std::string tmp;
        as.set(value, tmp);
        items_[key] = tmp;
      }

      /**
       * @param key key of value to remove
       * @return success
       */
      inline bool Remove(const std::string& key) {
        if (items_.find(key) != items_.end()) {
          items_.erase(key);
          return true;
        } else {
          assert(items_.find(key) != items_.end());
          return false;
        }
      }

      /**
       * @note This will remove all the values in the section
       */
      inline void RemoveAll() {
        items_.clear();
      }

      /**
       * @param key check if the key exists in the section
       * @return true if the key exists
       */
      [[nodiscard]] inline bool HasValue(const std::string& key) const {
        return items_.find(key) != items_.end();
      }

      /**
       * @return a stringified version of the section
       */
      [[nodiscard]] inline std::string Stringify() const {
        std::string res;
        for (auto& item : items_) {
          res += item.first + "=" + item.second.as<std::string>() + "\n";
        }
        return res;
      }

      /**
       * @return Amount of members in the section
       */
      [[nodiscard]] inline size_t Size() const {
        return items_.size();
      }

      /**
       * @param key key of the value to get
       * @return a reference to the key
       */
      inline IniValue& operator[](const std::string& key) {
        auto entry = items_.find(key);

        if (entry != items_.end()) {
          return entry->second;
        } else {
          assert(entry != items_.end());
          throw std::runtime_error("Section does not have a value with the key: "+key);
        }
      }

      inline std::unordered_map<std::string, IniValue>::iterator begin() noexcept { return items_.begin(); }
      inline std::unordered_map<std::string, IniValue>::const_iterator cbegin() const noexcept { return items_.cbegin(); }
      inline std::unordered_map<std::string, IniValue>::iterator end() noexcept { return items_.end(); }
      inline std::unordered_map<std::string, IniValue>::const_iterator cend() const noexcept { return items_.cend(); }

    private:
      std::unordered_map<std::string, IniValue> items_;
    };

    /**
     * @param section name of the section to add
     * @return a reference to the section
     */
    inline IniSection& AddSection(const std::string& section) {
      root_->sections[section] = IniSection();
      return root_->sections[section];
    }

    /**
     * @param section name of the section to check for
     * @return returns true if the section exists
     */
    [[nodiscard]] inline bool HasSection(const std::string& section) const {
      return root_->sections.find(section) != root_->sections.end();
    }

    /**
     * @return count of non root sections
     */
    [[nodiscard]] inline std::uint32_t GetSectionCount() const {
      return static_cast<std::uint32_t>(root_->sections.size());
    }

    /**
     * @param section name of the section to remove
     * @return returns true if the section is removed
     */
    inline bool RemoveSection(const std::string& section) {
      if (HasSection(section)) {
        root_->sections.erase(section);
        return true;
      } else {
        assert(HasSection(section));
        return false;
      }
    }

    /**
     * @return a reference to the root section
     */
    inline IniSection& GetRootSection() {
      return root_->root_section;
    }

    /**
     * @param section name of the section to get
     * @return a reference to the section
     */
    inline IniSection& operator[](const std::string& section) {
      auto entry = root_->sections.find(section);

      if (entry != root_->sections.end()) {
        return entry->second;
      } else {
        assert(entry != root_->sections.end());
        throw std::runtime_error("Section: "+section+" does not exist");
      }
    }

    inline std::unordered_map<std::string, IniSection>::iterator begin() noexcept { return root_->sections.begin(); }
    inline std::unordered_map<std::string, IniSection>::const_iterator cbegin() const noexcept { return root_->sections.cbegin(); }
    inline std::unordered_map<std::string, IniSection>::iterator end() noexcept { return root_->sections.end(); }
    inline std::unordered_map<std::string, IniSection>::const_iterator cend() const noexcept { return root_->sections.cend(); }

    /**
     * @return a string representation of the ini file
     */
    [[nodiscard]] inline std::string Stringify() const {
      std::stringstream ss;

      ss << root_->root_section.Stringify();
      for (const auto& section : root_->sections) {
        ss << "[" << section.first << "]" << std::endl;
        ss << section.second.Stringify();
      }
      return ss.str();
    }

  private:
    struct IniRoot {
      IniSection root_section;
      std::unordered_map<std::string, IniSection> sections;
    };

    std::string current_section_;
    std::unique_ptr<IniRoot> root_;
    bool wipe_on_parse_;

  private:
#define TRIM_STR(str, c) TrimR(Trim(str, c), c)

    /**
     * @param lines a vector of lines to parse
     */
    void ImplParse(std::vector<std::string>& lines) {
      if (wipe_on_parse_) {
        current_section_.clear();
        root_ = std::make_unique<IniRoot>();
      }

      if (lines.empty()) {return;}

      for (auto&& line : lines) {
        if (line.empty()) continue;

        RemoveComment(line);
        if (line.empty()) continue;

        auto item = GetItem(line);
        if (!item.first.empty() && !item.second.empty()) {
          if (current_section_.empty()) {
            GetRootSection().Add(item.first, item.second);
          } else if (HasSection(current_section_)) {
            (*this)[current_section_].Add(item.first, item.second);
          } else {
            assert(HasSection(current_section_));
            throw std::runtime_error("Section does not have a value with the key: "+current_section_);
          }
          continue;
        }

        if (auto section = GetSection(line); !section.empty()) {
          current_section_ = section;
          AddSection(current_section_);
          continue;
        }

        // if it gets to here it's an empty line
      }
    }

    /**
     * @param line removes a ini comment from the given string
     */
    static inline void RemoveComment(std::string& line) {
      if (line[0] == ';' || line[0] == '#') {
        line.clear();
        return;
      }

      if (auto semicollon_pos = line.find(';'); semicollon_pos != std::string::npos) {
        if (line[semicollon_pos - 1] != '\\') {
          line.erase(semicollon_pos, line.back());
        }
      } else if (auto hash_pos = line.find('#'); hash_pos != std::string::npos) {
        if (line[hash_pos - 1] != '\\') {
          line.erase(hash_pos, line.back());
        }
      }
    }

    /**
     * @param line to check for a valid ini item
     * @return if it is a valid item a kv
     */
    static inline std::pair<std::string, std::string> GetItem(std::string& line) {
      std::smatch match;
      if (std::regex_match(line, match, std::regex(R"((.*)= ?(.*))"))) {
        return std::make_pair(TRIM_STR(match[1].str(), ' '), TRIM_STR(match[2].str(), ' '));
      }
      return {};
    }

    /**
     * @param line to check for a valid section
     * @return the name of the section
     */
    static inline std::string GetSection(std::string& line) {
      Trim(line, ' ');
      if (line.empty()) return {};

      if (line[0] == '[') {
        std::size_t search_pos = 1;
        auto close_pos = line.find(']', search_pos);
        while(close_pos != std::string::npos && line[close_pos - 1] == '\\') {
          search_pos = close_pos + 1;
          close_pos = line.find(']', search_pos);
        }
        return line.substr(1, close_pos - 1);
      } else {
        return {};
      }
    }

    /**
     * @tparam T the stream type
     * @param stream a open stream to read from
     * @return a vector of with the content seperated by new lines
     */
    template<typename T>
    static inline std::vector<std::string> ReadFile(T& stream) {
      std::string tmp;
      std::vector<std::string> lines;
      while (std::getline(stream, tmp)) {
        lines.emplace_back(tmp);
      }
      return lines;
    }

    /**
     * @param str to split
     * @param c to split by
     * @return a split string inside a vector
     */
    static inline std::vector<std::string> Split(const std::string& str, char c) {
      std::vector<std::string> res;
      std::string tmp;
      for (auto& ch : str) {
        if (ch == c) {
          res.emplace_back(tmp);
          tmp.clear();
        } else {
          tmp.push_back(ch);
        }
      }
      if (!tmp.empty()) {
        res.emplace_back(tmp);
      }
      return res;
    }

    /// trim the front of the string by given character
    static inline std::string Trim(std::string str, char trim_c) {
      while(str.front() == trim_c) {
        str.erase(0, 1);
      }
      return str;
    }

    /// trim the back of the string by given character
    static inline std::string TrimR(std::string str, char trim_c) {
      while (str.back() == trim_c) {
        str.pop_back();
      }
      return str;
    }
  };
}
#ifdef TRIM_STR
#undef TRIM_STR
#endif

#endif // INIREADER_HPP
