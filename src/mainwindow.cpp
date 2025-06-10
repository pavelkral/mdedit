#include <QtGui>
#include <QIcon>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>


#include "mainwindow.h"

extern "C" {
#include "cmark.h"
}

MainWindow::MainWindow()
{

    ui.setupUi(this);
    setWindowTitle(tr("Wedit"));
    setWindowIcon(QIcon("img/wedit.png"));
    connect(ui.ted, &QTextEdit::textChanged, this, &MainWindow::updatePreview);
    updatePreview();
}


MainWindow::~ MainWindow()
{
}

void MainWindow::updatePreview()
{
    QString markdownText = ui.ted->toPlainText();
    QString htmlText = convertMarkdownToHtml(markdownText);
    ui.textEditPreview->setStyleSheet("h1 { color: navy; } strong { color: black; } em { font-style: italic; }");
    ui.textEditPreview->setHtml(htmlText);
    ui.textEditHtml->setPlainText(htmlText);
    ui.textEditPreview->setStyleSheet("h1 { color: red; } ul { color: black; } em { font-style: italic; }");
}

void MainWindow::on_actionMDtoHtml_triggered()
{
    QString text = ui.ted->toPlainText();
    QString htmlResult = convertMarkdownToHtml(text);

    ui.textEditPreview->setHtml(htmlResult);
    ui.textEditPreview->setStyleSheet("h1 { color: red; } strong { color: black; } em { font-style: italic; }");
}
QString MainWindow::convertMarkdownToHtml(const QString &markdown)
{
    QByteArray markdownBytes = markdown.toUtf8();

    int options = CMARK_OPT_DEFAULT | CMARK_OPT_UNSAFE | CMARK_OPT_SMART;

    cmark_node *doc = cmark_parse_document(markdownBytes.constData(), markdownBytes.length(), options);

    if (!doc) {
        qWarning() << "cmark_parse_document failed to parse Markdown.";
        return QString();
    }


    char* htmlCStr = cmark_render_html(doc, options);

    if (!htmlCStr) {
        qWarning() << "cmark_render_html failed to render HTML.";
        cmark_node_free(doc);
        return QString();
    }

    QString htmlResult = QString::fromUtf8(htmlCStr);

    free(htmlCStr);
    cmark_node_free(doc);

    return htmlResult;




}
void MainWindow::on_actionToMD_triggered()
{
    QString text = ui.ted->toPlainText();
    QString html = text;

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

    // --- 2.  ---
    // **text** / __text__
    html.replace(QRegularExpression("\\*\\*(.*?)\\*\\*|__(.*?)__"), "<strong>\\1\\2</strong>");

    // --- 3.italic ---// *text* / _text_
    html.replace(QRegularExpression("(?<!\\*)\\*(?!\\*)(.*?)(?<!\\*)\\*(?!\\*)|(?<!_)(?!__)_(.*?)(?<!_)_(?!__)"),
                 "<em>\\1\\2</em>");

    //
    // [text](url)
    html.replace(QRegularExpression("\\[(.*?)\\]\\((.*?)\\)"), "<a href=\"\\2\">\\1</a>");

    //  In-line  ---
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

    //  <ul>
    if (!listItems.isEmpty()) {
        QString ul = "<ul>\n" + listItems.join("\n") + "\n</ul>";
        html.replace(listItemRegExp, ""); // Odstraní původní řádky
        html.prepend(ul + "\n");
    }

    // --- 7. <br> ---
    html.replace("\n", "<br>");

    // Výstup
    ui.ted->setHtml(html);
    ui.ted->setStyleSheet("h1 { color: navy; } strong { color: black; } em { font-style: italic; }");
}


void MainWindow::on_actionFileOpen_triggered()
{
    //QMessageBox::information(this, "info","otviram soubor");
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),"/home",tr("Text files (*.*)"));
	
    if(!filename.isEmpty()) {
            QFile f(filename);
            if(f.open(QFile::ReadOnly)) {
                            //ui.ted->setText(f.readAll());
                            QByteArray data = f.readAll();
                //QTextCodec *codec = Qt::codecForHtml(data);
                 QString str = QString::fromUtf8(data);
                if (Qt::mightBeRichText(str)) {
                ui.ted->setPlainText(str);
                } else {
                str = QString::fromLocal8Bit(data);
                ui.ted->setPlainText(str);
                }
             }
    }
}
void MainWindow::on_actionFileSave_triggered()
{
	//QMessageBox::information(this, "info","ukladam soubor ");
	 QString stt = QFileDialog::getSaveFileName(this, tr("Save As "),
                    "/home",
                 tr("Text files (*.html )"));   
    QFile file(stt);
    if ( file.open(QFile::WriteOnly ) ) {
       QTextStream  stream( &file );
        stream.setEncoding(QStringConverter::Utf8);
        //stream.setCodec(QTextCodec::codecForName("windows-1250"));
        stream << ui.ted->toPlainText(); 
        file.close();
    }

    
}
void MainWindow::on_actionExit_triggered()
{
	QApplication::quit();

    
}
void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this, "info","Created in Qt.");
	 
    
}
void MainWindow::on_actionTohtml_triggered()
{
	//QMessageBox::information(this, "info"," tags ");
	QString st = ui.ted->toPlainText();
    st.replace(QString("\n"), QString("\n<br />"));
    st.prepend( "<!DOCTYPE HTML PUBLIC-//W3C//DTD HTML 4.01 Transitional//EN>\n<html>\n<head><title>title</title>\n<link href=styl2.css rel=stylesheet type=text/css />\n<META http-equiv=content-type content=text/html; charset=windows-1250>\n<META http-equiv=Content-language content=cs>\n</head>\n<body bgcolor=white>\n" );
    st.append( "\n\n<body>\n</body>\n</html>" );
    ui.ted->setPlainText(st);
   
  
}
void MainWindow::on_actionPrint_triggered()
{
#ifndef QT_NO_PRINTER
     QPrinter printer(QPrinter::HighResolution);
     printer.setFullPage(true);
     QPrintDialog *dlg = new QPrintDialog(&printer, this);
     if (ui.ted->textCursor().hasSelection())
         dlg->setOption(QAbstractPrintDialog::PrintSelection, true);
     dlg->setWindowTitle(tr("Print Document"));
     if (dlg->exec() == QDialog::Accepted) {
         ui.ted->print(&printer);
     }
     delete dlg;
 #endif
   
  
}

