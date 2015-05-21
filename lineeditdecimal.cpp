#include <QEvent>
#include <QLocale>

#include "lineeditdecimal.h"

LineEditDecimal::LineEditDecimal(QWidget *parent) : QLineEdit(parent), bottom(0), top(99999999) {
  connect(this, &QLineEdit::textEdited, this, &LineEditDecimal::processDecimal);
  setAlignment(Qt::AlignRight);
  setProperty("value", 0.0);
}

double LineEditDecimal::getValue() const { return QLocale(QLocale::Portuguese).toDouble(text()); }

void LineEditDecimal::setValue(double value) {
  setText(QLocale(QLocale::Portuguese).toString(value, 'f', 2));
}

double LineEditDecimal::getBottom() const { return bottom; }

void LineEditDecimal::setBottom(double value) { bottom = value; }

double LineEditDecimal::getTop() const { return top; }

void LineEditDecimal::setTop(double value) { top = value; }

void LineEditDecimal::processDecimal(QString value) {
  QString nbr, res;
  if (value.size() > 0 and value.at(0) == '-' and bottom < 0) {
    res += '-';
  }
  foreach (QChar c, value) {
    if (c.isNumber()) {
      nbr += c;
    }
  }
  for (int i = 0; i < nbr.size() - 2; i++) {
    if ((res + nbr[i]).toDouble() < top)
      res.append(nbr[i]);
  }
  if (nbr.size() > 2) {
    res.append(',');
  }
  if (nbr.size() > 1) {
    if ((res + nbr[nbr.size() - 2]).toDouble() < top)
      res.append(nbr[nbr.size() - 2]);
  }
  if (nbr.size() > 0) {
    if ((res + nbr[nbr.size() - 1]).toDouble() < top)
      res.append(nbr[nbr.size() - 1]);
  }
  setText(res);
}
