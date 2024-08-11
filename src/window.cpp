#include "window.h"
#include "json.hpp"
#include "qmarkdowntextedit.h"
#include "qnamespace.h"
#include <QFileDialog>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <filesystem>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

namespace BML {

std::filesystem::path getUserHomeDirectory() {
  const char *homeDir = std::getenv("HOME"); // For Unix-based systems

  if (homeDir) {
    return std::filesystem::path(homeDir);
  } else {
    // Fallback for Windows
    homeDir = std::getenv("USERPROFILE");
    if (homeDir) {
      return std::filesystem::path(homeDir);
    } else {
      // Further fallback if necessary
      return std::filesystem::path();
    }
  }
}

bool findBorderlandsGameFolder(const std::filesystem::path &startDir,
                               std::filesystem::path &foundPath) {
  try {
    // Check if the start directory exists and is a directory
    if (!std::filesystem::exists(startDir) ||
        !std::filesystem::is_directory(startDir)) {
      return false;
    }

    // Iterate recursively through the directory
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(startDir)) {
      if (entry.is_directory() && entry.path().filename() == "Borderlands") {
        foundPath = entry.path();
        return true;
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
  } catch (const std::exception &e) {
  }

  return false;
}

Window::Window(QWidget *parent) : QMainWindow(parent) {
  // Window Settings
  this->setFixedSize(975, 1040);

  // Mods Path
  modsPathButton = new QPushButton("...", this);
  modsPathLine = new QLineEdit("Insert Mods Folder Location Here", this);
  modsPathLabel = new QLabel("Mods Folder Location:", this);

  modsPathLabel->setGeometry(QRect(QPoint(10, 10), QSize(120, 20)));
  modsPathLine->setGeometry(QRect(QPoint(140, 10), QSize(300, 20)));
  modsPathButton->setGeometry(QRect(QPoint(450, 10), QSize(25, 20)));
  modsPathButton->setToolTip("Set Mods Folder Location");

  connect(modsPathButton, &QPushButton::released, this,
          &Window::handleModsFolderButton);

  // Game Path
  gamePathButton = new QPushButton("...", this);
  gamePathLine = new QLineEdit("Insert Game Folder Location Here", this);
  gamePathLabel = new QLabel("Game Folder Location:", this);

  gamePathLabel->setGeometry(QRect(QPoint(10, 40), QSize(120, 20)));
  gamePathLine->setGeometry(QRect(QPoint(140, 40), QSize(300, 20)));
  gamePathButton->setGeometry(QRect(QPoint(450, 40), QSize(25, 20)));
  gamePathButton->setToolTip("Set Game Folder Location");

  connect(gamePathButton, &QPushButton::released, this,
          &Window::handleGameFolderButton);
  // BML Info
  bmlLabel = new QLabel("Borderlands Mod Loader\nVersion " +
                            QString(versionNum) + "\nBuilt By JKBC",
                        this);
  bmlLabel->setGeometry(QRect(QPoint(485, 10), QSize(400, 50)));
  bmlLabel->setAlignment(Qt::AlignTop | Qt::AlignCenter);

  // Mod info
  modInfo = new QMarkdownTextEdit(this);
  modInfoLabel = new QLabel("Selected Mod Info", this);

  modInfoLabel->setGeometry(QRect(QPoint(10, 65), QSize(500, 15)));
  modInfoLabel->setAlignment(Qt::AlignCenter);
  modInfo->setGeometry(QRect(QPoint(10, 80), QSize(500, 500)));
  modInfo->setText("Select mod in Detected Mods list to view README");
  modInfo->setReadOnly(true);

  // Loaded mods list
  loadedList = new QListWidget(this);
  loadedListLabel = new QLabel("Detected Mods", this);

  loadedListLabel->setGeometry(QRect(QPoint(520, 65), QSize(200, 15)));
  loadedListLabel->setAlignment(Qt::AlignCenter);
  loadedList->setGeometry(QRect(QPoint(520, 80), QSize(200, 500)));
  loadedList->setDragEnabled(false);
  loadedList->setToolTip("Detected mods list");

  connect(loadedList, &QListWidget::itemSelectionChanged, this,
          &Window::handleLoadedListSelect);

  // Add Mod Button
  addModButton = new QPushButton("->", this);
  addModButton->setGeometry(QRect(QPoint(730, 80), QSize(25, 20)));
  addModButton->setToolTip("Add selected mod to active mods list");

  connect(addModButton, &QPushButton::released, this,
          &Window::handleAddModButton);

  // Remove Mod Button
  removeModButton = new QPushButton("<-", this);
  removeModButton->setGeometry(QRect(QPoint(730, 110), QSize(25, 20)));
  removeModButton->setToolTip("Remove selected mod from active mods list");

  connect(removeModButton, &QPushButton::released, this,
          &Window::handleRemoveModButton);

  // Up Mod Button
  upModButton = new QPushButton("/\\", this);
  upModButton->setGeometry(QRect(QPoint(730, 140), QSize(25, 20)));
  upModButton->setToolTip("Move selected mod up 1 in active mods list");

  connect(upModButton, &QPushButton::released, this,
          &Window::handleUpModButton);

  // Down Mod Button
  downModButton = new QPushButton("\\/", this);
  downModButton->setGeometry(QRect(QPoint(730, 170), QSize(25, 20)));
  downModButton->setToolTip("Move selected mod down 1 in active mods list");

  connect(downModButton, &QPushButton::released, this,
          &Window::handleDownModButton);

  // Applied mods list
  appliedList = new QListWidget(this);
  appliedListLabel = new QLabel("Active Mods", this);

  appliedListLabel->setGeometry(QRect(QPoint(765, 65), QSize(200, 15)));
  appliedListLabel->setAlignment(Qt::AlignCenter);
  appliedList->setGeometry(QRect(QPoint(765, 80), QSize(200, 500)));
  appliedList->setDragEnabled(true);
  appliedList->setDragDropMode(QAbstractItemView::InternalMove);
  appliedList->setToolTip("Applied mods list");

  // Search For Mods Button
  searchForModsButton = new QPushButton("Search Mods Folder", this);
  searchForModsButton->setGeometry(QRect(QPoint(520, 590), QSize(200, 20)));
  searchForModsButton->setToolTip("Search Mods Folder Location for mods");

  connect(searchForModsButton, &QPushButton::released, this,
          &Window::handleSearchForModsButton);

  // Apply Button
  applyModsButton = new QPushButton("Apply Mods", this);
  applyModsButton->setGeometry(QRect(QPoint(765, 590), QSize(90, 20)));
  applyModsButton->setToolTip(
      "Mark the current active mods list for installation");

  connect(applyModsButton, &QPushButton::released, this,
          &Window::handleApplyModsButton);

  // Compile Button
  compileModsButton = new QPushButton("Install Mods", this);
  compileModsButton->setGeometry(QRect(QPoint(875, 590), QSize(90, 20)));
  compileModsButton->setToolTip("Install the applied mods");

  connect(compileModsButton, &QPushButton::released, this,
          &Window::handleCompileModsButton);

  // Export Log Button
  exportLogButton = new QPushButton("Export Log", this);
  exportLogButton->setGeometry(QRect(QPoint(10, 590), QSize(90, 20)));
  exportLogButton->setToolTip("Export the log");

  connect(exportLogButton, &QPushButton::released, this,
          &Window::handleExportLogButton);

  // Log
  log = new Logger(this);
  logLabel = new QLabel("Log Output", this);

  logLabel->setGeometry(QRect(QPoint(0, 625), QSize(975, 15)));
  logLabel->setAlignment(Qt::AlignCenter);
  log->setGeometry(QRect(QPoint(0, 630), QSize(975, 400)));
  log->setToolTip("Logs");

  // Compiler
  compiler = new Compiler(log);

  log->appendLogMessage(
      "\n******************************************************");
  log->appendLogMessage("*** START OF LOG");
  log->appendLogMessage("*** BORDERLANDS MOD LOADER");
  log->appendLogMessage("*** VERSION " + QString(versionNum));
  log->appendLogMessage("*** BUILT BY JKBC");
  log->appendLogMessage(
      "******************************************************\n");

  std::filesystem::path modsFolder = std::filesystem::current_path() / "Mods";
  std::filesystem::path gameFolder =
      std::filesystem::current_path() / "Borderlands";

  try {
    // Check if the directory exists
    if (std::filesystem::exists(modsFolder) &&
        std::filesystem::is_directory(modsFolder)) {
      log->appendLogMessage("Found Mods Folder at " +
                            QString(modsFolder.c_str()));
    } else {
      if (std::filesystem::create_directory(modsFolder)) {
        log->appendLogMessage("Created Mods Folder at " +
                              QString(modsFolder.c_str()));
      }
    }

    if (std::filesystem::exists(gameFolder) &&
        std::filesystem::is_directory(gameFolder)) {
      log->appendLogMessage("Found Game Folder at " +
                            QString(gameFolder.c_str()));
      gamePathLine->setText(gameFolder.c_str());
    } else {
      std::filesystem::path startDir =
          getUserHomeDirectory(); // Set the start directory to the user's
                                  // home directory

      if (!startDir.empty()) {

        std::filesystem::path foundPath;
        if (findBorderlandsGameFolder(startDir, foundPath)) {
          log->appendLogMessage("Found Game Folder at " +
                                QString(foundPath.c_str()));
          gamePathLine->setText(foundPath.c_str());
        }
      }
    }

  } catch (const std::filesystem::filesystem_error &e) {
    log->appendLogMessage("!! ERROR !! " + QString(e.what()));
  } catch (const std::exception &e) {
    log->appendLogMessage("!! ERROR !! " + QString(e.what()));
  }

  modsPathLine->setText(modsFolder.c_str());
  handleSearchForModsButton();
}

void Window::handleModsFolderButton() {
  QString path = QFileDialog::getExistingDirectory(
      this, tr("Open Directory"), ".",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  modsPathLine->setText(path);
  handleSearchForModsButton();
}

void Window::handleGameFolderButton() {
  QString path = QFileDialog::getExistingDirectory(
      this, tr("Open Directory"), ".",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  gamePathLine->setText(path);
}

void Window::handleLoadedListSelect() {
  modInfo->setText("Failed to find or open README.md");

  for (auto item : loadedList->selectedItems()) {

    std::filesystem::path dirPath = item->toolTip().toStdString();

    if (!std::filesystem::exists(dirPath) ||
        !std::filesystem::is_directory(dirPath)) {
      return;
    }

    for (const auto &file : std::filesystem::directory_iterator(dirPath)) {
      if (!file.is_regular_file() || file.path().filename() != "README.md") {
        continue;
      }

      QFile f(file.path().c_str());
      if (!f.open(QIODevice::ReadOnly)) {
        return;
      }

      QTextStream stream(&f);
      QString content = stream.readAll();
      f.close();
      modInfo->setText(content);
    }
  }
}

void Window::handleAddModButton() {

  for (auto item : loadedList->selectedItems()) {

    bool dupe = false;
    for (int i = 0; i < appliedList->count(); i++) {
      if (appliedList->item(i)->text() == item->text()) {
        dupe = true;
        break;
      }
    }
    if (dupe) {
      continue;
    }

    QListWidgetItem *tmp = new QListWidgetItem(item->text());
    tmp->setToolTip(item->toolTip());
    appliedList->addItem(tmp);
  }
}

void Window::handleRemoveModButton() {

  for (auto item : appliedList->selectedItems()) {
    delete item;
  }
}

void Window::handleUpModButton() {

  QListWidgetItem *currentItem = appliedList->currentItem();
  if (!currentItem) {
    return; // No item selected
  }

  int currentIndex = appliedList->row(currentItem);

  // Check if the item can be moved up
  if (currentIndex > 0) {
    // Get the item above the current item

    // Swap the items
    appliedList->takeItem(currentIndex);
    appliedList->insertItem(currentIndex - 1, currentItem);

    // Set the current item to the one that was moved
    appliedList->setCurrentItem(currentItem);
  }
}

void Window::handleDownModButton() {

  QListWidgetItem *currentItem = appliedList->currentItem();
  if (!currentItem) {
    return; // No item selected
  }

  int currentIndex = appliedList->row(currentItem);

  // Check if the item can be moved up
  if (currentIndex < appliedList->count()) {
    // Get the item above the current item

    // Swap the items
    appliedList->takeItem(currentIndex);
    appliedList->insertItem(currentIndex + 1, currentItem);

    // Set the current item to the one that was moved
    appliedList->setCurrentItem(currentItem);
  }
}

bool Window::loadMod(Mod mod) {
  switch (mod.checkValid()) {
  case 0:
    log->appendLogMessage("-- Mod " + mod.printQString() +
                          " is valid! Adding to mod list");
    return true;
    break;

  case 1:
    log->appendLogMessage("-- !! Mod is invalid! [Invalid name] !!");
    return false;
    break;

  case 2:
    log->appendLogMessage("-- !! Mod is invalid! [Invalid author] !!");
    return false;
    break;

  case 3:
    log->appendLogMessage("-- !! Mod is invalid! [Invalid version] !!");
    return false;
    break;

  case 4:
    log->appendLogMessage("-- !! Mod is invalid! [Invalid major version] !!");
    return false;
    break;

  case 5:
    log->appendLogMessage("-- !! Mod is invalid! [Invalid minor version] !!");
    return false;
    break;

  case 6:
    log->appendLogMessage("-- !! Mod is invalid! [Invalid path] !!");
    return false;
    break;

  case 12:
    log->appendLogMessage("-- !! Mod is invalid! [No Data folder] !!");
    return false;
    break;

  default:
    log->appendLogMessage(
        "-- !! Mod is invalid! [Unknown error, contact BML dev] !!");
    return false;
    break;
  }
}

void Window::handleSearchForModsButton() {
  loadedList->clear();
  loadedMods.clear();
  appliedList->clear();

  std::filesystem::path modsPath = modsPathLine->text().toStdString();
  log->appendLogMessage("\nSearching for mods at " + modsPathLine->text());

  if (!std::filesystem::exists(modsPath) ||
      !std::filesystem::is_directory(modsPath)) {
    log->appendLogMessage("- Invalid Mods Folder Path \"" +
                          modsPathLine->text() +
                          "\". Directory does not exist "
                          "or is not a directory!");
    return;
  }

  for (const auto &entry : std::filesystem::directory_iterator(modsPath)) {

    if (!std::filesystem::exists(entry.path()) ||
        !std::filesystem::is_directory(entry.path())) {
      continue;
    }

    for (const auto &file : std::filesystem::directory_iterator(entry.path())) {
      if (!file.is_regular_file() || file.path().filename() != "bml.json") {
        continue;
      }

      log->appendLogMessage("\n- Found mod in folder: " +
                            QString(entry.path().filename().c_str()));
      std::ifstream f(file.path());
      if (!f.is_open()) {
        log->appendLogMessage("-- !! Failed to open bml.json !!");
        continue;
      }

      json data;
      try {
        data = json::parse(f);
        f.close();
      } catch (const json::parse_error &e) {
        log->appendLogMessage("-- !! Failed to parse bml.json !! [" +
                              QString(e.what()) + "]");
        continue;
      } catch (const std::exception &e) {
        log->appendLogMessage("-- !! Failed to parse bml.json !! [" +
                              QString(e.what()) + "]");
        continue;
      }

      Mod mod = Mod();

      if (data.contains("name") &&
          data.find("name")->type() == json::value_t::string) {
        mod.setName(data.find("name").value());
      } else {
        log->appendLogMessage("-- !! No valid name in bml.json !!");
      }

      if (data.contains("author") &&
          data.find("author")->type() == json::value_t::string) {
        mod.setAuthor(data.find("author").value());
      } else {
        log->appendLogMessage("-- !! No valid author in bml.json !!");
      }

      if (data.contains("version") &&
          data.find("version")->type() == json::value_t::string) {
        mod.setVersion(data.find("version").value());
      } else {
        log->appendLogMessage("-- !! No valid version in bml.json !!");
      }

      mod.setPath(entry.path());

      bool validDeps = true;
      if (data.contains("dependencies") &&
          data.find("dependencies")->type() == json::value_t::array) {
        log->appendLogMessage("-- Adding dependencies");

        for (auto &dep : data.find("dependencies").value()) {
          Mod depend = Mod();

          if (dep.contains("name") &&
              dep.find("name")->type() == json::value_t::string) {
            depend.setName(dep.find("name").value());
          } else {
            log->appendLogMessage("--- ! Dependency with no valid name !");
          }

          if (dep.contains("author") &&
              dep.find("author")->type() == json::value_t::string) {
            depend.setAuthor(dep.find("author").value());
          } else {
            log->appendLogMessage("--- ! Dependency with no valid author !");
          }

          if (dep.contains("version") &&
              dep.find("version")->type() == json::value_t::string) {
            depend.setVersion(dep.find("version").value());
          } else {
            log->appendLogMessage("--- ! Dependency with no valid version !");
          }
          if (depend.checkValid() == 0 || depend.checkValid() == 6) {
            log->appendLogMessage("--- Dependency added: " +
                                  depend.printQString());
            mod.dependencies.push_back(depend);
          } else {
            log->appendLogMessage("-- !! Broken bml.json dependencies detected "
                                  "!! Ignoring mod in folder \"" +
                                  QString(entry.path().filename().c_str()) +
                                  "\" !!");
            validDeps = false;
          }
        }
      }
      if (!validDeps) {
        continue;
      }

      bool validIncompats = true;
      if (data.contains("incompatibilities") &&
          data.find("incompatibilities")->type() == json::value_t::array) {
        log->appendLogMessage("-- Adding incompatibilities");

        for (auto &incompatible : data.find("incompatibilities").value()) {
          Mod incompat = Mod();

          if (incompatible.contains("name") &&
              incompatible.find("name")->type() == json::value_t::string) {
            incompat.setName(incompatible.find("name").value());
          } else {
            log->appendLogMessage("--- ! Incompatibility with no valid name !");
          }

          if (incompatible.contains("author") &&
              incompatible.find("author")->type() == json::value_t::string) {
            incompat.setAuthor(incompatible.find("author").value());
          } else {
            log->appendLogMessage(
                "--- ! Incompatibility with no valid author !");
          }

          if (incompatible.contains("version") &&
              incompatible.find("version")->type() == json::value_t::string) {
            incompat.setVersion(incompatible.find("version").value());
          } else {
            log->appendLogMessage(
                "--- ! Incompatibility with no valid version !");
          }
          if (incompat.checkValid() == 0 || incompat.checkValid() == 6) {
            log->appendLogMessage("--- Incompatibility added: " +
                                  incompat.printQString());
            mod.incompatibilities.push_back(incompat);
          } else {
            log->appendLogMessage(
                "-- !! Broken bml.json incompatibilities detected "
                "!! Ignoring mod in folder \"" +
                QString(entry.path().filename().c_str()) + "\" !!");
            validIncompats = false;
          }
        }
      }
      if (!validIncompats) {
        continue;
      }

      bool dupe = false;
      for (auto m : loadedMods) {
        if (mod.compare(m) == 0) {
          log->appendLogMessage(
              "-- !! Duplicate mod detected !! This mod is a duplicate of " +
              m.printQString() + ". Ignoring mod in folder \"" +
              QString(entry.path().filename().c_str()) + "\" !!");
          dupe = true;
          break;
        }
      }
      if (dupe) {
        continue;
      }

      if (loadMod(mod)) {
        loadedMods.push_back(mod);
      }
    }
  }

  for (auto mod : loadedMods) {
    QListWidgetItem *item = new QListWidgetItem(mod.print().c_str());
    item->setToolTip(mod.path().c_str());
    loadedList->addItem(item);
  }
}

void Window::handleApplyModsButton() {
  std::vector<Mod> modlist;
  for (int i = 0; i < appliedList->count(); i++) {
    for (auto &mod : loadedMods) {
      if (appliedList->item(i)->toolTip().toStdString() == mod.path()) {
        modlist.push_back(mod);
        break;
      }
    }
  }
  compiler->setModList(modlist);
}

void Window::handleCompileModsButton() {
  compiler->setPath(gamePathLine->text().toStdString());
  compiler->compile();
}

void Window::handleExportLogButton() {
  QString fileName = QFileDialog::getSaveFileName(
      this, tr("Export Log"), "", tr("Text Files (*.txt);;All Files (*)"));
  log->appendLogMessage("\nExporting log to " + fileName);

  if (!fileName.isEmpty()) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream out(&file);
      out << log->exportLog(); // Write log content to the file
      log->appendLogMessage("Wrote to " + fileName);
      file.close();
    } else {
      // Handle the error, e.g., show a message box
      log->appendLogMessage("Failed to open file " + fileName);
      QMessageBox::warning(this, tr("Error"),
                           tr("Unable to open file for writing."));
    }
  }
}

} // namespace BML
