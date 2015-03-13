#ifndef IMPORTAEXPORT_H
#define IMPORTAEXPORT_H

#include <QString>

class ImportaExport
{
  public:
    ImportaExport();
    ~ImportaExport();
    QString importar(QString file);
    int buscarCadastrarFornecedor(QString id, QString fornecedor);
};

#endif // IMPORTAEXPORT_H
