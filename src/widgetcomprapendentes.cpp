#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "inputdialog.h"
#include "produtospendentes.h"
#include "searchdialog.h"
#include "ui_widgetcomprapendentes.h"
#include "widgetcomprapendentes.h"

WidgetCompraPendentes::WidgetCompraPendentes(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompraPendentes) {
  ui->setupUi(this);

  setupTables();

  connect(ui->checkBoxFiltroPendentes, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroIniciados, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroCompra, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroFaturamento, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroColeta, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroRecebimento, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->checkBoxFiltroEstoque, &QAbstractButton::toggled, this, &WidgetCompraPendentes::montaFiltro);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetCompraPendentes::montaFiltro);

  ui->itemBoxProduto->setSearchDialog(SearchDialog::produto(this));

  connect(ui->itemBoxProduto, &QLineEdit::textChanged, this, &WidgetCompraPendentes::setarDadosAvulso);

  ui->checkBoxFiltroPendentes->setChecked(true);

  QSqlQuery("set lc_time_names = 'pt_BR'").exec();
}

WidgetCompraPendentes::~WidgetCompraPendentes() { delete ui; }

void WidgetCompraPendentes::setarDadosAvulso() {
  if (ui->itemBoxProduto->value().isNull()) {
    ui->doubleSpinBoxQuantAvulso->setValue(0);
    ui->doubleSpinBoxQuantAvulsoCaixas->setValue(0);
    ui->lineEditUn->clear();

    return;
  }

  QSqlQuery query;
  query.prepare("SELECT * FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", ui->itemBoxProduto->value());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando produto: " + query.lastError().text());
    return;
  }

  QString un = query.value("un").toString();

  ui->doubleSpinBoxQuantAvulso->setSingleStep(
        query.value(un.contains("M2") or un.contains("M²") or un.contains("ML") ? "m2cx" : "pccx").toDouble());

  ui->lineEditUn->setText(un);
}

QString WidgetCompraPendentes::updateTables() {
  if (not model.select()) return "Erro lendo tabela produtos pendentes: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetCompraPendentes::setupTables() {
  model.setTable("view_produtos_pendentes");

  if (not model.select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela produtos pendentes: " + model.lastError().text());
  }

  model.setHeaderData("Form", "Form.");
  model.setHeaderData("Quant", "Quant.");
  model.setHeaderData("Un", "Un.");
  model.setHeaderData("Cód Com", "Cód. Com.");

  ui->table->setModel(&model);
  ui->table->resizeColumnsToContents();
}

void WidgetCompraPendentes::on_table_activated(const QModelIndex &index) {
  const QString status = model.data(index.row(), "Status").toString();

  if (status != "PENDENTE") {
    QMessageBox::critical(this, "Erro!", "Produto não está PENDENTE!");
    return;
  }

  const QString codComercial = model.data(index.row(), "Cód Com").toString();

  ProdutosPendentes *produtos = new ProdutosPendentes(this);

  produtos->viewProduto(codComercial, status);
}

void WidgetCompraPendentes::on_groupBoxStatus_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

void WidgetCompraPendentes::montaFiltro() {
  QString filtroCheck;

  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtroCheck += QString(filtroCheck.isEmpty() ? "" : " OR ") + "status = '" + child->text().toUpper() + "'";
    }
  }

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca = textoBusca.isEmpty() ? "" : " AND ((Fornecedor LIKE '%" + textoBusca +
                                                     "%') OR (Descrição LIKE '%" + textoBusca +
                                                     "%') OR (`Cód Com` LIKE '%" + textoBusca + "%'))";

  const QString filtroStatus = QString((filtroCheck + filtroBusca).isEmpty() ? "" : " AND ") + "status != 'CANCELADO'";

  model.setFilter(filtroCheck + filtroBusca + filtroStatus);

  ui->table->resizeColumnsToContents();
}

