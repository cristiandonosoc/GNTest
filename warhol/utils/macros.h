// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

#pragma once

// Combine permits to stringify arguments like __LINE__
#define COMBINE_INTERNAL(x, y) x##y
#define COMBINE(x, y) COMBINE_INTERNAL(x, y)

// Tell the compiler a function is using a printf-style format string.
// |format_param| is the one-based index of the format string parameter;
// |dots_param| is the one-based index of the "..." parameter.
// For v*printf functions (which take a va_list), pass 0 for dots_param.
// (This is undocumented but matches what the system C headers do.)
#if defined(__GNUC__) || defined(__clang__)
#define PRINTF_FORMAT(format_param, dots_param) \
  __attribute__((format(printf, format_param, dots_param)))
#else
#define PRINTF_FORMAT(format_param, dots_param)
#endif

// Ignore warnings for Windows because they don't have a good way
// of ignoring warnings for certain includes
// (I mean, they did add it in 2018...)
#ifdef _WIN32
#define BEGIN_IGNORE_WARNINGS() __pragma(warning(push, 0))
#define END_IGNORE_WARNINGS() __pragma(warning(pop))
#else
// Other compilers have decent ways of dealing with warnings on external code.
#define BEGIN_IGNORE_WARNINGS()
#define END_IGNORE_WARNINGS()
#endif

#if defined(_MSC_VER)
  #define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define DECLARE_CONSTRUCTOR(class_name) class_name();
#define DELETE_CONSTRUCTOR(class_name) class_name() = delete;
#define DEFAULT_CONSTRUCTOR(class_name) class_name() = default;

#define DECLARE_DESTRUCTOR(class_name) ~class_name();
#define DEFAULT_DESTRUCTOR(class_name) ~class_name() = default;

#define DECLARE_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&);            \
  class_name& operator=(const class_name&);

#define DEFAULT_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&) = default;  \
  class_name& operator=(const class_name&) = default;

#define DELETE_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&) = delete;  \
  class_name& operator=(const class_name&) = delete;

#define DECLARE_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&);                 \
  class_name& operator=(class_name&&);

#define DEFAULT_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&) = default;       \
  class_name& operator=(class_name&&) = default;

#define DELETE_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&) = delete;       \
  class_name& operator=(class_name&&) = delete;

#define ARRAY_SIZE(array) (int)(sizeof((array)) / sizeof((array)[0]))

#define SWAP(a, b) \
  {                \
    auto tmp = a;  \
    a = b;         \
    b = tmp;       \
  }
