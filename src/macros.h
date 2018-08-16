// Copyright 2018, Cristián Donoso.
// This code has a BSD license. See LICENSE.

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

#define DECLARE_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&);            \
  class_name& operator=(const class_name&);

#define DELETE_COPY_AND_ASSIGN(class_name) \
  class_name(const class_name&) = delete;  \
  class_name& operator=(const class_name&) = delete;

#define DECLARE_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&);                 \
  class_name& operator=(class_name&&);

#define DELETE_MOVE_AND_ASSIGN(class_name) \
  class_name(class_name&&) = delete;       \
  class_name& operator=(class_name&&) = delete;

