#-------------------------------------------------
#
# Project created by QtCreator 2014-12-04T14:47:28
#
#-------------------------------------------------

QT       += core gui sql network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Loja
TEMPLATE = app

VERSION = 0.5
QMAKE_TARGET_COMPANY = Staccato Revestimentos
QMAKE_TARGET_PRODUCT = ERP
QMAKE_TARGET_DESCRIPTION = ERP da Staccato Revestimentos
QMAKE_TARGET_COPYRIGHT = Rodrigo Torres

CONFIG += c++14

#QMAKE_CXXFLAGS += -std=c++14

QMAKE_CXXFLAGS += -Wall -Wextra
QMAKE_CXXFLAGS_DEBUG += -O0
#QMAKE_CXXFLAGS_RELEASE  = -Ofast
QMAKE_CXXFLAGS_RELEASE  = -O0
QMAKE_LFLAGS_DEBUG += -O0
#QMAKE_LFLAGS_RELEASE += -O3
QMAKE_LFLAGS_RELEASE += -O0

#QMAKE_CXXFLAGS += -flto
#QMAKE_LFLAGS += -flto -fuse-linker-plugin

macx{
QMAKE_CXXFLAGS += -stdlib=libc++ -std=c++14
QMAKE_LFLAGS += -stdlib=libc++
}

RESOURCES += \
    qrs/resources.qrc

RC_ICONS = Staccato.ico

CONFIG -= console

include(QtXlsxWriter/src/xlsx/qtxlsx.pri)
include(QSimpleUpdater/qsimpleupdater.pri)
include(LimeReport-1.4.11/limereport/limereport.pri)

SOURCES += \
    src/acbr.cpp \
    src/anteciparrecebimento.cpp \
    src/baixaorcamento.cpp \
    src/cadastrarnfe.cpp \
    src/cadastrocliente.cpp \
    src/cadastrofornecedor.cpp \
    src/cadastroloja.cpp \
    src/cadastroproduto.cpp \
    src/cadastroprofissional.cpp \
    src/cadastrotransportadora.cpp \
    src/cadastrousuario.cpp \
    src/calendarioentregas.cpp \
    src/cepcompleter.cpp \
    src/checkboxdelegate.cpp \
    src/combobox.cpp \
    src/comboboxdelegate.cpp \
    src/contas.cpp \
    src/dateformatdelegate.cpp \
    src/devolucao.cpp \
    src/doubledelegate.cpp \
    src/estoque.cpp \
    src/estoqueprazoproxymodel.cpp \
    src/estoqueproxymodel.cpp \
    src/excel.cpp \
    src/financeiroproxymodel.cpp \
    src/followup.cpp \
    src/followupproxymodel.cpp \
    src/importaprodutos.cpp \
    src/importaprodutosproxy.cpp \
    src/importarxml.cpp \
    src/impressao.cpp \
    src/inputdialog.cpp \
    src/inputdialogconfirmacao.cpp \
    src/inputdialogfinanceiro.cpp \
    src/inputdialogproduto.cpp \
    src/inserirlancamento.cpp \
    src/inserirtransferencia.cpp \
    src/itembox.cpp \
    src/itemboxdelegate.cpp \
    src/lineeditcep.cpp \
    src/lineeditdecimal.cpp \
    src/lineeditdelegate.cpp \
    src/lineedittel.cpp \
    src/logindialog.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/noeditdelegate.cpp \
    src/orcamento.cpp \
    src/orcamentoproxymodel.cpp \
    src/pagamentosdia.cpp \
    src/porcentagemdelegate.cpp \
    src/produtospendentes.cpp \
    src/reaisdelegate.cpp \
    src/registeraddressdialog.cpp \
    src/registerdialog.cpp \
    src/searchdialog.cpp \
    src/searchdialogproxy.cpp \
    src/sendmail.cpp \
    src/singleeditdelegate.cpp \
    src/smtp.cpp \
    src/sqlquerymodel.cpp \
    src/sqltablemodel.cpp \
    src/tableview.cpp \
    src/userconfig.cpp \
    src/usersession.cpp \
    src/validadedialog.cpp \
    src/venda.cpp \
    src/vendaproxymodel.cpp \
    src/widgetcalendario.cpp \
    src/widgetcompra.cpp \
    src/widgetcompraconfirmar.cpp \
    src/widgetcompradevolucao.cpp \
    src/widgetcomprafaturar.cpp \
    src/widgetcompragerar.cpp \
    src/widgetcompraoc.cpp \
    src/widgetcomprapendentes.cpp \
    src/widgetestoque.cpp \
    src/widgetfinanceiro.cpp \
    src/widgetfinanceirocompra.cpp \
    src/widgetfluxocaixa.cpp \
    src/widgetlogistica.cpp \
    src/widgetlogisticaagendarcoleta.cpp \
    src/widgetlogisticacaminhao.cpp \
    src/widgetlogisticacoleta.cpp \
    src/widgetlogisticaentrega.cpp \
    src/widgetlogisticaentregues.cpp \
    src/widgetlogisticarecebimento.cpp \
    src/widgetlogisticarepresentacao.cpp \
    src/widgetnfe.cpp \
    src/widgetnfeentrada.cpp \
    src/widgetnfesaida.cpp \
    src/widgetorcamento.cpp \
    src/widgetpagamento.cpp \
    src/widgetrelatorio.cpp \
    src/widgetvenda.cpp \
    src/xml.cpp \
    src/xml_viewer.cpp

