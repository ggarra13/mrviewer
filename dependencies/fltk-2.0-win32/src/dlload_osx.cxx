/*
  Support for dynamic loading of MH_BUNDLEs on Darwin / Mac OS X
  Emulates dyld functions
*/

#include <mach-o/dyld.h>

const char* dlerror(void) { return "Some error loading plugin"; }

void* dlopen(const char* path, int mode) {

  NSObjectFileImage bundle_image;
  if (NSCreateObjectFileImageFromFile(path, &bundle_image)
      != NSObjectFileImageSuccess) {
    return 0;
  }
 
  NSModule handle =
    NSLinkModule(bundle_image, path, NSLINKMODULE_OPTION_PRIVATE);
  // can this fail here with handle being zero?
  NSDestroyObjectFileImage(bundle_image);

  /* call the init function of the bundle */
  NSSymbol bundle_init_nssymbol =
    NSLookupSymbolInModule(handle, "__init");
  if (bundle_init_nssymbol != 0) {
    void (*bundle_init)(void) = 
      (void(*)())NSAddressOfSymbol(bundle_init_nssymbol);
    bundle_init();
  }

  return handle;
}

void* dlsym(void* handle, const char *symbol) {
  NSSymbol nss_symbol =
    NSLookupSymbolInModule((NSModule)handle, symbol);
  if (nss_symbol) return NSAddressOfSymbol(nss_symbol);
  return 0;
}

int dlclose(void* handle) {

  /* call the fini function of the bundle */
  NSSymbol bundle_fini_nssymbol =
    NSLookupSymbolInModule((NSModule)handle, "__fini");
  if (bundle_fini_nssymbol != 0) {
    void (*bundle_fini)(void) =
      (void(*)())NSAddressOfSymbol(bundle_fini_nssymbol);
    bundle_fini();
  }

  return (int)NSUnLinkModule((NSModule)handle, 0);
}
