#include "htmlhighlighter.h"
#include <QFont>

HtmlHighlighter::HtmlHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // 1. Pravidlo pro HTML tagy (např. <h1>, </p>)
    QTextCharFormat tagFormat;
    tagFormat.setForeground(QColor(255, 140, 0));
    tagFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression(R"(</?\w+.*?>)"); // Zjednodušený výraz pro tagy
    rule.format = tagFormat;
    highlightingRules.append(rule);

    // 2. Pravidlo pro atributy (např. href=, class=)
    QTextCharFormat attributeFormat;
    attributeFormat.setForeground(QColor(0, 128, 128)); // Tyrkysová/Teal
    attributeFormat.setFontItalic(true);
    rule.pattern = QRegularExpression(R"(\b([a-zA-Z0-9_-]+)\s*=)");
    rule.format = attributeFormat;
    highlightingRules.append(rule);

    // 3. Pravidlo pro hodnoty v uvozovkách
    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(QColor(0, 128, 128));
    rule.pattern = QRegularExpression(R"(".+?")"); // Nekonzumní (non-greedy)
    rule.format = quotationFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression(R"('.+?')"); // Pro apostrofy
    highlightingRules.append(rule);

    // 4. Pravidlo pro HTML komentáře (víceřádkové)
    multiLineCommentFormat.setForeground(Qt::darkGreen);
    multiLineCommentFormat.setFontItalic(true);

    commentStartExpression = QRegularExpression(R"(<!--)");
    commentEndExpression = QRegularExpression(R"(-->)");
}

// Tato metoda je Qt volána automaticky pro každý blok (řádek) textu.
void HtmlHighlighter::highlightBlock(const QString &text)
{
    // Aplikujeme jednoduchá pravidla (tagy, atributy, hodnoty)
    for (const HighlightingRule &rule : highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    setCurrentBlockState(0); // Výchozí stav

    // Zpracování víceřádkových komentářů
    int startIndex = 0;
    if (previousBlockState() != 1) {
        startIndex = text.indexOf(commentStartExpression);
    }

    while (startIndex >= 0) {
        QRegularExpressionMatch endMatch;
        int endIndex = text.indexOf(commentEndExpression, startIndex, &endMatch);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1); // Jsme uvnitř komentáře, který na tomto řádku nekončí
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex + endMatch.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}

