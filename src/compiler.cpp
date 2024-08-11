#include "compiler.h"
#include <cstdint>
#include <filesystem>
#include <fstream>

namespace BML {

Compiler::Compiler(Logger *log) : log(log) {}

Compiler::Compiler(std::string path, Logger *log) : log(log) {
  std::filesystem::path gameFolder = path;
}

Compiler::Compiler(std::vector<Mod> list, std::string path, Logger *log)
    : modList(list), log(log) {
  std::filesystem::path gameFolder = path;
}

Compiler::~Compiler() {}

uint8_t Compiler::compile() {
  log->appendLogMessage("\n****************************\n");
  log->appendLogMessage("Beginning compile!");

  compiledList.clear();
  std::filesystem::path stagingFolder =
      std::filesystem::current_path() / "Staging";

  try {
    // Check if the directory exists
    if (std::filesystem::exists(stagingFolder) &&
        std::filesystem::is_directory(stagingFolder)) {
      log->appendLogMessage("- Deleting old staging folder.");
      std::filesystem::remove_all(stagingFolder);
    }
    log->appendLogMessage("- Generating empty staging folder.");
    if (!std::filesystem::create_directory(stagingFolder)) {
      log->appendLogMessage("!! ERROR !! COMPILE FAILED : Failed to create "
                            "empty staging folder");
      return 7;
    }

  } catch (const std::filesystem::filesystem_error &e) {
    log->appendLogMessage("\n!! ERROR !! COMPILE FAILED : " +
                          QString(e.what()));
    return 8;
  } catch (const std::exception &e) {
    log->appendLogMessage("\n!! ERROR !! COMPILE FAILED : " +
                          QString(e.what()));
    return 9;
  }

  for (auto &mod : modList) {
    log->appendLogMessage("\n- Loading mod: " + mod.printQString());

    log->appendLogMessage("--  Checking dependencies of " + mod.printQString());
    if (!dependCheck(mod)) {
      log->appendLogMessage("\n!! ERROR !! COMPILE FAILED : Dependency error!");
      return 1;
    }

    log->appendLogMessage("--  Checking incompatibilities of " +
                          mod.printQString());
    if (!incompatibleCheck(mod)) {
      log->appendLogMessage(
          "\n!! ERROR !! COMPILE FAILED : Incompatibility error!");
      return 2;
    }
    if (!stageMod(mod)) {
      log->appendLogMessage("\n!! ERROR !! COMPILE FAILED : Staging error!");
      return 3;
    }

    compiledList.push_back(mod);
  }
  if (!inject()) {
    log->appendLogMessage("\n!! ERROR !! COMPILE FAILED : Inject error!");
    return 4;
  }

  log->appendLogMessage("\n** INSTALLATION SUCCEEDED **\nThe following " +
                        QString(std::to_string(modList.size()).c_str()) +
                        " mods have been installed:");
  for (auto &mod : compiledList) {
    log->appendLogMessage("  - " + mod.printQString());
  }
  log->appendLogMessage("\n****************************\n");
  return 0;
}

void Compiler::setModList(std::vector<Mod> mods) {
  modList.clear();
  modList = mods;
  log->appendLogMessage("\nMod list set! " +
                        QString(std::to_string(modList.size()).c_str()) +
                        " mods loaded!");
  for (auto &mod : modList) {
    log->appendLogMessage("  - " + mod.printQString());
  }
}
void Compiler::setPath(std::string path) { gameFolder = path; }

bool Compiler::dependCheck(Mod mod) {
  bool failed = false;
  for (auto &dep : mod.dependencies) {
    bool met = false;
    if (!dependCheck(dep)) {
      return false;
    }

    for (auto &compiled : compiledList) {
      if (dep.name() != compiled.name()) {
        continue;
      }
      if (dep.author() != compiled.author()) {
        continue;
      }
      if (dep.version() != compiled.version()) {
        if (dep.majorVersion() == "X") {
          if (dep.minorVersion() == "X" ||
              dep.minorVersion() == compiled.minorVersion()) {
            log->appendLogMessage("---  Dependency found! : " +
                                  compiled.printQString());
            met = true;
            break;
          }
        }
        if (dep.minorVersion() == "X") {
          if (dep.majorVersion() == "X" ||
              dep.majorVersion() == compiled.majorVersion()) {
            log->appendLogMessage("---  Dependency found! : " +
                                  compiled.printQString());
            met = true;
            break;
          }
        }
      }
    }
    if (!met) {
      log->appendLogMessage("!! ERROR !! " + mod.printQString() +
                            " is missing dependency " + dep.printQString());

      for (auto &modlist : modList) {
        if (dep.name() != modlist.name()) {
          continue;
        }
        if (dep.author() != modlist.author()) {
          continue;
        }
        if (dep.version() != modlist.version()) {
          if (dep.majorVersion() == "X") {
            if (dep.minorVersion() == "X" ||
                dep.minorVersion() == modlist.minorVersion()) {

              log->appendLogMessage("\n!! FIX !! Incorrect load order! : " +
                                    modlist.printQString() +
                                    " must be compiled before " +
                                    mod.printQString() + "!");
              log->appendLogMessage(
                  ">> Solution << Move " + modlist.printQString() + " above " +
                  mod.printQString() + " in the active mods list!\n");

              met = true;
              break;
            }
          }
          if (dep.minorVersion() == "X") {
            if (dep.majorVersion() == "X" ||
                dep.majorVersion() == modlist.majorVersion()) {

              log->appendLogMessage("\n!! FIX !! Incorrect load order! : " +
                                    modlist.printQString() +
                                    " must be compiled before " +
                                    mod.printQString() + "!");
              log->appendLogMessage(
                  ">> Solution << Move " + modlist.printQString() + " above " +
                  mod.printQString() + " in the active mods list!\n");

              met = true;
              break;
            }
          }
        }
      }
      failed = true;
    }
  }
  if (failed) {
    return false;
  } else {
    return true;
  }
}

bool Compiler::incompatibleCheck(Mod mod) {
  bool failed = false;
  for (auto &incompatible : mod.incompatibilities) {
    if (!incompatibleCheck(incompatible)) {
      return false;
    }

    for (auto &modlist : modList) {
      if (incompatible.name() != modlist.name()) {
        continue;
      }
      if (incompatible.author() != modlist.author()) {
        continue;
      }
      if (incompatible.version() != modlist.version()) {
        if (incompatible.majorVersion() == "X") {
          if (incompatible.minorVersion() == "X" ||
              incompatible.minorVersion() == modlist.minorVersion()) {
            log->appendLogMessage("!! ERROR !! " + mod.printQString() +
                                  " is incompatible with " +
                                  incompatible.printQString());
            failed = true;
            break;
          }
        }
        if (incompatible.minorVersion() == "X") {
          if (incompatible.majorVersion() == "X" ||
              incompatible.majorVersion() == modlist.majorVersion()) {
            log->appendLogMessage("!! ERROR !! " + mod.printQString() +
                                  " is incompatible with " +
                                  incompatible.printQString());
            failed = true;
            break;
          }
        }
      }
    }
  }
  if (failed) {
    return false;
  } else {
    return true;
  }
}

bool Compiler::stageMod(Mod mod) {
  log->appendLogMessage("-- Staging mod: " + mod.printQString());
  std::filesystem::path stagingFolder =
      std::filesystem::current_path() / "Staging";

  std::filesystem::path dataFolder = mod.path();
  dataFolder = dataFolder / "Data";

  try {
    // Check if the directory exists
    if (!std::filesystem::exists(stagingFolder) ||
        !std::filesystem::is_directory(stagingFolder)) {
      if (!std::filesystem::create_directory(stagingFolder)) {
        log->appendLogMessage("!! ERROR !! Failed to create staging folder");
        return false;
      }
      log->appendLogMessage("-- Created Staging Folder at " +
                            QString(stagingFolder.c_str()));
    }

    if (!std::filesystem::exists(dataFolder) ||
        !std::filesystem::is_directory(dataFolder)) {
      log->appendLogMessage("!! ERROR !! Failed to find data Folder !");
      return false;
    }

    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(dataFolder)) {
      const auto &path = entry.path();
      auto relativePath = std::filesystem::relative(path, dataFolder);
      auto destPath = stagingFolder / relativePath;

      if (std::filesystem::is_directory(path)) {
        // If it's a directory, create it in the destination if it does not
        // exist
        if (!std::filesystem::exists(destPath)) {
          std::filesystem::create_directories(destPath);
        }
      } else if (std::filesystem::is_regular_file(path)) {
        // If it's a file, move it to the destination
        std::filesystem::copy(
            path, destPath, std::filesystem::copy_options::overwrite_existing);
      }
    }

  } catch (const std::filesystem::filesystem_error &e) {
    log->appendLogMessage("\n!! ERROR !! STAGING FAILED : " +
                          QString(e.what()));
    return false;
  } catch (const std::exception &e) {
    log->appendLogMessage("\n!! ERROR !! STAGING FAILED : " +
                          QString(e.what()));
    return false;
  }

