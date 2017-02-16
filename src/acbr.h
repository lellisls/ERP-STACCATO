#ifndef ACBR_H
#define ACBR_H

#include <QObject>

class ACBr : public QObject {
  Q_OBJECT

public:
  explicit ACBr(QObject *parent = 0);
  static bool enviarComando(const QString &comando, QString &resposta);
  static bool gerarDanfe(const int idNFe);
  static bool gerarDanfe(const QByteArray &fileContent, QString &resposta, const bool openFile = true);

private:
  static bool abrirPdf(QString &resposta);
};

#endif // ACBR_H
