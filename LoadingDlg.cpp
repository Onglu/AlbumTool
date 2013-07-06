#include "LoadingDlg.h"
#include <QLabel>
#include <QMovie>
#include <QVBoxLayout>
#include <QDebug>

LoadingDlg::LoadingDlg() : QDialog(0, Qt::FramelessWindowHint)
{
    m_movieLabel = new QLabel(this);
    m_movieLabel->setFixedSize(126, 22);

    QMovie *movie = new QMovie(":/images/loading.gif");
    movie->setSpeed(100);
    m_movieLabel->setMovie(movie);

    m_textLabel = new QLabel(this);
    QFont font = m_textLabel->font();
    font.setBold(true);
    m_textLabel->setFixedHeight(16);
    m_textLabel->setFont(font);
    m_textLabel->setStyleSheet("color:blue;");

    QVBoxLayout *vbl = new QVBoxLayout;
    vbl->addWidget(m_movieLabel);
    vbl->addWidget(m_textLabel);
    vbl->setMargin(0);

    setFixedSize(126, 38);
    setLayout(vbl);
    setAttribute(Qt::WA_TranslucentBackground);
}

void LoadingDlg::showProcess(bool show, QRect global, const QString &info)
{
    if (isHidden() && show)
    {
        m_movieLabel->movie()->start();
        m_textLabel->setText(info);
        qDebug() << __FILE__ << __LINE__ << info;

        QPoint pos((global.width() - this->width()) / 2, (global.height() - this->height()) / 2);
        move(global.topLeft() + pos);
        //exec();
        this->show();
    }

    if (isVisible() && !show)
    {
        m_movieLabel->movie()->stop();
        m_textLabel->clear();
        //accept();
        this->hide();
    }
}

void LoadingDlg::showEvent(QShowEvent *)
{
//    qDebug() << __FILE__ << __LINE__ << "stttt" << m_textLabel->text();
//    emit ss(true);
}
