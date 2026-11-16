// common.cpp — definicje globali wspólnych
#if defined(_MSC_VER) && !defined(__clang__)
#pragma execution_character_set("utf-8")
#endif

#include "common.h"

namespace rpg {

Rng g_rng(0xC0FFEEu);

}  // namespace rpg
