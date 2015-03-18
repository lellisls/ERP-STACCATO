#ifndef IMPORTAPORTINARI_H
#define IMPORTAPORTINARI_H

#include <QObject>
#include <QString>

class ImportaPortinari : public QObject {
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
    QString importar(QString file, int validade);
    int buscarCadastrarFornecedor(QString fornecedor);

  private:
    bool canceled;
};

#endif // IMPORTAPORTINARI_H
