#pragma once

#include <ll/api/base/CompilerPredefine.h>

#ifndef ILAAPI
#  ifdef ILA_EXPORT
#    define ILAPI [[maybe_unused]] LL_SHARED_EXPORT
#  else
#    define ILAPI [[maybe_unused]] LL_SHARED_IMPORT
#  endif
#endif

#ifndef ILACAPI
#  define ILACAPI extern "C" ILAPI
#endif

#ifndef ILANDAPI
#  define ILANDAPI [[nodiscard]] ILAPI
#endif