#-------------------------------------------------
#
# Project created by QtCreator 2014-12-04T14:47:28
#
#-------------------------------------------------

QT       += core gui sql printsupport webkitwidgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Loja
TEMPLATE = app

SOURCES += backgroundproxymodel.cpp \
    cadastrocliente.cpp \
    cadastroloja.cpp \
    cadastroproduto.cpp \
    cadastroprofissional.cpp \
    cadastrotransportadora.cpp \
    cadastrousuario.cpp \
    combobox.cpp \
    comboboxdelegate.cpp \
    contasapagar.cpp \
    contasareceber.cpp \
    entregascliente.cpp \
    lineeditcep.cpp \
    lineeditdecimal.cpp \
    lineedittel.cpp \
    logindialog.cpp \
    main.cpp\
    mainwindow.cpp \
    orcamento.cpp \
    pedidoscompra.cpp \
    recebimentosfornecedor.cpp \
    sendmail.cpp \
    smtp.cpp \
    usersession.cpp \
    venda.cpp \
    widgetendereco.cpp \
    searchdialog.cpp \
    itembox.cpp \
    apagaorcamento.cpp \
    registerdialog.cpp \
    loginconfig.cpp \
    cepcompleter.cpp \
    cadastrofornecedor.cpp \
    cadastrarnfe.cpp \
    endereco.cpp \
    importaexportproxy.cpp \
    importateste.cpp \
    editablesqlmodel.cpp \
    dateformatdelegate.cpp \
    validadedialog.cpp

HEADERS  += backgroundproxymodel.h \
    cadastrocliente.h \
    cadastroloja.h \
    cadastroproduto.h \
    cadastroprofissional.h \
    cadastrotransportadora.h \
    cadastrousuario.h \
    combobox.h \
    comboboxdelegate.h \
    contasapagar.h \
    contasareceber.h \
    entregascliente.h \
    initdb.h \
    lineeditcep.h \
    lineeditdecimal.h \
    lineedittel.h \
    logindialog.h \
    mainwindow.h \
    orcamento.h \
    pedidoscompra.h \
    recebimentosfornecedor.h \
    sendmail.h \
    smtp.h \
    usersession.h \
    venda.h \
    widgetendereco.h \
    searchdialog.h \
    itembox.h \
    apagaorcamento.h \
    registerdialog.h \
    loginconfig.h \
    cepcompleter.h \
    cadastrofornecedor.h \
    cadastrarnfe.h \
    importaexportproxy.h \
    importateste.h \
    editablesqlmodel.h \
    dateformatdelegate.h \
    endereco.h \
    validadedialog.h

FORMS += cadastrocliente.ui \
    cadastroloja.ui \
    cadastroproduto.ui \
    cadastroprofissional.ui \
    cadastrotransportadora.ui \
    cadastrousuario.ui \
    contasapagar.ui \
    contasareceber.ui \
    entregascliente.ui \
    importabd.ui \
    logindialog.ui \
    mainwindow.ui \
    orcamento.ui \
    pedidoscompra.ui \
    recebimentosfornecedor.ui \
    sendmail.ui \
    venda.ui \
    widgetendereco.ui \
    searchdialog.ui \
    apagaorcamento.ui \
    loginconfig.ui \
    cadastrofornecedor.ui \
    cadastrarnfe.ui \
    importateste.ui \
    validadedialog.ui

QMAKE_CXXFLAGS_RELEASE  = -Ofast
QMAKE_CXXFLAGS_DEBUG += -O0 -Wall
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -O3

RESOURCES += \
    qrs/resources.qrc

install_it.path = $$OUT_PWD
install_it.files = $$PWD/libmysql.dll \
                   $$PWD/logo.png \
                   $$PWD/logo.jpg \
                   $$PWD/orcamento.html \
                   $$PWD/itens.html
INSTALLS += install_it

DISTFILES += \
    qt_portuguese.ts

test {
    message(Test build)
    QT += testlib
    TARGET = UnitTests

    SOURCES -= main.cpp

    HEADERS += test/testSuite1.h \
        test/testSuite2.h

    SOURCES += test/main.cpp \
        test/testSuite1.cpp \
        test/testSuite2.cpp
} else {
    message(Normal build)
}

CONFIG(debug, debug|release){
message(Activating terminal)
CONFIG += console
}

CONFIG(release, debug|release){
message(Deactivating terminal)
CONFIG -= console
}

include(QtRptProject/QtRPT/QtRPT.pri)
