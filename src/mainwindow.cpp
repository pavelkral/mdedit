
#include <QIcon>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include "mdparser.h"

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
    QString filename = "";
    updatePreview();


}


MainWindow::~ MainWindow()
{

}

void MainWindow::on_actionheadOne_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection())
    {

        QString selectedText = cursor.selectedText();
        QString newText = QString("# %1").arg(selectedText);
        cursor.insertText(newText);
    }
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

void MainWindow::on_actionFileOpen_triggered() {
    // QMessageBox::information(this, "info","otviram soubor");
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open File"), "/home", tr("Text files (*.*)"));


    if (!filename.isEmpty()) {
        QFile f(filename);
        if (f.open(QFile::ReadOnly)) {
            // ui.ted->setText(f.readAll());
            QByteArray data = f.readAll();
            // QTextCodec *codec = Qt::codecForHtml(data);
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

    QString markdownText = ui.ted->toPlainText();
    QString htmlText = MdParser::addHtmlHeader(markdownText);
   // ui.textEditPreview->setStyleSheet("h1 { color: navy; } strong { color: black; } em { font-style: italic; }");
   // ui.textEditPreview->setHtml(htmlText);
    ui.ted->setPlainText(htmlText);

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

