#pragma once
#ifdef PAPAGO_EXPORT
#define PAPAGO_API __declspec(dllexport)
#else
#define PAPAGO_API __declspec(dllimport)
#endif

#include <memory>
