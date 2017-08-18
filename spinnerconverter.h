#ifndef SPINNERCONVERTER_H
#define SPINNERCONVERTER_H

#include <QObject>
#include <QVector>
#include <QImage>

class SpinnerConverter : public QObject
{
    Q_OBJECT
public:
    explicit SpinnerConverter(int size, QObject *parent = nullptr);
    void convert(const QString &filename, int ledNumber,
                 int segmentsNumber);
    const QVector<QImage> *getAnimation() const {return &animation_;}

private:
    struct ColorCounter
    {
        int count;
        double r;
        double g;
        double b;
    };

    int size_;
    QImage frame_;
    QVector<QImage> animation_;
    double holeRadius_;
    double segment_;
    double led_;

    void convertFrame(QImage &frame);
    QColor floodColor(QPoint start, QImage &flood,
                      const QImage &frame, QPainter &painter);
    void recursFloodColor(QPoint point, QImage &flood,
                          const QImage &frame, ColorCounter &counter,
                          QPainter &painter);
    void floodPaint(QPoint point, const QColor &color, QImage &flood,
                    QImage &frame, QPainter &painter,
                    QPainter &floodPainter);

signals:
    void progress(int progress);
    void done();

public slots:
};

#endif // SPINNERCONVERTER_H
