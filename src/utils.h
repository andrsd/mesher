#pragma once

#include <QString>

class GEntity;

/// Get entity name based on the type and tag. This is the name shown to the user
///
/// @param e Entity
/// @return Entity name (like "Point 1", etc.)
QString getEntityName(GEntity * e);
