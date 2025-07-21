
#include "mainwindow.h"

#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPrintDialog>
#include <QPrinter>
#include <QInputDialog>
#include <QRegularExpression>
#include "htmlhighlighter.h"
#include "markdownhighlighter.h"
#include "qstringconvertor.h"

extern "C" {
#include "cmark.h"
}

MainWindow::MainWindow() {

    ui.setupUi(this);
    setWindowTitle(tr("Wedit"));
    setWindowIcon(QIcon("img/wedit.png"));
    connect(ui.ted, &QTextEdit::textChanged, this, &MainWindow::updatePreview);
    QString filename = "";
    updatePreview();
    networkManager = new QNetworkAccessManager(this);
    resize(1280,720);
    highlighter = new HtmlHighlighter(ui.textEditHtml->document());
    mdhighlighter = new MarkdownHighlighter(ui.ted->document());

    connect(ui.actionFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);

    ui.ted->setPlainText(
        "# First level heading\n\n"
        "This is *italics* and this is **bold** text.\n\n"
        "## Subheading\n\n"
        "- List item 1\n- List item 2\n\n"
        "Link to [Qt framework](https://www.qt.io/).\n\n"
        "![Image from web](https://placehold.co/400x200/28A745/FFFFFF?text=Web+Image)\n\n"
        "Inline code: `int main() {}`\n\n"
        "```cpp\n"
        "// Code block\n"
        "int x = 5;\n"
        "```\n"
        );

    updatePreview();
}

MainWindow::~MainWindow() {

}
void MainWindow::updatePreview() {
    QString markdownText = ui.ted->toPlainText();
    QString htmlText = convertMarkdownToHtml(markdownText);
    ui.textEditPreview->setHtml(htmlText);
    ui.textEditHtml->setPlainText(htmlText);
    ui.textEditPreview->setStyleSheet("h1 { color: red; } ul { color: black; } em { font-style: italic; }");
    findAndDownloadImages(htmlText);

}
QString MainWindow::convertMarkdownToHtml(const QString &markdown) {
    QByteArray markdownBytes = markdown.toUtf8();

    int options = CMARK_OPT_DEFAULT | CMARK_OPT_UNSAFE | CMARK_OPT_SMART;

    cmark_node *doc = cmark_parse_document(markdownBytes.constData(),
                                           markdownBytes.length(), options);

    if (!doc) {
        qWarning() << "cmark_parse_document failed to parse Markdown.";
        return QString();
    }

    char *htmlCStr = cmark_render_html(doc, options);

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

//===========================================image functions===========================================

void MainWindow::findAndDownloadImages(const QString &html)
{
    static const QRegularExpression imgRegex("<img[^>]+src=[\"'](https?://[^\"']+)[\"'][^>]*>");
    auto matches = imgRegex.globalMatch(html);

    while (matches.hasNext()) {
        auto match = matches.next();
        QUrl url(match.captured(1));

        if (url.isValid() && !downloadedImageUrls.contains(url)) {
            qDebug() << "Img found:" << url;
            downloadedImageUrls.insert(url);
            QNetworkRequest request(url);
            QNetworkReply *reply = networkManager->get(request);

            connect(reply, &QNetworkReply::finished, this, &MainWindow::onImageDownloaded);
        }
    }
}
//TODU FIX

void MainWindow::onImageDownloaded() {

    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "err" << reply->url().toString() << ":" << reply->errorString();
        reply->deleteLater();
        return;
    }

    QUrl url = reply->url();
    QByteArray data = reply->readAll();
    qDebug() << "Downloaded:" << url.toString();

    ui.textEditPreview->document()->addResource(QTextDocument::ImageResource, url, QVariant(data));
    updatePreview();

    reply->deleteLater(); //
}

void MainWindow::insertImageProgrammatically() {

    QString imagePath = QFileDialog::getOpenFileName(
        this, "Img select", "", "format (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (imagePath.isEmpty()) {
        return;
    }

    QTextCursor cursor = ui.ted->textCursor();
    cursor.movePosition(QTextCursor::End);

    QUrl imageUrl = QUrl::fromLocalFile(imagePath);
    ui.ted->document()->addResource(QTextDocument::ImageResource, imageUrl,QVariant(imageUrl));

    QTextImageFormat imageFormat;
    imageFormat.setName(imageUrl.toString());
    imageFormat.setWidth(400);

    cursor.insertImage(imageFormat);
}
//===========================================md functions===========================================

