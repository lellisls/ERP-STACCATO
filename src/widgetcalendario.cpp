#include <QCheckBox>
#include <QDebug>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>

#include "ui_widgetcalendario.h"
#include "widgetcalendario.h"

WidgetCalendario::WidgetCalendario(QWidget *parent) : QWidget(parent), ui(new Ui::WidgetCalendario) { ui->setupUi(this); }

WidgetCalendario::~WidgetCalendario() { delete ui; }

bool WidgetCalendario::updateTables() {
  if (not setup) {
    QSqlQuery query;

    if (not query.exec("SELECT t.razaoSocial, tv.modelo FROM transportadora t LEFT JOIN transportadora_has_veiculo tv ON "
                       "t.idTransportadora = tv.idTransportadora ORDER BY razaoSocial, modelo")) {
      QMessageBox::critical(this, "Erro!", "Erro buscando veiculos: " + query.lastError().text());
      return false;
    }

    while (query.next()) {
      auto *cb = new QCheckBox(this);
      cb->setText(query.value("razaoSocial").toString() + " / " + query.value("modelo").toString());
      cb->setChecked(true);
      connect(cb, &QAbstractButton::toggled, this, &WidgetCalendario::updateFilter);
      ui->groupBoxVeiculos->layout()->addWidget(cb);
    }

    ui->groupBoxVeiculos->layout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    setup = true;
  }

  const QDate date = ui->calendarWidget->selectedDate();
  return updateCalendar(date.addDays(date.dayOfWeek() * -1));
}

void WidgetCalendario::updateFilter() {
  const QDate date = ui->calendarWidget->selectedDate();
  updateCalendar(date.addDays(date.dayOfWeek() * -1));
}

bool WidgetCalendario::updateCalendar(const QDate &startDate) {
  ui->tableWidget->clearContents();

  int veiculos = 0;
  const int start = startDate.day();

  QStringList list;

  for (auto const &item : ui->groupBoxVeiculos->findChildren<QCheckBox *>()) {
    if (not item->isChecked()) continue;

    veiculos++;

    QStringList temp = item->text().split(" / ");

    list << "Manhã\n" + temp.at(0) + "\n" + temp.at(1);
    list << "Tarde\n" + temp.at(0) + "\n" + temp.at(1);
  }

  ui->tableWidget->setRowCount(veiculos * 2); // manha/tarde
  ui->tableWidget->setVerticalHeaderLabels(list);

  ui->tableWidget->setColumnCount(7); // dias
  ui->tableWidget->setHorizontalHeaderLabels({"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"});

  int dia = start;
  const QDate date = ui->calendarWidget->selectedDate();
  const int diasMes = date.addDays(date.dayOfWeek() * -1).daysInMonth();

  for (int col = 0; col < ui->tableWidget->columnCount(); ++col) {
    const auto item = ui->tableWidget->horizontalHeaderItem(col);
    item->setText(QString::number(dia) + " " + item->text());
    dia++;
    if (dia > diasMes) dia = 1;
  }

  QSqlQuery query;
  query.prepare("SELECT * FROM view_calendario WHERE data BETWEEN :start AND :end");
  query.bindValue(":start", startDate);
  query.bindValue(":end", startDate.addDays(6));

  if (not query.exec()) {
    QMessageBox::critical(this, "Erro!", "Erro query: " + query.lastError().text());
    return false;
  }

  while (query.next()) {
    const QString transportadora = query.value("razaoSocial").toString() + "\n" + query.value("modelo").toString();

    int row = -1;

    for (int i = 0; i < list.size(); ++i) {
      if (list.at(i).contains(transportadora)) {
        row = query.value("data").toTime().hour() < 12 ? i : i + 1; // manha/tarde
        break;
      }
    }

    if (row == -1) continue;

    const int dia = query.value("data").toDate().dayOfWeek();

    QTableWidgetItem *item = ui->tableWidget->item(row, dia) ? ui->tableWidget->item(row, dia) : new QTableWidgetItem();

    const QString oldText = item->text();

    QString text = oldText.isEmpty() ? "" : oldText + "\n\n------------------------------------\n\n";

    text += query.value("data").toTime().toString("hh:mm") + "  Kg: " + query.value("kg").toString() + ", Cx.: " + query.value("caixas").toString();

    if (not query.value("idVenda").toString().isEmpty()) text += "\n           " + query.value("idVenda").toString();

    if (not query.value("bairro").toString().isEmpty()) {
      text += " - " + query.value("bairro").toString() + " - " + query.value("cidade").toString();
    }

    // TODO: dont show this to compact screen? or show this only on doubleclick
    text += query.value("text").toString();

    text += "\n           Status: " + query.value("status").toString();

    item->setText(text);

    if (not ui->tableWidget->item(row, dia)) ui->tableWidget->setItem(row, dia, item);
  }

  ui->tableWidget->resizeColumnsToContents();
  ui->tableWidget->resizeRowsToContents();

  const QString range = startDate.toString("dd-MM-yyyy") + " - " + startDate.addDays(6).toString("dd-MM-yyyy");

  ui->lineEditRange->setText(range);

  return true;
}

void WidgetCalendario::on_checkBoxMostrarFiltros_toggled(bool checked) {
  ui->calendarWidget->setVisible(checked);
  ui->groupBoxVeiculos->setVisible(checked);
}

void WidgetCalendario::on_pushButtonProximo_clicked() { ui->calendarWidget->setSelectedDate(ui->calendarWidget->selectedDate().addDays(7)); }

void WidgetCalendario::on_pushButtonAnterior_clicked() { ui->calendarWidget->setSelectedDate(ui->calendarWidget->selectedDate().addDays(-7)); }

void WidgetCalendario::on_calendarWidget_selectionChanged() {
  const QDate date = ui->calendarWidget->selectedDate();

  updateCalendar(date.addDays(date.dayOfWeek() * -1));
}
