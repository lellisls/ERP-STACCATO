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
    QString importar(QString file, int validade);
    int buscarCadastrarFornecedor(QString fornecedor);

  private:
    bool canceled;
};

#endif // IMPORTAEXPORT_H
