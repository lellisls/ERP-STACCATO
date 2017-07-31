#ifndef LINEEDITCEP_H
#define LINEEDITCEP_H

#include <QLineEdit>

class LineEditCEP : public QLineEdit {
  Q_OBJECT

public:
  explicit LineEditCEP(QWidget *parent);
  ~LineEditCEP() = default;
  bool isValid() const;

private:
  QString getValue() const;
  void setValue(const QString &value);
};

#endif // LINEEDITCEP_H
