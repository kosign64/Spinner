#include "mainwindow.h"
#include "spinnerconverter.h"
#include <QLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QKeyEvent>
#include <QFileDialog>
#include <QMovie>
#include <QImage>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mainWidget_ = new QWidget(this);
    originalImageLabel_ = new QLabel(mainWidget_);
    resultImageLabel_ = new QLabel(mainWidget_);
    openFileButton_ = new QPushButton(trUtf8("Открыть файл"),
                                      mainWidget_);
    playFileButton_ = new QPushButton(mainWidget_);
    openSpnButton_ = new QPushButton(trUtf8("Открыть spn файл"),
                                     mainWidget_);
    playSpnButton_ = new QPushButton(mainWidget_);
    convertButton_ = new QPushButton(trUtf8("Преобразовать"),
                                     mainWidget_);
    saveAsButton_ = new QPushButton(trUtf8("Сохранить как..."),
                                    mainWidget_);
    ledsNumberEdit_ = new QLineEdit(trUtf8("8"), mainWidget_);
    ledsLabel_ = new QLabel(trUtf8("Количество светодиодов:"),
                            mainWidget_);
    segmentsNumberEdit_ = new QLineEdit(trUtf8("32"), mainWidget_);
    segmentsLabel_ = new QLabel(trUtf8("Количество сегментов:"),
                                mainWidget_);
    mainLayout_ = new QGridLayout;
    mainLayout_->addWidget(originalImageLabel_, 0, 0, 1, 2,
                           Qt::AlignCenter);
    mainLayout_->addWidget(resultImageLabel_, 0, 2, 1, 2,
                           Qt::AlignCenter);
    mainLayout_->addWidget(openFileButton_, 1, 0, 1, 1,
                           Qt::AlignCenter);
    mainLayout_->addWidget(playFileButton_, 1, 1, 1, 1,
                           Qt::AlignCenter);
    mainLayout_->addWidget(openSpnButton_, 1, 2, 1, 1,
                           Qt::AlignCenter);
    mainLayout_->addWidget(playSpnButton_, 1, 3, 1, 1,
                           Qt::AlignCenter);
    mainLayout_->addWidget(ledsLabel_, 2, 1, 1, 2,
                           Qt::AlignCenter);
    mainLayout_->addWidget(ledsNumberEdit_, 3, 1, 1, 2,
                           Qt::AlignCenter);
    mainLayout_->addWidget(segmentsLabel_, 4, 1, 1, 2,
                           Qt::AlignCenter);
    mainLayout_->addWidget(segmentsNumberEdit_, 5, 1, 1, 2,
                           Qt::AlignCenter);
    mainLayout_->addWidget(convertButton_, 6, 1, 1, 1,
                           Qt::AlignCenter);
    mainLayout_->addWidget(saveAsButton_, 6, 2, 1, 1,
                           Qt::AlignCenter);

    QRect screenSize = QApplication::desktop()->screenGeometry();
    const int imageWidgetSize = screenSize.width() / 3;

    originalImageLabel_->setFixedSize(imageWidgetSize,
                                      imageWidgetSize);
    resultImageLabel_->setFixedSize(originalImageLabel_->size());
    originalImageLabel_->setScaledContents(true);

    mainWidget_->setLayout(mainLayout_);
    setCentralWidget(mainWidget_);

    originalAnimation_ = new QMovie(this);
    converter_ = new SpinnerConverter(imageWidgetSize, this);

    connect(openFileButton_, &QPushButton::clicked, this,
            &MainWindow::openFile);
    connect(playFileButton_, &QPushButton::clicked, this,
            &MainWindow::playFile);
    connect(convertButton_, &QPushButton::clicked, this,
            &MainWindow::convert);
}

MainWindow::~MainWindow()
{

}

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
    {
        close();
    }
}

void MainWindow::timerEvent(QTimerEvent *)
{
    static int i = 0;
    if(i >= resultAnimation_->size()) i = 0;
    resultImageLabel_->setPixmap(QPixmap::fromImage(resultAnimation_->operator [](i++)));
}

void MainWindow::openFile()
{
    filename_ = QFileDialog::getOpenFileName(this,
                                             tr("Открыть анимацию"),
                                             QString(),
                     tr("Анимация *.gif (*.gif);; Все файлы (*.*)"));
    if(filename_.isEmpty()) return;
    originalAnimation_->stop();
    originalAnimation_->setFileName(filename_);
    originalImageLabel_->setMovie(originalAnimation_);
}

void MainWindow::playFile()
{
    if(originalAnimation_->state() == QMovie::Running)
    {
        originalAnimation_->setPaused(true);
    }
    else
    {
        originalAnimation_->start();
    }
}

void MainWindow::convert()
{
    if(filename_.isEmpty()) return;
    converter_->convert(filename_, ledsNumberEdit_->text().toInt(),
                        segmentsNumberEdit_->text().toInt());
    resultAnimation_ = converter_->getAnimation();
    startTimer(1000 / 10);
}
