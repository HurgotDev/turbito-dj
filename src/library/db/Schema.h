#pragma once

#include "library/db/Database.h"

// Runs the pending migrations, tracked by PRAGMA user_version.
void migrate(Database& db);
