//
// Copyright(c) 2016 Gabi Melman.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)
//

#pragma once
//
// include bundled or external copy of fmtlib's chrono support
//

#if !defined(SPDLOG_USE_STD_FORMAT)
#    if !defined(SPDLOG_FMT_EXTERNAL)
#        ifdef SPDLOG_HEADER_ONLY
#            ifndef FMT_HEADER_ONLY
#                define FMT_HEADER_ONLY
#            endif
#        endif
#        if __cplusplus >= 202002L && defined(__clang__)
#            pragma clang diagnostic push
#            pragma clang diagnostic ignored "-Wdeprecated" // Workaround for Clang C++20; remove once fmt >= 9.x
#        endif

#        include <spdlog/fmt/bundled/chrono.h>

#        if __cplusplus >= 202002L && defined(__clang__)
#            pragma clang diagnostic pop
#        endif
#    else
#        include <fmt/chrono.h>
#    endif
#endif
