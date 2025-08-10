
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QFileSystemModel>
#include <QMainWindow>

class QNetworkAccessManager;
class QNetworkReply;
class HtmlHighlighter;
class MarkdownHighlighter;
class CodeEditor;

class MainWindow : public QMainWindow {
    Q_OBJECT
private:
    Ui::MainWindow ui;

public:
    MainWindow();
    QString convertMarkdownToHtml(const QString &markdown);
    void insertImageFromWeb();
    void insertImageProgrammatically();

    virtual ~MainWindow();

private:
    QString currentFile;
    QNetworkAccessManager *networkManager;
    HtmlHighlighter *highlighter;
    MarkdownHighlighter *mdhighlighter;
    CodeEditor *editor;
    QSet<QUrl> downloadedImageUrls;
    void findAndDownloadImages(const QString &html);
    void updateStatusBar();

protected:
private slots:

    void updatePreview();
    void onImageDownloaded();
    // todo remove old style connection
    void onAddH1();
    void onAddH2();
    void onAddH3();
    void onAddBold();
    void onAddItalic();
    void onAddP();
    void onAddLink();
    void onAddUl();
    void onAddCode();
    void onAddImg();
    void onAddVideo();
    void onAddColor();

    void onFileOpen();
    void onFileSaveAs();
    void onFileSave();
    void onHtmlExport();
    void onPdfExport();
    void onExit();
    void onAbout();
    void onPrint();
    void onToHtml();
    void onRedo();
    void onUndo();
    void on_fileView_doubleClicked(const QModelIndex &index);
      void on_backButton_clicked();
private:
    QFileSystemModel *m_fileSystemModel;
    QString m_currentPath;
};

#endif
