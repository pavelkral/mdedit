
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
//#include "parentdirproxymodel.h"

#include <QFileSystemModel>
#include <QImage>
#include <QImageReader>
#include <QScrollBar>
#include <QSplitter>
#include <QTimer>
#include <utils.h>
#include "codeeditor.h"

extern "C" {
#include "cmark.h"

}

MainWindow::MainWindow() {

    ui.setupUi(this);
    setWindowTitle(tr("Mdedit"));
    setWindowIcon(QIcon("img/mdedit.png"));
    QString filename = "";

    m_fileSystemModel = new QFileSystemModel(this);
    m_fileSystemModel->setFilter(QDir::AllEntries | QDir::NoDot);
    m_fileSystemModel->setRootPath(QDir::homePath());

    ui.fileView1->setModel(m_fileSystemModel);
    ui.fileView1->setRootIndex(m_fileSystemModel->index(QDir::homePath()));
    ui.textEditPreview->setReadOnly(true);
    ui.textEditHtml->setReadOnly(true);
    editor = new CodeEditor(this);
    ui.editorLayout->addWidget(editor);

    networkManager = new QNetworkAccessManager(this);
    resize(1600,900);
    highlighter = new HtmlHighlighter(ui.textEditHtml->document());
    mdhighlighter = new MarkdownHighlighter(editor->document());

    connect(editor, &QPlainTextEdit::textChanged, this, &MainWindow::updatePreview);
    connect(ui.actionFileOpen, &QAction::triggered, this, &MainWindow::onFileOpen);
    connect(ui.actionFileSave, &QAction::triggered, this, &MainWindow::onFileSave);
    connect(ui.actionFileSaveAs, &QAction::triggered, this, &MainWindow::onFileSaveAs);
    connect(ui.actionPrint, &QAction::triggered, this, &MainWindow::onPrint);
    connect(ui.actionExportHtml,&QAction::triggered, this, &MainWindow::onHtmlExport);
    connect(ui.actionExportPdf,&QAction::triggered, this, &MainWindow::onPdfExport);
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
    connect(ui.actionColor, &QAction::triggered, this, &MainWindow::onAddColor);
    connect(ui.fileView1, &QListView::doubleClicked, this, &MainWindow::onFileViewDoubleClicked);


    editor->setPlainText(
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

    QWidget* widget1 = new QWidget(this);
    widget1->setLayout(ui.fileLayout);
    widget1->setMaximumWidth(240);
    QWidget* widget2 = new QWidget(this);
    widget2->setLayout(ui.editLayout);

    QWidget* widget3 = new QWidget(this);
    widget3->setLayout(ui.previewLayout);

    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(widget1);
    splitter->addWidget(widget2);
    splitter->addWidget(widget3);
    connect(splitter, &QSplitter::splitterMoved, this, [this]() {
        updatePreview();
    });

    setCentralWidget(splitter);
    updatePreview();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    updatePreview();
}

void MainWindow::showEvent(QShowEvent *event) {
    QMainWindow::showEvent(event);
    QTimer::singleShot(0, this, &MainWindow::updatePreview);
}

MainWindow::~MainWindow() {
    delete networkManager;
    delete highlighter;
    delete mdhighlighter;
    delete editor;
}
void MainWindow::updatePreview() {
    const int scrollPos = ui.textEditPreview->verticalScrollBar()->value();

    const QString baseDir = !currentFile.isEmpty()
        ? QFileInfo(currentFile).absolutePath()
        : (!m_currentPath.isEmpty() ? m_currentPath : QDir::currentPath());
    const QUrl baseUrl = QUrl::fromLocalFile(baseDir + "/");
    ui.textEditPreview->document()->setBaseUrl(baseUrl);

    QString markdownText = editor->toPlainText();
    QString htmlText = convertMarkdownToHtml(markdownText);

    findAndDownloadImages(htmlText);

    const QString previewHtml = preparePreviewHtml(htmlText);
    ui.textEditPreview->setHtml(previewHtml);
    ui.textEditPreview->verticalScrollBar()->setValue(scrollPos);
    ui.textEditHtml->setPlainText(htmlText);
}

QString MainWindow::convertMarkdownToHtml(const QString &markdown) {
    // Normalize backslashes in Markdown image paths ![alt](C:\path\img.png) -> ![alt](C:/path/img.png)
    QString normalizedMarkdown = markdown;
    static const QRegularExpression mdImgRegex(R"(!\[([^\]]*)\]\(([^)]+)\))");
    auto matches = mdImgRegex.globalMatch(markdown);
    int offset = 0;
    while (matches.hasNext()) {
        auto m = matches.next();
        QString path = m.captured(2);
        if (path.contains('\\')) {
            path.replace('\\', '/');
            const QString replacement = QString("![%1](%2)").arg(m.captured(1), path);
            normalizedMarkdown.replace(m.capturedStart() + offset, m.capturedLength(), replacement);
            offset += replacement.length() - m.capturedLength();
        }
    }

    QByteArray markdownBytes = normalizedMarkdown.toUtf8();

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
    static const QRegularExpression imgRegex("<img[^>]+src=[\"']([^\"']+)[\"'][^>]*>",
                                             QRegularExpression::CaseInsensitiveOption);
    auto matches = imgRegex.globalMatch(html);

    const QString baseDir = !currentFile.isEmpty()
        ? QFileInfo(currentFile).absolutePath()
        : (!m_currentPath.isEmpty() ? m_currentPath : QDir::currentPath());
    const QUrl baseUrl = ui.textEditPreview->document()->baseUrl();

    while (matches.hasNext()) {
        auto match = matches.next();
        const QString rawSrc = match.captured(1).trimmed();
        if (rawSrc.isEmpty()) {
            continue;
        }

        // 1. Web image (http / https)
        if (rawSrc.startsWith("http://", Qt::CaseInsensitive) ||
            rawSrc.startsWith("https://", Qt::CaseInsensitive)) {
            QUrl url(rawSrc);
            if (downloadedImages.contains(url)) {
                ui.textEditPreview->document()->addResource(
                    QTextDocument::ImageResource, url, QVariant(downloadedImages.value(url)));
            } else if (url.isValid() && !downloadedImageUrls.contains(url)) {
                qDebug() << "Web img found:" << url;
                downloadedImageUrls.insert(url);
                QNetworkRequest request(url);
                QNetworkReply *reply = networkManager->get(request);

                connect(reply, &QNetworkReply::finished, this, &MainWindow::onImageDownloaded);
            }
            continue;
        }

        // 2. Local image (relative path, C:/..., C:\..., or file:///...)
        const QUrl resourceKey(rawSrc);
        const QUrl resolvedKey = baseUrl.resolved(resourceKey);

        QString localPath;
        QUrl parsedUrl(rawSrc);
        if (parsedUrl.isLocalFile()) {
            localPath = parsedUrl.toLocalFile();
        } else {
            QString decoded = QUrl::fromPercentEncoding(rawSrc.toUtf8());
            decoded.replace('\\', '/');
            QFileInfo fi(decoded);
            if (fi.isAbsolute()) {
                localPath = fi.absoluteFilePath();
            } else {
                localPath = QDir(baseDir).filePath(decoded);
            }
        }

        QFileInfo localInfo(localPath);
        if (localInfo.exists() && localInfo.isFile()) {
            const QString absFilePath = localInfo.absoluteFilePath();
            QImage img;
            if (localImageCache.contains(absFilePath)) {
                img = localImageCache.value(absFilePath);
            } else {
                QImageReader reader(absFilePath);
                reader.setAutoTransform(true);
                const QSize origSize = reader.size();
                if (origSize.isValid() && origSize.width() > 960) {
                    reader.setScaledSize(origSize.scaled(960, 960, Qt::KeepAspectRatio));
                }
                img = reader.read();
                if (!img.isNull()) {
                    localImageCache.insert(absFilePath, img);
                }
            }
            if (!img.isNull()) {
                ui.textEditPreview->document()->addResource(
                    QTextDocument::ImageResource, resourceKey, QVariant(img));
                ui.textEditPreview->document()->addResource(
                    QTextDocument::ImageResource, resolvedKey, QVariant(img));
            }
        }
    }
}

QString MainWindow::preparePreviewHtml(const QString &html) const
{
    int viewportW = ui.textEditPreview->viewport()->width();
    if (viewportW < 200) {
        viewportW = qMax(450, width() / 3);
    }
    const int availW = qMax(160, viewportW - 32);

    QString result = html;
    static const QRegularExpression imgTagRegex("<img\\b[^>]*>", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression widthPctRegex("\\bwidth\\s*=\\s*[\"'](\\d+)%[\"']", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression heightPctRegex("\\bheight\\s*=\\s*[\"'](\\d+)%[\"']", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression styleWidthPctRegex("width\\s*:\\s*(\\d+)%", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression tdWidthPctRegex("<td\\b[^>]*\\bwidth\\s*=\\s*[\"'](\\d+)%[\"'][^>]*>\\s*$", QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression tdOpenRegex("<td\\b[^>]*>\\s*$", QRegularExpression::CaseInsensitiveOption);

    auto matches = imgTagRegex.globalMatch(html);
    int offset = 0;

    while (matches.hasNext()) {
        auto m = matches.next();
        QString tag = m.captured(0);
        const QString before = html.left(m.capturedStart());

        // Check if this <img> is inside a <td> cell
        double cellFraction = 1.0;
        auto tdPctMatch = tdWidthPctRegex.match(before);
        if (tdPctMatch.hasMatch()) {
            cellFraction = qBound(0.1, tdPctMatch.captured(1).toDouble() / 100.0, 1.0) * 0.94;
        } else if (tdOpenRegex.match(before).hasMatch()) {
            cellFraction = 0.47;
        }

        // Remove percentage height attributes which Qt parses as 0
        tag.remove(heightPctRegex);

        auto wMatch = widthPctRegex.match(tag);
        if (wMatch.hasMatch()) {
            const double pct = qBound(5.0, wMatch.captured(1).toDouble(), 100.0) / 100.0;
            const int pxWidth = qMax(40, int(availW * cellFraction * pct));
            tag.replace(wMatch.capturedStart(), wMatch.capturedLength(), QString("width=\"%1\"").arg(pxWidth));
        } else {
            auto swMatch = styleWidthPctRegex.match(tag);
            if (swMatch.hasMatch()) {
                const double pct = qBound(5.0, swMatch.captured(1).toDouble(), 100.0) / 100.0;
                const int pxWidth = qMax(40, int(availW * cellFraction * pct));
                tag.remove(swMatch.capturedStart(), swMatch.capturedLength());
                tag.insert(4, QString(" width=\"%1\" ").arg(pxWidth));
            } else {
                static const QRegularExpression anyWidthRegex("\\bwidth\\s*=", QRegularExpression::CaseInsensitiveOption);
                static const QRegularExpression srcAttrRegex("\\bsrc\\s*=\\s*[\"']([^\"']+)[\"']", QRegularExpression::CaseInsensitiveOption);
                if (!anyWidthRegex.match(tag).hasMatch()) {
                    auto srcMatch = srcAttrRegex.match(tag);
                    if (srcMatch.hasMatch()) {
                        const QUrl resUrl = ui.textEditPreview->document()->baseUrl().resolved(QUrl(srcMatch.captured(1)));
                        const QVariant res = ui.textEditPreview->document()->resource(QTextDocument::ImageResource, resUrl);
                        if (res.canConvert<QImage>()) {
                            const QImage img = res.value<QImage>();
                            const int maxPxWidth = qMax(40, int(availW * cellFraction));
                            if (img.width() > maxPxWidth) {
                                tag.insert(4, QString(" width=\"%1\" ").arg(maxPxWidth));
                            }
                        }
                    }
                }
            }
        }

        if (tag != m.captured(0)) {
            result.replace(m.capturedStart() + offset, m.capturedLength(), tag);
            offset += tag.length() - m.capturedLength();
        }
    }

    return result;
}

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

    downloadedImages.insert(url, data);
    ui.textEditPreview->document()->addResource(QTextDocument::ImageResource, url, QVariant(data));
    updatePreview();

    reply->deleteLater();
}

void MainWindow::insertImageProgrammatically() {

    QString imagePath = QFileDialog::getOpenFileName(
        this, "Img select", "", "format (*.png *.jpg *.jpeg *.bmp *.gif)");

    if (imagePath.isEmpty()) {
        return;
    }

    QTextCursor cursor = editor->textCursor();
    cursor.movePosition(QTextCursor::End);

    QUrl imageUrl = QUrl::fromLocalFile(imagePath);
    editor->document()->addResource(QTextDocument::ImageResource, imageUrl,QVariant(imageUrl));

    QTextImageFormat imageFormat;
    imageFormat.setName(imageUrl.toString());
    imageFormat.setWidth(400);

    cursor.insertImage(imageFormat);
}

//===========================================md functions===========================================

void MainWindow::onAddH1() {

    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("# %1").arg(selectedText);
        cursor.insertText(newText);
    }
    else {
        QString md = QString("# ");
        cursor.insertText(md);

    }
}

void MainWindow::onAddH2()
{
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("## %1").arg(selectedText);
        cursor.insertText(newText);

    }
    else {
        QString md = QString("## ");
        cursor.insertText(md);

    }
}

void MainWindow::onAddH3()
{
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("### %1").arg(selectedText);
        cursor.insertText(newText);
    }
    else {
        QString md = QString("### ");
        cursor.insertText(md);

    }
}

void MainWindow::onAddBold()
{
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("**%1**").arg(selectedText);
        cursor.insertText(newText);
        //cursor.movePosition(QTextCursor::End);
        //ui.ted->setTextCursor(cursor);
    }
    else{
        int originalPos = cursor.position();
        cursor.setPosition(originalPos);
        cursor.insertText("**");
        cursor.setPosition(originalPos + QString("**").length());
        cursor.insertText("**");
        cursor.setPosition(originalPos + QString("**").length());
        editor->setTextCursor(cursor);
    }
}

void MainWindow::onAddItalic()
{
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("*%1*").arg(selectedText);
        cursor.insertText(newText);
    }
    else{
        int originalPos = cursor.position();
        cursor.setPosition(originalPos);
        cursor.insertText("*");
        cursor.setPosition(originalPos + QString("*").length());
        cursor.insertText("*");
        cursor.setPosition(originalPos + QString("*").length());
        editor->setTextCursor(cursor);
    }

}

