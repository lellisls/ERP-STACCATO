#ifndef IMPORTAEXPORT_H
#define IMPORTAEXPORT_H

#include <QObject>
#include <QString>

class ImportaExport : public QObject {
  Q_OBJECT

signals:
  void progressRangeChanged(int max);
  void progressValueChanged(int val);
  void progressTextChanged(QString str);
  void progressFinished();

public slots:
  void cancel();

public:
  ImportaExport();
  ~ImportaExport();
  QString importar(QString file, int validade);
  int buscarCadastrarFornecedor(QString fornecedor);
  int getSize(QString file);
  bool canceled;
};

#endif // IMPORTAEXPORT_H
