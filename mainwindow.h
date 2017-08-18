#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QGridLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QProgressBar;
class QMovie;
class QTimer;
class SpinnerConverter;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *ev);

private:
    // Widgets
    QWidget *mainWidget_;
    QGridLayout *mainLayout_;
    QLabel *originalImageLabel_;
    QLabel *resultImageLabel_;
    QPushButton *openFileButton_;
    QPushButton *playFileButton_;
    QPushButton *openSpnButton_;
    QPushButton *playSpnButton_;
    QPushButton *convertButton_;
    QPushButton *saveAsButton_;
    QLineEdit *ledsNumberEdit_;
    QLineEdit *segmentsNumberEdit_;
    QLabel *ledsLabel_;
    QLabel *segmentsLabel_;
    QProgressBar *progressBar_;

    // Objects
    QMovie *originalAnimation_;
    const QVector<QImage> *resultAnimation_;
    SpinnerConverter *converter_;
    QString filename_;
    QTimer *timer_;

private slots:
    void openFile();
    void playFile();
    void playSpn();
    void convert();

    void onTimer();
    void onConvertDone();
};

#endif // MAINWINDOW_H
