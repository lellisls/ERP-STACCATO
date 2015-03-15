#ifndef IMPORTAPORTINARI_H
#define IMPORTAPORTINARI_H

#include <QObject>
#include <QString>

class ImportaPortinari  : public QObject {
  Q_OBJECT

signals:
  progressRangeChanged(int max);
  progressValueChanged(int val);
  progressTextChanged(QString str);
  progressFinished();

public slots:
  void cancel();
public:
  ImportaPortinari();
  ~ImportaPortinari();
  QString importar(QString file);
  int buscarCadastrarFornecedor(QString column0);

private:
  bool canceled;
};

#endif // IMPORTAPORTINARI_H
