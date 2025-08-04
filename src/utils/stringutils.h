#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <QObject>

class StringUtils : public QObject
{
    Q_OBJECT
public:
    explicit StringUtils(QObject *parent = nullptr);
    StringUtils(const StringUtils &) = delete;
    StringUtils(StringUtils &&) = delete;
    StringUtils &operator=(const StringUtils &) = delete;
    StringUtils &operator=(StringUtils &&) = delete;

    static QString mdToHtml(QString &S);
    static QString addHtmlStyle(const QString &str);

signals:
};

#endif // STRINGUTILS_H
