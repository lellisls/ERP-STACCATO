#ifndef IMPORTAPORTINARI_H
#define IMPORTAPORTINARI_H

#include <QObject>
#include <QString>

class ImportaPortinari : public QObject {
  Q_OBJECT

signals:
  void progressRangeChanged(int max);
  void progressValueChanged(int val);
  void progressTextChanged(QString str);
  void progressFinished();

public slots:
  void cancel();

public:
  ImportaPortinari();
  ~ImportaPortinari();
  QString importar(QString file, int validade);
  int buscarCadastrarFornecedor(QString fornecedor);

private:
  bool canceled;
};

#endif // IMPORTAPORTINARI_H
