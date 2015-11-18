#include "src/widgetvenda.h"
#include "doubledelegate.h"
#include "orcamentoproxymodel.h"
#include "ui_widgetvenda.h"
#include "usersession.h"
#include "venda.h"

#include <QMessageBox>
#include <QSqlError>

WidgetVenda::WidgetVenda(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetVenda) {
  ui->setupUi(this);

  setupTables();

  ui->radioButtonVendLimpar->click();

  if (UserSession::getTipoUsuario() != "ADMINISTRADOR") {
    ui->groupBoxLojas->hide();
  }

  if (UserSession::getTipoUsuario() == "VENDEDOR") {
    ui->radioButtonVendProprios->click();
  }

  connect(ui->radioButtonVendLimpar, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->radioButtonVendProprios, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaPendente, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaIniciado, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaEmCompra, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaEmFaturamento, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaEmColeta, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaEmRecebimento, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaEstoque, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);
  connect(ui->checkBoxVendaFinalizado, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltroVendas);

  QSqlQuery query("SELECT * FROM loja WHERE descricao != 'Geral'");

  ui->comboBoxLojas->addItem("");

  while (query.next()) {
    ui->comboBoxLojas->addItem(query.value("sigla").toString(), query.value("idLoja"));
  }

  ui->comboBoxLojas->setCurrentValue(UserSession::getLoja());
}

WidgetVenda::~WidgetVenda() { delete ui; }

void WidgetVenda::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  modelVendas = new SqlTableModel(this);
  modelVendas->setTable("view_venda");

  ui->tableVendas->setModel(new OrcamentoProxyModel(modelVendas, "Dias restantes", this));
  ui->tableVendas->setItemDelegateForColumn("Bruto", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Líquido", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Frete", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Total R$", doubledelegate);
  ui->tableVendas->sortByColumn("Código");
}

void WidgetVenda::montaFiltroVendas() {
  QString loja = ui->groupBoxLojas->isVisible() ? ui->comboBoxLojas->currentText() : UserSession::getSiglaLoja();

  const QString filtro = ui->radioButtonVendLimpar->isChecked()
                         ? "(Código LIKE '%" + loja + "%')"
                         : "(Código LIKE '%" + loja + "%') AND Vendedor = '" + UserSession::getNome() + "'";

  QString filtro2;

  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtro2 += filtro2.isEmpty() ? "status = '" + child->text().toUpper() + "'"
                                   : " OR status = '" + child->text().toUpper() + "'";
    }
  }

  modelVendas->setFilter(filtro2.isEmpty() ? filtro : filtro + " AND (" + filtro2 + ")");

  ui->tableVendas->resizeColumnsToContents();
}

void WidgetVenda::on_groupBoxStatusVenda_toggled(bool enabled) {
  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
  }

  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    child->setChecked(enabled);
  }
}

void WidgetVenda::on_comboBoxLojas_currentTextChanged(const QString &) { montaFiltroVendas(); }

bool WidgetVenda::updateTables() {
  if (not modelVendas->select()) {
    QMessageBox::critical(this, "Erro!", "Erro lendo tabela vendas: " + modelVendas->lastError().text());
    return false;
  }

  ui->tableVendas->resizeColumnsToContents();

  return true;
}

void WidgetVenda::on_lineEditBuscaVendas_textChanged(const QString &text) {
  if (text.isEmpty()) {
    montaFiltroVendas();
    return;
  }

  modelVendas->setFilter("(Código LIKE '%" + text + "%') OR (Vendedor LIKE '%" + text + "%') OR (Cliente LIKE '%" +
                         text + "%')");

  ui->tableVendas->resizeColumnsToContents();
}

void WidgetVenda::on_tableVendas_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelVendas->data(index.row(), "Código"));
}
