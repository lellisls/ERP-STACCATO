#include <QDebug>

#include "lineeditcep.h"

LineEditCEP::LineEditCEP(QWidget *parent) : QLineEdit(parent) { setProperty("value", ""); }

bool LineEditCEP::isValid() const { return (text().size() == 9); }

LineEditCEP::~LineEditCEP() {}

QString LineEditCEP::getValue() const {
  if (not isValid()) {
    return QString();
  }

  return text();
}

void LineEditCEP::setValue(QString value) { setText(value); }
