#ifndef LINEEDITCEP_H
#define LINEEDITCEP_H

#include <QLineEdit>

class LineEditCEP : public QLineEdit {
  Q_OBJECT

public:
  explicit LineEditCEP(QWidget *parent);
  ~LineEditCEP();
  bool isValid() const;

private:
  Q_PROPERTY(QString value READ getValue WRITE setValue STORED false)

  QString getValue() const;
  void setValue(const QString &value);
};

#endif // LINEEDITCEP_H
