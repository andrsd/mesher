#include "utils.h"
#include "GEntity.h"

QString
getEntityName(GEntity * e)
{
    return QString("%1 %2").arg(e->getTypeString().c_str(), QString::number(e->tag()));
}