HEADERS  += \
    src/acbr.h \
    src/anteciparrecebimento.h \
    src/baixaorcamento.h \
    src/cadastrarnfe.h \
    src/cadastrocliente.h \
    src/cadastrofornecedor.h \
    src/cadastroloja.h \
    src/cadastroproduto.h \
    src/cadastroprofissional.h \
    src/cadastrotransportadora.h \
    src/cadastrousuario.h \
    src/calendarioentregas.h \
    src/cepcompleter.h \
    src/checkboxdelegate.h \
    src/combobox.h \
    src/comboboxdelegate.h \
    src/contas.h \
    src/dateformatdelegate.h \
    src/devolucao.h \
    src/doubledelegate.h \
    src/estoque.h \
    src/estoqueprazoproxymodel.h \
    src/estoqueproxymodel.h \
    src/excel.h \
    src/financeiroproxymodel.h \
    src/followup.h \
    src/followupproxymodel.h \
    src/importaprodutos.h \
    src/importaprodutosproxy.h \
    src/importarxml.h \
    src/impressao.h \
    src/inputdialog.h \
    src/inputdialogconfirmacao.h \
    src/inputdialogfinanceiro.h \
    src/inputdialogproduto.h \
    src/inserirlancamento.h \
    src/inserirtransferencia.h \
    src/itembox.h \
    src/itemboxdelegate.h \
    src/lineeditcep.h \
    src/lineeditdecimal.h \
    src/lineeditdelegate.h \
    src/lineedittel.h \
    src/logindialog.h \
    src/mainwindow.h \
    src/noeditdelegate.h \
    src/orcamento.h \
    src/orcamentoproxymodel.h \
    src/pagamentosdia.h \
    src/porcentagemdelegate.h \
    src/produtospendentes.h \
    src/reaisdelegate.h \
    src/registeraddressdialog.h \
    src/registerdialog.h \
    src/searchdialog.h \
    src/searchdialogproxy.h \
    src/sendmail.h \
    src/singleeditdelegate.h \
    src/smtp.h \
    src/sqlquerymodel.h \
    src/sqltablemodel.h \
    src/tableview.h \
    src/userconfig.h \
    src/usersession.h \
    src/validadedialog.h \
    src/venda.h \
    src/vendaproxymodel.h \
    src/widgetcalendario.h \
    src/widgetcompra.h \
    src/widgetcompraconfirmar.h \
    src/widgetcompradevolucao.h \
    src/widgetcomprafaturar.h \
    src/widgetcompragerar.h \
    src/widgetcompraoc.h \
    src/widgetcomprapendentes.h \
    src/widgetestoque.h \
    src/widgetfinanceiro.h \
    src/widgetfinanceirocompra.h\
    src/widgetfluxocaixa.h \
    src/widgetlogistica.h \
    src/widgetlogisticaagendarcoleta.h \
    src/widgetlogisticacaminhao.h \
    src/widgetlogisticacoleta.h \
    src/widgetlogisticaentrega.h \
    src/widgetlogisticaentregues.h \
    src/widgetlogisticarecebimento.h \
    src/widgetlogisticarepresentacao.h \
    src/widgetnfe.h \
    src/widgetnfeentrada.h \
    src/widgetnfesaida.h \
    src/widgetorcamento.h \
    src/widgetpagamento.h \
    src/widgetrelatorio.h \
    src/widgetvenda.h \
    src/xml.h \
    src/xml_viewer.h

FORMS += \
    ui/anteciparrecebimento.ui \
    ui/baixaorcamento.ui \
    ui/cadastrarnfe.ui \
    ui/cadastrocliente.ui \
    ui/cadastrofornecedor.ui \
    ui/cadastroloja.ui \
    ui/cadastroproduto.ui \
    ui/cadastroprofissional.ui \
    ui/cadastrotransportadora.ui \
    ui/cadastrousuario.ui \
    ui/calendarioentregas.ui \
    ui/contas.ui \
    ui/devolucao.ui \
    ui/estoque.ui \
    ui/followup.ui \
    ui/importaprodutos.ui \
    ui/importarxml.ui \
    ui/inputdialog.ui \
    ui/inputdialogconfirmacao.ui \
    ui/inputdialogfinanceiro.ui \
    ui/inputdialogproduto.ui \
    ui/inserirlancamento.ui \
    ui/inserirtransferencia.ui \
    ui/logindialog.ui \
    ui/mainwindow.ui \
    ui/orcamento.ui \
    ui/pagamentosdia.ui \
    ui/produtospendentes.ui \
    ui/searchdialog.ui \
    ui/sendmail.ui \
    ui/userconfig.ui \
    ui/validadedialog.ui \
    ui/venda.ui \
    ui/widgetcalendario.ui \
    ui/widgetcompra.ui \
    ui/widgetcompraconfirmar.ui \
    ui/widgetcompradevolucao.ui \
    ui/widgetcomprafaturar.ui \
    ui/widgetcompragerar.ui \
    ui/widgetcompraoc.ui \
    ui/widgetcomprapendentes.ui \
    ui/widgetestoque.ui \
    ui/widgetfinanceiro.ui \
    ui/widgetfinanceirocompra.ui \
    ui/widgetfluxocaixa.ui \
    ui/widgetlogistica.ui \
    ui/widgetlogisticaagendarcoleta.ui \
    ui/widgetlogisticacaminhao.ui \
    ui/widgetlogisticacoleta.ui \
    ui/widgetlogisticaentrega.ui \
    ui/widgetlogisticaentregues.ui \
    ui/widgetlogisticarecebimento.ui \
    ui/widgetlogisticarepresentacao.ui \
    ui/widgetnfe.ui \
    ui/widgetnfeentrada.ui \
    ui/widgetnfesaida.ui \
    ui/widgetorcamento.ui \
    ui/widgetpagamento.ui \
    ui/widgetrelatorio.ui \
    ui/widgetvenda.ui \
    ui/xml_viewer.ui
