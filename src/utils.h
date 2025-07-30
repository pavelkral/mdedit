#ifndef UTILS_H
#define UTILS_H

#include <QObject>

class Utils : public QObject
{
    Q_OBJECT
public:
    explicit Utils(QObject *parent = nullptr);
    Utils(const Utils &) = delete;
    Utils(Utils &&) = delete;
    Utils &operator=(const Utils &) = delete;
    Utils &operator=(Utils &&) = delete;

    static QString mdToHtml(QString &S);
    static QString addHtmlStyle(const QString &str);

signals:
};

#endif // UTILS_H
