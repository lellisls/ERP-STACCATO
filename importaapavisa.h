#ifndef IMPORTAAPAVISA_H
#define IMPORTAAPAVISA_H

#include <QString>

class ImportaApavisa
{
  public:
    ImportaApavisa();
    ~ImportaApavisa();
    QString importar(QString file, int idFornecedor);
};

#endif // IMPORTAAPAVISA_H
