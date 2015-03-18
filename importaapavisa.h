#ifndef IMPORTAAPAVISA_H
#define IMPORTAAPAVISA_H

#include <QObject>
#include <QString>

class ImportaApavisa : public QObject
{
    Q_OBJECT

  signals:
    progressRangeChanged(int max);
    progressValueChanged(int val);
    progressTextChanged(QString str);
    progressFinished();

  public slots:
    void cancel();

  public:
    ImportaApavisa();
    ~ImportaApavisa();
    QString importar(QString file, int validade);
    int buscarCadastrarFornecedor(QString column0);

  private:
    bool canceled;
};

#endif // IMPORTAAPAVISA_H
