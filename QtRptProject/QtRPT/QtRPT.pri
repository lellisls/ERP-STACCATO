QT       += gui xml script sql

greaterThan(QT_MAJOR_VERSION, 4){
    QT += widgets printsupport
    DEFINES += HAVE_QT5
}

DEFINES += NO_BARCODE
include(../CommonFiles/CommonFiles_QtRpt.pri)

INCLUDEPATH += $$PWD

SOURCES += $$PWD/qtrpt.cpp \
           $$PWD/RptSql.cpp \
           $$PWD/RptFieldObject.cpp \
           $$PWD/RptBandObject.cpp \
           $$PWD/RptPageObject.cpp
HEADERS += $$PWD/qtrpt.h \
           $$PWD/qtrptnamespace.h \
           $$PWD/RptSql.h \
           $$PWD/RptFieldObject.h \
           $$PWD/RptBandObject.h \
           $$PWD/RptPageObject.h

RESOURCES += \
    $$PWD/imagesRpt.qrc
