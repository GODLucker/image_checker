#include "AppWidget.h"
#include <QBuffer>
#include <QSqlQuery>
#include <QSqlError>



AppWidget::AppWidget(QWidget* parent) : QWidget(parent), dbManager() {
    auto layout = new QVBoxLayout(this);

    startStopButton = new QPushButton("Почати", this);
    resultLabel = new QLabel("Схожість: Немає", this);

    imageView = new QLabel(this);
    layout->addWidget(imageView);

    layout->addWidget(startStopButton);
    layout->addWidget(resultLabel);

    timer = new QTimer(this);

    connect(startStopButton, &QPushButton::clicked, this, &AppWidget::toggleScreenCapture);
    connect(timer, &QTimer::timeout, this, &AppWidget::captureScreen);

    // Ініціалізація порожньої матриці для lastMat
    lastMat = cv::Mat();
}

AppWidget::~AppWidget() {

}

void AppWidget::toggleScreenCapture() {
    if (timer->isActive()) {
        timer->stop();
        startStopButton->setText("Почати");
    } else {
        timer->start(3000); // Захоплення екрану кожні 3 секунди
        startStopButton->setText("Зупинити");
    }
}

void AppWidget::captureScreen() {
    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen)
        return;

    QPixmap pixmap = screen->grabWindow(0);
    imageView->setPixmap(pixmap);
    QImage currentImage = pixmap.toImage();
    cv::Mat currentMat = QImageToMat(currentImage);

    cv::Mat currentMatGray;
    cv::cvtColor(currentMat, currentMatGray, cv::COLOR_BGR2GRAY);

    int hashValue = cv::countNonZero(currentMatGray);

    // Розрахунок відсотка схожості з попереднім зображенням
    double similarityPercentage = 0.0;
    if (!lastMat.empty()) {
        similarityPercentage = ImageProcessor::compareImages(currentMat, lastMat);
         // 64 bits for the 8x8 hash
    }

    storeImageToDatabase(currentImage, hashValue, similarityPercentage);

    lastMat = currentMatGray;
    if (similarityPercentage > 0) {
        resultLabel->setText(QString("Схожість: %1%").arg(similarityPercentage, 0, 'f', 2));
    }
    lastMat = currentMat.clone();

}

cv::Mat AppWidget::QImageToMat(const QImage& image) {
    cv::Mat mat(image.height(), image.width(), CV_8UC4, const_cast<uchar*>(image.bits()), static_cast<size_t>(image.bytesPerLine()));
    cv::Mat mat_rgb;
    cv::cvtColor(mat, mat_rgb, cv::COLOR_RGBA2BGR);
    return mat_rgb;
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
