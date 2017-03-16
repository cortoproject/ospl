/* _project.h
 *
 * This file contains generated code. Do not modify!
 */

#if BUILDING_OSPL_READ && defined _MSC_VER
#define OSPL_READ_EXPORT __declspec(dllexport)
#elif BUILDING_OSPL_READ
#define OSPL_READ_EXPORT __attribute__((__visibility__("default")))
#elif defined _MSC_VER
#define OSPL_READ_EXPORT __declspec(dllimport)
#else
#define OSPL_READ_EXPORT
#endif

