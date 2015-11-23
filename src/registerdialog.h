#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDataWidgetMapper>
#include <QLineEdit>
#include <QDialog>

#include "sqltablemodel.h"

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
    explicit RegisterDialog(const QString &table, const QString &primaryKey, QWidget *parent);

    /*!
* \brief Utilizada para selecionar um item a partir de um QModelIndex.
* \param index Índice do Model relacionado ao item, normalmente obtido ao clicar na tabela.
* \return
*/
    virtual bool viewRegister(const QModelIndex &index);

  public slots:
    /*!
* \brief Seleciona um item a partir do valor da PK.
* \param id Valor da PK.
* \return
*/
    virtual bool viewRegisterById(const QVariant &id);
    void saveSlot();
    void marcarDirty();

  signals:
    /*!
* \brief Sinal enviado ao atualizar um campo, normalmente utilizada pela ItemBox.
* \param idCliente Id do cadastro atualizado/cadastrado.
* \param text Descrição do item, utilizada para representá-lo em um ItemBox, por exemplo.
*/
    void registerUpdated(const QVariant &idCliente, const QString &text);

  protected:
    /*!
* \brief Verifica se campo obrigatório está vazio.
* \param line QLineEdit a ser analisado.
* \return
*/
    virtual bool verifyRequiredField(QLineEdit *line, const bool &silent = false);
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
    virtual bool save(const bool &isUpdate = false);
    /*!
* \brief Chama verifyRequiredField() sobre cada elemento da lista.
* \param list Lista de QLineEdits a serem verificados.
* \return
*/
    bool verifyFields(const QList<QLineEdit *> &list);
    /*!
* \brief Função padrão para verificar campos obrigatórios.
* \return
*/
    virtual bool verifyFields() = 0;
    /*!
* \brief Onde ocorre o model.setData(), baseada nas informações da view.
*/
    virtual bool savingProcedures() = 0;
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
    void setData(const QString &key, const QVariant value);

    /*!
* \brief Acelerador para diminuir um pouco a verbosidade do código.
*        Equivale à função 'data' do model, e usa o currentIndex do mapper.
* \param key Chave da coluna.
* \return
*/
    QVariant data(const QString &key);
    /*!
* \brief Acelerador para diminuir um pouco a verbosidade do código.
*        Equivale à função 'data' do model.
* \param row Linha.
* \param key Chave da coluna.
* \return
*/
    QVariant data(const int &row, const QString &key);
    /*!
* \brief Acelerador para diminuir um pouco a verbosidade do código.
*        Equivale à função addMapping do mapper;
* \param widget Widget relacionado.
* \param key Nome da coluna mapeada.
*/
    void addMapping(QWidget *widget, const QString &key);
    /*!
* \brief Acelerador para diminuir um pouco a verbosidade do código.
*        Equivale à função addMapping do mapper;
* \param widget Widget relacionado.
* \param key Nome da coluna mapeada.
* \param propertyName Nome da propriedade mapeada.
*/
    void addMapping(QWidget *widget, const QString &key, const QByteArray &propertyName);
    /*!
* \brief Gera o output correto e envia o signal 'registerUpdated';

*/
    void sendUpdateMessage();

    /*!
* \brief É um style padrão a ser aplicado nos campos obrigatórios.
* \return
*/
    QString requiredStyle();

    /*!
* \brief QDataWidgetMapper que mapeia os itens da tabela com as views, como um QLineEdit, entre outros.
*/
    QDataWidgetMapper mapper;
    /*!
* \brief SqlTableModel que 'conversa' com a tabela, buscando e armazenando as informações. A 'View' seria a
* interface gráfica, em geral.
*/
    SqlTableModel model;
    /*!
* \brief Uma forma simples de guardar o nome da chave primária.
*/
    QString primaryKey;
    /*!
* \brief Chave das colunas que formam o texto de descrição
*/
    QStringList textKeys;

  protected:
    // attributes
    int row = 0;
    int rowEnd = 0;
    bool isOk = true;
    bool isDirty = false;
    bool silent = false;
    bool incompleto = false;
    // methods
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    bool validaCNPJ(const QString &text);
    bool validaCPF(const QString &text);
    bool update();
    QStringList getTextKeys() const;
    void setTextKeys(const QStringList &value);
};

#endif // REGISTERDIALOG_H