void WidgetCompraPendentes::on_pushButtonComprarAvulso_clicked() {
  InputDialog *inputDlg = new InputDialog(InputDialog::Carrinho, this);

  if (inputDlg->exec() != InputDialog::Accepted) return;

  QDate dataPrevista = inputDlg->getNextDate();

  QSqlQuery query;
  query.prepare("SELECT * FROM pedido_fornecedor_has_produto WHERE idProduto = :idProduto AND status = 'PENDENTE'");
  query.bindValue(":idProduto", ui->itemBoxProduto->value());

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando produto: " + query.lastError().text());
    return;
  }

  bool ok = query.first() ? atualiza(query) : insere(dataPrevista);

  if (ok) QMessageBox::information(this, "Aviso!", "Produto enviado para compras com sucesso!");
  if (not ok) QMessageBox::critical(this, "Erro!", "Erro ao enviar produto para compras!");

  ui->itemBoxProduto->clear();
}

bool WidgetCompraPendentes::atualiza(const QSqlQuery &query) {
  QSqlQuery query2;
  query2.prepare("UPDATE pedido_fornecedor_has_produto SET quant = :quant WHERE idProduto = :idProduto AND "
                 "status = 'PENDENTE'");
  query2.bindValue(":quant", query.value("quant").toDouble() + ui->doubleSpinBoxQuantAvulso->value());
  query2.bindValue(":idProduto", ui->itemBoxProduto->value());

  if (not query2.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro atualizando pedido_fornecedor_has_produto: " + query2.lastError().text());
    return false;
  }

  return true;
}

bool WidgetCompraPendentes::insere(const QDate &dataPrevista) {
  QSqlQuery query;
  query.prepare("SELECT * FROM produto WHERE idProduto = :idProduto");
  query.bindValue(":idProduto", ui->itemBoxProduto->value());

  if (not query.exec() or not query.first()) {
    QMessageBox::critical(this, "Erro!", "Erro buscando custo do produto: " + query.lastError().text());
    return false;
  }

  QSqlQuery query2;
  query2.prepare(
        "INSERT INTO pedido_fornecedor_has_produto (fornecedor, idProduto, descricao, colecao, quant, un, un2, caixas, "
        "preco, kgcx, formComercial, codComercial, codBarras, dataPrevCompra) VALUES (:fornecedor, :idProduto, "
        ":descricao, :colecao, :quant, :un, :un2, :caixas, :preco, :kgcx, :formComercial, :codComercial, :codBarras, "
        ":dataPrevCompra)");

  query2.bindValue(":fornecedor", query.value("fornecedor"));
  query2.bindValue(":idProduto", query.value("idProduto"));
  query2.bindValue(":descricao", query.value("descricao"));
  query2.bindValue(":colecao", query.value("colecao"));
  query2.bindValue(":quant", ui->doubleSpinBoxQuantAvulso->value());
  query2.bindValue(":un", query.value("un"));
  query2.bindValue(":un2", query.value("un2"));
  query2.bindValue(":caixas", ui->doubleSpinBoxQuantAvulsoCaixas->value());
  query2.bindValue(":preco", query.value("custo").toDouble() * ui->doubleSpinBoxQuantAvulso->value());
  query2.bindValue(":kgcx", query.value("kgcx"));
  query2.bindValue(":formComercial", query.value("formComercial"));
  query2.bindValue(":codComercial", query.value("codComercial"));
  query2.bindValue(":codBarras", query.value("codBarras"));
  query2.bindValue(":dataPrevCompra", dataPrevista);

  if (not query2.exec()) {
    QMessageBox::critical(this, "Erro!",
                          "Erro inserindo dados em pedido_fornecedor_has_produto: " + query2.lastError().text());
    return false;
  }

  return true;
}

void WidgetCompraPendentes::on_doubleSpinBoxQuantAvulsoCaixas_valueChanged(const double &value) {
  ui->doubleSpinBoxQuantAvulso->setValue(value * ui->doubleSpinBoxQuantAvulso->singleStep());
}

void WidgetCompraPendentes::on_doubleSpinBoxQuantAvulso_valueChanged(const double &value) {
  ui->doubleSpinBoxQuantAvulsoCaixas->setValue(value / ui->doubleSpinBoxQuantAvulso->singleStep());
}
