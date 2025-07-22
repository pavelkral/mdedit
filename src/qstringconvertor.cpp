#include "qstringconvertor.h"

#include <QRegularExpression>

QStringConvertor::QStringConvertor(QObject* parent) : QObject{ parent } {}

QString QStringConvertor::mdToHtml(QString& S) {

    QString html = S;

    // --- 1. Code blocks (```language\ncode```) ---
    static const QRegularExpression codeBlockRegExp(
        "```\\s*(\\S*)?\n(.*?)```",
        QRegularExpression::DotMatchesEverythingOption
    );

    QRegularExpressionMatchIterator codeIter = codeBlockRegExp.globalMatch(html);
    while (codeIter.hasNext()) {
        QRegularExpressionMatch match = codeIter.next();
        QString fullMatch = match.captured(0);       // Full match (```...```)
        QString language = match.captured(1);       // Language hint (e.g. cpp)
        QString code = match.captured(2);       // Code content

        QString classAttr;
        if (!language.isEmpty()) {
            classAttr = QString(" class=\"language-%1\"").arg(language);
        }

        QString replacement = QString("<pre><code%1>%2</code></pre>")
            .arg(classAttr, code.toHtmlEscaped());

        html.replace(fullMatch, replacement);
    }

    // --- 2. Inline code (`code`) ---
    // This must run *after* the multiline code blocks!
    static const QRegularExpression inlineCodeRegExp("`(.*?)`");
    html.replace(inlineCodeRegExp, "<code>\\1</code>");

    // --- 3. Headers ---
    // ### Header → <h3>, ## Header → <h2>, # Header → <h1>
    html.replace(
        QRegularExpression("^###\\s*(.*)$", QRegularExpression::MultilineOption),
        "<h3>\\1</h3>"
    );
    html.replace(
        QRegularExpression("^##\\s*(.*)$", QRegularExpression::MultilineOption),
        "<h2>\\1</h2>"
    );
    html.replace(
        QRegularExpression("^#\\s*(.*)$", QRegularExpression::MultilineOption),
        "<h1>\\1</h1>"
    );

    // --- 4. Bold text ---
    // **text** or __text__ → <strong>
    html.replace(
        QRegularExpression("\\*\\*(.*?)\\*\\*|__(.*?)__"),
        "<strong>\\1\\2</strong>"
    );

    // --- 5. Italic text ---
    // *text* or _text_ → <em>
    html.replace(
        QRegularExpression("(?<!\\*)\\*(?!\\*)(.*?)(?<!\\*)\\*(?!\\*)|(?<!_)(?!__)_(.*?)(?<!_)_(?!__)"),
        "<em>\\1\\2</em>"
    );

    // --- 6. Hyperlinks ---
    // [text](url) → <a href="url">text</a>
    html.replace(
        QRegularExpression("\\[(.*?)\\]\\((.*?)\\)"),
        "<a href=\"\\2\">\\1</a>"
    );

    // --- 7. Unordered list ---
    // Lines starting with - or * → <ul><li>...</li></ul>
    QStringList listItems;
    static const QRegularExpression listItemRegExp(
        "^\\s*[-*]\\s*(.*)$",
        QRegularExpression::MultilineOption
    );

    QRegularExpressionMatchIterator listIter = listItemRegExp.globalMatch(html);
    while (listIter.hasNext()) {
        QRegularExpressionMatch match = listIter.next();
        listItems << "<li>" + match.captured(1).trimmed() + "</li>";
    }

    if (!listItems.isEmpty()) {
        QString ul = "<ul>\n" + listItems.join("\n") + "\n</ul>";
        html.replace(listItemRegExp, ""); // Remove original list lines
        html.prepend(ul + "\n");
    }

    // --- 8. Line breaks ---
    // Convert remaining newlines to <br>
    html.replace("\n", "<br>");

    return html;

}

QString QStringConvertor::addHtmlHeader(QString& str) {
	// QMessageBox::information(this, "info"," tags ");
	qDebug() << "ok";
	QString st = str;
	// st.replace(QString("\n"), QString("\n<br />"));
	st.prepend(
		"<!DOCTYPE HTML PUBLIC-//W3C//DTD HTML 4.01 "
		"Transitional//EN>\n<html>\n<head><title>title</title>\n<link "
		"href=styl2.css rel=stylesheet type=text/css />\n<META "
		"http-equiv=content-type content=text/html; charset=windows-1250>\n<META "
		"http-equiv=Content-language content=cs>\n</head>\n<body>\n");
	st.append("\n</body>\n</html>");
	return st;
}
