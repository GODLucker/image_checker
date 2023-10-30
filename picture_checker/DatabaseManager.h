#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QImage>
#include <QBuffer>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


class DatabaseManager {
public:
    DatabaseManager();
    void storeImageToDatabase(const QImage &image, int hashValue, double similarityPercentage);
private:
    QSqlDatabase db;
};

#endif

