#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QSqlRelationalTableModel>
#include <QDataWidgetMapper>
#include <QSqlTableModel>
#include <QTableView>
#include <QMessageBox>
#include <QLineEdit>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDialog>
#include <QDebug>
/*!
  \brief A Classe RegisterDialog é base para todas as janelas de cadastro.
         Ela guarda o mapper e o model, auxiliando no gerenciamento de conteúdo,
         e estabelece uma interface padrão procedimental.
  */
class RegisterDialog : public QDialog {
  Q_OBJECT
public:
  /*!
  \brief Construtor da RegisterDialog.
  \param table Nome da tabela do BD, relacionada ao Cadastro.
  \param primaryKey Nome da PK da tabela.
  \param parent Janela "mãe" do QDialog.
  */
  explicit RegisterDialog(QString table, QString primaryKey, QWidget *parent);
  /*!
  * \brief Seleciona um item a partir do valor da PK.
  * \param id Valor da PK.
  * \return
  */
  virtual bool viewRegisterById(QVariant id);
  /*!
  * \brief Utilizada para selecionar um item a partir de um QModelIndex.
  * \param idx Índice do Model relacionado ao item, normalmente obtido ao clicar na tabela.
  * \return
  */
  virtual bool viewRegister(QModelIndex idx);
  /*!
  * \brief Informa para a superclasse qual é a sua table view.
  * \param table Tabela da interface gráfica.
  */
  void setTable(QAbstractItemView *table);

  QStringList getTextKeys() const;
  void setTextKeys(const QStringList &value);

public slots:
  virtual void show();
  virtual void cancel();
  virtual void accept();
  virtual void reject();
  virtual void changeItem(QVariant value, QString text);

signals:
  /*!
  * \brief Sinal enviado ao atualizar um campo, normalmente utilizada pela ItemBox.
  * \param idCadastro Id do cadastro atualizado/cadastrado.
  * \param text Descrição do item, utilizada para representá-lo em um ItemBox, por exemplo.
  */
  void registerUpdated(QVariant idCadastro, QString text);

protected:
  /*!
  * \brief Verifica se campo obrigatório está vazio.
  * \param line QLineEdit a ser analisado.
  * \return
  */
  virtual bool verifyRequiredField(QLineEdit *line);
  /*!
  * \brief Mensagem de confirmação padrão, para evitar que o trabalho seja perdido.
  * \return
  */
  virtual bool confirmationMessage();
  /*!
  * \brief Mensagem de erro padrão para quando cadastro não é realizado com sucesso.
  */
  virtual void errorMessage();
  /*!
  * \brief Mensagem de confirmação padrão para quando cadastro é realizado com sucesso.
  */
  virtual void successMessage();
  /*!
  * \brief Procedimentos padrão para quando um novo cadastro é iniciado. Como limpar a tela, e etc.
  */
  virtual bool newRegister();
  /*!
  * \brief Remover item atual
  */
  virtual void remove();
  /*!
  * \brief Procedimento padrão para salvar informações no BD.
  * \return
  */
  virtual bool save();
  /*!
  * \brief Chama verifyRequiredField() sobre cada elemento da lista.
  * \param list Lista de QLineEdits a serem verificados.
  * \return
  */
  bool verifyFields(QList<QLineEdit *> list);
  /*!
  * \brief Função padrão para verificar campos obrigatórios.
  * \return
  */
  virtual bool verifyFields() = 0;
  /*!
  * \brief Onde ocorre o model.setData(), baseada nas informações da view.
  */
  virtual bool savingProcedures(int row) = 0;
  /*!
  * \brief Limpar os campos da tela.
  */
  virtual void clearFields() = 0;
  /*!
  * \brief Função onde os mapeamentos são configurados.
  */
  virtual void setupMapper() = 0;
  /*!
  * \brief Função chamada para atualizar a view, escondendo botão atualizar, por exemplo.
  */
  virtual void registerMode() = 0;
  /*!
  * \brief Função chamada para atualizar a view, escondendo botão cadastrarm, por exemplo.
  */
  virtual void updateMode() = 0;
  // Final

  /*!
  * \brief Acelerador para diminuir um pouco a verbosidade do código.
  *        Equivate ao setData do model.
  * \param key Chave da coluna.
  * \param value Valor a ser atualizado.
  */
  inline void setData(const QString &key, QVariant value) {
    model.setData(model.index(mapper.currentIndex(), model.fieldIndex(key)), value);
  }

  /*!
  * \brief Acelerador para diminuir um pouco a verbosidade do código.
  *        Equivate ao setData do model.
  * \param row Linha.
  * \param key Chave da coluna.
  * \param value Valor a ser atualizado.
  */
  inline void setData(int row, const QString &key, QVariant value) {
    model.setData(model.index(row, model.fieldIndex(key)), value);
  }
  /*!
  * \brief Acelerador para diminuir um pouco a verbosidade do código.
  *        Equivale à função 'data' do model, e usa o currentIndex do mapper.
  * \param key Chave da coluna.
  * \return
  */
  inline QVariant data(const QString &key) {
    return (model.data(model.index(mapper.currentIndex(), model.fieldIndex(key))));
  }
  /*!
  * \brief Acelerador para diminuir um pouco a verbosidade do código.
  *        Equivale à função 'data' do model.
  * \param row Linha.
  * \param key Chave da coluna.
  * \return
  */
  inline QVariant data(int row, const QString &key) {
    return (model.data(model.index(row, model.fieldIndex(key))));
  }
  /*!
  * \brief Acelerador para diminuir um pouco a verbosidade do código.
  *        Equivale à função addMapping do mapper;
  * \param widget Widget relacionado.
  * \param key Nome da coluna mapeada.
  */
  inline void addMapping(QWidget *widget, const QString &key) {
    mapper.addMapping(widget, model.fieldIndex(key));
  }
  /*!
  * \brief Acelerador para diminuir um pouco a verbosidade do código.
  *        Equivale à função addMapping do mapper;
  * \param widget Widget relacionado.
  * \param key Nome da coluna mapeada.
  * \param propertyName Nome da propriedade mapeada.
  */
  inline void addMapping(QWidget *widget, const QString &key, const QByteArray &propertyName) {
    if (model.fieldIndex(key) == -1) {
      qDebug() << objectName() << " : Key " << key << " not found on model!";
    }
    mapper.addMapping(widget, model.fieldIndex(key), propertyName);
  }
  /*!
  * \brief Gera o output correto e envia o signal 'registerUpdated';

  */
  void sendUpdateMessage();

  /*!
  * \brief É um style padrão a ser aplicado nos campos obrigatórios.
  * \return
  */
  inline QString requiredStyle() {
    return (QString("background-color: rgb(255, 255, 127);"));
  }
  // Attributes
  /*!
  * \brief QDataWidgetMapper que mapeia os itens da tabela com as views, como um QLineEdit, entre outros.
  */
  QDataWidgetMapper mapper;
  /*!
  * \brief QSqlTableModel que 'conversa' com a tabela, buscando e armazenando as informações. A 'View' seria a
  * interface gráfica, em geral.
  */
  QSqlTableModel model;
  /*!
  * \brief Uma forma simples de guardar o nome da chave primária.
  */
  QString primaryKey;
  /*!
  * \brief Chave das colunas que formam o texto de descrição
  */
  QStringList textKeys;

private:
  /*!
  * \brief Tabela da interface gráfica.
  */
  QAbstractItemView *table;
};

#endif // REGISTERDIALOG_H
