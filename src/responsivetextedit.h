#ifndef RESPONSIVETEXTEDIT_H
#define RESPONSIVETEXTEDIT_H
#include <QTextEdit>
#include <QTimer>

class ResponsiveTextEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit ResponsiveTextEdit(QWidget* parent = nullptr);

    void setHtmlResponsive(const QString& html);

protected:
    void resizeEvent(QResizeEvent* e) override;

private:
    void fitOversizeImages();
    QTimer m_fitLater;
};
#endif // RESPONSIVETEXTEDIT_H