void MainWindow::on_actionheadOne_triggered() {
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("# %1").arg(selectedText);
        cursor.insertText(newText);
    }
}

void MainWindow::on_actionheadTwo_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("## %1").arg(selectedText);
        cursor.insertText(newText);
    }
}

void MainWindow::on_actionheadThree_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("### %1").arg(selectedText);
        cursor.insertText(newText);
    }
}

void MainWindow::on_actionbold_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("**%1**").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
        //ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::on_actionitalic_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("*%1*").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::on_actionp_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("%1\n").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
        //ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::on_actionlink_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("[%1](%1)").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
        //ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::on_actionul_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("- %1").arg(selectedText);
        cursor.insertText(newText);
       // cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::on_actioncode_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("```%1```  \n").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}
void MainWindow::on_actionimg_triggered() {


    QTextCursor cursor = ui.ted->textCursor();
   // cursor.insertText(R"(<img src='' alt=''>)");
    //cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
    QString url = "https://www.pavelkral.net/images/aplication/min/min_qmetronom.png";
    bool succes;
    url = QInputDialog::getText ( this, tr ( "New group" ),
                                       tr ( "Enter the group name:" ), QLineEdit::Normal,
                                       url, &succes );

    if ( succes && !url.isEmpty() ){
        QString selectedText = cursor.selectedText();
        QString newText = QString(R"(<p style="text-align: center;"><img style="margin:2px auto;width:100%;" src='%1' /></p>)").arg(url);
        cursor.insertText(newText);
    }
    else
    {
        // QMessageBox::information(this, "info"," You must entrer url");
    }


}
// todo add yotube for blog
void MainWindow::on_actionvideo_triggered()
{
    QTextCursor cursor = ui.ted->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("**%1**  \n").arg(selectedText);
        cursor.insertText(newText);
       // cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}
void MainWindow::on_actionTohtml_triggered() {

    QString markdownText = ui.ted->toPlainText();
    QString htmlText = QStringConvertor::addHtmlHeader(markdownText);
    ui.ted->setPlainText(htmlText);
}

void MainWindow::on_actionRedo_triggered()
{
   ui.ted->redo();
}

void MainWindow::on_actionUndo_triggered()
{
  ui.ted->undo();
}
//===========================================file functions===========================================
void MainWindow::updateStatusBar()
{
    statusBar()->showMessage(currentFile);
}
void MainWindow::onFileOpen() {

    /*
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Open File"), "/home", tr("Text files (*.*)"));

    if (!filename.isEmpty()) {
        QFile f(filename);
        if (f.open(QFile::ReadOnly)) {
            QByteArray data = f.readAll();
            QString str = QString::fromUtf8(data);
            if (Qt::mightBeRichText(str)) {
                ui.ted->setPlainText(str);
            } else {
                str = QString::fromLocal8Bit(data);

            }
        }
    }
    */

    QString fileName = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.*);;All Files (*)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream in(&file);
            QString text = in.readAll();
            ui.ted->setPlainText(text);
            file.close();
            currentFile = fileName;
            updateStatusBar();
        } else {
            QMessageBox::warning(this, "Error", "Could not open file.");
        }
    }

}
void MainWindow::on_actionFileSave_triggered() {
    if (currentFile.isEmpty()) {
        on_actionFileSaveAs_triggered() ;
        return;
    }

    QFile file(currentFile);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << ui.ted->toPlainText();
        file.close();
        updateStatusBar();
    } else {
        QMessageBox::warning(this, "Error", "Could not save file.");
    }
}

void MainWindow::on_actionFileSaveAs_triggered() {

    //QString stt = QFileDialog::getSaveFileName(this, tr("Save As "), "/home",
                                              // tr("Text files (*.html )"));

     QString fileName = QFileDialog::getSaveFileName(this, "Save File As", "",
                                                    "Text Files (*.MD);;All Files (*)");
    currentFile = fileName;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << ui.ted->toPlainText();
        file.close();
        updateStatusBar();
    }
}
void MainWindow::on_actionExit_triggered() {
    QApplication::quit();
}
void MainWindow::on_actionAbout_triggered() {
    QMessageBox::information(this, "info", "Created in Qt.");
}


void MainWindow::on_actionPrint_triggered() {
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