  log->appendLogMessage("--- STAGING SUCCEEDED");
  return true;
}

bool Compiler::restoreSnapshot() {
  std::filesystem::path snapshotFolder =
      std::filesystem::current_path() / "Snapshot";

  // Check if the directory exists
  if (std::filesystem::exists(snapshotFolder) &&
      std::filesystem::is_directory(snapshotFolder)) {
    log->appendLogMessage("-- Found Snapshot Folder at " +
                          QString(snapshotFolder.c_str()));
  } else {
    log->appendLogMessage("-- There is no snapshot folder!");
    return true;
  }

  for (const auto &entry :
       std::filesystem::recursive_directory_iterator(snapshotFolder)) {
    const auto &path = entry.path();
    auto relativePath = std::filesystem::relative(path, snapshotFolder);
    auto destPath = gameFolder / relativePath;

    if (std::filesystem::is_directory(path)) {
      // If it's a directory, create it in the destination if it does not
      // exist
      if (!std::filesystem::exists(destPath)) {
        std::filesystem::create_directories(destPath);
      }
    } else if (std::filesystem::is_regular_file(path)) {
      // If it's a file, move it to the destination
      log->appendLogMessage("--- Restoring " + QString(path.c_str()));
      std::filesystem::rename(path, destPath);
    }
  }
  log->appendLogMessage("-- Successfully restored snapshot");
  log->appendLogMessage("-- Deleting old snapshot folder.");
  std::filesystem::remove_all(snapshotFolder);
  return true;
}

