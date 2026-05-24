#ifndef TEXTDOCUMENT_H
#define TEXTDOCUMENT_H

#pragma once
#include "Document.h"

class TextDocument : public Document {
public:
    TextDocument(std::string id, std::string name)
        : Document(std::move(id), std::move(name))
    {}

    QString content() const override { return content_; }
    void setContent(const QString& content) override {
        content_ = content;
        updated_ = QDateTime::currentDateTime();
    }

private:
    QString content_;
};

#endif
