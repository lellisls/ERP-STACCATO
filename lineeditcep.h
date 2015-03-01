#ifndef LINEEDITCEP_H
#define LINEEDITCEP_H

#include <QLineEdit>

class LineEditCEP : public QLineEdit {
  Q_OBJECT

public:
  explicit LineEditCEP(QWidget *parent);
  ~LineEditCEP();
  bool isValid() const;

  QString getValue() const;
  void setValue(QString value);

private:
  Q_PROPERTY(QString value READ getValue WRITE setValue STORED false)
};

#endif // LINEEDITCEP_H
