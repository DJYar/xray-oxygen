#pragma once
#ifdef AICORE_BUILD
#define AI_API __declspec(dllexport)
#else
#define AI_API __declspec(dllimport)
#endif