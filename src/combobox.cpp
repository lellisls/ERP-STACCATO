#include "combobox.h"

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent) {}

QVariant ComboBox::getCurrentValue() const { return (currentData()); }

bool ComboBox::setCurrentValue(const QVariant &value) {
  for (int index = 0, size = count(); index < size; ++index) {
    if (value == itemData(index)) {
      setCurrentIndex(index);
      return true;
    }
  }

  return false;
}
