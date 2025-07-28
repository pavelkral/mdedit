
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
#include "utils.h"

extern "C" {
#include "cmark.h"
}

MainWindow::MainWindow() {

    ui.setupUi(this);
    setWindowTitle(tr("Wedit"));
    setWindowIcon(QIcon("img/mdedit.png"));
    connect(ui.textEditMain, &QTextEdit::textChanged, this, &MainWindow::updatePreview);
    QString filename = "";
    updatePreview();
    networkManager = new QNetworkAccessManager(this);
    resize(1280,800);
    highlighter = new HtmlHighlighter(ui.textEditHtml->document());
    mdhighlighter = new MarkdownHighlighter(ui.textEditMain->document());

    connect(ui.actionFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    connect(ui.actionFileSave, &QAction::triggered, this, &MainWindow::onFileSave);
    connect(ui.actionFileSaveAs, &QAction::triggered, this, &MainWindow::onFileSaveAs);
	connect(ui.actionPrint, &QAction::triggered, this, &MainWindow::onPrint);
	connect(ui.actionExit, &QAction::triggered, this, &MainWindow::onExit);
	connect(ui.actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
	connect(ui.actionPrint, &QAction::triggered, this, &MainWindow::onPrint);
	connect(ui.actionUndo, &QAction::triggered, this, &MainWindow::onUndo);
	connect(ui.actionRedo, &QAction::triggered, this, &MainWindow::onRedo);
    connect(ui.actionAddStyle, &QAction::triggered, this, &MainWindow::onToHtml);

    connect(ui.actionBold, &QAction::triggered, this, &MainWindow::onAddBold);
    connect(ui.actionItalic, &QAction::triggered, this, &MainWindow::onAddItalic);
    connect(ui.actionCode, &QAction::triggered, this, &MainWindow::onAddCode);
    connect(ui.actionLink, &QAction::triggered, this, &MainWindow::onAddLink);
    connect(ui.actionImg, &QAction::triggered, this, &MainWindow::onAddImg);
    connect(ui.actionUl, &QAction::triggered, this, &MainWindow::onAddUl);
    connect(ui.actionP, &QAction::triggered, this, &MainWindow::onAddP);
    connect(ui.actionVideo, &QAction::triggered, this, &MainWindow::onAddVideo);
    connect(ui.actionH1, &QAction::triggered, this, &MainWindow::onAddH1);
    connect(ui.actionH2, &QAction::triggered, this, &MainWindow::onAddH2);
    connect(ui.actionH3, &QAction::triggered, this, &MainWindow::onAddH3);



    ui.textEditMain->setPlainText(
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
    QString markdownText = ui.textEditMain->toPlainText();
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

    QTextCursor cursor = ui.textEditMain->textCursor();
    cursor.movePosition(QTextCursor::End);

    QUrl imageUrl = QUrl::fromLocalFile(imagePath);
    ui.textEditMain->document()->addResource(QTextDocument::ImageResource, imageUrl,QVariant(imageUrl));

    QTextImageFormat imageFormat;
    imageFormat.setName(imageUrl.toString());
    imageFormat.setWidth(400);

    cursor.insertImage(imageFormat);
}
//===========================================md functions===========================================

void MainWindow::onAddH1() {
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("# %1").arg(selectedText);
        cursor.insertText(newText);
    }
}

void MainWindow::onAddH2()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("## %1").arg(selectedText);
        cursor.insertText(newText);
    }
}

void MainWindow::onAddH3()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("### %1").arg(selectedText);
        cursor.insertText(newText);
    }
}

void MainWindow::onAddBold()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("**%1**").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
        //ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::onAddItalic()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("*%1*").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::onAddP()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("%1\n").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
        //ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::onAddLink()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("[%1](%1)").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
        //ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::onAddUl()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("- %1").arg(selectedText);
        cursor.insertText(newText);
       // cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}

void MainWindow::onAddCode()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("```%1```  \n").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}
void MainWindow::onAddImg() {


    QTextCursor cursor = ui.textEditMain->textCursor();
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
void MainWindow::onAddVideo()
{
    QTextCursor cursor = ui.textEditMain->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("**%1**  \n").arg(selectedText);
        cursor.insertText(newText);
       // cursor.movePosition(QTextCursor::End);
       // ui.ted->setTextCursor(cursor);
    }
}
void MainWindow::onToHtml() {

    QString markdownText = ui.textEditMain->toPlainText();
    QString htmlText = Utils::addHtmlStyle(markdownText);
    ui.textEditMain->setPlainText(htmlText);
}

void MainWindow::onRedo()
{
   ui.textEditMain->redo();
}

void MainWindow::onUndo()
{
  ui.textEditMain->undo();
}

//===========================================file and header functions===========================================

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
            ui.textEditMain->setPlainText(text);
            file.close();
            currentFile = fileName;
            updateStatusBar();
        } else {
            QMessageBox::warning(this, "Error", "Could not open file.");
        }
    }

}
void MainWindow::onFileSave() {
    if (currentFile.isEmpty()) {
        onFileSaveAs() ;
        return;
    }

    QFile file(currentFile);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << ui.textEditMain->toPlainText();
        file.close();
        updateStatusBar();
    } else {
        QMessageBox::warning(this, "Error", "Could not save file.");
    }
}

void MainWindow::onFileSaveAs() {

    //QString stt = QFileDialog::getSaveFileName(this, tr("Save As "), "/home",
                                              // tr("Text files (*.html )"));

     QString fileName = QFileDialog::getSaveFileName(this, "Save File As", "",
                                                    "Text Files (*.MD);;All Files (*)");
    currentFile = fileName;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << ui.textEditMain->toPlainText();
        file.close();
        updateStatusBar();
    }
}
void MainWindow::onExit() {
    QApplication::quit();
}
void MainWindow::onAbout() {
    QMessageBox::information(this, "info", "Created in Qt.");
}


void MainWindow::onPrint() {
#ifndef QT_NO_PRINTER
    QPrinter printer(QPrinter::HighResolution);
    printer.setFullPage(true);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (ui.textEditMain->textCursor().hasSelection())
        dlg->setOption(QAbstractPrintDialog::PrintSelection, true);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
        ui.textEditMain->print(&printer);
    }
    delete dlg;
#endif
}
