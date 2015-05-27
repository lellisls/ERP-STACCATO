#ifndef QXYZ_LOGGER_H
#define QXYZ_LOGGER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

class QxyzLogger : public QObject
{
    Q_OBJECT
public:
    explicit QxyzLogger(QObject *parent, QString fileName, QPlainTextEdit *editor = 0);
    ~QxyzLogger();
    void setShowDateTime(bool value);

private:
    QFile *file;
    QPlainTextEdit *m_editor;
    bool m_showDate;

signals:

public slots:
    void write(const QString &value);

};

#endif // QXYZ_LOGGER_H
