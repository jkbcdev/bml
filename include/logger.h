#ifndef LOGGER_H
#define LOGGER_H

#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace BML {

class Logger : public QWidget {
  Q_OBJECT

public:
  explicit Logger(QWidget *parent = nullptr);

public slots:
  void appendLogMessage(const QString &message);
  QString exportLog();

private:
  QTextEdit *textEdit;
};

} // namespace BML

#endif // LOGGER_H
