#include "combobox.h"

ComboBox::ComboBox(QWidget *parent) : QComboBox(parent) {}

ComboBox::~ComboBox() {}

QVariant ComboBox::getCurrentValue() const { return (currentData()); }

bool ComboBox::setCurrentValue(const QVariant &value) {
  for (int index = 0, size = count(); index < size; ++index) {
    if (value.toInt() == itemData(index).toInt()) {
      setCurrentIndex(index);
      return true;
    }
  }

  return false;
}
