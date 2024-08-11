#ifndef MOD_H
#define MOD_H

#include <QString>
#include <cstdint>
#include <string>
#include <vector>

namespace BML {

class Mod {

public:
  Mod();
  Mod(std::string name);
  Mod(std::string name, std::string author);
  Mod(std::string name, std::string author, std::string version);
  ~Mod();

  std::string name();
  std::string author();
  std::string version();
  std::string majorVersion();
  std::string minorVersion();
  std::string path();
  std::string print();
  QString printQString();

  std::vector<Mod> dependencies;
  std::vector<Mod> incompatibilities;

  void setName(std::string name);
  void setAuthor(std::string author);
  void setVersion(std::string version);
  void setPath(std::string path);

  uint8_t checkValid();
  bool compare(Mod mod);

private:
  std::string m_name;
  std::string m_author;
  std::string m_version;
  std::string m_majorVersion;
  std::string m_minorVersion;

  std::string m_path;
};

} // namespace BML

#endif // MOD_H
