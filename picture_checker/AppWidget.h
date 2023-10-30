#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QScreen>
#include <QGuiApplication>
#include <QPixmap>
#include <QImage>
#include <opencv2/opencv.hpp>
#include "DatabaseManager.h"
#include "ImageProcessor.h"

class AppWidget : public QWidget {
    Q_OBJECT

public:
    AppWidget(QWidget* parent = nullptr);
    ~AppWidget();

private slots:
    void toggleScreenCapture();
    void captureScreen();

private:
    QPushButton* startStopButton;
    QLabel* resultLabel;
    QTimer* timer;
    QLabel* imageView;
    cv::Mat lastMat; // Для збереження попереднього зображення
    cv::Mat currentMatGray; // Для збереження поточного зображення в відтінках сірого
    DatabaseManager dbManager; // Добавьте эту строку

    cv::Mat QImageToMat(const QImage& image);
    double compareImages(const cv::Mat& image1, const cv::Mat& image2);
    void storeImageToDatabase(const QImage& image, int hashValue, double similarityPercentage);
};


#endif
