#ifndef FILESYSTEMSTORAGE_H
#define FILESYSTEMSTORAGE_H

#include <QString>
#include <optional>
#include <QJsonDocument>

class FileSystemStorage {
public:
    static bool ensureDir(const QString& dir);
    static bool writeText(const QString& path, const QString& data);
    static std::optional<QString> readText(const QString& path);

    static bool writeJson(const QString& path, const QJsonDocument& doc);
    static std::optional<QJsonDocument> readJson(const QString& path);
};

#endif // FILESYSTEMSTORAGE_H
