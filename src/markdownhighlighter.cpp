#include "markdownhighlighter.h"
#include <QFont>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Nadpisy (#, ##, ...)
    QTextCharFormat headingFormat;
    headingFormat.setForeground(Qt::blue);
    headingFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(^#{1,6}\s)");
    rule.format = headingFormat;
    highlightingRules.append(rule);

    // Tučné písmo (**text** nebo __text__)
    QTextCharFormat boldFormat;
    boldFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"((\*\*|__)(?=\S)(.+?)(?<=\S)\1)");
    rule.format = boldFormat;
    highlightingRules.append(rule);

    // Kurzíva (*text* nebo _text_)
    QTextCharFormat italicFormat;
    italicFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"((\*|_)(?=\S)(.+?)(?<=\S)\1)");
    rule.format = italicFormat;
    highlightingRules.append(rule);

    // Odkazy ([text](url))
    QTextCharFormat linkFormat;
    linkFormat.setForeground(Qt::darkCyan);
    linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    rule.pattern = QRegularExpression(R"(\[([^\]]+)\]\(([^)]+)\))");
    rule.format = linkFormat;
    highlightingRules.append(rule);

    // Inline kód (`code`)
    QTextCharFormat inlineCodeFormat;
    inlineCodeFormat.setBackground(QColor(230, 230, 230));
    inlineCodeFormat.setFontFamily("Courier");
    rule.pattern = QRegularExpression(R"(`[^`]+`)");
    rule.format = inlineCodeFormat;
    highlightingRules.append(rule);

    // Bloky kódu (```)
    codeBlockFormat.setBackground(QColor(240, 240, 240));
    codeBlockFormat.setFontFamily("Courier");
    codeBlockStartExpression = QRegularExpression(R"(^```)");
    codeBlockEndExpression = QRegularExpression(R"(^```$)");
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(codeBlockStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(codeBlockEndExpression, startIndex, &endMatch);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, codeBlockFormat);
        startIndex = text.indexOf(codeBlockStartExpression, startIndex + commentLength);
    }
}
