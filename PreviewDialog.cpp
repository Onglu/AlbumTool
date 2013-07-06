#include "PreviewDialog.h"
#include "ui_PreviewDialog.h"
#include "proxy/PictureProxyWidget.h"
#include <QDebug>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QBuffer>
#include <QImageWriter>
#include <QDir>

extern QRect g_rcDesktop;

PreviewDialog::PreviewDialog(QWidget *parent):
    QDialog(parent, Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint),  // Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
    ui(new Ui::PreviewDialog),
    m_nTimerId(0),
    m_bFit(false),
    m_bDraggable(false),
    m_current(0),
    m_scene(NULL)
{
    ui->setupUi(this);
    //setGeometry(g_rcDesktop);
    //showMaximized();

//    ui->tooltipLabel->setText(tr("C:/图片素材/myimages/%1.jpg").arg(1));
//    ui->tooltipLabel->adjustSize();
//    ui->tooltipLabel->move((g_rcDesktop.width() - ui->tooltipLabel->width()) / 2, 10);
//    ui->tooltipLabel->setAttribute(Qt::WA_TranslucentBackground, true);

//    QPalette pal = palette();
//    pal.setColor(QPalette::Background, Qt::transparent);
//    setPalette(pal);

    //ui->nextPushButton->setPalette(QPalette(Qt::transparent));
    //ui->nextPushButton->setMask(m_bk.mask());
    //ui->nextPushButton->setAutoFillBackground(true);

    //m_nTimerId = startTimer(2400);

    //ui->closePushButton->move(g_rcDesktop.width() - 32, 0);
    //ui->pannelFrame->move((g_rcDesktop.width() - ui->pannelFrame->width()) / 2, g_rcDesktop.height() - 36);
    //ui->pictureLabel->setPixmap(m_bk);
}

PreviewDialog::PreviewDialog(const QStringList &pictures, int current, QWidget *parent):
    QDialog(parent, Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint),
    ui(new Ui::PreviewDialog),
    m_nTimerId(0),
    m_bFit(false),
    m_bDraggable(false),
    m_picturesList(pictures),
    m_scene(NULL)
{
    ui->setupUi(this);
    showMaximized();

    if (0 <= current && current < pictures.size())
    {
        switchPage(current);
    }
}

PreviewDialog::~PreviewDialog()
{
    if (m_nTimerId)
    {
        killTimer(m_nTimerId);
    }

    //m_ori.save(m_photoName);

//    if (isWindowModified())
//    {
////        QByteArray bytes;
////        QBuffer buffer(&bytes);
////        buffer.open(QIODevice::WriteOnly);
////        m_ori.save(&buffer);
//        m_ori.save(m_photoName);

//        qDebug() << "picture has been modified";
//    }

    delete ui;
}

void PreviewDialog::timerEvent(QTimerEvent *)
{
//    QPalette pal = palette();
//    pal.setColor(QPalette::Background, QColor(0x00, 0xff, 0x00, 0x00));
//    ui->tooltipLabel->setPalette(pal);
    //ui->tooltipLabel->setMask(m_ori.mask());
//    ui->tooltipLabel->hide();
//    ui->tooltipLabel->setMask(m_ori.mask());
    //ui->tooltipLabel->setAttribute(Qt::WA_TranslucentBackground, true);
}

void PreviewDialog::updateList(const QStringList &pictures, int current)
{
    m_scene = PictureProxyWidget::getFocusScene();
    m_picturesList = pictures;
    if (0 <= current && current < pictures.size())
    {
        switchPage(m_current = current);
    }
}

void PreviewDialog::mousePressEvent(QMouseEvent *event)
{
    if (ui->pannelFrame != childAt(event->pos()) && Qt::LeftButton == event->button() && !m_bFit)
    {
        ui->pictureLabel->setCursor(Qt::ClosedHandCursor);
        m_x = event->pos().x();
        m_y = event->pos().y();
        m_bDraggable = true;
    }
}

void PreviewDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bDraggable)
    {
        int dx = ui->scrollArea->horizontalScrollBar()->value() - ((event->pos().x() - m_x));
        int dy = ui->scrollArea->verticalScrollBar()->value() - ((event->pos().y() - m_y));
        m_x = event->pos().x();
        m_y = event->pos().y();
        ui->scrollArea->horizontalScrollBar()->setValue(dx);
        ui->scrollArea->verticalScrollBar()->setValue(dy);
    }
}

void PreviewDialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (Qt::LeftButton == event->button() && m_bDraggable)
    {
        ui->pictureLabel->setCursor(Qt::OpenHandCursor);
        m_bDraggable = false;
    }
}

