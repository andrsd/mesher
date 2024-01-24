#include "document.h"
#include <QDebug>
#include "GModel.h"
#include "GModelIO_GEO.h"
#if defined(HAVE_PARSER)
    #include "Parser.h"
    #include "FunctionManager.h"
#endif

Document::Document() : has_file(false), gmodel(new GModel()) {}

Document::~Document()
{
    delete this->gmodel;
}

bool
Document::hasFile() const
{
    return this->has_file;
}

QString
Document::getFileName() const
{
    return { this->gmodel->getFileName().c_str() };
}

void
Document::destroy()
{
    if (this->gmodel != nullptr) {
        this->gmodel->destroy();
        this->gmodel->getGEOInternals()->destroy();
        delete this->gmodel;
    }
    this->gmodel = new GModel();
    this->has_file = false;
}

void
Document::create()
{
    this->gmodel->setFileName("");
    this->gmodel->setName("");
    Msg::ResetErrorCounter();
}

void
Document::load(const QString & file_name)
{
    Msg::ResetErrorCounter();
    this->gmodel->setFileName(file_name.toStdString());
    this->gmodel->setName("ASDF");

#if defined(HAVE_PARSER)
    gmsh_yysymbols.clear();
    gmsh_yystringsymbols.clear();
    std::map<std::string, std::vector<double>> cln(Msg::GetCommandLineNumbers());
    for (auto it = cln.begin(); it != cln.end(); it++)
        gmsh_yysymbols[it->first].value = it->second;
    std::map<std::string, std::string> cls(Msg::GetCommandLineStrings());
    for (auto it = cls.begin(); it != cls.end(); it++)
        gmsh_yystringsymbols[it->first] = std::vector<std::string>(1, it->second);
    gmsh_yyfactory.clear();
    gmsh_yynamespaces.clear();
    FunctionManager::Instance()->clear();
#endif
    // FIXME: merge the file
    // MergeFile(fileName, errorIfMissing);
    
    qDebug() << "load file" << file_name;
    this->has_file = true;
}
