
#if defined(EXPORT)
#define LOADER_SIGN __declspec(dllexport)
#elif defined(IMPORT)
#define LOADER_SIGN __declspec(dllimport)
#else
#define LOADER_SIGN
#endif
