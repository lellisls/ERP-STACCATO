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
    importaapavisa.cpp \
    importabd.cpp \
    importaportinari.cpp \
    lineeditcep.cpp \
    lineeditdecimal.cpp \
    lineedittel.cpp \
    logindialog.cpp \
    main.cpp\
    mainwindow.cpp \
    nfe.cpp \
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
    cepcompleter.cpp
#    cadastrarcliente.cpp \

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
    importaapavisa.h \
    importabd.h \
    importaportinari.h \
    initdb.h \
    lineeditcep.h \
    lineeditdecimal.h \
    lineedittel.h \
    logindialog.h \
    mainwindow.h \
    nfe.h \
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
    cepcompleter.h
#    cadastrarcliente.h \

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
#    cadastrarcliente.ui


QMAKE_CXXFLAGS_RELEASE  = -Ofast
QMAKE_CXXFLAGS_DEBUG += -O0 -Wall
QMAKE_CXXFLAGS += -std=c++11
QMAKE_LFLAGS += -O3

RESOURCES += \
    resources.qrc

install_it.path = $$OUT_PWD
install_it.files = $$PWD/libmysql.dll

INSTALLS += install_it
