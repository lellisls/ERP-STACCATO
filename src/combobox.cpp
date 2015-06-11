#include <QDebug>

#include "combobox.h"

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent) {}

ComboBox::~ComboBox() {}

QVariant ComboBox::getCurrentValue() const { return (currentData()); }

bool ComboBox::setCurrentValue(QVariant value) {
  for (int index = 0; index < count(); ++index) {
    if (value.toInt() == itemData(index).toInt()) {
      setCurrentIndex(index);
      return true;
    }
  }

  return false;
}
