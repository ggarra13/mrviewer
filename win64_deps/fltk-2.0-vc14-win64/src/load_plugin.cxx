/*
 * "$Id: load_plugin.cxx 4649 2005-11-17 08:27:19Z spitzak $"
 *
 * This is a wrapper to make it simple to load plugins on various
 * systems. load_plugin(file, symbol) will load the file as a
 * plugin and then return a pointer to "symbol" in that file.
 *
 * If symbol is null it will return a non-zero value if the plugin
 * loads but you cannot use this value for anything.
 *
 * If there is any problem (file not found, does not load as a plugin,
 * the symbol is not found) it will return null if there is any problem
 * and print debugging info on stderr.
 *
 */

#include <fltk/load_plugin.h>
#include <stdio.h>
#include <config.h>

#if defined(_WIN32) && !defined(__CYGWIN__)

# include <fltk/x.h>
# include <fltk/utf.h>
# include <windows.h>
# include <winbase.h>

void* load_plugin(const char* name, const char* symbol) {
  wchar_t ucs[1024];
  utf8towc(name, strlen(name), ucs, 1024);
  HINSTANCE handle = __LoadLibraryW(ucs);
  if (handle) {
    if (!symbol) return (void*)handle;
    void* f = (void*)GetProcAddress(handle, symbol);
    if (f) return f;
  }
  char* msgbuf = 0;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&msgbuf,
		0,
		NULL);
  fprintf(stderr, "%s\n", msgbuf);
  LocalFree(msgbuf);
  return 0;
}

#else
#if HAVE_DLOPEN
#include <unistd.h>

#ifdef __APPLE__
# include "dlload_osx.cxx"
# define RTLD_NOW 1 // set to anything for now
#else
# include <dlfcn.h>
#endif

void* load_plugin(const char* name, const char* symbol) {
  // do not allow plugins if this executable is setuid
  if (getuid() != geteuid())  {
    fprintf(stderr, "%s: plugins disabled in setuid programs\n", name);
    return 0;
  }
  void* handle = dlopen(name, RTLD_NOW);
  if (handle) {
    if (!symbol) return handle;
    void* f = dlsym(handle, symbol);
    if (f) return f;
  }
  fprintf(stderr, "%s\n", dlerror());
  return 0;
}

#else

void* load_plugin(const char* name, const char*) {
  fprintf(stderr, "%s: loading of plugins not supported\n", name);
  return 0;
}

#endif
#endif

//
// End of "$Id: load_plugin.cxx 4649 2005-11-17 08:27:19Z spitzak $"
//
