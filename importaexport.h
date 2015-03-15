#ifndef IMPORTAEXPORT_H
#define IMPORTAEXPORT_H

#include <QObject>
#include <QString>

class ImportaExport : public QObject {
  Q_OBJECT

signals:
  progressRangeChanged(int max);
  progressValueChanged(int val);
  progressTextChanged(QString str);
  progressFinished();

public slots:
  void cancel();

public:
  ImportaExport();
  ~ImportaExport();
  QString importar(QString file);
  int buscarCadastrarFornecedor(QString id, QString fornecedor);
  int getSize(QString file);
  bool canceled;
};

#endif // IMPORTAEXPORT_H
