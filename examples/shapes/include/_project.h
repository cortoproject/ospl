/* _project.h
 *
 * This file contains generated code. Do not modify!
 */

#if BUILDING_SHAPETYPE && defined _MSC_VER
#define SHAPETYPE_EXPORT __declspec(dllexport)
#elif BUILDING_SHAPETYPE
#define SHAPETYPE_EXPORT __attribute__((__visibility__("default")))
#elif defined _MSC_VER
#define SHAPETYPE_EXPORT __declspec(dllimport)
#else
#define SHAPETYPE_EXPORT
#endif

