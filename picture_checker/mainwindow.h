#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QTimer>
#include <opencv2/opencv.hpp>

class AppWidget : public QWidget {
    Q_OBJECT
public:
    AppWidget();

public slots:
    void toggleScreenCapture();
    void captureScreen();
    void storeImageToDatabase(const QImage &image, int, double);
private:
    double compareImages(const cv::Mat &image1, const cv::Mat &image2);

    QPushButton *startStopButton;
    QLabel *resultLabel;
    QTimer *timer;
    cv::Mat lastMat;
};

#endif
