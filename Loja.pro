#-------------------------------------------------
#
# Project created by QtCreator 2014-12-04T14:47:28
#
#-------------------------------------------------

QT       += core gui sql network xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Loja
TEMPLATE = app

SOURCES += src/apagaorcamento.cpp \
    src/cadastrarnfe.cpp \
    src/cadastrocliente.cpp \
    src/cadastrofornecedor.cpp \
    src/cadastroloja.cpp \
    src/cadastroproduto.cpp \
    src/cadastroprofissional.cpp \
    src/cadastrotransportadora.cpp \
    src/cadastrousuario.cpp \
    src/cepcompleter.cpp \
    src/combobox.cpp \
    src/comboboxdelegate.cpp \
    src/contasapagar.cpp \
    src/contasareceber.cpp \
    src/dateformatdelegate.cpp \
    src/entregascliente.cpp \
    src/itembox.cpp \
    src/lineeditcep.cpp \
    src/lineeditdecimal.cpp \
    src/lineedittel.cpp \
    src/loginconfig.cpp \
    src/logindialog.cpp \
    src/main.cpp \
    src/mainwindow.cpp \
    src/orcamento.cpp \
    src/registerdialog.cpp \
    src/searchdialog.cpp \
    src/sendmail.cpp \
    src/smtp.cpp \
    src/usersession.cpp \
    src/validadedialog.cpp \
    src/venda.cpp \
    src/doubledelegate.cpp \
    src/importaprodutosproxy.cpp \
    src/importaprodutos.cpp \
    src/porcentagemdelegate.cpp \
    src/checkboxdelegate.cpp \
    src/registeraddressdialog.cpp \
    src/estoque.cpp \
    src/xml_viewer.cpp \
    src/xml.cpp \
    src/produtospendentes.cpp \
    src/importarxml.cpp \
    src/inputdialog.cpp \
    src/estoqueproxymodel.cpp \
    src/sqltablemodel.cpp \
    src/orcamentoproxymodel.cpp \
    src/sqlquerymodel.cpp \
    src/tableview.cpp \
    src/widgetorcamento.cpp \
    src/widgetvenda.cpp \
    src/widgetcompra.cpp \
    src/widgetlogistica.cpp \
    src/widgetnfe.cpp \
    src/widgetestoque.cpp \
    src/widgetconta.cpp \
    src/userconfig.cpp

HEADERS  += src/apagaorcamento.h \
    src/cadastrarnfe.h \
    src/cadastrocliente.h \
    src/cadastrofornecedor.h \
    src/cadastroloja.h \
    src/cadastroproduto.h \
    src/cadastroprofissional.h \
    src/cadastrotransportadora.h \
    src/cadastrousuario.h \
    src/cepcompleter.h \
    src/combobox.h \
    src/comboboxdelegate.h \
    src/contasapagar.h \
    src/contasareceber.h \
    src/dateformatdelegate.h \
    src/entregascliente.h \
    src/itembox.h \
    src/lineeditcep.h \
    src/lineeditdecimal.h \
    src/lineedittel.h \
    src/loginconfig.h \
    src/logindialog.h \
    src/mainwindow.h \
    src/orcamento.h \
    src/registerdialog.h \
    src/searchdialog.h \
    src/sendmail.h \
    src/smtp.h \
    src/usersession.h \
    src/validadedialog.h \
    src/venda.h \
    src/doubledelegate.h \
    src/importaprodutosproxy.h \
    src/importaprodutos.h \
    src/porcentagemdelegate.h \
    src/checkboxdelegate.h \
    src/registeraddressdialog.h \
    src/estoque.h \
    src/xml_viewer.h \
    src/xml.h \
    src/produtospendentes.h \
    src/importarxml.h \
    src/inputdialog.h \
    src/estoqueproxymodel.h \
    src/sqltablemodel.h \
    src/orcamentoproxymodel.h \
    src/sqlquerymodel.h \
    src/tableview.h \
    src/widgetorcamento.h \
    src/widgetvenda.h \
    src/widgetcompra.h \
    src/widgetlogistica.h \
    src/widgetnfe.h \
    src/widgetestoque.h \
    src/widgetconta.h \
    src/userconfig.h

FORMS += ui/apagaorcamento.ui \
    ui/cadastrarnfe.ui \
    ui/cadastrocliente.ui \
    ui/cadastrofornecedor.ui \
    ui/cadastroloja.ui \
    ui/cadastroproduto.ui \
    ui/cadastroprofissional.ui \
    ui/cadastrotransportadora.ui \
    ui/cadastrousuario.ui \
    ui/contasapagar.ui \
    ui/contasareceber.ui \
    ui/entregascliente.ui \
    ui/loginconfig.ui \
    ui/logindialog.ui \
    ui/mainwindow.ui \
    ui/orcamento.ui \
    ui/searchdialog.ui \
    ui/sendmail.ui \
    ui/validadedialog.ui \
    ui/venda.ui \
    ui/importaprodutos.ui \
    ui/estoque.ui \
    ui/xml_viewer.ui \
    ui/produtospendentes.ui \
    ui/importarxml.ui \
    ui/inputdialog.ui \
    ui/widgetorcamento.ui \
    ui/widgetvenda.ui \
    ui/widgetcompra.ui \
    ui/widgetlogistica.ui \
    ui/widgetnfe.ui \
    ui/widgetestoque.ui \
    ui/widgetconta.ui \
    ui/userconfig.ui

CONFIG += c++11

#QMAKE_CXX = ccache g++

QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS_RELEASE  = -Ofast
#QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS_DEBUG += -O0
QMAKE_LFLAGS_RELEASE += -O3

#QMAKE_CXXFLAGS_RELEASE  = -O0
#QMAKE_LFLAGS_RELEASE += -O0

#QMAKE_CXXFLAGS += -flto
#QMAKE_LFLAGS += -flto -fuse-linker-plugin

RESOURCES += \
    qrs/resources.qrc

RC_ICONS = Staccato.ico

CONFIG -= console

include(QtRptProject/QtRPT/QtRPT.pri)
include(QtXlsxWriter/src/xlsx/qtxlsx.pri)
include(QSimpleUpdater/qsimpleupdater.pri)
