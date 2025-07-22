
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
 
#include "ui_mainwindow.h"
#include <QMainWindow>

class QNetworkAccessManager;
class QNetworkReply;
class HtmlHighlighter;
class MarkdownHighlighter;


class MainWindow : public QMainWindow
{
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
    QSet<QUrl> downloadedImageUrls;
    void findAndDownloadImages(const QString &html);
    void updateStatusBar();
protected:

private slots:

        void updatePreview();
        void onImageDownloaded();
        // todo remove old style connection
        void on_actionheadOne_triggered();
        void on_actionheadTwo_triggered();
        void on_actionheadThree_triggered();
        void on_actionbold_triggered();
        void on_actionitalic_triggered();
        void on_actionp_triggered();
        void on_actionlink_triggered();
        void on_actionul_triggered();
        void on_actioncode_triggered();
        void on_actionimg_triggered();
        void on_actionvideo_triggered();

        void onFileOpen();
        void onFileSaveAs();
        void onFileSave();
        void onExit();
        void onAbout();       
        void onPrint();
        void onToHtml();
        void onRedo();
        void onUndo();

private:
};

#endif
