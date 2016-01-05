#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "orcamentoproxymodel.h"
#include "ui_widgetvenda.h"
#include "usersession.h"
#include "venda.h"
#include "widgetvenda.h"

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

  connect(ui->radioButtonVendLimpar, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->radioButtonVendProprios, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaPendente, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaIniciado, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaEmCompra, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaEmFaturamento, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaEmColeta, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaEmRecebimento, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaEstoque, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxVendaFinalizado, &QAbstractButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->lineEditBuscaVendas, &QLineEdit::textChanged, this, &WidgetVenda::montaFiltro);
  connect(ui->comboBoxLojas, &QComboBox::currentTextChanged, this, &WidgetVenda::montaFiltro);

  QSqlQuery query("SELECT * FROM loja WHERE descricao != 'Geral' AND desativado = FALSE");

  ui->comboBoxLojas->addItem("");

  // NOTE: verificar uma forma melhor de registrar a loja 'Geral'
  while (query.next()) {
    ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));
  }

  ui->comboBoxLojas->setCurrentValue(UserSession::getLoja());
}

WidgetVenda::~WidgetVenda() { delete ui; }

void WidgetVenda::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  modelVendas.setTable("view_venda");

  ui->tableVendas->setModel(new OrcamentoProxyModel(&modelVendas, "Dias restantes", this));
  ui->tableVendas->setItemDelegateForColumn("Bruto", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Líquido", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Frete", doubledelegate);
  ui->tableVendas->setItemDelegateForColumn("Total R$", doubledelegate);
  ui->tableVendas->sortByColumn("Código");
}

void WidgetVenda::montaFiltro() {
  QString textoBusca = ui->lineEditBuscaVendas->text();

  QString filtroBusca = "((Código LIKE '%" + textoBusca + "%') OR (Vendedor LIKE '%" + textoBusca +
                        "%') OR (Cliente LIKE '%" + textoBusca + "%'))";

  const QString loja =
      ui->groupBoxLojas->isVisible() ? ui->comboBoxLojas->currentText() : UserSession::getFromLoja("loja.descricao");

  const QString filtro = ui->radioButtonVendLimpar->isChecked()
                         ? "(Loja LIKE '%" + loja + "')"
                         : "(Loja LIKE '%" + loja + "') AND Vendedor = '" + UserSession::getNome() + "'";
  QString filtro2;

  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtro2 += filtro2.isEmpty() ? "status = '" + child->text().toUpper() + "'"
                                   : " OR status = '" + child->text().toUpper() + "'";
    }
  }

  modelVendas.setFilter(filtro + (filtro2.isEmpty() ? "" : " AND (" + filtro2 + ")") +
                        (textoBusca.isEmpty() ? "" : " AND " + filtroBusca));

  ui->tableVendas->resizeColumnsToContents();
}

void WidgetVenda::on_groupBoxStatusVenda_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatusVenda->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

QString WidgetVenda::updateTables() {
  if (not modelVendas.select()) return "Erro lendo tabela vendas: " + modelVendas.lastError().text();

  ui->tableVendas->resizeColumnsToContents();

  return QString();
}

void WidgetVenda::on_tableVendas_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(modelVendas.data(index.row(), "Código"));
}

// NOTE: verificar como lidar com brinde/reposicao
// NOTE: cancelamento de pedido: se todos os itens estiverem pendentes marcar status cancelado, senao fazer processo
// inverso de estorno
