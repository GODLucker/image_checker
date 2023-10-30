#include "ImageProcessor.h"
#include <QDebug>

cv::Mat ImageProcessor::QImageToMat(const QImage &image) {
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

cv::Mat ImageProcessor::compute_pHash(const cv::Mat &img) {
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

double ImageProcessor::compareImages(const cv::Mat &image1, const cv::Mat &image2) {
    cv::Mat hash1 = compute_pHash(image1);
    cv::Mat hash2 = compute_pHash(image2);
    double distance = cv::norm(hash1, hash2, cv::NORM_HAMMING);
    double similarity = (1.0 - distance/64.0) * 100.0; // 64 bits for the 8x8 hash
    return similarity;
}
