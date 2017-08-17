#include "spinnerconverter.h"
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QLabel>

SpinnerConverter::SpinnerConverter(int size, QObject *parent) :
    QObject(parent),
    size_(size),
    holeRadius_(size * 0.21 / 2)
{
}

void SpinnerConverter::convert(const QString &filename,
                               int ledNumber, int segmentsNumber)
{
    QMovie movie(filename);
    movie.stop();
    movie.jumpToFrame(0);
    animation_.clear();
    qDebug() << segmentsNumber;
    segment_ = 360. / segmentsNumber;
    led_ = (size_ / 2 - holeRadius_) / ledNumber;
    for(int i = 0; i < movie.frameCount(); ++i)
    {
        frame_ = movie.currentImage();
        frame_ = frame_.scaled(size_, size_, Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);
        convertFrame(frame_);
        animation_.push_back(frame_);
        movie.jumpToNextFrame();
    }
}

void SpinnerConverter::convertFrame(QImage &frame)
{
    QImage tmp = frame;
    QPainter painter(&frame);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::black));
    painter.drawRect(0, 0, size_, size_);
    QPainterPath path;
    path.addEllipse(QPointF(frame.width() / 2,
                            frame.height() / 2),
                    frame.width() / 2,
                    frame.height() / 2);
    painter.setClipPath(path);
    painter.drawImage(0, 0, tmp);
    painter.translate(frame.width() / 2, frame.height() / 2);
    painter.drawEllipse(QPoint(0, 0), holeRadius_,
                        holeRadius_);
    painter.save();
    QPen pen(Qt::black);
    pen.setWidth(1);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);
    for(double r = holeRadius_; r < size_ / 2; r += led_)
    {
        painter.drawEllipse(QPointF(0, 0), r, r);
    }
    for(double degree = 0.; degree <= 360.; degree += segment_)
    {
        painter.rotate(segment_);
        painter.drawLine(QPointF(0, 0), QPointF(0, size_ / 2));
    }
    painter.restore();
    painter.end();
    QImage floodImg = QImage(frame.size(), QImage::Format_RGB888);
    QPainter floodPainter(&floodImg);
    floodPainter.setPen(Qt::NoPen);
    floodPainter.setBrush(QBrush(Qt::white));
    floodPainter.drawRect(0, 0, size_, size_);
    QPen pen2(Qt::black);
    pen2.setWidth(1);
    floodPainter.setPen(pen2);
    floodPainter.setBrush(QBrush(Qt::black));
    floodPainter.translate(size_ / 2, size_ / 2);
    floodPainter.drawEllipse(QPoint(0, 0), holeRadius_, holeRadius_);
    floodPainter.setBrush(QBrush(Qt::NoBrush));
    floodPainter.drawEllipse(QPoint(0, 0), size_ / 2, size_ / 2);
    for(double r = holeRadius_; r < size_ / 2; r += led_)
    {
        floodPainter.drawEllipse(QPoint(0, 0), r, r);
    }
    floodPainter.save();
    for(double degree = 0.; degree <= 360.; degree += segment_)
    {
        floodPainter.rotate(segment_);
        floodPainter.drawLine(QPoint(0, 0), QPoint(0, size_ / 2));
    }
    floodPainter.restore();
    floodPainter.end();
    for(double seg = segment_ / 2; seg < 360; seg += segment_)
    {
        for(double led = holeRadius_ + led_ / 2; led < size_ / 2;
            led += led_)
        {
            int x = led * cos(seg * M_PI / 180.) + size_ / 2;
            int y = led * sin(seg * M_PI / 180.) + size_ / 2;
            qDebug() << size_ << led << seg << x << y <<
                        x - size_ / 2 << y - size_ / 2 <<
                        floodImg.pixelColor(x, y) <<
                        QColor(Qt::black);
//            floodImg.setPixelColor(x, y, Qt::black);
            QColor color = floodColor(QPoint(x, y),
                                      floodImg,
                                      frame);
            floodPaint(QPoint(x, y), color, floodImg, frame);
        }
    }

//    QLabel *lab = new QLabel;
//    lab->setPixmap(QPixmap::fromImage(floodImg));
//    lab->show();
}

QColor SpinnerConverter::floodColor(QPoint start, QImage &flood,
                                    const QImage &frame)
{
    ColorCounter colorCounter{0, 0.0, 0.0, 0.0};
    recursFloodColor(start, flood, frame, colorCounter);
    return QColor(colorCounter.r / colorCounter.count,
                  colorCounter.g / colorCounter.count,
                  colorCounter.b / colorCounter.count);
}

void SpinnerConverter::recursFloodColor(QPoint point, QImage &flood,
                                        const QImage &frame,
                                        ColorCounter &counter)
{
    //qDebug() << point.x() << point.y() << flood.pixelColor(point);
    if(flood.pixelColor(point) != Qt::white) return;
    counter.count++;
    counter.r += frame.pixelColor(point).red();
    counter.g += frame.pixelColor(point).green();
    counter.b += frame.pixelColor(point).blue();
    flood.setPixelColor(point, Qt::red);
    recursFloodColor(QPoint(point.x() - 1, point.y()), flood,
                     frame, counter);
    recursFloodColor(QPoint(point.x() + 1, point.y()), flood,
                     frame, counter);
    recursFloodColor(QPoint(point.x(), point.y() - 1), flood,
                     frame, counter);
    recursFloodColor(QPoint(point.x(), point.y() + 1), flood,
                     frame, counter);
}

void SpinnerConverter::floodPaint(QPoint point, const QColor &color,
                                  QImage &flood, QImage &frame)
{
    if(flood.pixelColor(point) != Qt::red) return;
    flood.setPixelColor(point, Qt::green);
    frame.setPixelColor(point, color);
    floodPaint(QPoint(point.x() - 1, point.y()), color,
               flood, frame);
    floodPaint(QPoint(point.x() + 1, point.y()), color,
               flood, frame);
    floodPaint(QPoint(point.x(), point.y() - 1), color,
               flood, frame);
    floodPaint(QPoint(point.x(), point.y() + 1), color,
               flood, frame);
}
