#pragma once

#include <list>
#include <map>
namespace sqkctl {
class ClassProperty {
  public:
    ClassProperty() {}

    std::map<std::string, std::string> descList; // tag, value
    std::string name;
    std::string type;
};

class ClassRecord {
  public:
    ClassRecord() {}

    struct RecordColDesc {
        int index;
        std::string type;
        std::string desc;
    };

    std::string className;
    std::map<std::string, std::string> descList;    // tag, value
    std::map<std::string, RecordColDesc *> colList; // tag, desc
};

class ClassStruct {
  public:
    ClassStruct() {}
    std::string className;
    std::map<std::string, ClassProperty *> xPropertyList; // key, desc
    std::map<std::string, ClassRecord *> xRecordList;     // name, desc
};

class ClassElement {
  public:
    ClassElement() {}

    class ElementData {
      public:
        std::map<std::string, std::string> xPropertyList;
    };

    std::map<std::string, ElementData *> xElementList; // key, iniList
};

class ClassData {
  public:
    ClassStruct xStructData;
    ClassElement xIniData;
    bool beIncluded = false;
    bool beParted = false;
    std::string filePath;
    std::string fileFolder;
    std::list<std::string> includes;
    std::list<std::string> parts;
    std::list<std::string> ref;
    std::list<std::string> parents;
};

} // namespace sqkctl