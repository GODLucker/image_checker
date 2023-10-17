#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QDebug>
#include <QByteArray>
#include <QBuffer>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

cv::Mat QImageToMat(const QImage &image) {
    cv::Mat mat;
    switch (image.format()) {
    case QImage::Format_RGB888:
        mat = cv::Mat(image.height(), image.width(), CV_8UC3, (uchar*)image.bits(), image.bytesPerLine());
        break;
    case QImage::Format_RGB32:
        mat = cv::Mat(image.height(), image.width(), CV_8UC4, (uchar*)image.bits(), image.bytesPerLine());
        cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
        break;
    default:
        qWarning() << "ERROR: QImage format not handled in switch:" << image.format();
        break;
    }
    return mat;
}

cv::Mat compute_pHash(const cv::Mat &img) {
    cv::Mat img_gray, img_resized, dct_result, dct_low_freq;
    cv::cvtColor(img, img_gray, cv::COLOR_BGR2GRAY);
    cv::resize(img_gray, img_resized, cv::Size(32, 32), 0, 0, cv::INTER_LINEAR);

    img_resized.convertTo(img_resized, CV_32FC1);  // Конвертація в потрібний тип даних
    cv::dct(img_resized, dct_result);

    dct_low_freq = dct_result(cv::Rect(0, 0, 8, 8));

    cv::Scalar mean_scalar = cv::mean(dct_low_freq);
    double mean = mean_scalar.val[0];
    cv::Mat hash = dct_low_freq >= mean;
    return hash;
}

void AppWidget::storeImageToDatabase(const QImage &image, int hashValue, double similarityPercentage) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    image.save(&buffer, "PNG");

    QSqlQuery query;
    query.prepare("INSERT INTO images (image, hashValue, similarityPercentage) VALUES (?, ?, ?)");
    query.addBindValue(byteArray);
    query.addBindValue(hashValue);
    query.addBindValue(similarityPercentage);

    if (!query.exec    ()) {
        qWarning() << "Error saving image to database:" << query.lastError().text();
    }
}

AppWidget::AppWidget() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/lucker/Desktop/image_checker/picture_checker/data.db");
    if (!db.open()) {
        qWarning() << "Error opening database:" << db.lastError().text();
    }
    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qWarning() << "SQLite driver not available";
        return;
    }
    if (!db.tables().contains("images")) {
        QSqlQuery query;
        query.exec("CREATE TABLE images (id INTEGER PRIMARY KEY AUTOINCREMENT, image BLOB, hashValue INT, similarityPercentage DOUBLE)");
    }
    if (!db.open()) {
        qWarning() << "Error opening database:" << db.lastError().text();
    }

    startStopButton = new QPushButton("Start", this);
    resultLabel = new QLabel("Similarity: N/A", this);

    layout->addWidget(startStopButton);
    layout->addWidget(resultLabel);

    timer = new QTimer(this);

    connect(startStopButton, &QPushButton::clicked, this, &AppWidget::toggleScreenCapture);
    connect(timer, &QTimer::timeout, this, &AppWidget::captureScreen);
}

void AppWidget::toggleScreenCapture() {
    if (timer->isActive()) {
        timer->stop();
        startStopButton->setText("Start");
    } else {
        //timer->start(60000) //Знімок 60 секунд
	  timer->start(3000);
	//Test Знімок екрану кожні 3 секунди
        startStopButton->setText("Stop");
    }
}

void AppWidget::captureScreen() {
    QScreen *screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QPixmap pixmap = screen->grabWindow(0);
    QImage currentImage = pixmap.toImage();
    cv::Mat currentMat = QImageToMat(currentImage);

    cv::Mat currentHash = compute_pHash(currentMat);
    int hashValue = cv::countNonZero(currentHash);

    // Розрахунок відсотка подібності з попереднім зображенням
    double similarityPercentage = 0.0;
    if (!lastMat.empty()) {
        similarityPercentage = compareImages(lastMat, currentMat);
    }

    storeImageToDatabase(currentImage, hashValue, similarityPercentage);

    // Збереження поточного зображення як попереднього для наступного знімка
    lastMat = currentMat;

    // Виведення результату подібності
    if (similarityPercentage > 0) {
        resultLabel->setText(QString("Similarity: %1%").arg(similarityPercentage, 0, 'f', 2));
    }
}

double AppWidget::compareImages(const cv::Mat &image1, const cv::Mat &image2) {
    cv::Mat hash1 = compute_pHash(image1);
    cv::Mat hash2 = compute_pHash(image2);

    double distance = cv::norm(hash1, hash2, cv::NORM_HAMMING);
    double similarity = (1.0 - distance/64.0) * 100.0; // 64 bits for the 8x8 hash
    return similarity;
}

