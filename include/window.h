#ifndef WINDOW_H
#define WINDOW_H

#include "compiler.h"
#include "logger.h"
#include "mod.h"
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <qmarkdowntextedit.h>
#include <vector>

namespace BML {

class Window : public QMainWindow {
  Q_OBJECT
public:
  explicit Window(QWidget *parent = nullptr);
private slots:
  void handleModsFolderButton();
  void handleGameFolderButton();

  void handleAddModButton();
  void handleRemoveModButton();

  void handleUpModButton();
  void handleDownModButton();

  void handleLoadedListSelect();

  void handleSearchForModsButton();
  void handleApplyModsButton();
  void handleCompileModsButton();

  void handleExportLogButton();

  bool loadMod(Mod mod);

private:
  QPushButton *modsPathButton;
  QLineEdit *modsPathLine;
  QLabel *modsPathLabel;

  QPushButton *gamePathButton;
  QLineEdit *gamePathLine;
  QLabel *gamePathLabel;

  QMarkdownTextEdit *modInfo;
  QLabel *modInfoLabel;

  QListWidget *loadedList;
  QLabel *loadedListLabel;

  QListWidget *appliedList;
  QLabel *appliedListLabel;

  QPushButton *addModButton;
  QPushButton *removeModButton;
  QPushButton *upModButton;
  QPushButton *downModButton;

  QPushButton *searchForModsButton;
  QPushButton *applyModsButton;
  QPushButton *compileModsButton;

  QPushButton *exportLogButton;

  Logger *log;
  QLabel *logLabel;

  Compiler *compiler;

  QLabel *bmlLabel;

  std::vector<Mod> loadedMods;

  const char *versionNum = "0.1";
};

} // namespace BML

#endif // WINDOW_H
