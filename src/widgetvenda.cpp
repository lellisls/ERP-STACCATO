#include <QDate>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>

#include "doubledelegate.h"
#include "orcamentoproxymodel.h"
#include "ui_widgetvenda.h"
#include "usersession.h"
#include "venda.h"
#include "vendasmes.h"
#include "widgetvenda.h"

WidgetVenda::WidgetVenda(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetVenda) {
  ui->setupUi(this);

  connect(ui->checkBoxCancelado, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxDevolucao, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmColeta, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmCompra, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmFaturamento, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEmRecebimento, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxEstoque, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxFinalizado, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxIniciado, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->checkBoxPendente, &QCheckBox::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->comboBoxLojas, &ComboBox::currentTextChanged, this, &WidgetVenda::montaFiltro);
  connect(ui->lineEditBusca, &QLineEdit::textChanged, this, &WidgetVenda::montaFiltro);
  connect(ui->radioButtonProprios, &QRadioButton::toggled, this, &WidgetVenda::montaFiltro);
  connect(ui->radioButtonTodos, &QRadioButton::toggled, this, &WidgetVenda::montaFiltro);

  QSqlQuery query("SELECT descricao, idLoja FROM loja WHERE desativado = FALSE");

  while (query.next()) {
    ui->comboBoxLojas->addItem(query.value("descricao").toString(), query.value("idLoja"));
  }

  ui->comboBoxLojas->setCurrentValue(UserSession::idLoja());

  if (UserSession::tipoUsuario() != "ADMINISTRADOR") ui->groupBoxLojas->hide();
  ui->radioButtonTodos->click();
  if (UserSession::tipoUsuario() == "VENDEDOR") ui->radioButtonProprios->click();
  if (UserSession::tipoUsuario() != "VENDEDOR") ui->pushButtonCalcularTotal->hide();
}

WidgetVenda::~WidgetVenda() { delete ui; }

void WidgetVenda::setupTables() {
  DoubleDelegate *doubledelegate = new DoubleDelegate(this);

  model.setTable("view_venda");

  ui->table->setModel(new OrcamentoProxyModel(&model, this));
  ui->table->setItemDelegateForColumn("Bruto", doubledelegate);
  ui->table->setItemDelegateForColumn("Líquido", doubledelegate);
  ui->table->setItemDelegateForColumn("Frete", doubledelegate);
  ui->table->setItemDelegateForColumn("Total R$", doubledelegate);
  ui->table->sortByColumn("Código");
}

void WidgetVenda::montaFiltro() {
  QString sigla = UserSession::fromLoja("sigla");

  if (ui->groupBoxLojas->isVisible() and not ui->comboBoxLojas->currentText().isEmpty()) {
    QSqlQuery query;
    query.prepare("SELECT sigla FROM loja WHERE descricao = :descricao");
    query.bindValue(":descricao", ui->comboBoxLojas->currentText());

    if (not query.exec() or not query.first()) {
      QMessageBox::critical(this, "Erro!", "Erro buscando sigla da loja: " + query.lastError().text());
      return;
    }

    sigla = query.value("sigla").toString();
  }

  const QString filtroLoja = "(Código LIKE '%" + sigla + "%')";

  const QString filtroRadio = ui->radioButtonTodos->isChecked() ? "" : " AND Vendedor = '" + UserSession::nome() + "'";

  QString filtroCheck;

  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtroCheck += filtroCheck.isEmpty() ? "status = '" + child->text().toUpper() + "'"
                                           : " OR status = '" + child->text().toUpper() + "'";
    }
  }

  filtroCheck = filtroCheck.isEmpty() ? "" : " AND (" + filtroCheck + ")";

  const QString textoBusca = ui->lineEditBusca->text();

  const QString filtroBusca = textoBusca.isEmpty() ? "" : " AND ((Código LIKE '%" + textoBusca +
                                                     "%') OR (Vendedor LIKE '%" + textoBusca +
                                                     "%') OR (Cliente LIKE '%" + textoBusca + "%'))";

  model.setFilter(filtroLoja + filtroRadio + filtroCheck + filtroBusca);

  ui->table->resizeColumnsToContents();
}

void WidgetVenda::on_groupBoxStatus_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatus->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

QString WidgetVenda::updateTables() {
  if (model.tableName().isEmpty()) {
    setupTables();
    montaFiltro();
  }

  if (not model.select()) return "Erro lendo tabela vendas: " + model.lastError().text();

  ui->table->resizeColumnsToContents();

  return QString();
}

void WidgetVenda::on_table_activated(const QModelIndex &index) {
  Venda *vendas = new Venda(this);
  vendas->viewRegisterById(model.data(index.row(), "Código"));
}

void WidgetVenda::on_pushButtonCalcularTotal_clicked() {
  VendasMes *vendas = new VendasMes(UserSession::idUsuario(), this);
  vendas->show();
}

// NOTE: verificar como lidar com brinde/reposicao
// NOTE: cancelamento de pedido: se todos os itens estiverem pendentes marcar status cancelado, senao fazer processo
// inverso de estorno
