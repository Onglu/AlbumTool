#include "RegulableWidget.h"
#include "ui_RegulableWidget.h"
#include "page/EditPageWidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QScrollBar>

RegulableWidget::RegulableWidget(EditPageWidget *container) :
    QWidget(container),
    ui(new Ui::RegulableWidget),
    m_container(container),
    m_moveable(false),
    m_opacity(1),
    m_angle(0)
{
    ui->setupUi(this);

    ui->movePushButton->setVisible(false);
    ui->lineEdit->setVisible(false);

    ui->rotationHorizontalSlider->installEventFilter(this);

    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::transparent);
    setPalette(pal);

    m_visiableRect[VISIABLE_RECT_TYPE_CANVAS].setRect(0, 0, 0, 0);
    m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER].setRect(0, 0, 0, 0);

    hide();
}

RegulableWidget::~RegulableWidget()
{
    delete ui;
}

void RegulableWidget::on_movePushButton_clicked()
{
    QString offset = ui->lineEdit->text();
    QStringList xy = offset.split(',');

    if (xy.size() /*&& m_container*/)
    {
        QPoint pos = geometry().topLeft();
        pos.rx() += xy.first().toInt();
        pos.ry() += xy.last().toInt();

        this->move(pos);
        qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this->geometry();
    }
}

const QImage &RegulableWidget::loadPicture(const QVariantMap &photoLayer,
                                           QSizeF ratioSize,
                                           QRect bgdRect,
                                           const QString &replaced)
{
    int width = 0, height = 0, cx = 0, cy = 0;
    QSize maskSize;
    QPoint maskPos;
    QString maskFile;
    QVariantMap frame, maskLayer;

    if (photoLayer.isEmpty() ||
        m_picFile == photoLayer["filename"].toString() ||
        (!replaced.isEmpty() && m_picFile == replaced))
    {
        return m_visiableImg;
    }

    m_picFile = !replaced.isEmpty() ? replaced : photoLayer["filename"].toString();

    m_ratioSize = ratioSize;
    m_bgdRect = bgdRect;

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_ratioSize << bgdRect << m_picFile;

    frame = photoLayer["frame"].toMap();
    width = frame["width"].toInt() * ratioSize.width();
    height = frame["height"].toInt() * ratioSize.height();
    cx = frame["x"].toInt() * ratioSize.width();
    cy = frame["y"].toInt() * ratioSize.height();
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << cx << cy << width << height;

    maskLayer = photoLayer["maskLayer"].toMap();
    frame = maskLayer["frame"].toMap();
    maskSize = QSize(frame["width"].toInt() * ratioSize.width(), frame["height"].toInt() * ratioSize.height());
    maskPos.rx() = bgdRect.x() + frame["x"].toInt() * ratioSize.width() - maskSize.width() / 2;
    maskPos.ry() = bgdRect.y() + frame["y"].toInt() * ratioSize.height() - maskSize.height() / 2;
    maskFile = maskLayer["filename"].toString();

    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_picFile << maskFile << maskSize;
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << maskSize << maskPos << bgdRect;

    m_opacity = photoLayer["opacity"].toReal();
    m_angle = photoLayer["rotation"].toReal();

    if (isHidden())
    {
        showButtons(true);
    }

    m_ori = QPixmap(m_picFile);
    if (!m_ori.isNull())
    {
        m_bk = m_ori.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QSize picSize = m_bk.size();
        ui->pictureLabel->setFixedSize(picSize);
        ui->pictureLabel->setPixmap(m_bk);

        QPoint topLeft = bgdRect.topLeft();
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this->geometry().topLeft();

        /* Get the relative top-left coordinates of the picture widget */
        topLeft.rx() += cx - width / 2;
        topLeft.ry() += cy - height / 2 - ui->topFrame->height();

        if (this->minimumWidth() > picSize.width())
        {
            topLeft.rx() -= (this->minimumWidth() - picSize.width()) / 2;
        }

        if (this->minimumHeight() > picSize.height())
        {
            topLeft.ry() -= (this->minimumHeight() - ui->topFrame->height() - picSize.height()) / 2;
        }

        setGeometry(QRect(topLeft, QSize(width, height + ui->topFrame->height())));

        QImage maskImg(maskSize, QImage::Format_ARGB32);
        maskImg.load(maskFile);
        m_maskImg = maskImg.scaled(maskSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        m_maskRect = QRect(maskPos, maskSize);

        blend();

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << this->geometry() << bgdRect << this->minimumSize();

        updateView();

        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << x << y << m_visiableRect[VISIABLE_RECT_TYPE_CANVAS] << m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER];
    }

    return m_visiableImg;
}

