#include "lineeditcep.h"

LineEditCEP::LineEditCEP(QWidget *parent) : QLineEdit(parent) { setProperty("value", ""); }

bool LineEditCEP::isValid() const { return (text().size() == 9); }

QString LineEditCEP::getValue() const { return isValid() ? text() : QString(); }

void LineEditCEP::setValue(const QString &value) { setText(value); }
