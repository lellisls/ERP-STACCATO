#include "lineedittel.h"

LineEditTel::LineEditTel(QWidget *parent) : QLineEdit(parent) {
  setPlaceholderText("(99)99999-9999");
  connect(this, &QLineEdit::textEdited, this, &LineEditTel::processTel);
}

void LineEditTel::processTel(const QString &value) {
  QString nbr, res = value;

  foreach (QChar c, value) {
    if (c.isNumber()) {
      nbr += c;
    }
  }

  const int size = nbr.size();

  if (nbr.isEmpty()) {
    res = nbr;
  }

  if (size > 0) {
    res = '(';
    res += nbr.at(0);
  }

  if (size > 1) {
    res += nbr.at(1);
  }

  if (size > 2) {
    res += ')';
  }

  if (size < 11) {
    for (int i = 2; i < 6 and i < size; ++i) {
      res += nbr.at(i);
    }

    if (size > 6) {
      res += '-';
    }

    for (int i = 6; i < 10 and i < size; ++i) {
      res += nbr.at(i);
    }

  } else {
    res += nbr.at(2); // + '-';

    for (int i = 3; i < 7 and i < size; ++i) {
      res += nbr.at(i);
    }

    if (size >= 7) {
      res += '-';
    }

    for (int i = 7; i < 11 and i < size; ++i) {
      res += nbr.at(i);
    }
  }

  setText(res);
}

LineEditTel::~LineEditTel() {}
