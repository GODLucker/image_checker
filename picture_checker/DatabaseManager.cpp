#include "DatabaseManager.h"

// Реалізація DatabaseManager
DatabaseManager::DatabaseManager() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/lusker/Desktop/Qt/image_checker/picture_checker/data.db");

    if (!QSqlDatabase::isDriverAvailable("QSQLITE")) {
        qWarning() << "SQLite driver not available";
        return;
    }
    if (!db.open()) {
        qWarning() << "Error opening database:" << db.lastError().text();
    }

    if (!db.tables().contains("images")) {
        QSqlQuery query;
        query.exec("CREATE TABLE images (id INTEGER PRIMARY KEY AUTOINCREMENT, image BLOB, hashValue INT, similarityPercentage DOUBLE)");
    }
}


void DatabaseManager::storeImageToDatabase(const QImage &image, int hashValue, double similarityPercentage) {
    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    image.save(&buffer, "PNG");

    QSqlQuery query;
    query.prepare("INSERT INTO images (image, hashValue, similarityPercentage) VALUES (?, ?, ?)");
    query.addBindValue(byteArray);
    query.addBindValue(hashValue);
    query.addBindValue(similarityPercentage);

    if (!query.exec()) {
        throw std::runtime_error(QString("Error saving image to database: %1").arg(query.lastError().text()).toStdString());
    }
}
