#ifndef QSTRINGCONVERTOR_H
#define QSTRINGCONVERTOR_H

#include <QObject>

class QStringConvertor : public QObject
{
    Q_OBJECT
public:
    explicit QStringConvertor(QObject *parent = nullptr);
    QStringConvertor(const QStringConvertor &) = delete;
    QStringConvertor(QStringConvertor &&) = delete;
    QStringConvertor &operator=(const QStringConvertor &) = delete;
    QStringConvertor &operator=(QStringConvertor &&) = delete;

    static QString mdToHtml(QString &S);
    static QString addHtmlHeader(QString &str);

signals:
};

#endif // QSTRINGCONVERTOR_H
