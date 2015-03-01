#include <QDebug>

#include "combobox.h"

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent) {}

ComboBox::~ComboBox() {}

QVariant ComboBox::getCurrentValue() const {
  return (currentData());
}

bool ComboBox::setCurrentValue(QVariant value) {
  for (int idx = 0; idx < count(); ++idx) {
    if (value.toInt() == itemData(idx).toInt()) {
      setCurrentIndex(idx);
      return true;
    }
  }
  return false;
}