void RegulableWidget::changePicture(const QVariantMap &belongings)
{
    QString picFile = belongings["picture_file"].toString();
    if (m_picFile == picFile)
    {
        return;
    }

    m_picFile = picFile;
    m_ori = QPixmap(m_picFile);
    if (m_ori.isNull())
    {
        return;
    }

    qreal angle = belongings["rotation_angle"].toReal();
    Qt::Axis axis = (Qt::Axis)belongings["rotation_axis"].toInt();
    m_bk = m_ori.scaled(m_bk.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    rotate(ui->pictureLabel, m_bk, angle, axis);

    blend();
    updateView();

    emit hasUpdated();
}

void RegulableWidget::blend()
{
    if (m_bk.isNull() || m_maskImg.isNull())
    {
        return;
    }

    QImage srcImg = m_bk.toImage().convertToFormat(QImage::Format_ARGB32);
    QPoint picPos = ui->pictureLabel->mapTo(m_container, QPoint(0, 0));
    QRect picRect(picPos, m_bk.size());
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << maskRect.intersects(picRect)<< maskRect.contains(picRect) << picRect.contains(maskRect)<< maskRect << picRect << xrect;

    if (m_maskRect.intersects(picRect))
    {
        QRect xrect = m_maskRect.intersected(picRect);
        int maskx, masky, srcx = 0, srcy = 0;

        for (int y = 0; y < xrect.height(); y++)
        {
            for (int x = 0; x < xrect.width(); x++)
            {
                srcx = x + xrect.x() - picPos.x();
                srcy = y + xrect.y() - picPos.y();
                maskx = x + xrect.x() - m_maskRect.x();
                masky = y + xrect.y() - m_maskRect.y();
                QRgb rgb = srcImg.pixel(srcx, srcy);
                int a = qAlpha(m_maskImg.pixel(maskx, masky));
                srcImg.setPixel(srcx, srcy, qRgba(qRed(rgb), qGreen(rgb), qBlue(rgb), a));
            }
        }

        int cx = xrect.x() - picPos.x();
        int cy = xrect.y() - picPos.y();

        if (0 < cx && 0 < cy)
        {
            for (int y = 0; y < picRect.height(); y++)
            {
                for (int x = 0; x < cx; x++)
                {
                    srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }

            for (int y = 0; y < cy; y++)
            {
                for (int x = 0; x < picRect.width(); x++)
                {
                    srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
                }
            }
        }

        for (int y = 0; y < picRect.height(); y++)
        {
            for (int x = srcx; x < picRect.width(); x++)
            {
                srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
            }
        }

        for (int y = srcy; y < picRect.height(); y++)
        {
            for (int x = 0; x < picRect.width(); x++)
            {
                srcImg.setPixel(x, y, qRgba(0, 0, 0, 0));
            }
        }
    }

//    if (picRect.contains(maskRect))
//    {

//    }

    m_composedImg = srcImg;
    //m_bk = QPixmap::fromImage(srcImg);
    //ui->pictureLabel->setPixmap(m_bk);
}

inline void RegulableWidget::updateView()
{
    QPoint topLeft = ui->pictureLabel->mapTo(m_container, QPoint(0, 0));
    //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << topLeft << picSize;

    /* Form where */
    int x = m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() - topLeft.x() : 0;
    int y = m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() - topLeft.y() : 0;

    /* How big */
    QSize picSize = m_bk.size();
    int width = m_bgdRect.width() < picSize.width() - x ? m_bgdRect.width() : picSize.width() - x;
    int height = m_bgdRect.height() < picSize.height() - y ? m_bgdRect.height() : picSize.height() - y;

    m_visiableImg = m_composedImg.convertToFormat(QImage::Format_ARGB32_Premultiplied).copy(x, y, width, height);

    m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER].setRect(m_bgdRect.x() > topLeft.x() ? m_bgdRect.x() : topLeft.x(),
                                                         m_bgdRect.y() > topLeft.y() ? m_bgdRect.y() : topLeft.y(),
                                                         width,
                                                         height);

    topLeft = m_visiableRect[VISIABLE_RECT_TYPE_CONTAINER].topLeft();
    m_visiableRect[VISIABLE_RECT_TYPE_CANVAS].setRect(topLeft.x() - m_bgdRect.x(),
                                                      topLeft.y() - m_bgdRect.y(),
                                                      width,
                                                      height);
}

void RegulableWidget::showButtons(bool visiable)
{
    if (visiable == ui->zoomInPushButton->isVisible())
    {
        return;
    }

    setVisible(visiable);
    ui->zoomInPushButton->setVisible(visiable);
    ui->zoomOutPushButton->setVisible(visiable);
    ui->scalelessPushButton->setVisible(visiable);
    ui->fitPushButton->setVisible(visiable);
    ui->mirroredPushButton->setVisible(visiable);
    ui->rotationHorizontalSlider->setVisible(visiable);
    ui->angleLabel->setVisible(visiable);
}

void RegulableWidget::resizeEvent(QResizeEvent *)
{
    if (isVisible() && ui->pictureLabel->pixmap())
    {
        //qDebug() << __FILE__ << __FUNCTION__ << __LINE__ << m_picFile;
        ui->pictureLabel->setPixmap(QPixmap(m_picFile).scaled(ui->pictureLabel->size(),
                                                              Qt::KeepAspectRatio,
                                                              Qt::SmoothTransformation));
    }
}

void RegulableWidget::mousePressEvent(QMouseEvent *event)
{
    if (Qt::LeftButton & event->buttons() && ui->pictureLabel == childAt(event->pos()) && !m_bk.isNull())
    {
        showButtons(true);
        ui->pictureLabel->setCursor(Qt::ClosedHandCursor);
        m_x = event->pos().x();
        m_y = event->pos().y();
        m_moveable = true;
    }
}

void RegulableWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_moveable)
    {
        int dx = ui->scrollArea->horizontalScrollBar()->value() - ((event->pos().x() - m_x));
        int dy = ui->scrollArea->verticalScrollBar()->value() - ((event->pos().y() - m_y));

        ui->scrollArea->horizontalScrollBar()->setValue(dx);
        ui->scrollArea->verticalScrollBar()->setValue(dy);

        m_x = event->pos().x();
        m_y = event->pos().y();

        if (ui->scrollArea->width() > ui->pictureLabel->width() && ui->scrollArea->height() > ui->pictureLabel->height())
        {
            QRect rect = ui->pictureLabel->geometry();
            rect.translate(-1 * dx, -1 * dy);
            ui->pictureLabel->setGeometry(rect);
            blend();
            updateView();
            emit hasUpdated();
        }

        //qDebug() << __FILE__ << __LINE__ << dx << dy;
    }
}

void RegulableWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && m_moveable)
    {
        ui->pictureLabel->setCursor(Qt::OpenHandCursor);
        m_moveable = false;
    }
}

bool RegulableWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (ui->rotationHorizontalSlider == watched)
    {
        if (QEvent::MouseButtonPress == event->type() || QEvent::MouseMove == event->type())
        {
            QMouseEvent *me = static_cast<QMouseEvent *>(event);
            if (me)
            {
                QSize size = ui->rotationHorizontalSlider->size();
                int cur = ui->rotationHorizontalSlider->sliderPosition();
                int duration = ui->rotationHorizontalSlider->maximum() - ui->rotationHorizontalSlider->minimum();
                int pos = ui->rotationHorizontalSlider->orientation() == Qt::Vertical ? ui->rotationHorizontalSlider->minimum() + duration * (size.height() - me->y()) / size.height() : ui->rotationHorizontalSlider->minimum() + duration * ((double)me->x() / size.width());

                if (ui->rotationHorizontalSlider->minimum() > pos)pos = 0;
                if (ui->rotationHorizontalSlider->maximum() < pos)pos = ui->rotationHorizontalSlider->maximum();

                if (pos != cur)
                {
                    //rotate(ui->pictureLabel, m_bk, cur - pos);
                    ui->rotationHorizontalSlider->setValue(pos);
                    ui->angleLabel->setText(QString("%1").arg(pos - 180));
                }

                me->accept();

                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}

inline void RegulableWidget::rotate(QLabel *label,
                                    QPixmap &pix,
                                    qreal angle,
                                    Qt::Axis axis)
{
    if (!pix.isNull())
    {
        pix = pix.transformed(QTransform().rotate(angle, axis), Qt::SmoothTransformation);
    }

    if (label)
    {
        label->setFixedSize(pix.size());
        label->setPixmap(pix);
    }
}

void RegulableWidget::scaledZoom(float scale)
{
    QSize size;

    if (m_bk.isNull() || 2 <= scale)
    {
        return;
    }

    if (!scale)
    {
        m_bk = m_ori;
        size = m_bk.size();
    }
    else
    {
        size = 1 == scale ? ui->scrollArea->size() : QSize(m_bk.width() * scale, m_bk.height() * scale);
        m_bk = m_ori.scaled(size, Qt::KeepAspectRatio);
        size = m_bk.size();
    }

    ui->pictureLabel->setFixedSize(size);
    ui->pictureLabel->setPixmap(m_bk);

    blend();
    updateView();
    emit hasUpdated();
}

void RegulableWidget::on_mirroredPushButton_clicked()
{
    rotate(ui->pictureLabel, m_bk, 180.0f, Qt::YAxis);
}
