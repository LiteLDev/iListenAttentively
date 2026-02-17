#pragma once

#include <ll/api/base/CompilerPredefine.h>

#ifndef ILAAPI
#  ifdef ILA_EXPORT
#    define ILAAPI [[maybe_unused]] LL_SHARED_EXPORT
#  else
#    define ILAAPI [[maybe_unused]] LL_SHARED_IMPORT
#  endif
#endif

#ifndef ILACAPI
#  define ILACAPI extern "C" ILAAPI
#endif

#ifndef ILANDAPI
#  define ILANDAPI [[nodiscard]] ILAAPI
#endif