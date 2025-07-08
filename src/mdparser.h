#ifndef MDPARSER_H
#define MDPARSER_H

#include <QObject>

class MdParser : public QObject
{
    Q_OBJECT
public:
    explicit MdParser(QObject *parent = nullptr);
    static QString parseMD(QString &S);
    static QString addHtmlHeader(QString &str);

signals:
};

#endif // MDPARSER_H
