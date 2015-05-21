#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "mainwindow.h"
#include "nfe.h"
#include "recebimentosfornecedor.h"
#include "ui_recebimentosfornecedor.h"

RecebimentosFornecedor::RecebimentosFornecedor(QWidget *parent)
  : QDialog(parent), ui(new Ui::RecebimentosFornecedor) {
  ui->setupUi(this);

  modelRecebimentos.setTable("pedidotransportadora");
  modelRecebimentos.setEditStrategy(QSqlTableModel::OnManualSubmit);
  if (not modelRecebimentos.select()) {
    qDebug() << "Failed to populate TableRecebimentos! " << modelRecebimentos.lastError();
  }

  ui->tableRecebimentosForncecedor->setModel(&modelRecebimentos);

  ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());

  show();
}

RecebimentosFornecedor::~RecebimentosFornecedor() { delete ui; }

void RecebimentosFornecedor::on_pushButtonSalvar_clicked() {
  if (ui->checkBoxEntregue->isChecked()) {
    QSqlQuery qry;
    if (not qry.exec("UPDATE pedidotransportadora SET status = 'RECEBIDO' WHERE idPedido = '" + idPedido +
                     "' AND tipo = 'fornecedor'")) {
      qDebug() << "Erro ao marcar como recebido: " << qry.lastError();
    }

    // gerar NFe
    //    NFe nota(idPedido, this);
    //    qDebug() << nota.TXT();
    //    qDebug() << "arquivo: " << nota.getArquivo();
    //    if(not qry.exec("SET @xml = LOAD_FILE('"+ nota.getArquivo() +"')")){
    //      qDebug() << "Erro ao ler arquivo xml: " << qry.lastError();
    //      qDebug() << "qry: " << qry.lastQuery();
    //    }

    //    if(not qry.exec("INSERT INTO nfe (NFe, idVenda, idLoja, chaveAcesso) VALUES (@xml, '"+ idPedido +"',
    //    1, '"+ nota.getChaveAcesso() +"')")){
    //      qDebug() << "Erro ao inserir NFe na tabela: " << qry.lastError();
    //      qDebug() << "qry: " << qry.lastQuery();
    //    }

  } else {
    QSqlQuery qry;
    if (not qry.exec("UPDATE pedidotransportadora SET status = 'PENDENTE' WHERE idPedido = '" + idPedido +
                     "' AND tipo = 'fornecedor'")) {
      qDebug() << "Erro ao marcar como não recebido: " << qry.lastError();
    }
  }

  if (MainWindow *window = qobject_cast<MainWindow *>(parentWidget())) {
    window->updateTables();
  }

  close();
}

void RecebimentosFornecedor::on_pushButtonCancelar_clicked() { close(); }

void RecebimentosFornecedor::viewRecebimento(QString idPedido) {
  this->idPedido = idPedido;
  modelRecebimentos.setFilter("idPedido = '" + idPedido + "' AND tipo = 'fornecedor'");
}