void PreviewDialog::wheelEvent(QWheelEvent *event)
{
    if (/*event->modifiers() & Qt::ControlModifier && */
        ui->scrollAreaWidgetContents == childAt(event->pos())->parent() &&
        Qt::Vertical == event->orientation() && !m_bFit)
    {
        QSize scaleSize;

        scaleSize.setWidth(m_bk.width() + event->delta());
        scaleSize.setHeight(m_bk.height() + event->delta());

        if (m_ori.width() < scaleSize.width() / 3 || m_ori.height() < scaleSize.height() / 3)
        {
            ui->zoomInPushButton->setEnabled(false);
            ui->zoomOutPushButton->setEnabled(true);
        }

        if (128 > scaleSize.width() || 96 > scaleSize.height())
        {
            ui->zoomInPushButton->setEnabled(true);
            ui->zoomOutPushButton->setEnabled(false);
        }

        if ((!ui->zoomInPushButton->isEnabled() && 0 < event->delta()) ||
            (!ui->zoomOutPushButton->isEnabled() && 0 > event->delta()))
        {
            event->ignore();
            return;
        }

        if (!ui->zoomInPushButton->isEnabled() && 0 > event->delta())
        {
            ui->zoomInPushButton->setEnabled(true);
        }

        if (!ui->zoomOutPushButton->isEnabled() && 0 < event->delta())
        {
            ui->zoomOutPushButton->setEnabled(true);
        }

        m_bk = m_ori.scaled(scaleSize, Qt::KeepAspectRatio, m_transformMode);
        ui->pictureLabel->setPixmap(m_bk);

        qDebug() <<"delta"<< event->delta();
    }

    event->accept();
}

void PreviewDialog::keyPressEvent(QKeyEvent *event)
{
    if (Qt::Key_Escape == event->key())
    {
        showNormal();
        setGeometry(m_rect);
        ui->pannelFrame->show();
    }
}

void PreviewDialog::resizeEvent(QResizeEvent *)
{
    QSize size = ui->scrollArea->size();

    if (m_bk.isNull() || (96 == size.width() && 26 == size.height()))
    {
        return;
    }

    if (m_bFit)
    {
        on_fitWindowPushButton_clicked();
    }

    //qDebug() << "scrollAreaWidgetContents:" << ui->scrollAreaWidgetContents->rect() << e->size();
}

void PreviewDialog::on_zoomInPushButton_clicked()
{
    if (!ui->zoomOutPushButton->isEnabled())
    {
        ui->zoomOutPushButton->setEnabled(true);
    }

    QSize scaleSize;
    QSize size = ui->scrollArea->size();

    scaleSize.setWidth(m_bk.width() * 1.25);
    scaleSize.setHeight(m_bk.height() * 1.25);

    if ((size.width() < scaleSize.width() || size.height() < scaleSize.height()) && m_bFit)
    {
        m_bFit = false;
        ui->pictureLabel->setCursor(Qt::OpenHandCursor);
    }

    if (m_ori.width() < scaleSize.width() / 6 || m_ori.height() < scaleSize.height() / 6)
    {
        ui->zoomInPushButton->setEnabled(false);
    }

//    QSize imageSize = m_reader.size();
//    imageSize.scale(scaleSize, Qt::KeepAspectRatio);
//    m_reader.setScaledSize(imageSize);
//    m_bk = QPixmap::fromImage(m_reader.read());

    m_bk = m_ori.scaled(scaleSize, Qt::KeepAspectRatio, m_transformMode);
    ui->pictureLabel->setPixmap(m_bk);
    ui->pictureLabel->setToolTip(tr("放大25%"));
}

void PreviewDialog::on_zoomOutPushButton_clicked()
{
    if (!ui->zoomInPushButton->isEnabled())
    {
        ui->zoomInPushButton->setEnabled(true);
    }

    QSize scaleSize;
    QSize size = ui->scrollArea->size();

    scaleSize.setWidth(m_bk.width() * 0.75);
    scaleSize.setHeight(m_bk.height() * 0.75);

    if ((size.width() > scaleSize.width() || size.height() > scaleSize.height()) && m_bFit)
    {
        m_bFit = false;
        ui->pictureLabel->setCursor(Qt::ArrowCursor);
    }

    if (128 > scaleSize.width() || 96 > scaleSize.height())
    {
        ui->zoomOutPushButton->setEnabled(false);
    }

    m_bk = m_ori.scaled(scaleSize, Qt::KeepAspectRatio, m_transformMode);
    ui->pictureLabel->setPixmap(m_bk);
    ui->pictureLabel->setToolTip(tr("缩小25%"));
}