void MainWindow::onAddP()
{
    QTextCursor cursor = editor->textCursor();
    QString selectedText = cursor.selectedText();
    QString newText = QString("\n%1\n").arg(selectedText);
    cursor.insertText(newText);
}

void MainWindow::onAddLink()
{
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("[%1](%1)").arg(selectedText);
        cursor.insertText(newText);
    }
    else {
        int originalPos = cursor.position();
        cursor.setPosition(originalPos);
        cursor.insertText("\n[link]");
        cursor.setPosition(originalPos + QString("\n[link]").length());
        cursor.insertText("(url)\n");
        cursor.setPosition(originalPos + QString("\n[link]").length());
        editor->setTextCursor(cursor);
    }

}

void MainWindow::onAddUl(){
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("- %1").arg(selectedText);
        cursor.insertText(newText);
    } else {
        QString md = QString("- ");
        cursor.insertText(md);
    }
}

void MainWindow::onAddCode(){
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {

        QString selectedText = cursor.selectedText();
        QString newText = QString("```%1```  \n").arg(selectedText);
        cursor.insertText(newText);
    } else {
        QTextCursor cursor = editor->textCursor();
        QString beforeCursor = "```cpp\n";
        QString cursorLine = "code\n";
        QString afterCursor = "```";
        cursor.insertText(beforeCursor + cursorLine + afterCursor);
        int cursorLineStart =
            cursor.position() - afterCursor.length() - cursorLine.length();
        cursor.setPosition(cursorLineStart);
        editor->setTextCursor(cursor);
    }
}
void MainWindow::onAddImg() {

    QTextCursor cursor = editor->textCursor();
    // cursor.insertText(R"(<img src='' alt=''>)");
    // cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::KeepAnchor);
    QString url ="https://www.pavelkral.net/images/aplication/min/min_qmetronom.png";
    bool succes;
    url = QInputDialog::getText(this, tr("New group"), tr("Enter the group name:"),
                                QLineEdit::Normal, url, &succes);

    if (succes && !url.isEmpty()) {
        QString selectedText = cursor.selectedText();
        QString newText =
            QString(R"(<p style="text-align: center;"><img style="margin:2px auto;width:100%;" src='%1' /></p>)").arg(url);
        cursor.insertText(newText);
    } else {
        cursor.insertText(
            R"(![Image](https://placehold.co/400x200/28A745/FFFFFF?text=Web+Image))");
    }
}
// todo add yotube for blog
void MainWindow::onAddVideo(){
    QTextCursor cursor = editor->textCursor();

    if (cursor.hasSelection()) {   

    }
}

