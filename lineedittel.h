#ifndef LINEEDITTEL_H
#define LINEEDITTEL_H

#include <QLineEdit>
#include <QString>

class LineEditTel : public QLineEdit {
    Q_OBJECT

  public:
    explicit LineEditTel(QWidget *parent);
    void processTel(QString value);
    ~LineEditTel();
};

#endif // LINEEDITTEL_H