bool Compiler::inject() {
  log->appendLogMessage("\n- BEGINNING INJECTION");
  try {
    // Check if the directory exists
    if (std::filesystem::exists(gameFolder) &&
        std::filesystem::is_directory(gameFolder)) {
      log->appendLogMessage("-- Found Game Folder at " +
                            QString(gameFolder.c_str()));
    } else {
      log->appendLogMessage(
          "\n!! ERROR !! INJECTION FAILED : Failed to open game folder!");
      return false;
    }

    if (!restoreSnapshot()) {
      log->appendLogMessage(
          "\n!! ERROR !! INJECTION FAILED : Failed to restore snapshot!");
      return false;
    }

    std::filesystem::path stagingFolder =
        std::filesystem::current_path() / "Staging";

    std::filesystem::path snapshotFolder =
        std::filesystem::current_path() / "Snapshot";

    if (std::filesystem::exists(snapshotFolder) &&
        std::filesystem::is_directory(snapshotFolder)) {
      log->appendLogMessage("-- Found Snapshot Folder at " +
                            QString(snapshotFolder.c_str()));
    } else {
      if (!std::filesystem::create_directory(snapshotFolder)) {
        log->appendLogMessage("!! ERROR !! Failed to create snapshot folder");
        return false;
      }
      log->appendLogMessage("-- Created empty Snapshot Folder at " +
                            QString(snapshotFolder.c_str()));
    }

    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(stagingFolder)) {
      const auto &path = entry.path();
      auto relativePath = std::filesystem::relative(path, stagingFolder);
      auto destPath = gameFolder / relativePath;
      auto snapshotPath = snapshotFolder / relativePath;

      if (std::filesystem::is_directory(path)) {
        // If it's a directory, create it in the destination if it does not
        // exist
        if (!std::filesystem::exists(destPath)) {
          std::filesystem::create_directories(destPath);
        }
        if (!std::filesystem::exists(snapshotPath)) {
          std::filesystem::create_directories(snapshotPath);
        }
      } else if (std::filesystem::is_regular_file(path)) {
        // If it's a file, move it to the destination
        log->appendLogMessage("--- Injecting " + QString(path.c_str()));

        if (std::filesystem::exists(destPath)) {
          log->appendLogMessage("--- Saving snapshot to " +
                                QString(snapshotPath.c_str()));
          std::filesystem::copy(
              destPath, snapshotPath,
              std::filesystem::copy_options::overwrite_existing);
        } else {
          log->appendLogMessage("--- Saving dummy to " +
                                QString(snapshotPath.c_str()));
          std::ofstream f(snapshotPath);
          if (!f) {
            log->appendLogMessage(
                "\n ** ERROR ** Failed to create dummy file at " +
                QString(snapshotPath.c_str()));
            return false;
          }
          f.close();
        }

        std::filesystem::rename(path, destPath);
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    log->appendLogMessage("\n!! ERROR !! INJECTION FAILED : " +
                          QString(e.what()));
    return false;
  } catch (const std::exception &e) {
    log->appendLogMessage("\n!! ERROR !! INJECTION FAILED : " +
                          QString(e.what()));
    return false;
  }
  return true;
}

} // namespace BML
