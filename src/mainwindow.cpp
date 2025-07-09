
#include <QIcon>
#include <QPrintDialog>
#include <QPrinter>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "qstringconvertor.h"
#include <QNetworkAccessManager>
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
    networkManager = new QNetworkAccessManager(this);

}
MainWindow::~ MainWindow()
{

}
void MainWindow::on_actionimg_triggered()
{

    //ui.ted->moveCursor(QTextCursor::End); // Přesuneme kurzor na konec
    //ui.ted->insertPlainText(
    //   "<p>Obrázek vložený z webu:</p>"
    //   "<img src='https://placehold.co/400x200/007BFF/FFFFFF?text=Obrázek+z+webu' />"
    //   "<p>HTML metoda je velmi jednoduchá.</p>"
    //   );

    QUrl imageUrl("https://placehold.co/400x200/007BFF/FFFFFF?");
    QNetworkRequest request(imageUrl);
    QNetworkReply *reply = networkManager->get(request);

    // Propojíme signál 'finished' od odpovědi s naším slotem, který data zpracuje.
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onImageDownloaded(reply);
    });

    ui.ted->moveCursor(QTextCursor::End);
ui.ted->insertPlainText(
       "<p>Obrázek vložený z webu:</p>"
       "<img src='https://placehold.co/400x200/007BFF/FFFFFF?text=Obrázek+z+webu' />"
       "<p>HTML metoda je velmi jednoduchá.</p>"
       );
}
void MainWindow::onImageDownloaded(QNetworkReply *reply)
{
    // Zkontrolujeme, zda nedošlo k chybě sítě
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Chyba při stahování obrázku:" << reply->errorString();
        ui.ted->insertPlainText(QString("<p><b>Chyba stahování: %1</b></p>").arg(reply->errorString()));
        reply->deleteLater(); // Důležité pro uvolnění paměti
        return;
    }

    // Přečteme stažená data
    QByteArray imageData = reply->readAll();

    // Vytvoříme URL, které bude sloužit jako unikátní ID pro náš zdroj
    QUrl url = reply->url();

    // Přidáme stažená data jako zdroj do dokumentu
    ui.textEditPreview->document()->addResource(QTextDocument::ImageResource, url, QVariant(imageData));

    // Vložíme obrázek na konec dokumentu
    QTextCursor cursor = ui.textEditPreview->textCursor();
    cursor.movePosition(QTextCursor::End);

    QTextImageFormat imageFormat;
    imageFormat.setName(url.toString()); // Použijeme URL jako ID
    //imageFormat.setWidth(400);

    cursor.insertImage(imageFormat);

    // Uvolníme paměť po objektu odpovědi
    reply->deleteLater();
}


/**
 * @brief Vloží obrázek z lokálního souboru programově.
 * Otevře dialog pro výběr souboru.
 */
void MainWindow::insertImageProgrammatically()
{
    QString imagePath = QFileDialog::getOpenFileName(this, "Vyberte obrázek", "", "Obrázky (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (imagePath.isEmpty()) {
        return;
    }

    QTextCursor cursor = ui.ted->textCursor();
    cursor.movePosition(QTextCursor::End);

    QUrl imageUrl = QUrl::fromLocalFile(imagePath);
    ui.ted->document()->addResource(QTextDocument::ImageResource, imageUrl, QVariant(imageUrl));

    QTextImageFormat imageFormat;
    imageFormat.setName(imageUrl.toString());
    imageFormat.setWidth(400);

    cursor.insertImage(imageFormat);
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
    QString htmlText = QStringConvertor::addHtmlHeader(markdownText);
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

