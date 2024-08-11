#include "mod.h"
#include <cstdint>
#include <filesystem>

namespace BML {

Mod::Mod() {}
Mod::~Mod() {}

Mod::Mod(std::string name) : m_name(name) {}

Mod::Mod(std::string name, std::string author)
    : m_name(name), m_author(author) {}

Mod::Mod(std::string name, std::string author, std::string version)
    : m_name(name), m_author(author) {
  setVersion(version);
}

std::string Mod::name() { return m_name; }
std::string Mod::author() { return m_author; }
std::string Mod::version() { return m_version; }
std::string Mod::majorVersion() { return m_majorVersion; }
std::string Mod::minorVersion() { return m_minorVersion; }
std::string Mod::path() { return m_path; }
std::string Mod::print() {
  return (m_name + " | [v" + m_version + "] (" + m_author + ")");
}
QString Mod::printQString() { return ("\"" + QString(print().c_str()) + "\""); }

void Mod::setName(std::string name) { m_name = name; }
void Mod::setAuthor(std::string author) { m_author = author; }
void Mod::setPath(std::string path) { m_path = path; }

bool validVersionNumber(std::string str) {
  if (str.empty()) {
    return false;
  }

  if (str == "X") {
    return true;
  }

  // Check each character to ensure it's a digit
  for (char c : str) {
    if (!std::isdigit(c)) {
      return false;
    }
  }

  return true;
}

void Mod::setVersion(std::string version) {
  m_version = version;

  size_t delimiterPos = version.find('.');

  if (delimiterPos != std::string::npos) {
    // Extract substrings
    std::string major_version = version.substr(0, delimiterPos);
    if (validVersionNumber(major_version)) {
      m_majorVersion = major_version;
    } else {
      m_majorVersion.clear();
    }

    std::string minor_version = version.substr(delimiterPos + 1);
    if (validVersionNumber(minor_version)) {
      m_minorVersion = minor_version;
    } else {
      m_minorVersion.clear();
    }
  }
}

uint8_t Mod::checkValid() {
  if (m_name.empty()) {
    return 1;
  }
  if (m_author.empty()) {
    return 2;
  }
  if (m_version.empty()) {
    return 3;
  }
  if (m_majorVersion.empty()) {
    return 4;
  }
  if (m_minorVersion.empty()) {
    return 5;
  }
  try {
    // Check if the start directory exists and is a directory
    if (!std::filesystem::exists(m_path) ||
        !std::filesystem::is_directory(m_path)) {
      return 6;
    }

    // Iterate recursively through the directory
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(m_path)) {
      if (entry.is_directory() && entry.path().filename() == "Data") {
        return 0;
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    return 10;
  } catch (const std::exception &e) {
    return 11;
  }
  return 12;
}

bool Mod::compare(Mod mod) {
  if (mod.name() != m_name) {
    return 1;
  }
  if (mod.author() != m_author) {
    return 1;
  }
  if (mod.version() != m_version) {
    return 1;
  }
  return 0;
}

} // namespace BML
