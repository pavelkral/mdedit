#include "responsivetextedit.h"

#include <QTextDocument>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextImageFormat>
#include <QImageReader>
#include <QAbstractTextDocumentLayout>

ResponsiveTextEdit::ResponsiveTextEdit(QWidget* parent)
    : QTextEdit(parent)
{
    setAcceptRichText(true);
    setLineWrapMode(QTextEdit::WidgetWidth);

    m_fitLater.setSingleShot(true);
    m_fitLater.setInterval(0);
    connect(&m_fitLater, &QTimer::timeout, this, [this]{ fitOversizeImages(); });

    // When the document layout changes / an image finishes loading, schedule another fit
    connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged,
            this, [this]{ if (!m_fitLater.isActive()) m_fitLater.start(); });
}

void ResponsiveTextEdit::setHtmlResponsive(const QString& html)
{
    setHtml(html);
    fitOversizeImages();          // run immediately
    if (!m_fitLater.isActive())   // and once more after returning to the event loop
        m_fitLater.start();
}

void ResponsiveTextEdit::resizeEvent(QResizeEvent* e)
{
    QTextEdit::resizeEvent(e);
    fitOversizeImages();          // recompute on width change
}

// not used

void ResponsiveTextEdit::fitOversizeImages()
{
    QTextDocument* doc = document();
    if (!doc) return;

    // Usable content width: viewport's inner rect minus the document margin
    const int contentW = qMax(0, int(viewport()->contentsRect().width() - 2 * doc->documentMargin()));
    if (contentW <= 0) return;

    bool changed = false;

    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next()) {
        for (auto it = block.begin(); !it.atEnd(); ++it) {
            const QTextFragment frag = it.fragment();
            if (!frag.isValid()) continue;

            QTextCharFormat cf = frag.charFormat();
            if (!cf.isImageFormat()) continue;

            QTextImageFormat imgf = cf.toImageFormat();
            const QString name = imgf.name();

            // Natural size, if we can determine it
            QSizeF natural;
            if (const QVariant res = doc->resource(QTextDocument::ImageResource, QUrl(name)); res.canConvert<QImage>()) {
                const QImage img = res.value<QImage>();
                const qreal dpr = img.devicePixelRatio() > 0 ? img.devicePixelRatio() : 1.0;
                natural = QSizeF(img.width() / dpr, img.height() / dpr);
            } else {
                QImageReader r(name);
                const QSize s = r.size();
                if (s.isValid()) natural = s;
            }

            const qreal curW = imgf.width();               // 0 = not set in HTML
            const qreal curH = imgf.height();              // 0 = not set
            const qreal baseW = (curW > 0) ? curW
                                           : (natural.width() > 0 ? natural.width() : 0.0);

            // If it already fits, leave it as is (no upscale)
            if (baseW > 0 && baseW <= contentW)
                continue;

            // Otherwise shrink to contentW; keep aspect ratio if known
            const qreal targetW = contentW;
            qreal targetH = 0.0;
            if (natural.isValid() && natural.width() > 0) {
                targetH = targetW * (natural.height() / natural.width());
            } else if (curW > 0 && curH > 0) {
                targetH = targetW * (curH / curW);
            }

            const bool widthDiff  = !qFuzzyCompare(imgf.width(),  targetW);
            const bool heightDiff = (targetH > 0) && !qFuzzyCompare(imgf.height(), targetH);
            if (widthDiff || heightDiff) {
                imgf.setWidth(targetW);
                if (targetH > 0) imgf.setHeight(targetH);

                QTextCursor c(doc);
                c.setPosition(frag.position());
                c.setPosition(frag.position() + frag.length(), QTextCursor::KeepAnchor);
                c.setCharFormat(imgf);
                changed = true;
            }
        }
    }

    if (changed) {
        doc->adjustSize();
        viewport()->update();
    }
}
