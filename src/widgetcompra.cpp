#include <QSqlError>

#include "produtospendentes.h"
#include "ui_widgetcompra.h"
#include "usersession.h"
#include "widgetcompra.h"

WidgetCompra::WidgetCompra(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCompra) {
  ui->setupUi(this);

  setupTables();

  if (UserSession::getTipoUsuario() == "VENDEDOR") ui->labelPedidosCompra->hide();

  connect(ui->checkBoxFiltroPendentes, &QAbstractButton::toggled, this, &WidgetCompra::montaFiltro);
  connect(ui->checkBoxFiltroIniciados, &QAbstractButton::toggled, this, &WidgetCompra::montaFiltro);
  connect(ui->checkBoxFiltroCompra, &QAbstractButton::toggled, this, &WidgetCompra::montaFiltro);
  connect(ui->checkBoxFiltroFaturamento, &QAbstractButton::toggled, this, &WidgetCompra::montaFiltro);
  connect(ui->checkBoxFiltroColeta, &QAbstractButton::toggled, this, &WidgetCompra::montaFiltro);
  connect(ui->checkBoxFiltroRecebimento, &QAbstractButton::toggled, this, &WidgetCompra::montaFiltro);
  connect(ui->checkBoxFiltroEstoque, &QAbstractButton::toggled, this, &WidgetCompra::montaFiltro);
  connect(ui->lineEditBuscaProdutosPend, &QLineEdit::textChanged, this, &WidgetCompra::montaFiltro);

  ui->checkBoxFiltroPendentes->setChecked(true);

  QSqlQuery("set lc_time_names = 'pt_BR'").exec();
}

WidgetCompra::~WidgetCompra() { delete ui; }

void WidgetCompra::setupTables() {
  modelProdPend.setTable("view_produtos_pendentes");

  modelProdPend.setHeaderData("Form", "Form.");
  modelProdPend.setHeaderData("Quant", "Quant.");
  modelProdPend.setHeaderData("Un", "Un.");
  modelProdPend.setHeaderData("Cód Com", "Cód. Com.");

  ui->tableProdutosPend->setModel(&modelProdPend);
}

QString WidgetCompra::updateTables() {
  switch (ui->tabWidget->currentIndex()) {
    case 0: { // Pendentes
        if (not modelProdPend.select()) return "Erro lendo tabela produtos pendentes: " + modelProdPend.lastError().text();

        ui->tableProdutosPend->resizeColumnsToContents();
        break;
      }

    case 1: { // Gerar Compra
        ui->widgetGerar->updateTables();
        break;
      }

    case 2: { // Confirmar
        ui->widgetConfirmar->updateTables();
        break;
      }

    case 3: { // Faturar
        ui->widgetFaturar->updateTables();
        break;
      }
  }

  return QString();
}

void WidgetCompra::on_tableProdutosPend_activated(const QModelIndex &index) {
  ProdutosPendentes *produtos = new ProdutosPendentes(this);

  const QString codComercial = modelProdPend.data(index.row(), "Cód Com").toString();
  const QString status = modelProdPend.data(index.row(), "Status").toString();

  produtos->viewProduto(codComercial, status);
}

void WidgetCompra::on_tabWidget_currentChanged(const int &) { updateTables(); }

void WidgetCompra::on_groupBoxStatusPendentes_toggled(const bool &enabled) {
  for (auto const &child : ui->groupBoxStatusPendentes->findChildren<QCheckBox *>()) {
    child->setEnabled(true);
    child->setChecked(enabled);
  }
}

void WidgetCompra::montaFiltro() {
  QString filtro;

  for (auto const &child : ui->groupBoxStatusPendentes->findChildren<QCheckBox *>()) {
    if (child->isChecked()) {
      filtro += filtro.isEmpty() ? "status = '" + child->text().toUpper() + "'"
                                 : " OR status = '" + child->text().toUpper() + "'";
    }
  }

  const QString textoBusca = ui->lineEditBuscaProdutosPend->text();

  const QString filtroBusca = "((Fornecedor LIKE '%" + textoBusca + "%') OR (Descrição LIKE '%" + textoBusca +
                              "%') OR (`Cód Com` LIKE '%" + textoBusca + "%'))";

  modelProdPend.setFilter(filtro + (textoBusca.isEmpty() ? "" : " AND " + filtroBusca));

  ui->tableProdutosPend->resizeColumnsToContents();
}

// NOTE: reorganizar interface para não ter botão em um lado de uma tela e do outro lado em outra, consistência!
// NOTE: reorganizar para que seja mais intuitivo quando se deve marcar linhas
