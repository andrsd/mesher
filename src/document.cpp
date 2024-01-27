#include "document.h"
#include <QDebug>
#include <QThread>
#include "GModel.h"
#include "GModelIO_GEO.h"
#if defined(HAVE_PARSER)
    #include "Parser.h"
    #include "FunctionManager.h"
#endif
#include "Context.h"
#include "OpenFile.h"

class LoadThread : public QThread {
public:
    explicit LoadThread(QString file_name) : QThread(), file_name(file_name) {}

    const QString &
    fileName() const
    {
        return this->file_name;
    }

protected:
    void run() override;

    QString file_name;
};

void
LoadThread::run()
{
    auto fname = this->file_name.toStdString();

    Msg::StatusBar(true, "Reading '%s'...", fname.c_str());

    // don't draw the model while reading
    CTX::instance()->geom.draw = 0;

    auto gmodel = GModel::current();

    gmodel->setFileName(fname);
    gmodel->setName("ASDF");

    int status;
    if (this->file_name.endsWith(".geo"))
        status = GModel::readGEO(fname);
    else if (this->file_name.endsWith(".msh"))
        status = gmodel->readMSH(fname);
    else {
        Msg::Error("Unknown format of '%s' file", fname.c_str());
        CTX::instance()->geom.draw = 1;
        return;
    }

    auto geo_internals = gmodel->getGEOInternals();
    geo_internals->setMaxTag(
        0,
        std::max(geo_internals->getMaxTag(0), gmodel->getMaxElementaryNumber(0)));
    geo_internals->setMaxTag(
        1,
        std::max(geo_internals->getMaxTag(1), gmodel->getMaxElementaryNumber(1)));
    geo_internals->setMaxTag(
        2,
        std::max(geo_internals->getMaxTag(2), gmodel->getMaxElementaryNumber(2)));
    geo_internals->setMaxTag(
        3,
        std::max(geo_internals->getMaxTag(3), gmodel->getMaxElementaryNumber(3)));

    SetBoundingBox();

    CTX::instance()->geom.draw = 1;
    CTX::instance()->mesh.changed = ENT_ALL;

    if (status)
        Msg::StatusBar(true, "Done reading '%s'", fname.c_str());
    else
        Msg::Error("Error loading '%s'", fname.c_str());

    CTX::instance()->fileread = true;

    // TODO: merge the associated option file if there is one
}

//

class SaveThread : public QThread {
public:
    explicit SaveThread(QString file_name) : QThread(), file_name(file_name) {}

    const QString &
    fileName() const
    {
        return this->file_name;
    }

protected:
    void run() override;

    QString file_name;
};

void
SaveThread::run()
{
}

//

Document::Document() :
    QObject(),
    load_thread(nullptr),
    save_thread(nullptr),
    has_file(false),
    gmodel(new GModel())
{
}

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

    this->load_thread = new LoadThread(file_name);
    connect(this->load_thread, &LoadThread::finished, this, &Document::onLoadFinished);
    this->load_thread->start(QThread::IdlePriority);
}

void
Document::onLoadFinished()
{
    this->has_file = true;

    delete this->load_thread;
    this->load_thread = nullptr;
    emit loadFinished();
}

void
Document::save(const QString & file_name)
{
    this->save_thread = new LoadThread(file_name);
    connect(this->save_thread, &SaveThread::finished, this, &Document::onSaveFinished);
    this->save_thread->start(QThread::IdlePriority);
}

void
Document::onSaveFinished()
{
    delete this->save_thread;
    this->save_thread = nullptr;
    emit saveFinished();
}