void MainWindow::onAddColor()
{
    //qWarning() << "add color not implemented.";
    ColorPicker picker(this);
    picker.resize(320,240);
    picker.exec();
}

//===========================================file and header functions===========================================

void MainWindow::onToHtml() {

    const QString markdown = editor->toPlainText();
    if (markdown.trimmed().isEmpty())
        return;

    const QString html = StringUtils::addHtmlStyle(markdown);

    //QSignalBlocker blocker(editor);
    //editor->setAcceptRichText(true);
    editor->setPlainText(html);
}

void MainWindow::onRedo() {
    editor->redo();
}

void MainWindow::onUndo() {
    editor->undo();
}

void MainWindow::updateStatusBar() {
    statusBar()->showMessage(currentFile);
}

bool MainWindow::openFile(const QString &filePath) {
    if (filePath.isEmpty()) {
        return false;
    }

    QFileInfo info(filePath);
    const QString absPath = info.absoluteFilePath();

    QFile file(absPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not open file:\n%1").arg(absPath));
        return false;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    const QString content = in.readAll();
    file.close();

    currentFile = absPath;
    m_currentPath = info.absolutePath();
    m_fileSystemModel->setRootPath(m_currentPath);
    ui.fileView1->setRootIndex(m_fileSystemModel->index(m_currentPath));

    setWindowTitle(QString("Mdedit - %1").arg(info.fileName()));
    updateStatusBar();
    editor->setPlainText(content);
    return true;
}

void MainWindow::onFileOpen() {
    const QString startDir = m_currentPath.isEmpty() ? QDir::homePath() : m_currentPath;
    QString fileName = QFileDialog::getOpenFileName(
        this, tr("Open File"), startDir, tr("Markdown & Text files (*.md *.MD *.markdown *.txt *.*);;All Files (*)"));

    if (!fileName.isEmpty()) {
        openFile(fileName);
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
        out.setEncoding(QStringConverter::Utf8);
        out << editor->toPlainText();
        file.close();
        updateStatusBar();
    } else {
        QMessageBox::warning(this, "Error", "Could not save file.");
    }
}

void MainWindow::onFileSaveAs() {
    const QString startDir = m_currentPath.isEmpty() ? QDir::homePath() : m_currentPath;
    const QString fileName = QFileDialog::getSaveFileName(
        this, "Save File As", startDir, "Markdown Files (*.md *.MD);;All Files (*)");

    if (fileName.isEmpty()) return;
    currentFile = fileName;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream stream(&file);
        stream.setEncoding(QStringConverter::Utf8);
        stream << editor->toPlainText();
        file.close();
        setWindowTitle(QString("Mdedit - %1").arg(QFileInfo(fileName).fileName()));
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
    if (editor->textCursor().hasSelection())
        dlg->setOption(QAbstractPrintDialog::PrintSelection, true);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted) {
        editor->print(&printer);
    }
    delete dlg;
#endif
}

