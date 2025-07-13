
#include "mainwindow.h"
#include "qstringconvertor.h"
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
void MainWindow::findAndDownloadImages(const QString &html)
{
    QRegularExpression imgRegex("<img[^>]+src=[\"'](https?://[^\"']+)[\"'][^>]*>");
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
//.......................................................................................

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

    ui.ted->moveCursor(QTextCursor::End);

    QTextCursor cursor = ui.ted->textCursor();
   // cursor.insertText(R"(<img src='' alt=''>)");
   // cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);


    QString url = "https://www.pavelkral.net/images/aplication/min/min_qmetronom.png";
    bool succes;
    url = QInputDialog::getText ( this, tr ( "New group" ),
                                       tr ( "Enter the group name:" ), QLineEdit::Normal,
                                       url, &succes );

    if ( succes && !url.isEmpty() )
    {

        QString selectedText = cursor.selectedText();
        QString newText = QString(R"(<p style="text-align: center;">
            <img style="margin:2px auto;width:100%;" src='%1' /></p><p></p>)").arg(url);
        cursor.insertText(newText);



    }
    else
    {
        // QMessageBox::information(this, "info"," You must entrer group name");
    }


    //ui.ted->moveCursor(QTextCursor::End);

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

void MainWindow::on_actionFileOpen_triggered() {
    // QMessageBox::information(this, "info","OPEN ");
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
                ui.ted->setPlainText(str);
            }
        }
    }
}
void MainWindow::on_actionFileSave_triggered() {

    QString stt = QFileDialog::getSaveFileName(this, tr("Save As "), "/home",
                                               tr("Text files (*.html )"));
    QFile file(stt);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << ui.ted->toPlainText();
        file.close();
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
