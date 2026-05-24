#ifndef DOCUMENT_H
#define DOCUMENT_H

#pragma once
#include <string>
#include <QDateTime>

// Abstract base class
class Document {
public:
    virtual ~Document() = default;

    // Methods to be implemented by subclasses
    virtual QString content() const = 0;
    virtual void setContent(const QString& content) = 0;

    // Common functionality for all documents
    void rename(const std::string& newName) { name_ = newName; }

    const std::string& id() const noexcept { return id_; }
    const std::string& name() const noexcept { return name_; }
    QDateTime created() const noexcept { return created_; }
    QDateTime updated() const noexcept { return updated_; }

protected:
    // Constructor only accessible to derived classes
    Document(std::string id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name))
        , created_(QDateTime::currentDateTime())
        , updated_(QDateTime::currentDateTime())
    {}

    std::string id_;
    std::string name_;
    QDateTime created_;
    QDateTime updated_;
};

#endif
