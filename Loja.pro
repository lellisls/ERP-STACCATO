#-------------------------------------------------
#
# Project created by QtCreator 2014-12-04T14:47:28
#
#-------------------------------------------------

QT       += core gui sql network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Loja
TEMPLATE = app

SOURCES += \
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
    src/contasapagar.cpp \
    src/contasareceber.cpp \
    src/dateformatdelegate.cpp \
    src/devolucao.cpp \
    src/doubledelegate.cpp \
    src/entregascliente.cpp \
    src/estoque.cpp \
    src/estoqueproxymodel.cpp \
    src/excel.cpp \
    src/followup.cpp \
    src/followupproxy.cpp \
    src/importaprodutos.cpp \
    src/importaprodutosproxy.cpp \
    src/importarxml.cpp \
    src/impressao.cpp \
    src/inputdialog.cpp \
    src/itembox.cpp \
    src/lineeditcep.cpp \
    src/lineeditdecimal.cpp \
    src/lineedittel.cpp \
    src/logindialog.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/noeditdelegate.cpp \
    src/orcamento.cpp \
    src/orcamentoproxymodel.cpp \
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
    src/widgetcompra.cpp \
    src/widgetcompraconfirmar.cpp \
    src/widgetcompradevolucao.cpp \
    src/widgetcomprafaturar.cpp \
    src/widgetcompragerar.cpp \
    src/widgetcomprapendentes.cpp \
    src/widgetcontapagar.cpp \
    src/widgetcontareceber.cpp \
    src/widgetestoque.cpp \
    src/widgetlogistica.cpp \
    src/widgetlogisticaagendarcoleta.cpp \
    src/widgetlogisticacoleta.cpp \
    src/widgetlogisticaentrega.cpp \
    src/widgetlogisticarecebimento.cpp \
    src/widgetlogisticarepresentacao.cpp \
    src/widgetnfe.cpp \
    src/widgetnfeentrada.cpp \
    src/widgetnfesaida.cpp \
    src/widgetorcamento.cpp \
    src/widgetrelatorio.cpp \
    src/widgetvenda.cpp \
    src/xml.cpp \
    src/xml_viewer.cpp

HEADERS  += \
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
    src/contasapagar.h \
    src/contasareceber.h \
    src/dateformatdelegate.h \
    src/devolucao.h \
    src/doubledelegate.h \
    src/entregascliente.h \
    src/estoque.h \
    src/estoqueproxymodel.h \
    src/excel.h \
    src/followup.h \
    src/followupproxy.h \
    src/importaprodutos.h \
    src/importaprodutosproxy.h \
    src/importarxml.h \
    src/impressao.h \
    src/inputdialog.h \
    src/itembox.h \
    src/lineeditcep.h \
    src/lineeditdecimal.h \
    src/lineedittel.h \
    src/logindialog.h \
    src/mainwindow.h \
    src/noeditdelegate.h \
    src/orcamento.h \
    src/orcamentoproxymodel.h \
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
    src/widgetcompra.h \
    src/widgetcompraconfirmar.h \
    src/widgetcompradevolucao.h \
    src/widgetcomprafaturar.h \
    src/widgetcompragerar.h \
    src/widgetcomprapendentes.h \
    src/widgetcontapagar.h \
    src/widgetcontareceber.h \
    src/widgetestoque.h \
    src/widgetlogistica.h \
    src/widgetlogisticaagendarcoleta.h \
    src/widgetlogisticacoleta.h \
    src/widgetlogisticaentrega.h \
    src/widgetlogisticarecebimento.h \
    src/widgetlogisticarepresentacao.h \
    src/widgetnfe.h \
    src/widgetnfeentrada.h \
    src/widgetnfesaida.h \
    src/widgetorcamento.h \
    src/widgetrelatorio.h \
    src/widgetvenda.h \
    src/xml.h \
    src/xml_viewer.h

FORMS += \
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
    ui/contasapagar.ui \
    ui/contasareceber.ui \
    ui/devolucao.ui \
    ui/entregascliente.ui \
    ui/estoque.ui \
    ui/followup.ui \
    ui/importaprodutos.ui \
    ui/importarxml.ui \
    ui/inputdialog.ui \
    ui/logindialog.ui \
    ui/mainwindow.ui \
    ui/orcamento.ui \
    ui/produtospendentes.ui \
    ui/searchdialog.ui \
    ui/sendmail.ui \
    ui/userconfig.ui \
    ui/validadedialog.ui \
    ui/venda.ui \
    ui/widgetcompra.ui \
    ui/widgetcompraconfirmar.ui \
    ui/widgetcompradevolucao.ui \
    ui/widgetcomprafaturar.ui \
    ui/widgetcompragerar.ui \
    ui/widgetcomprapendentes.ui \
    ui/widgetcontapagar.ui \
    ui/widgetcontareceber.ui \
    ui/widgetestoque.ui \
    ui/widgetlogistica.ui \
    ui/widgetlogisticaagendarcoleta.ui \
    ui/widgetlogisticacoleta.ui \
    ui/widgetlogisticaentrega.ui \
    ui/widgetlogisticarecebimento.ui \
    ui/widgetlogisticarepresentacao.ui \
    ui/widgetnfe.ui \
    ui/widgetnfeentrada.ui \
    ui/widgetnfesaida.ui \
    ui/widgetorcamento.ui \
    ui/widgetrelatorio.ui \
    ui/widgetvenda.ui \
    ui/xml_viewer.ui

CONFIG += c++14

QMAKE_CXXFLAGS += -Wall
QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE  = -Ofast
#QMAKE_CXXFLAGS_RELEASE  = -O0
QMAKE_LFLAGS_DEBUG += -O0
QMAKE_LFLAGS_RELEASE += -O3
#QMAKE_LFLAGS_RELEASE += -O0

#QMAKE_CXXFLAGS += -flto
#QMAKE_LFLAGS += -flto -fuse-linker-plugin

RESOURCES += \
    qrs/resources.qrc

RC_ICONS = Staccato.ico

CONFIG -= console

include(QtXlsxWriter/src/xlsx/qtxlsx.pri)
include(QSimpleUpdater/qsimpleupdater.pri)
include(LimeReport-1.3.11/limereport/limereport.pri)
