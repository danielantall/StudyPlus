#ifndef DOCUMENTFACTORY_H
#define DOCUMENTFACTORY_H
#include "core/models/Document.h"

class DocumentFactory {
public:
    virtual std::unique_ptr<Document> createDocument(std::string id, std::string name) = 0;
    virtual ~DocumentFactory() {}
    };
#endif
