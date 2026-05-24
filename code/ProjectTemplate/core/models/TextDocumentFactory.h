#ifndef TEXTDOCUMENTFACTORY_H
#define TEXTDOCUMENTFACTORY_H

#include "DocumentFactory.h"
#include "TextDocument.h"

class TextDocumentFactory : public DocumentFactory {
public:
    std::unique_ptr<Document> createDocument(std::string id, std::string name) override {
        return std::make_unique<TextDocument>(id, name);
    }
};

#endif // TEXTDOCUMENTFACTORY_H