void PreviewDialog::on_zoomResetPushButton_clicked()
{
    if (!ui->zoomInPushButton->isEnabled())
    {
        ui->zoomInPushButton->setEnabled(true);
    }

    if (!ui->zoomOutPushButton->isEnabled())
    {
        ui->zoomOutPushButton->setEnabled(true);
    }

    QSize size = ui->scrollArea->size();
    if ((size.width() < m_ori.width() || size.height() < m_ori.height()) && m_bFit)
    {
        m_bFit = false;
        ui->pictureLabel->setCursor(Qt::ArrowCursor);
    }

    m_bk = m_ori;
    ui->pictureLabel->setPixmap(m_ori);
    ui->pictureLabel->setToolTip(tr("恢复到图片原始大小"));
}

void PreviewDialog::on_fitWindowPushButton_clicked()
{
    QSize size = ui->scrollArea->size();

    if (!m_bFit)
    {
        m_bFit = true;
        ui->pictureLabel->setCursor(Qt::ArrowCursor);
        ui->scrollArea->horizontalScrollBar()->setValue(0);
        ui->scrollArea->verticalScrollBar()->setValue(0);
    }

    if ((size.width() < m_ori.width() + 15 || size.height() < m_ori.height() + 15))
    {
        size -= QSize(15, 15);
    }
    else
    {
        size = m_ori.size();
    }

    m_bk = m_ori.scaled(size, Qt::KeepAspectRatio, m_transformMode);
    ui->pictureLabel->setPixmap(m_bk);
    ui->pictureLabel->setToolTip(tr("匹配到当前窗口大小"));
}

void PreviewDialog::on_fullScreenPushButton_clicked()
{
    if (ui->pannelFrame->isVisible())
    {
        m_rect = geometry();
        showFullScreen();
        ui->pannelFrame->hide();
    }
}

inline void PreviewDialog::switchPage(int index)
{
    if (!m_picturesList.size())
    {
        close();
        return;
    }

    QString fileName(m_picturesList.at(index));
    QFile file(fileName);
    QPixmap pix(fileName);

    fileName = fileName.right(fileName.length() - fileName.lastIndexOf(QDir::separator()) - 1);
    setWindowTitle(tr("%1（%2KB，%3x%4像素） - 第%5/%6张").arg(fileName).arg(file.size() / 1024).arg(pix.width()).arg(pix.height()).arg(index + 1).arg(m_picturesList.size()));

    //qDebug() << "switchPage picture:" << fileName;

    if (1440 < pix.width() || 900 < pix.height())
    {
        m_transformMode = Qt::FastTransformation;
    }
    else
    {
        m_transformMode = Qt::SmoothTransformation;
    }

    //m_reader.setFileName(file);
    m_current = index;
    m_ori = pix;
    m_bk = pix.scaled(pix.size(), Qt::KeepAspectRatio, m_transformMode);
    on_fitWindowPushButton_clicked();
}

void PreviewDialog::on_prevPushButton_clicked()
{
    int index = m_current - 1;

    if (0 > index)
    {
        index = m_picturesList.size() - 1;
    }

    switchPage(index);
}

void PreviewDialog::on_nextPushButton_clicked()
{
    int index = m_current + 1;

    if (index >= m_picturesList.size())
    {
        index = 0;
    }

    switchPage(index);
}

inline void PreviewDialog::rotate(qreal angle, Qt::Axis axis)
{
    QTransform trans;

    trans.rotate(angle, axis);
    //m_ori = m_ori.transformed(transformation);
    m_bk = m_bk.transformed(trans, m_transformMode);

    if (m_bFit)
    {
//        QSize size = ui->scrollArea->size();

//        if ((size.width() < m_ori.width() + 15 || size.height() < m_ori.height() + 15))
//        {
//            size -= QSize(15, 15);
//        }
//        else
//        {
//            size = m_ori.size();
//        }

//        m_bk = m_bk.scaled(size, Qt::KeepAspectRatio, m_transformMode);
    }

    ui->pictureLabel->setPixmap(m_bk);
}

void PreviewDialog::on_leftRotationPushButton_clicked()
{
    rotate(-90.0f);
}

void PreviewDialog::on_rightRotationPushButton_clicked()
{
    rotate(90.0f);
}

void PreviewDialog::on_mirroredPushButton_clicked()
{
    rotate(180.0f, Qt::YAxis);
}

void PreviewDialog::on_deletePushButton_clicked()
{
    if (m_scene)
    {
        emit itemDetached(m_scene, m_picturesList.at(m_current));

        m_picturesList.removeAt(m_current);

        if (m_current >= m_picturesList.size())
        {
            m_current = 0;
        }

        switchPage(m_current);
    }
}
