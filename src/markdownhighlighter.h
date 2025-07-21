#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>
#include <QTextCharFormat>
#include <QVector>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    MarkdownHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> highlightingRules;
    QRegularExpression codeBlockStartExpression;
    QRegularExpression codeBlockEndExpression;
    QTextCharFormat codeBlockFormat;
};

#endif // MARKDOWNHIGHLIGHTER_H
