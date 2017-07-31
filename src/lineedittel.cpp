#include "lineedittel.h"

LineEditTel::LineEditTel(QWidget *parent) : QLineEdit(parent) {
  setPlaceholderText("(99)99999-9999");
  connect(this, &QLineEdit::textEdited, this, &LineEditTel::processTel);
}

void LineEditTel::processTel(const QString &value) {
  QString temp;
  QString tel;

  for (auto const &c : value) {
    if (c.isNumber()) temp += c;
  }

  const int size = temp.size();

  if (size > 0) tel = '(' + temp.at(0);
  if (size > 1) tel += temp.at(1);
  if (size > 2) tel += ')';

  if (size < 11) {
    for (int i = 2; i < 6 and i < size; ++i) tel += temp.at(i);
    if (size > 6) tel += '-';
    for (int i = 6; i < 10 and i < size; ++i) tel += temp.at(i);

  } else {
    tel += temp.at(2); // + '-';
    for (int i = 3; i < 7 and i < size; ++i) tel += temp.at(i);
    if (size >= 7) tel += '-';
    for (int i = 7; i < 11 and i < size; ++i) tel += temp.at(i);
  }

  setText(tel);
}
