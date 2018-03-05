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
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QProgressBar>
#include <QTimer>
#include <QIntValidator>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      converting(false)
{
    mainWidget_ = new QWidget(this);
    originalImageLabel_ = new QLabel(mainWidget_);
    resultImageLabel_ = new QLabel(mainWidget_);
    openFileButton_ = new QPushButton(tr("Открыть файл"),
                                      mainWidget_);
    playFileButton_ = new QPushButton(tr("Проиграть"), mainWidget_);
    openSpnButton_ = new QPushButton(tr("Открыть spn файл"),
                                     mainWidget_);
    playSpnButton_ = new QPushButton(tr("Проиграть"), mainWidget_);
    convertButton_ = new QPushButton(tr("Преобразовать"),
                                     mainWidget_);
    saveAsButton_ = new QPushButton(tr("Сохранить как..."),
                                    mainWidget_);
    ledsNumberEdit_ = new QLineEdit(tr("8"), mainWidget_);
    ledsLabel_ = new QLabel(tr("Количество светодиодов:"),
                            mainWidget_);
    segmentsNumberEdit_ = new QLineEdit(tr("64"), mainWidget_);
    segmentsLabel_ = new QLabel(tr("Количество сегментов:"),
                                mainWidget_);
    convertButton_->setEnabled(false);
    progressBar_ = new QProgressBar(mainWidget_);
    progressBar_->setValue(0);
    progressBar_->setRange(0, 100);

    playFileButton_->setEnabled(false);
    playSpnButton_->setEnabled(false);
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
    mainLayout_->addWidget(progressBar_, 7, 1, 1, 2,
                           Qt::AlignCenter);

    QRect screenSize = QApplication::desktop()->screenGeometry();
    const int imageWidgetSize = screenSize.width() / 3;

    originalImageLabel_->setFixedSize(imageWidgetSize,
                                      imageWidgetSize);
    resultImageLabel_->setFixedSize(originalImageLabel_->size());
    progressBar_->setFixedWidth(imageWidgetSize);
    originalImageLabel_->setScaledContents(true);

    mainWidget_->setLayout(mainLayout_);
    setCentralWidget(mainWidget_);

    QIntValidator *validator = new QIntValidator(this);
    validator->setBottom(1);
    ledsNumberEdit_->setValidator(validator);
    segmentsNumberEdit_->setValidator(validator);

    originalAnimation_ = new QMovie(this);
    converter_ = new SpinnerConverter(imageWidgetSize, this);

    timer_ = new QTimer(this);

    connect(openFileButton_, &QPushButton::clicked, this,
            &MainWindow::openFile);
    connect(playFileButton_, &QPushButton::clicked, this,
            &MainWindow::playFile);
    connect(playSpnButton_, &QPushButton::clicked, this,
            &MainWindow::playSpn);
    connect(convertButton_, &QPushButton::clicked, this,
            &MainWindow::convert);
    connect(timer_, &QTimer::timeout, this,
            &MainWindow::onTimer);
    connect(converter_, &SpinnerConverter::progress,
            progressBar_, &QProgressBar::setValue);
    connect(converter_, &SpinnerConverter::done,
            this, &MainWindow::onConvertDone);
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

void MainWindow::onTimer()
{
    static int i = 0;
    if(i >= resultAnimation_->size()) i = 0;
    resultImageLabel_->setPixmap(
             QPixmap::fromImage(resultAnimation_->operator [](i++)));
}

void MainWindow::onConvertDone()
{
    playSpnButton_->setEnabled(true);
    convertButton_->setEnabled(true);
    converting = false;
}

void MainWindow::openFile()
{
    filename_ = QFileDialog::getOpenFileName(this,
                                             tr("Открыть анимацию"),
                                             QString(),
                     tr("Анимация *.gif (*.gif);; Все файлы (*.*)"));
    if(filename_.isEmpty()) return;
    playFileButton_->setEnabled(true);
    originalAnimation_->stop();
    originalAnimation_->setFileName(filename_);
    originalImageLabel_->setMovie(originalAnimation_);
    playFileButton_->setText(tr("Проиграть"));
    if(!converting)
    {
        convertButton_->setEnabled(true);
    }
}

void MainWindow::playFile()
{
    if(originalAnimation_->state() == QMovie::Running)
    {
        playFileButton_->setText(tr("Проиграть"));
        originalAnimation_->setPaused(true);
    }
    else
    {
        playFileButton_->setText(tr("Пауза"));
        originalAnimation_->start();
    }
}

void MainWindow::playSpn()
{
    if(timer_->isActive())
    {
        playSpnButton_->setText(tr("Проиграть"));
        timer_->stop();
    }
    else
    {
        playSpnButton_->setText(tr("Пауза"));
        timer_->start(1000 / 10);
    }
}

void MainWindow::convert()
{
    if(filename_.isEmpty()) return;
    playSpnButton_->setEnabled(false);
    convertButton_->setEnabled(false);
    converting = true;
    playSpnButton_->setText(tr("Проиграть"));
    if(timer_->isActive()) timer_->stop();
    QtConcurrent::run(converter_,
                      &SpinnerConverter::convert,
                      filename_,
                      ledsNumberEdit_->text().toInt(),
                      segmentsNumberEdit_->text().toInt());
    resultAnimation_ = converter_->getAnimation();
}
