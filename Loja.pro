#-------------------------------------------------
#
# Project created by QtCreator 2014-12-04T14:47:28
#
#-------------------------------------------------

QT       += core gui sql printsupport webkitwidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Loja
TEMPLATE = app

SOURCES += src/apagaorcamento.cpp \
    src/backgroundproxymodel.cpp \
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
    src/editablesqlmodel.cpp \
    src/endereco.cpp \
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
    src/pedidoscompra.cpp \
    src/recebimentosfornecedor.cpp \
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
    src/registeraddressdialog.cpp

HEADERS  += src/apagaorcamento.h \
    src/backgroundproxymodel.h \
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
    src/editablesqlmodel.h \
    src/endereco.h \
    src/entregascliente.h \
    src/initdb.h \
    src/itembox.h \
    src/lineeditcep.h \
    src/lineeditdecimal.h \
    src/lineedittel.h \
    src/loginconfig.h \
    src/logindialog.h \
    src/mainwindow.h \
    src/orcamento.h \
    src/pedidoscompra.h \
    src/recebimentosfornecedor.h \
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
    src/registeraddressdialog.h

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
    ui/pedidoscompra.ui \
    ui/recebimentosfornecedor.ui \
    ui/searchdialog.ui \
    ui/sendmail.ui \
    ui/validadedialog.ui \
    ui/venda.ui \
    ui/importaprodutos.ui

QMAKE_CXXFLAGS_RELEASE  = -Ofast
QMAKE_CXXFLAGS_DEBUG += -O0
QMAKE_CXXFLAGS += -std=c++14
QMAKE_LFLAGS += -O3

RESOURCES += \
    qrs/resources.qrc

test {
    DEFINES += "TEST"
    message(Test build)
    QT += testlib
    TARGET = UnitTests

    SOURCES -= src/main.cpp

    HEADERS += test/testmainwindow.h


    SOURCES += test/main.cpp \
    test/testmainwindow.cpp

} else {
    message(Normal build)
}

CONFIG -= console

include(QtRptProject/QtRPT/QtRPT.pri)
