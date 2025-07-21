#include "markdownhighlighter.h"
#include <QFont>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // headings (#, ##, ...) -
    QTextCharFormat headingFormat;
    headingFormat.setForeground(QColor(255, 140, 0)); // Tmavě oranžová
    headingFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(^#{1,6}.*$)");
    rule.format = headingFormat;
    highlightingRules.append(rule);

    // list (- text)
    QTextCharFormat listFormat;
    listFormat.setForeground(QColor(139, 69, 19)); // Hnědá
    rule.pattern = QRegularExpression(R"(^\s*-\s.*$)");
    rule.format = listFormat;
    highlightingRules.append(rule);

    // bold (**text** nebo __text__)
    QTextCharFormat boldFormat;
    boldFormat.setForeground(QColor(255, 140, 0));
    boldFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"((\*\*|__)(?=\S)(.+?)(?<=\S)\1)");
    rule.format = boldFormat;
    highlightingRules.append(rule);

    // italic (*text* nebo _text_)
    QTextCharFormat italicFormat;
    italicFormat.setForeground(QColor(255, 140, 0));
    italicFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"((\*|_)(?=\S)(.+?)(?<=\S)\1)");
    rule.format = italicFormat;
    highlightingRules.append(rule);

    // img ![text](url)
    QTextCharFormat imageFormat;
    imageFormat.setForeground(Qt::magenta);
    rule.pattern = QRegularExpression(R"(!\[([^\]]*)\]\(([^)]+)\))");
    rule.format = imageFormat;
    highlightingRules.append(rule);

    // links ([text](url))
    QTextCharFormat linkFormat;
    linkFormat.setForeground(Qt::darkCyan);
    linkFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    rule.pattern = QRegularExpression(R"(\[([^\]]+)\]\(([^)]+)\))");
    rule.format = linkFormat;
    highlightingRules.append(rule);

    // inline code (`code`)
    QTextCharFormat inlineCodeFormat;
    inlineCodeFormat.setForeground(QColor(255, 140, 0));
    inlineCodeFormat.setFontFamily("Courier");
    rule.pattern = QRegularExpression(R"(`[^`]+`)");
    rule.format = inlineCodeFormat;
    highlightingRules.append(rule);

    // code block (```)
    //codeBlockFormat.setBackground(QColor(240, 240, 240));
    codeBlockFormat.setForeground(Qt::darkGreen);
    codeBlockFormat.setFontFamily("Courier");
    codeBlockStartExpression = QRegularExpression(R"(^```)");
    codeBlockEndExpression = QRegularExpression(R"(^```$)");
}


void MarkdownHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : std::as_const( highlightingRules)) {
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
