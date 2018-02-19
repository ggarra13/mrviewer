#include <fltk/fl_config.h>
#include <fltk/x.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <config.h>

#ifndef PATH_MAX
#define PATH_MAX 128
#endif

#ifndef _WIN32
// Stolen from KDE!
static int _getprop(Window w, Atom a, Atom type, long len, unsigned char **p){
  Atom real_type;
  int format;
  unsigned long n, extra;
  int status;

  status = XGetWindowProperty(fl_display, w, a, 0L, len, False, type, &real_type, &format, &n, &extra, p);
  if (status != Success || *p == 0)
    return -1;
  if (n == 0)
    XFree((char*) *p);
  return n;
}

// Stolen from KDE!
static bool getSimpleProperty(Window w, Atom a, long &result){
  long *p = 0;

  if (_getprop(w, a, a, 1L, (unsigned char**)&p) <= 0){
    return false;
  }

  result = p[0];
  XFree((char *) p);
  return true;
}
#endif

const char* pname;

void usage() {
  fprintf(stderr, "%s: Usage- %s <scheme>\n", pname, pname);
  exit(1);
}

int main(int argc, char* argv[]) {
  pname = argv[0];
  if (argc != 2) usage();

  char scheme[PATH_MAX];
  strncpy(scheme, argv[1], sizeof(scheme));

  Fl_Config flconfig("flconfig");
  int r;

  if ( (r = flconfig.set("default/scheme", scheme)) ) {
    fprintf(stderr, "%s: Error- Cannot set default scheme.\n", pname);
    fprintf(stderr, "%s: Error- %s.\n", pname, flconfig.strerror());
    exit(3);
  }

#ifndef _WIN32
  // stolen from KDE!
  fl_open_display();
  int screen = DefaultScreen(fl_display);
  Window root = RootWindow(fl_display, screen);

  XEvent ev;
  unsigned int i, nrootwins;
  Window dw1, dw2, *rootwins;

  XQueryTree(fl_display, root, &dw1, &dw2, &rootwins, &nrootwins);

  Atom a = XInternAtom(fl_display, "FLTK_STYLE_WINDOW", False);
  Atom Scheme = XInternAtom(fl_display, "FLTKChangeScheme", False);

  for (i = 0; i < nrootwins; i++) {
    long result = 0;
    getSimpleProperty(rootwins[i],a, result);
    if (result) {
      ev.xclient.type = ClientMessage;
      ev.xclient.display = fl_display;
      ev.xclient.window = rootwins[i];
      ev.xclient.message_type = Scheme;
      ev.xclient.format = 32;

      XSendEvent(fl_display, rootwins[i] , False, 0L, &ev);
    }
  }

  XFlush(fl_display);

  XFree((char *) rootwins);
#else
  // Need to figure out how to do this for Windows...
  // But why bother, we already handle Windows style change events.
#endif
}

