#pragma once
#include <QString>
#include <optional>

class IStorage {
public:
    virtual ~IStorage() = default;  
    virtual bool ensureDir(const QString& path) = 0;
    virtual bool writeText(const QString& path, const QString& content) = 0;
    virtual std::optional<QString> readText(const QString& path) = 0;
};
