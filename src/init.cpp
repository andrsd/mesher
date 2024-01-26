#include "init.h"
#include "GmshMessage.h"
#include "OS.h"
#if defined(HAVE_PLUGINS)
    #include "PluginManager.h"
#endif
#include "robustPredicates.h"
#include "BasisFactory.h"
#include "Options.h"

#if defined(HAVE_PARSER)
    #include "Parser.h"
#endif

Init::Init(int argc, char * argv[])
{
    Msg::Initialize(argc, argv);

    InitOptions(0);
    CheckResources();

#if defined(HAVE_PLUGINS)
    // Initialize the default plugins
    PluginManager::instance()->registerDefaultPlugins();
#endif
    robustPredicates::exactinit(1.0, 1.0, 1.0);
}

Init::~Init()
{
#if defined(HAVE_PARSER)
    // clear parser data
    gmsh_yysymbols.clear();
    gmsh_yystringsymbols.clear();
    gmsh_yyfactory.clear();
    gmsh_yynamespaces.clear();
#endif

    // clear remaining static data
#if defined(HAVE_PLUGINS)
    delete PluginManager::instance();
#endif
    BasisFactory::clearAll();
    Msg::Finalize();
}
