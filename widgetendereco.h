#ifndef WIDGETENDERECO_H
#define WIDGETENDERECO_H

#include <QDataWidgetMapper>
#include <QLineEdit>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QWidget>

namespace Ui {
  class WidgetEndereco;
}

class WidgetEndereco : public QWidget {
    Q_OBJECT

  public:
    explicit WidgetEndereco(QWidget *parent = 0);
    ~WidgetEndereco();
    bool cadastrar();
    bool isEmpty();
    bool verifyFields(bool silent = false);
    bool viewCadastro(int id);
    int getId() const;
    int nextId();
    QString getTable() const;
    void clearFields();
    void novoCadastro();
    void remove(int id);
    void setId(int value);
    void setTable(const QString &value);
    void setupMapper();
    void setupUi(QWidget *first = nullptr, QWidget *last = nullptr);

  private slots:
    void on_lineEditCEP_textEdited(const QString &cep);

  private:
    // attributes
    Ui::WidgetEndereco *ui;
    int id;
    QString table;
    // methods
    QString buscaUF(QString cep);
    bool inRange(QString cep, int st, int end);
    static QString requiredStyle();
    bool verifyField(QLineEdit *lineEdit, bool silent = false);
};

#endif // WIDGETENDERECO_H
