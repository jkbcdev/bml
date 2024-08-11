#include "logger.h"
#include <QScrollBar>

namespace BML {

Logger::Logger(QWidget *parent)
    : QWidget(parent), textEdit(new QTextEdit(this)) {
  textEdit->setReadOnly(true);
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(textEdit);
  setLayout(layout);
}

void Logger::appendLogMessage(const QString &message) {
  textEdit->append(message);
  textEdit->verticalScrollBar()->setValue(
      textEdit->verticalScrollBar()->maximum());
}

QString Logger::exportLog() { return textEdit->toPlainText(); }

} // namespace BML
