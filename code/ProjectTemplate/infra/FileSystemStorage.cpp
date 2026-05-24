#include "FileSystemStorage.h"
#include <QDir>

/**
 * @brief Ensures a directory exists, creating it if necessary
 * @param dir Path to the directory to create
 * @return true if directory exists or was created successfully, false otherwise
 *
 * Creates all necessary parent directories in the path.
 */
bool FileSystemStorage::ensureDir(const QString& dir) {
    QDir d;
    return d.mkpath(dir);
}

/**
 * @brief Writes text data to a file
 * @param path Path to the file to write
 * @param data Text content to write
 * @return true if write succeeded, false if file could not be opened
 *
 * Overwrites existing file content (Truncate mode).
 */
bool FileSystemStorage::writeText(const QString& path, const QString& data) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
        return false;
    QTextStream out(&f);
    out << data;
    return true;
}

/**
 * @brief Reads text data from a file
 * @param path Path to the file to read
 * @return Optional containing file content if successful, std::nullopt if file could not be opened
 */
std::optional<QString> FileSystemStorage::readText(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return std::nullopt;
    QTextStream in(&f);
    return in.readAll();
}

/**
 * @brief Writes a JSON document to a file
 * @param path Path to the file to write
 * @param doc QJsonDocument to write
 * @return true if write succeeded, false if file could not be opened
 *
 * Writes JSON with indentation for readability. Overwrites existing file content.
 */
bool FileSystemStorage::writeJson(const QString& path, const QJsonDocument& doc) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    f.write(doc.toJson(QJsonDocument::Indented));
    f.close();
    return true;
}

/**
 * @brief Reads and parses a JSON document from a file
 * @param path Path to the JSON file to read
 * @return Optional containing parsed QJsonDocument if successful, std::nullopt if file could not be opened or JSON is invalid
 *
 * Returns std::nullopt if the file doesn't exist or contains malformed JSON.
 */
std::optional<QJsonDocument> FileSystemStorage::readJson(const QString& path) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly))
        return std::nullopt;
    QByteArray data = f.readAll();
    f.close();
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError)
        return std::nullopt;
    return doc;
}
