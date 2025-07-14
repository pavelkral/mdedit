
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

    QNetworkAccessManager *networkManager;
    HtmlHighlighter *highlighter;
    MarkdownHighlighter *mdhighlighter;
    QSet<QUrl> downloadedImageUrls;
    void findAndDownloadImages(const QString &html);

protected:

private slots:

        void updatePreview();
        void onImageDownloaded();

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

        void on_actionFileOpen_triggered();
        void on_actionFileSave_triggered();
        void on_actionExit_triggered();
        void on_actionAbout_triggered();       
        void on_actionPrint_triggered();
        void on_actionTohtml_triggered();

private:
};

#endif
