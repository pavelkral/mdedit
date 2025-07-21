#include "htmlhighlighter.h"
#include <QFont>

HtmlHighlighter::HtmlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // 1. tags ( <h1>, </p>)
    QTextCharFormat tagFormat;
    tagFormat.setForeground(QColor(255, 140, 0));
    tagFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(</?\w+.*?>)"); //  tags
    rule.format = tagFormat;
    highlightingRules.append(rule);

    // 2. atributes ( href=, class=)
    QTextCharFormat attributeFormat;
    attributeFormat.setForeground(QColor(0, 128, 128));
    attributeFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"(\b([a-zA-Z0-9_-]+)\s*=)");
    rule.format = attributeFormat;
    highlightingRules.append(rule);

    // 3. "values"
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(QColor(0, 128, 128));
    rule.pattern = QRegularExpression(R"(".+?")"); //
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression(R"('.+?')"); //
    highlightingRules.append(rule);

    // 4.comments
    multiLineCommentFormat.setForeground(Qt::darkGreen);
    multiLineCommentFormat.setFontItalic(true);
    commentStartExpression = QRegularExpression(R"(<!--)");
    commentEndExpression = QRegularExpression(R"(-->)");
}

void HtmlHighlighter::highlightBlock(const QString &text)
{
    //  (tagy, atributy, hodnoty)
    for (const HighlightingRule &rule : std::as_const( highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0); //

    // multiline comments
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(commentStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(commentEndExpression, startIndex, &endMatch);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1); // inside not on end
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

