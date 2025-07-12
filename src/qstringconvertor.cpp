#include "qstringconvertor.h"

#include <QRegularExpression>

QStringConvertor::QStringConvertor(QObject *parent)
    : QObject{parent}
{}

QString QStringConvertor::mdToHtml(QString &S)
{
    QString html = S;

    // --- 1.  (Code Fences) ---
    // ```
    // code
    // ```
    //  (\\s*(\\S*)?): \S* = libovolný ne-mezerový znak
    // (.*?) = lazy
    QRegularExpression codeBlockRegExp("```\\s*(\\S*)?\n(.*?)```",
                                       QRegularExpression::DotMatchesEverythingOption);

    QRegularExpressionMatchIterator i = codeBlockRegExp.globalMatch(html);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString fullMatch = match.captured(0); // Celá shoda (```...```)
        QString language = match.captured(1);  // Zachycený jazyk (např. "cpp", "python")
        QString codeContent = match.captured(2); // Samotný kód

        QString classAttr = "";
        if (!language.isEmpty()) {
            classAttr = QString(" class=\"language-%1\"").arg(language);
        }

        QString replacement = QString("<pre><code%1>%2</code></pre>")
                                  .arg(classAttr)
                                  .arg(codeContent.toHtmlEscaped()); // escape HTML


        html.replace(fullMatch, replacement);
    }


    // --- 2. Inline  ---
    // `code`
    // Toto musí být až PO parsování víceřádkových bloků!
    QRegularExpression inlineCodeRegExp("`(.*?)`");
    html.replace(inlineCodeRegExp, "<code>\\1</code>");
    // --- 1. Nadpisy (H3 -> H1) ---H3 (### Text)
    html.replace(QRegularExpression("^###\\s*(.*)$", QRegularExpression::MultilineOption), "<h3>\\1</h3>");
    html.replace(QRegularExpression("^##\\s*(.*)$", QRegularExpression::MultilineOption), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^#\\s*(.*)$", QRegularExpression::MultilineOption), "<h1>\\1</h1>");

    // --- 2. bold ---
    // **text** / __text__
    html.replace(QRegularExpression("\\*\\*(.*?)\\*\\*|__(.*?)__"), "<strong>\\1\\2</strong>");

    // --- 3.italic ---// *text* / _text_
    html.replace(QRegularExpression("(?<!\\*)\\*(?!\\*)(.*?)(?<!\\*)\\*(?!\\*)|(?<!_)(?!__)_(.*?)(?<!_)_(?!__)"),
                 "<em>\\1\\2</em>");

    // 4 link
    // [text](url)
    html.replace(QRegularExpression("\\[(.*?)\\]\\((.*?)\\)"), "<a href=\"\\2\">\\1</a>");

    // 5 In-line  ---
    // `kód`
    //html.replace(QRegularExpression("`(.*?)`"), "<code>\\1</code>")
    // --- 6. S (ul) ---
    // Najdeme všechny řádky začínající - nebo *
    QStringList listItems;
    QRegularExpression listItemRegExp("^\\s*[-*]\\s*(.*)$", QRegularExpression::MultilineOption);
    QRegularExpressionMatchIterator it = listItemRegExp.globalMatch(html);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        listItems << "<li>" + match.captured(1).trimmed() + "</li>";
    }
    // 6
    //  <ul>
    if (!listItems.isEmpty()) {
        QString ul = "<ul>\n" + listItems.join("\n") + "\n</ul>";
        html.replace(listItemRegExp, ""); // Odstraní původní řádky
        html.prepend(ul + "\n");
    }

    // --- 7. <br> ---
    html.replace("\n", "<br>");
    return html;
}

QString QStringConvertor::addHtmlHeader(QString &str)
{
    //QMessageBox::information(this, "info"," tags ");
    qDebug() << "ok";
    QString st = str;
   // st.replace(QString("\n"), QString("\n<br />"));
    st.prepend( "<!DOCTYPE HTML PUBLIC-//W3C//DTD HTML 4.01 Transitional//EN>\n<html>\n<head><title>title</title>\n<link href=styl2.css rel=stylesheet type=text/css />\n<META http-equiv=content-type content=text/html; charset=windows-1250>\n<META http-equiv=Content-language content=cs>\n</head>\n<body>\n" );
    st.append( "\n</body>\n</html>" );
    return st;
}
