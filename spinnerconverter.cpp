#include "spinnerconverter.h"
#include <QMovie>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QLabel>
#include <cmath>

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
    segment_ = 360. / segmentsNumber;
    led_ = (size_ / 2 - holeRadius_) / ledNumber;
    for(int i = 0; i < movie.frameCount(); ++i)
    {
        frame_ = movie.currentImage();
        frame_ = frame_.scaled(size_, size_, Qt::IgnoreAspectRatio,
                               Qt::SmoothTransformation);
        QImage drawImage = QImage(frame_.size(),
                                  QImage::Format_RGBA8888);
        convertFrame(frame_, drawImage);
        animation_.push_back(drawImage);
        movie.jumpToNextFrame();
        emit progress(i / static_cast<double>(movie.frameCount()) * 100);
    }
    emit progress(100);
    emit done();
}

void SpinnerConverter::convertFrame(const QImage &frame,
                                    QImage &drawFrame)
{
    QPainter painter(&drawFrame);
    painter.setRenderHints(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(0, 0, 0, 255)));
    painter.drawRect(0, 0, size_, size_);
    painter.setBrush(QBrush(QColor(0, 0, 0, 255)));
    painter.drawEllipse(QPoint(size_ / 2, size_ / 2),
                        static_cast<int>(holeRadius_),
                        static_cast<int>(holeRadius_));
    QImage floodImg = QImage(frame.size(), QImage::Format_Mono);
    QPainter floodPainter(&floodImg);
    floodPainter.setPen(Qt::NoPen);
    floodPainter.setBrush(QBrush(Qt::color0));
    floodPainter.drawRect(0, 0, size_, size_);
    QPen pen2(Qt::color1);
    pen2.setWidth(1);
    floodPainter.setPen(pen2);
    floodPainter.setBrush(QBrush(Qt::color1));
    floodPainter.save();
    floodPainter.translate(size_ / 2, size_ / 2);
    floodPainter.drawEllipse(QPoint(0, 0),
                             static_cast<int>(holeRadius_),
                             static_cast<int>(holeRadius_));
    floodPainter.setBrush(QBrush(Qt::NoBrush));
    floodPainter.drawEllipse(QPoint(0, 0), size_ / 2, size_ / 2);
    for(double r = holeRadius_; r < size_ / 2; r += led_)
    {
        floodPainter.drawEllipse(QPoint(0, 0), static_cast<int>(r),
                                 static_cast<int>(r));
    }
    for(double degree = 0.; degree <= 360.; degree += segment_)
    {
        floodPainter.rotate(segment_);
        floodPainter.drawLine(QPoint(0, 0), QPoint(0, size_ / 1.9));
    }
    floodPainter.restore();
    QImage floodImg2 = floodImg;
    QPainter floodPainter2(&floodImg2);
    floodPainter2.setPen(Qt::color1);
    floodPainter2.setBrush(Qt::NoBrush);
    for(double seg = segment_ / 2; seg < 360; seg += segment_)
    {
        for(double led = holeRadius_ + led_ / 2; led < size_ / 2;
            led += led_)
        {
            int x = led * sin(seg * M_PI / 180.) + size_ / 2;
            int y = led * cos(seg * M_PI / 180.) + size_ / 2;
            QPoint point(x, y);
//            qDebug() << size_ << led << seg << x << y <<
//                        x - size_ / 2 << y - size_ / 2 <<
//                        floodImg.pixelColor(x, y) <<
//                        QColor(Qt::color1);
            floodPainter2.drawPoint(point);
            QColor color = floodColor(point,
                                      floodImg,
                                      frame, floodPainter);
            color.setAlpha(128);
            painter.setBrush(QBrush(color));
            painter.drawEllipse(point, static_cast<int>(size_ * 0.02),
                                static_cast<int>(size_ * 0.02));
//            floodPaint(QPoint(x, y), color, floodImg2, drawFrame,
//                       painter,
//                       floodPainter2);
        }
    }
    floodPainter.end();
    floodPainter2.end();
    painter.end();

//    QLabel *lab = new QLabel;
//    lab->setPixmap(QPixmap::fromImage(floodImg2));
//    lab->show();
}

QColor SpinnerConverter::floodColor(QPoint start, QImage &flood,
                                    const QImage &frame,
                                    QPainter &painter)
{
    ColorCounter colorCounter{0, 0.0, 0.0, 0.0};
    recursFloodColor(start, flood, frame, colorCounter, painter);
    return QColor(colorCounter.r / colorCounter.count,
                  colorCounter.g / colorCounter.count,
                  colorCounter.b / colorCounter.count);
}

void SpinnerConverter::recursFloodColor(QPoint point, QImage &flood,
                                        const QImage &frame,
                                        ColorCounter &counter,
                                        QPainter &painter)
{
    if(flood.pixelColor(point) != Qt::color0) return;
    counter.count++;
    counter.r += frame.pixelColor(point).red();
    counter.g += frame.pixelColor(point).green();
    counter.b += frame.pixelColor(point).blue();
    painter.drawPoint(point);
    recursFloodColor(QPoint(point.x() - 1, point.y()), flood,
                     frame, counter, painter);
    recursFloodColor(QPoint(point.x() + 1, point.y()), flood,
                     frame, counter, painter);
    recursFloodColor(QPoint(point.x(), point.y() - 1), flood,
                     frame, counter, painter);
    recursFloodColor(QPoint(point.x(), point.y() + 1), flood,
                     frame, counter, painter);
}

void SpinnerConverter::floodPaint(QPoint point, const QColor &color,
                                  QImage &flood, QImage &frame,
                                  QPainter &painter,
                                  QPainter &floodPainter)
{
    if(flood.pixelColor(point) != Qt::color0) return;
    floodPainter.drawPoint(point);
    painter.setPen(QPen(color));
    painter.drawPoint(point);
    floodPaint(QPoint(point.x() - 1, point.y()), color,
               flood, frame, painter, floodPainter);
    floodPaint(QPoint(point.x() + 1, point.y()), color,
               flood, frame, painter, floodPainter);
    floodPaint(QPoint(point.x(), point.y() - 1), color,
               flood, frame, painter, floodPainter);
    floodPaint(QPoint(point.x(), point.y() + 1), color,
               flood, frame, painter, floodPainter);
}
