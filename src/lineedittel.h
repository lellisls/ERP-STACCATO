#ifndef LINEEDITTEL_H
#define LINEEDITTEL_H

#include <QLineEdit>

class LineEditTel : public QLineEdit {
    Q_OBJECT

  public:
    explicit LineEditTel(QWidget *parent);
    void processTel(const QString &value);
    ~LineEditTel();
};

#endif // LINEEDITTEL_H
