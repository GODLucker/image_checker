#include <QImage>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

class ImageProcessor {
public:
    double processImage(const QImage& image);
    static cv::Mat compute_pHash(const cv::Mat &img);
    static double compareImages(const cv::Mat &image1, const cv::Mat &image2);

private:
    cv::Mat QImageToMat(const QImage &image);


};
