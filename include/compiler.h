#ifndef COMPILER_H
#define COMPILER_H

#include "logger.h"
#include "mod.h"
#include <filesystem>

namespace BML {

class Compiler {

public:
  Compiler(Logger *log);
  Compiler(std::string path, Logger *log);
  Compiler(std::vector<Mod> list, std::string path, Logger *log);
  ~Compiler();

  uint8_t compile();
  void setModList(std::vector<Mod> mods);
  void setPath(std::string path);

private:
  bool dependCheck(Mod mod);
  bool incompatibleCheck(Mod mod);
  bool stageMod(Mod mod);
  bool inject();
  bool restoreSnapshot();

  std::filesystem::path gameFolder;
  std::vector<Mod> modList;
  std::vector<Mod> compiledList;

  Logger *log;
};

} // namespace BML

#endif // COMPILER_H
