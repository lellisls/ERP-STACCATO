#ifndef IMPORTAPORTINARI_H
#define IMPORTAPORTINARI_H

#include <QString>

class ImportaPortinari {
  public:
    ImportaPortinari();
    ~ImportaPortinari();
    QString importar(QString file, int idFornecedor);
};

#endif // IMPORTAPORTINARI_H