//===========================================export functions===========================================

void MainWindow::onHtmlExport(){
    const QString fileName = QFileDialog::getSaveFileName(this, "Save File As", "","Text Files (*.html);;All Files (*)");

    if(!fileName.isEmpty()){
        currentFile = fileName;
        QFile file(fileName);
        if (file.open(QFile::WriteOnly)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            stream << ui.textEditHtml->toPlainText();
            file.close();
            updateStatusBar();
        }
    }
}

void MainWindow::onPdfExport(){

    // QString fileName = QFileDialog::getSaveFileName(this, "Save File As", "",
    //                                                 "Text Files (*.pdf);;All Files (*)");
    // if(!fileName.isEmpty()){
    //     QString html =ui.textEditHtml->toPlainText();
    //     QTextDocument document;
    //     document.setHtml(html);
    //     QPrinter printer(QPrinter::PrinterResolution);
    //     printer.setOutputFormat(QPrinter::PdfFormat);
    //     printer.setOutputFileName(fileName);
    //     printer.setPageMargins(QMarginsF(15, 15, 15, 15));
    //     QPageLayout pageLayout(QPageSize::A4, QPageLayout::Portrait, QMarginsF(15, 15, 15, 15));
    //     printer.setPageLayout(pageLayout);
    //     document.print(&printer);
    // }

    const QString suggestedName = "export.pdf";
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save as PDF"),
        QDir::homePath() + "/" + suggestedName,
        tr("PDF files (*.pdf)")
        );

    if (filePath.isEmpty())
        return;

    if (!filePath.endsWith(".pdf", Qt::CaseInsensitive))
        filePath += ".pdf";

    QFileInfo fi(filePath);
    QDir().mkpath(fi.dir().absolutePath());

    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(filePath);
    printer.setPageSize(QPageSize(QPageSize::A4));
    printer.setPageMargins(QMarginsF(12, 12, 12, 12));

    ui.textEditPreview->document()->print(&printer);

    QFileInfo outInfo(filePath);
    if (!outInfo.exists() || outInfo.size() == 0) {
        QMessageBox::warning(this, tr("Error"),
                             tr("PDF not saved:\n%1").arg(filePath));
        return;
    }

    QMessageBox::information(this, tr("Succes"),
                             tr("PDF saved:\n%1").arg(filePath));
}

void MainWindow::onFileViewDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }


    QString filePath = m_fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);


    QFileInfo info = m_fileSystemModel->fileInfo(index);

    if (info.fileName() == "..") {

        QDir dir = info.dir();
        dir.cdUp();
        ui.fileView1->setRootIndex(m_fileSystemModel->index(dir.absolutePath()));
    }
    else if (info.isDir()) {
        m_currentPath = filePath;
        ui.fileView1->setRootIndex(index);
    }
    else if (info.isFile()) {
        static const QStringList imgExts = {"png", "jpg", "jpeg", "gif", "bmp", "svg", "webp"};
        if (imgExts.contains(info.suffix().toLower())) {
            QString imgPath = info.absoluteFilePath();
            if (!currentFile.isEmpty()) {
                QDir docDir = QFileInfo(currentFile).absoluteDir();
                imgPath = docDir.relativeFilePath(imgPath);
            }
            imgPath.replace('\\', '/');
            editor->textCursor().insertText(QString("![%1](%2)").arg(info.completeBaseName(), imgPath));
            return;
        }
        openFile(info.absoluteFilePath());
    }
}
