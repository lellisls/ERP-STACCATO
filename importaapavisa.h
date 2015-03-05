#ifndef IMPORTAAPAVISA_H
#define IMPORTAAPAVISA_H

#include <QString>

class ImportaApavisa
{
  public:
    ImportaApavisa();
    ~ImportaApavisa();
    QString importar(QString file);
    int buscarCadastrarFornecedor(QString column0);
};

#endif // IMPORTAAPAVISA_H
