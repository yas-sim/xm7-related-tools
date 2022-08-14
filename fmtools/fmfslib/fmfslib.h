#ifdef FMFSLIB_EXPORTS
#define FMFSLIB_API __declspec(dllexport)
#else
#define FMFSLIB_API __declspec(dllimport)
#endif
