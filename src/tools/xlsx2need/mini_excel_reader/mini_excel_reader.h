#ifndef _TINYXLSX_H_
#define _TINYXLSX_H_
#include <vector>
#include <string>
#include <squick/core/platform.h>
#include "third_party/rapidxml/rapidxml.hpp"
#include "third_party/rapidxml/rapidxml_iterators.hpp"
#include "third_party/rapidxml/rapidxml_print.hpp"
#include "third_party/rapidxml/rapidxml_utils.hpp"
#include "third_party/common/lexical_cast.hpp"

namespace squick::tools::file_process::mini_excel_reader {
    struct Cell
    {
        std::string value;
        std::string type;
    };

    struct Range
    {
        int firstRow;
        int lastRow;
        int firstCol;
        int lastCol;
    };

    class Sheet
    {
    public:
        ~Sheet();

        bool visible() { return _visible; }
        const std::string& getName() { return _name; }
        Range& getDimension() { return _dimension; }

        Cell* getCell(int row, int col);
    private:
        friend class ExcelFile;

        int toIndex(int row, int col);

        int _sheetId;
        bool _visible;
        Range _dimension;

        std::string _rid;
        std::string _path;
        std::string _name;

        std::vector<Cell*> _cells;
    };

    class Zip;

    class ExcelFile
    {
    public:
        ~ExcelFile();
        bool open(const char* filename);

        Sheet* getSheet(const char* name);
        std::vector<Sheet>& sheets() { return _sheets; }

    private:

        void readWorkBook(const char* filename);
        void readWorkBookRels(const char* filename);
        void readSharedStrings(const char* filename);
        void readStyles(const char* filename);
        void readSheet(Sheet& sh);

        void parseCell(const std::string& value, int& row, int& col);
        void parseRange(const std::string& value, Range& range);

        std::vector<std::string> _sharedString;
        std::vector<Sheet> _sheets;
        Zip* _zip;
    };
}

#endif
