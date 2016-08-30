//
// "$Id: run.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// MacOS specific code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2006 by Bill Spitzak and others.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA.
//
// Please report all bugs and problems on the following page:
//
//    http://www.fltk.org/str.php
//

//// From the inner edge of a MetroWerks CodeWarrior CD:
// (without permission)
//
// Three Compiles for 68Ks under the sky,
// Seven Compiles for PPCs in their fragments of code,
// Nine Compiles for Mortal Carbon doomed to die,
// One Compile for Mach-O Cocoa on its Mach-O throne,
// in the Land of MacOS X where the Drop-Shadows lie.
// 
// One Compile to link them all, One Compile to merge them,
// One Compile to copy them all and in the bundle bind them,
// in the Land of MacOS X where the Drop-Shadows lie.

#include <config.h>
#include <fltk/events.h>
#include <fltk/x.h>
#include <fltk/Window.h>
#include <fltk/SystemMenuBar.h>
#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/Style.h>
#include <fltk/utf.h>
#include <fltk/Cursor.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

//#define DEBUG_SELECT		// UNCOMMENT FOR SELECT()/THREAD DEBUGGING
#ifdef DEBUG_SELECT
#include <stdio.h>	// testing
#define DEBUGMSG(msg)		fprintf(stderr, msg);
#define DEBUGPERRORMSG(msg)	perror(msg)
#else
#define DEBUGMSG(msg)
#define DEBUGPERRORMSG(msg)
#endif /*DEBUGSELECT*/

using namespace fltk;

// fabien: added Cairo support for Quartz
#if USE_CAIRO
# include <cairo.h>
# include <cairo-quartz.h>
  FL_API cairo_t * fltk::cr=0;
#endif

#if USE_CAIRO
namespace fltk {
    cairo_surface_t * cairo_create_surface(fltk::Window* w) {
      return cairo_quartz_surface_create_for_cg_context((CGContext*) w->backbuffer(), 
                                                        w->w(), w->h());
    }
}
#endif

// these pointers are set by the lock() function:
static void nothing() {}
void (*fl_lock_function)() = nothing;
void (*fl_unlock_function)() = nothing;

////////////////////////////////////////////////////////////////
// interface to select call:
//
// WAS: This is similar to the x.cxx version but with USE_POLL set to 0
// I modified this from Matt's version to more closely match the X
// version, with the addition of the locks for the parallel thread. It
// may be better to kill the parallel thread and launch it on the next
// wait call, however.
//
// Surely this can be done without a second thread, what we need
// is a file descriptor that becomes ready when an Apple UI event
// is pending.

static fd_set fdsets[3];
static int maxfd;
#define POLLIN  1
#define POLLOUT 4
#define POLLERR 8

static int nfds = 0;
static int fd_array_size = 0;
static struct FD {
  int fd;
  short events;
  void (*cb)(int, void*);
  void* arg;
} *fd = 0;
static pthread_t select_thread = 0;	// parallel thread running select()
static int G_pipe[2] = { 0,0 };		// for waking up that thread
static pthread_mutex_t select_mutex;	// lock for above data
static inline void wake_select_thread() {
  if (select_thread) write(G_pipe[1], "x", 1);
}

//+++ verify port to FLTK2
void fltk::add_fd(int n, int events, FileHandler cb, void *v) {
  remove_fd(n, events);
  pthread_mutex_lock(&select_mutex);
  int i = nfds++;
  if (i >= fd_array_size) {
    fd_array_size = 2*fd_array_size+1;
    fd = (FD*)realloc(fd, fd_array_size*sizeof(FD));
  }
  fd[i].cb = cb;
  fd[i].arg = v;
  fd[i].fd = n;
  fd[i].events = events;
  if (events & POLLIN) FD_SET(n, &fdsets[0]);
  if (events & POLLOUT) FD_SET(n, &fdsets[1]);
  if (events & POLLERR) FD_SET(n, &fdsets[2]);
  if (n > maxfd) maxfd = n;
  wake_select_thread();
  pthread_mutex_unlock(&select_mutex);
}

//+++ verify port to FLTK2
void fltk::add_fd(int fd, FileHandler cb, void* v) {
  add_fd(fd, POLLIN, cb, v);
}

//+++ verify port to FLTK2
void fltk::remove_fd(int n, int events) {
  pthread_mutex_lock(&select_mutex);
  int i,j;
  maxfd = 0;
  for (i=j=0; i<nfds; i++) {
    if (fd[i].fd == n) {
      int e = fd[i].events & ~events;
      if (!e) continue; // if no events left, delete this fd
      fd[i].events = e;
    }
    // move it down in the array if necessary:
    if (j<i) {
      fd[j] = fd[i];
    }
    if (fd[j].fd > maxfd) maxfd = fd[j].fd;
    j++;
  }
  nfds = j;
  if (events & POLLIN) FD_CLR(n, &fdsets[0]);
  if (events & POLLOUT) FD_CLR(n, &fdsets[1]);
  if (events & POLLERR) FD_CLR(n, &fdsets[2]);
  wake_select_thread();
  pthread_mutex_unlock(&select_mutex);
}

enum { kEventClassFLTK = 'fltk' };
enum { kEventFLTKBreakLoop = 1, kEventFLTKDataReady };

// Run select() and do the callbacks:
static int run_select(double time_to_wait, bool in_thread, bool callbacks) {
  // Thread safe local copy
  pthread_mutex_lock(&select_mutex);
  int maxfd = ::maxfd;
  fd_set r = fdsets[0];
  fd_set w = fdsets[1];
  fd_set x = fdsets[2];
  pthread_mutex_unlock(&select_mutex);
  // TACK ON FD'S FOR 'CANCEL PIPE'
  if (in_thread) {
    FD_SET(G_pipe[0], &r);
    if ( G_pipe[0] > maxfd ) maxfd = G_pipe[0];
  }
  DEBUGMSG("Calling select\n");
  int ret;
  if (time_to_wait < 2147483.648f) {
    timeval t;
    t.tv_sec = int(time_to_wait);
    t.tv_usec = int(1000000 * (time_to_wait-t.tv_sec));
    ret = ::select(maxfd+1, &r, &w, &x, &t);
  } else {
    ret = ::select(maxfd+1, &r, &w, &x, 0);
  }
  if (ret > 0) {
    DEBUGMSG("Select returned non-zero\n");
    if (!callbacks) return ret;
    for (int i=0; i<nfds; i++) {
      //fprintf(stderr, "CHECKING FD %d OF %d (%d)\n", i, nfds, fd[i].fd);
      int f = fd[i].fd;
      short revents = 0;
      if (FD_ISSET(f, &r)) revents |= POLLIN;
      if (FD_ISSET(f, &w)) revents |= POLLOUT;
      if (FD_ISSET(f, &x)) revents |= POLLERR;
      if (fd[i].events & revents) {
        DEBUGMSG("DOING CALLBACK: ");
        fl_lock_function();
        fd[i].cb(f, fd[i].arg);
        fl_unlock_function();
        DEBUGMSG("DONE\n");
      }
    }
    // see if we need to copy fd_sets again:
    if (in_thread && FD_ISSET(G_pipe[0], &r)) {
      DEBUGMSG("reading from G_pipe\n");
      char buf[1]; read(G_pipe[0],buf,1);
    }
    return ret;
  } else {
    return 0;
  }
}

// select_thread
//    Separate thread, watches for changes in user's file descriptors.
//    Sends a 'data ready event' to the main thread if any change.
// WAS: This new one locks and does the callback. Without this it is
// impossible to call select() again. Wonko was killing this thread
// on every event to stop it.
static void *select_thread_proc(void *userdata)
{
  while (1) {
    if (run_select(1e20, true, true)) {
      // wake up the main thread with a message:
      EventRef drEvent;
      CreateEvent( 0, kEventClassFLTK, kEventFLTKDataReady,
                   0, kEventAttributeUserEvent, &drEvent);
      PostEventToQueue(GetCurrentEventQueue(), drEvent, kEventPriorityStandard);
    }
    //pthread_testcancel();	// OSX 10.0.4 and under: need to do this
                          // so parent can cancel us :(
  }
}

// Main thread calls this when it gets the above data-ready message:
// This does nothing right now because above code did the callbacks:
static void HandleDataReady()
{
#if 0
  fl_lock_function();
  timeval t = { 0, 0 };		// quick check
  fd_set r = fdsets[0];
  fd_set w = fdsets[1];
  fd_set x = fdsets[2];
  if (::select(maxfd+1, &r, &w, &x, &t) > 0) {
    //DEBUGMSG("DATA READY EVENT: RECEIVED\n");
    for (int i=0; i<nfds; i++) {
      //fprintf(stderr, "CHECKING FD %d OF %d (%d)\n", i, nfds, fd[i].fd);
      int f = fd[i].fd;
      short revents = 0;
      if (FD_ISSET(f, &r)) revents |= POLLIN;
      if (FD_ISSET(f, &w)) revents |= POLLOUT;
      if (FD_ISSET(f, &x)) revents |= POLLERR;
      if (fd[i].events & revents) {
	DEBUGMSG("DOING CALLBACK: ");
	fd[i].cb(f, fd[i].arg);
	DEBUGMSG("DONE\n");
      }
    }
  }
  fl_unlock_function();
#endif
}

////////////////////////////////////////////////////////////////

// public variables
//Handle system_menu;
SystemMenuBar *fltk::system_menu_bar = 0;

static WindowRef capture = 0;
static WindowRef os_capture = 0;

static Window* resize_from_system;
static ::Cursor _default_cursor;
CursPtr fltk::default_cursor;
CursPtr fltk::current_cursor;
const Widget* fltk::cursor_for;

WindowPtr fltk::quartz_window;
CGContextRef fltk::quartz_gc;

/**
 * handle Apple Menu items (can be created using the SystemMenuBar
 * returns eventNotHandledErr if the menu item could not be handled
 */
//+++ verify port to FLTK2
OSStatus HandleMenu( HICommand *cmd )
{
  OSStatus ret = eventNotHandledErr;
  // attributes, commandIDm menu.menuRef, menu.menuItemIndex
  UInt32 ref;
  OSErr rrc = GetMenuItemRefCon( cmd->menu.menuRef, cmd->menu.menuItemIndex, &ref );
  //printf( "%d, %08x, %08x, %d, %d, %8x\n", rrc, cmd->attributes, cmd->commandID, cmd->menu.menuRef, cmd->menu.menuItemIndex, rrc );
  if ( rrc==noErr && ref )
  {
    Widget *m = (Widget*)ref;
    //printf( "Menu: %s\n", m->label() );
    if ( m->type()==Item::TOGGLE ) { // update the menu toggle symbol
      Item *j = (Item*)m;
      j->state(!j->state());
      SetItemMark( cmd->menu.menuRef, cmd->menu.menuItemIndex, j->state() ? 0x12 : 0 );
    } else if ( m->type()==Item::RADIO ) { // update all radio buttons in this menu
      Item *j = (Item*)m;
      Group *p = m->parent();
      j->state(1);
      int i = cmd->menu.menuItemIndex+1, mi = p->find(m), nn = p->children(), ix;
      for (ix = mi+1; ix<nn; ix++, i++)
      {
        Widget *c = p->child(ix);
        if (!c->label()) break; // break at the next divider
        if (c->type()!=Item::RADIO) break;
        ((Item*)c)->state(0);
        SetItemMark(cmd->menu.menuRef, i, 0 );
      }
      i = cmd->menu.menuItemIndex-1;
      for (ix = mi-1; ix>=0; ix--, i-- )
      {
        Widget *c = p->child(ix);
        if (!c->label()) break; // break at the next divider
        if (c->type()!=Item::RADIO) break;
        ((Item*)c)->state(0);
        SetItemMark(cmd->menu.menuRef, i, 0 );
      }
      SetItemMark( cmd->menu.menuRef, cmd->menu.menuItemIndex, 0x13 );
    }
    system_menu_bar->execute(m);
    ret = noErr; // done handling this event
  }
  HiliteMenu(0);
  return ret;
}

/**
 It appears that every event passes through this function. This can
 probably be rewritten so all the "Handle" functions are instead right
 here in a single case statement, which would match the other platform
 implementations of fltk much more.
*/
static pascal OSStatus carbonDispatchHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  OSStatus ret = eventNotHandledErr;
  HICommand cmd;

  switch ( GetEventClass( event ) )
  {
  case kEventClassMouse:
    switch ( GetEventKind( event ) )
    {
    case kEventMouseUp:
    case kEventMouseMoved:
    case kEventMouseDragged:
      if ( capture )
        ret = SendEventToEventTarget( event, GetWindowEventTarget( capture ) );
      else if ( os_capture ) {
        ret = SendEventToEventTarget( event, GetWindowEventTarget( os_capture ) );
	os_capture = 0;
      }
      break;
    }
    break;
  case kEventClassCommand:
    switch (GetEventKind( event ) )
    {
      case kEventCommandProcess:
        GetEventParameter( event, kEventParamDirectObject, typeHICommand, NULL, sizeof(HICommand), NULL, &cmd );
	fl_lock_function();
        ret = HandleMenu( &cmd );
	fl_unlock_function();
        break;
    }
    break;
  case kEventClassFLTK:
    switch ( GetEventKind( event ) )
    {
    case kEventFLTKBreakLoop:
      ret = noErr;
      break;
    case kEventFLTKDataReady:
      HandleDataReady();
      ret = noErr;
      break;
    }
  }
  if ( ret == eventNotHandledErr )
    ret = CallNextEventHandler( nextHandler, event ); // let the OS handle the activation, but continue to get a click-through effect

  return ret;
}

/*
 * Wait up to the given time for any events or sockets to become ready,
 * do the callbacks for the events and sockets. Returns non-zero if
 * anything happened during the time period.
 */
static inline int fl_wait(double time) 
{
  if (!CreatedWindow::first && !select_thread) {
    // If there are no windows, avoid calling event handler stuff. This
    // allows the program to work on a headless render farm or when
    // ssh'd in without admin privledges.
    // Also similar to how the X11 version works when DISPLAY is not set.
    fl_unlock_function();
    int ret = run_select(time, false, true);
    fl_lock_function();
    return ret;  
  }

  static EventTargetRef target = 0;
  if ( !target ) {
    target = GetEventDispatcherTarget();
    EventHandlerUPP dispatchHandler =
      NewEventHandlerUPP( carbonDispatchHandler );
    static EventTypeSpec dispatchEvents[] = {
        { kEventClassWindow, kEventWindowShown },
        { kEventClassWindow, kEventWindowHidden },
        { kEventClassWindow, kEventWindowActivated },
        { kEventClassWindow, kEventWindowDeactivated },
        { kEventClassWindow, kEventWindowClose },
        { kEventClassKeyboard, kEventRawKeyDown },
        { kEventClassKeyboard, kEventRawKeyRepeat },
        { kEventClassKeyboard, kEventRawKeyUp },
        { kEventClassKeyboard, kEventRawKeyModifiersChanged },
        { kEventClassMouse, kEventMouseDown },
        { kEventClassMouse, kEventMouseUp },
        { kEventClassMouse, kEventMouseMoved },
        { kEventClassMouse, kEventMouseWheelMoved },
        { kEventClassMouse, kEventMouseDragged },
        { kEventClassFLTK, kEventFLTKBreakLoop },
        { kEventClassFLTK, kEventFLTKDataReady } };
    OSStatus ret;
    ret = InstallEventHandler( target, dispatchHandler,
			       GetEventTypeCount(dispatchEvents),
			       dispatchEvents, 0, 0L );
    static EventTypeSpec appEvents[] = {
        { kEventClassCommand, kEventCommandProcess } };
    ret = InstallApplicationEventHandler( dispatchHandler,
					  GetEventTypeCount(appEvents),
					  appEvents, 0, 0L );
  }

  int got_events = 0;
  // START A THREAD TO WATCH FOR DATA READY
  if ( nfds && !select_thread) {
    // detect if calling program did not do fltk::lock() and do it:
    if (fl_lock_function == nothing) fltk::lock();
    pthread_mutex_init(&select_mutex, NULL);
    pipe(G_pipe);
    DEBUGMSG("*** START THREAD\n");
    pthread_create(&select_thread, NULL, select_thread_proc,
		   (void*)GetCurrentEventQueue());
  }

  fl_unlock_function();

  EventRef event;
  if (!ReceiveNextEvent(0, NULL, time, true, &event)) {
    ++got_events;
    OSErr ret = SendEventToEventTarget( event, target );
    // Not sure what this does, but it makes some UI actions work:
    if (ret!=noErr) {
      EventRecord clevent;
      ConvertEventRefToEventRecord(event, &clevent);
      if (clevent.what==kHighLevelEvent) {
	ret = AEProcessAppleEvent(&clevent);
      }
    }
    // This is needed to make the Mac menubar work:
    if (   ret==eventNotHandledErr
	   && GetEventClass(event)==kEventClassMouse
	   && GetEventKind(event)==kEventMouseDown ) {
      Point pos;
      GetEventParameter(event, kEventParamMouseLocation, typeQDPoint,
			NULL, sizeof(pos), NULL, &pos);

      WindowRef win;
      if (MacFindWindow(pos, &win)==inMenuBar) {
	MenuSelect(pos);
      }
    }
    ReleaseEvent(event);
    time = 0.0; // just peek for pending events
  }
  fl_lock_function();

  // we send LEAVE only if the mouse did not enter some other window:
  // I'm not sure if this is needed or if it works...
  //if (!xmousewin) handle(LEAVE, 0);

  // fix the cursor
  if (cursor_for && !cursor_for->contains(belowmouse_)) {
    cursor_for = 0;
    current_cursor = default_cursor;
    SetCursor(default_cursor);
  }

  return got_events;
}

/*
 * ready() is just like wait(0.0) except no callbacks are done.
 */
static inline int fl_ready() {
  if (!CreatedWindow::first && !select_thread) {
    return run_select(0.0, false, false);
  }
  EventRef event;
  return !ReceiveNextEvent(0, NULL, 0.0, false, &event);
}

////////////////////////////////////////////////////////////////

/**
 * event handler for Apple-Q key combination
 * This attempts to close all the windows. If that succeeds hopefully
 * run() will return and then the application will exit.
 */
//+++ verify port to FLTK2
static OSErr QuitAppleEventHandler( const AppleEvent *appleEvt, AppleEvent* reply, UInt32 refcon )
{
  if (modal()) {
    exit_modal();
  } else {
    CreatedWindow* px = 0;
    while (!modal()) {
      CreatedWindow *x = CreatedWindow::first;
      while (x && (x->window->parent()
		   // || x->window->child_of()
		   || !x->window->visible())) x = x->next;
      if (!x || x==px) break;
      px = x;
      x->window->do_callback();
    }
  }
  return noErr;
}

// Damage all the child windows as well as this one...
//+++ verify port to FLTK2
static void recursive_expose(CreatedWindow* i) {
  i->wait_for_expose = false;
  i->expose(Rectangle(i->window->w(), i->window->h()));
  for (CreatedWindow* c = i->children; c; c = c->brother) recursive_expose(c);
}

static void fix_xfocus(Window* window) {
  if (window == xfocus) return;
  if (window) {
    // help windows can't get the focus!
    WindowClass winClass;
    GetWindowClass( xid( window ), &winClass );
    if ( winClass == kHelpWindowClass ) return;
  }
  xfocus = window;
  fix_focus();
}

/* Carbon Window handler
 * This needs to be linked into all new window event handlers
 */
//+++ verify port to FLTK2
static pascal OSStatus carbonWindowHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  UInt32 kind = GetEventKind( event );
  OSStatus ret = noErr;
  Window *window = (Window*)userData;
  fl_lock_function();

  switch ( kind )
  {
  case kEventWindowBoundsChanging:
    {Rect r;
    GetEventParameter( event, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &r );
    // WAS: this if breaks window position snapping done by the layout() method:
//     if ((r.right - r.left == window->w()) && (r.bottom - r.top == window->h()))
// 	break;
    if (window->resize(r.left, r.top, r.right-r.left, r.bottom-r.top)) {
      resize_from_system = window;
      window->layout_damage( window->layout_damage() | LAYOUT_USER );
      window->layout(); // this is allowed to change the window size!
      r.left = window->x();
      r.top = window->y();
      r.right = r.left+window->w();
      r.bottom = r.top+window->h();
      SetEventParameter( event, kEventParamCurrentBounds, typeQDRectangle, sizeof(Rect), &r );
    }
    break;}
  case kEventWindowDrawContent:
    recursive_expose(CreatedWindow::find( window ));
    break;
  case kEventWindowBoundsChanged:
#if 0 // This is not needed, as the BoundsChanging event above did it already:
    {Rect r;
    GetEventParameter( event, kEventParamCurrentBounds, typeQDRectangle, NULL, sizeof(Rect), NULL, &r );
    window->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
    resize_from_system = window;}
#endif
    // But we do seem to need this, OS/X has the Windows-like bug of
    // calling the event handler directly without ever returning from
    // ReceiveNextEvent().
    fltk::flush();
    break;
  case kEventWindowShown:
    handle( SHOW, window);
    break;
  case kEventWindowHidden:
    handle( HIDE, window);
    break;
  case kEventWindowActivated:
    if (!xfocus) fix_xfocus(window);
    ret = eventNotHandledErr; // without this it blocks until mouse moves?
    break;
  case kEventWindowDeactivated:
    if ( window == xfocus ) {
      handle(LEAVE, 0); // temporary fix until we get real enter/leave events
      fix_xfocus(0);
    }
    ret = eventNotHandledErr;
    break;
  case kEventWindowClose:
    window->do_callback(); // this might or might not close the window
    break;
  default:
    ret = eventNotHandledErr;
    break;
  }
  fl_unlock_function();
  return ret;
}


/**
 * Carbon Mousewheel handler
 * This needs to be linked into all new window event handlers
 */
static pascal OSStatus carbonMousewheelHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  Window *window = (Window*)userData;
  EventMouseWheelAxis axis;

  GetEventParameter( event, kEventParamMouseWheelAxis, typeMouseWheelAxis, 
                     NULL, sizeof(EventMouseWheelAxis), NULL, &axis );
  long delta;
  GetEventParameter( event, kEventParamMouseWheelDelta, typeLongInteger, 
                     NULL, sizeof(long), NULL, &delta );
  OSStatus ret = noErr;
  fl_lock_function();
  if ( axis == kEventMouseWheelAxisX ) {
    e_dx = -delta;
    e_dy = 0;
    if ( e_dx) handle( MOUSEWHEEL, window );
  } else if ( axis == kEventMouseWheelAxisY ) {
    e_dx = 0;
    e_dy = -delta;
    if ( e_dy) handle( MOUSEWHEEL, window );
  } else {
    ret = eventNotHandledErr;
  }
  fl_unlock_function();
  return ret;
}

/*
 * convert the mouse button to fltk's keysym and state flags
 */
static void button_to_keysym( EventRef event ) {
  EventMouseButton btn;
  GetEventParameter( event, kEventParamMouseButton, typeMouseButton, NULL, sizeof(EventMouseButton), NULL, &btn );
  if (btn == 2) btn = 3;
  else if (btn == 3) btn = 2;
  e_keysym = btn;
  static const int state[] = { 
    0, BUTTON1, BUTTON3, BUTTON1|BUTTON3, BUTTON2,
    BUTTON2|BUTTON1, BUTTON2|BUTTON3, BUTTON2|BUTTON1|BUTTON3
  };
  UInt32 chord;
  GetEventParameter( event, kEventParamMouseChord,
		     typeUInt32, NULL, sizeof(UInt32), NULL, &chord );
  e_state = ( e_state & ANY_BUTTON ) | state[ chord & 0x07 ];

  // make the stylus data produce something useful if there's no pen
  //if (no_stylus) {
  e_pressure = e_state&BUTTON1 ? 1.0f : 0.0f;
  e_x_tilt = e_y_tilt = 0.0f;
  e_device = DEVICE_MOUSE;
  //}
}


EventRef os_event;		// last (mouse) event
static UInt32 recent_keycode = 0;

/**
 * Carbon Mouse Button Handler
 */
//+++ verify port to FLTK2
static pascal OSStatus carbonMouseHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  fl_lock_function();

  os_event = event;
  Window *window = (Window*)userData;
  Point pos;
  GetEventParameter( event, kEventParamMouseLocation,
		     typeQDPoint, NULL, sizeof(Point), NULL, &pos );
  e_x_root = e_x = pos.h;
  e_y_root = e_y = pos.v;
  for (Group* w = window; w; w = w->parent()) {
    e_x -= w->x();
    e_y -= w->y();
  }

  static int px, py;

  switch ( GetEventKind( event ) ) {

  case kEventMouseDown:
    fix_xfocus(window);
    if ( FindWindow( pos, 0 ) != inContent ) {
      // let the OS handle clicks in the title bar
      fl_unlock_function();
      return CallNextEventHandler( nextHandler, event );
    }
    if ( !IsWindowActive( fltk::xid(window) ) ) {
      // let the OS handle the activation,
      // but continue to get a click-through effect
      fl_unlock_function();
      CallNextEventHandler( nextHandler, event );
      fl_lock_function();
    }
    os_capture = fltk::xid(window); // make all mouse events go to this window
    px = pos.h; py = pos.v;
    {UInt32 clickCount;
    GetEventParameter( event, kEventParamClickCount,
		       typeUInt32, NULL, sizeof(UInt32), NULL, &clickCount );
    if (clickCount>1) e_clicks++; else e_clicks = 0;}
    button_to_keysym( event );
    e_is_click = e_keysym;
    recent_keycode = 0;

    handle( PUSH, window );
    break;

  case kEventMouseUp:
    recent_keycode = 0;
    button_to_keysym( event );
    if (e_is_click != e_keysym) e_is_click = 0;
    handle( RELEASE, window );
    break;

  case kEventMouseMoved:
  case kEventMouseDragged:
    if (abs(pos.h-px)>5 || abs(pos.v-py)>5) {
      e_is_click = 0;
      recent_keycode = 0;
    }
    handle( MOVE, window ); // handle will convert into DRAG
    break;
  }

  fl_unlock_function();
  return noErr;
}

// WAS: I never see the "right" bits turn on. It always turns on
// only the normal ones. Possibly a different call is needed to
// get the right-hand bits.

/**
 * convert the current mouse chord into the FLTK modifier state
 */
static void mods_to_e_state( UInt32 mods )
{
  int state = 0;
  if ( mods & kEventKeyModifierNumLockMask ) state |= NUMLOCK;
  if ( mods & cmdKey ) state |= META;
  if ( mods & (optionKey|rightOptionKey) ) state |= ALT;
  if ( mods & (controlKey|rightControlKey) ) state |= CTRL;
  if ( mods & (shiftKey|rightShiftKey) ) state |= SHIFT;
  if ( mods & alphaLock ) state |= CAPSLOCK;
  e_state = ( e_state & ANY_BUTTON ) | state;
  //printf( "State 0x%08x (%04x)\n", e_state, mods );
}


/**
 * convert the current mouse chord into the FLTK keysym
 */
static unsigned mods_to_e_keysym( UInt32 mods )
{
  if ( mods & cmdKey ) return LeftMetaKey;
  else if ( mods & kEventKeyModifierNumLockMask ) return NumLockKey;
  else if ( mods & rightOptionKey ) return RightAltKey;
  else if ( mods & optionKey ) return LeftAltKey;
  else if ( mods & rightControlKey ) return RightCtrlKey;
  else if ( mods & controlKey ) return LeftCtrlKey;
  else if ( mods & rightShiftKey ) return RightShiftKey;
  else if ( mods & shiftKey ) return LeftShiftKey;
  else if ( mods & alphaLock ) return CapsLockKey;
  else return 0;
}


static unsigned short macKeyLookUp[128] =
{
    'a', 's', 'd', 'f', 'h', 'g', 'z', 'x',
    'c', 'v', 0/*ISO extra ('#' on German keyboard)*/, 'b', 'q', 'w', 'e', 'r',

    'y', 't', '1', '2', '3', '4', '6', '5',
    '=', '9', '7', '-', '8', '0', ']', 'o',

    'u', '[', 'i', 'p', ReturnKey, 'l', 'j', '\'',
    'k', ';', '\\', ',', '/', 'n', 'm', '.',

    TabKey, SpaceKey, '`', BackSpaceKey, 0/*kp_enter on powerbook G4*/, EscapeKey, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,

    0, DecimalKey, RightKey, MultiplyKey, 0, AddKey, LeftKey, NumLockKey,
    DownKey, 0, 0, DivideKey, KeypadEnter, UpKey, SubtractKey, 0,

    0, Keypad+'=', Keypad0, Keypad1, Keypad2, Keypad3, Keypad4, Keypad5,
    Keypad6, Keypad7, 0, Keypad8, Keypad9, 0, 0, 0,

    F5Key, F6Key, F7Key, F3Key, F8Key, F9Key, 0, F11Key,
    0, F0Key+13, F0Key+16, F0Key+14, 0, F10Key, fltk::MenuKey, F12Key,

    0, F0Key+15, HelpKey, HomeKey, PageUpKey, DeleteKey, F4Key, EndKey,
    F2Key, PageDownKey, F1Key, LeftKey, RightKey, DownKey, UpKey, 0,
};

#include "mactoutf8.cxx"

/**
 * handle carbon keyboard events
 */
//+++ verify port to FLTK2
pascal OSStatus carbonKeyboardHandler( EventHandlerCallRef nextHandler, EventRef event, void *userData )
{
  static char buffer[5];
  int sendEvent = 0;
  
  UInt32 keyCode;
  GetEventParameter( event, kEventParamKeyCode, typeUInt32, NULL, sizeof(UInt32), NULL, &keyCode );
  unsigned char macchar;
  GetEventParameter( event, kEventParamKeyMacCharCodes, typeChar, NULL, sizeof(char), NULL, &macchar );
  unsigned short sym;

  fl_lock_function();
  if (!xfocus) fix_xfocus((Window*)userData);

  switch ( GetEventKind( event ) )
  {
  case kEventRawKeyDown:
    sendEvent = KEY;
    e_key_repeated = 0;
    recent_keycode = keyCode;
    goto GET_KEYSYM;
  case kEventRawKeyRepeat:
    sendEvent = KEY;
    e_key_repeated++;
    recent_keycode = 0;
    goto GET_KEYSYM;
  case kEventRawKeyUp:
    sendEvent = KEYUP;
    e_is_click = (recent_keycode == keyCode);
    recent_keycode = 0;
  GET_KEYSYM:
    sym = macKeyLookUp[ keyCode & 0x7f ];
    if (!sym) sym = keyCode|0x8000;
    e_keysym = sym;
    if ( keyCode==0x4c ) macchar=0x0d;
    if ( sym >= Keypad && sym <= KeypadLast ||
	 (sym&0xff00) == 0 ||
	 sym == BackSpaceKey ||
	 sym == TabKey ||
	 sym == ReturnKey ||
	 sym == EscapeKey) {
      if (macchar < 0x80) {
	buffer[0] = macchar;
	e_length = 1;
      } else {
	e_length = utf8encode(mactoutf8[macchar-0x80],buffer);
	buffer[e_length] = 0;
      }
    } else {
      buffer[0] = 0;
      e_length = 0;
    }
    e_text = buffer;
    // insert UnicodeHandling here!
    break;
  case kEventRawKeyModifiersChanged: {
    UInt32 mods;
    static UInt32 prevMods = 0;
    GetEventParameter( event, kEventParamKeyModifiers, typeUInt32, NULL, sizeof(UInt32), NULL, &mods );
    mods_to_e_state( mods );
    sym = mods_to_e_keysym( prevMods ^ mods );
    if (sym) {
      e_keysym = sym;
      if (prevMods < mods) {
	sendEvent = KEY;
	recent_keycode = sym;
      } else {
	sendEvent = KEYUP;
	e_is_click = (recent_keycode == sym);
	recent_keycode = 0;
      }
      e_length = 0;
      buffer[0] = 0;
    }
    prevMods = mods;
    break; }
  }
  bool r = (sendEvent && handle(sendEvent, xfocus));
  fl_unlock_function();
  if (r) return noErr;
  return CallNextEventHandler( nextHandler, event );
}

// Text editors tell the system where the insertion point is by using
// this function, so the system can move any Input Method widgets to
// that position:
//+++ verify port to FLTK2
void fl_set_spot(fltk::Font *f, Widget *w, int x, int y) {}

/*
 * initialize the Mac toolboxes, dock status, and set the default menubar
 */

#ifndef MAC_OS_X_VERSION_10_3
extern "C" {
extern OSErr CPSEnableForegroundOperation(ProcessSerialNumber *psn, UInt32 _arg2,
    UInt32 _arg3, UInt32 _arg4, UInt32 _arg5);
}
#endif

void fltk::open_display() {
  static char beenHereDoneThat = 0;
  if ( !beenHereDoneThat )  {
    beenHereDoneThat = 1;
    
    FlushEvents(everyEvent,0);

    //MoreMasters(); // \todo Carbon suggests MoreMasterPointers()
    AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
	NewAEEventHandlerUPP((AEEventHandlerProcPtr)QuitAppleEventHandler),
			   0, false );

    // get pointer to default cursor:
    GetQDGlobalsArrow(&_default_cursor);
    default_cursor = &_default_cursor;
    current_cursor = default_cursor;

    // This populates the menu bar, see fltk1 code:
    //ClearMenuBar();
    //AppendResMenu( GetMenuHandle( 1 ), 'DRVR' );
    //DrawMenuBar();

    // bring the application into foreground without a 'CARB' resource
    Boolean same_psn;
    ProcessSerialNumber cur_psn, front_psn;
    if (!GetCurrentProcess( &cur_psn ) && !GetFrontProcess( &front_psn ) &&
        !SameProcess( &front_psn, &cur_psn, &same_psn ) && !same_psn ) {
      // If we are in a .app bundle this is not necessary, so don't do it.
      // It also prevents some bundle functions such as LSBackgroundOnly.
      // This mess figures out if we are bundled:
      CFBundleRef bundle = CFBundleGetMainBundle();
      if ( bundle ) {
	FSRef execFs;
	CFURLRef execUrl = CFBundleCopyExecutableURL( bundle );
	CFURLGetFSRef( execUrl, &execFs );

	FSRef bundleFs;
	GetProcessBundleLocation( &cur_psn, &bundleFs );

	if ( !FSCompareFSRefs( &execFs, &bundleFs ) )
          bundle = NULL;

        CFRelease(execUrl);
      }

      if ( !bundle ) {
	OSErr err = 1;
#ifdef MAC_OS_X_VERSION_10_3
        // newer supported API
	if (TransformProcessType != NULL)
	  err = TransformProcessType( &cur_psn, kProcessTransformToForegroundApplication );
#else
        // undocumented API
        if (CPSEnableForegroundOperation != NULL)
	  err = CPSEnableForegroundOperation( &cur_psn, 0x03, 0x3C, 0x2C, 0x1103 );
#endif
	if (err == noErr)
	  SetFrontProcess( &cur_psn );
      }
    }
  }
}

////////////////////////////////////////////////////////////////

// this should be turned on by changes to monitor layout, nyi.
// Apparently CGDisplayReconfigurationCallBack is what you need
static bool reload_list = true; 
static bool reload_all = true;

/* Return a "monitor" that surrounds all the monitors.
    If you have a single monitor, this returns a monitor structure that
    defines it. If you have multiple monitors this returns a fake monitor
    that surrounds all of them.
*/
//+++ verify port to FLTK2
const Monitor& Monitor::all() {
  static Monitor monitor;
  if (reload_list || reload_all) {
    const Monitor* list;
    int count = Monitor::list(&list);
    reload_all = false;
    monitor = list[0];
    for (int i=1; i < count; i++) {
      monitor.merge(list[i]);
      // always keep the menubar off the first window out of work area!
      int oldy = monitor.work.y();
      monitor.work.merge(list[i].work);
      monitor.work.set_y(oldy);
    }
  }
  return monitor;
}

static Monitor* monitors = 0;
static int num_monitors = 0;

/* Return an array of all Monitors.
    p is set to point to a static array of Monitor structures describing
    all monitors connected to the system. Subsequent calls will usually
    return the same array, but if a signal comes in indicating a change
    it will probably delete the old array and return a new one.
*/
int Monitor::list(const Monitor** p) {
  if (reload_list) {
    reload_list = false;
    reload_all = true;
    const unsigned int Max = 10;
    CGDirectDisplayID list[Max];
    CGDisplayCount count = 0;
    /*CGDisplayErr err =*/ CGGetActiveDisplayList(Max, list, &count);
    // maybe do something if count < 1?
    num_monitors = count;
    delete[] monitors;
    monitors = new Monitor[count];
    for (unsigned int d = 0; d < count; ++d) {
      CGRect bounds = CGDisplayBounds(list[d]);
      monitors[d].set(bounds.origin.x,
                      bounds.origin.y,
                      bounds.size.width,
                      bounds.size.height);
      GDHandle gdhandle; DMGetGDeviceByDisplayID((DisplayIDType)(list[d]),&gdhandle,false);
      Rect work; GetAvailableWindowPositioningBounds(gdhandle, &work);
      monitors[d].work.set(work.left, work.top, work.right-work.left, work.bottom-work.top);
      monitors[d].depth_ = CGDisplayBitsPerPixel(list[d]);
      CGSize size = CGDisplayScreenSize(list[d]); //display size in mm
      monitors[d].dpi_x_ = bounds.size.width*25.4f/size.width;
      monitors[d].dpi_y_ = bounds.size.height*25.4f/size.height;
    }
  }
  *p = monitors;
  return num_monitors;
}

/* Return a pointer to a Monitor structure describing the monitor
    that contains or is closest to the given x,y, position.
*/
const Monitor& Monitor::find(int x, int y) {
  const Monitor* monitors;
  int count = list(&monitors);
  const Monitor* ret = monitors+0;
  if (count > 1) {
    int r = 0;
    for (int i = 0; i < count; i++) {
      const Monitor& m = monitors[i];
      // find distances to nearest edges:
      int rx;
      if (x <= m.x()) rx = m.x()-x;
      else if (x >= m.r()) rx = x-m.r();
      else rx = 0;
      int ry;
      if (y <= m.y()) ry = m.y()-y;
      else if (y >= m.b()) ry = y-m.b();
      else ry = 0;
      // return this screen if inside:
      if (rx <= 0 && ry <= 0) return m;
      // use larger of horizontal and vertical distances:
      if (rx < ry) rx = ry;
      // remember if this is the closest screen:
      if (!i || rx < r) {
	ret = monitors+i;
	r = rx;
      }
    }
  }
  return *ret;
}

////////////////////////////////////////////////////////////////

/*
 * get the current mouse pointer world coordinates
 */
//+++ verify port to FLTK2
void fltk::get_mouse(int &x, int &y) 
{
  open_display();
  Point loc; 
  GetMouse( &loc );
  LocalToGlobal( &loc );
  x = loc.h;
  y = loc.v;
}

bool fltk::warp_mouse(int x, int y) {
  CGPoint new_pos;
  new_pos.x = x;
  new_pos.y = y;
  return CGWarpMouseCursorPosition(new_pos) == 0;
}

////////////////////////////////////////////////////////////////

/*
 * convert a Mac FSSpec structure into a Unix filename 
 */
static int FSSpec2UnixPath( FSSpec *fs, char *dst )
{
  FSRef fsRef;
  FSpMakeFSRef( fs, &fsRef );
  FSRefMakePath( &fsRef, (UInt8*)dst, 1024 );
  return strlen(dst);
}
 
////////////////////////////////////////////////////////////////

/* current value of fltk::open_callback() */
static void	(*open_cb)(const char *) = 0;

/*
 * Event handler for Apple-O key combination and also for file opens
 * via the finder...
 */
static OSErr OpenAppleEventHandler(const AppleEvent *appleEvt,
                                   AppleEvent *reply,
				   UInt32 refcon) {
  if (!open_cb) return noErr;

  // Initialize the document list...
  AEDescList documents;
  AECreateDesc(typeNull, NULL, 0, &documents);

  // Get the open parameter(s)...
  OSErr err;
  err = AEGetParamDesc(appleEvt, keyDirectObject, typeAEList, &documents);
  if (err != noErr) {
    AEDisposeDesc(&documents);
    return err;
  }

  // Open the documents via the callback...
  long n; if (AECountItems(&documents, &n) == noErr) {
    for (long i = 1; i <= n; i ++) {
      // Get the next FSSpec record...
      FSSpec fileSpec;
      AEKeyword keyWd;
      DescType typeCd;
      Size actSz;
      AEGetNthPtr(&documents, i, typeFSS, &keyWd, &typeCd,
                  (Ptr)&fileSpec, sizeof(fileSpec),
		  (actSz = sizeof(fileSpec), &actSz));

      // Convert to a UNIX path...
      char filename[1024];
      FSSpec2UnixPath(&fileSpec, filename);

      // Call the callback with the filename...
      (*open_cb)(filename);
    }
  }

  // Get rid of the document list...
  AEDisposeDesc(&documents);

  return noErr;
}


/**
 * Install an open documents event handler...
 */

void fltk::open_callback(void (*cb)(const char *)) {
  if (cb == open_cb) return;
  open_cb = cb;
  if (cb)
    AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
                          NewAEEventHandlerUPP((AEEventHandlerProcPtr)
					       OpenAppleEventHandler),
			  0, false);
  else
    AERemoveEventHandler(kCoreEventClass, kAEOpenDocuments,
			 NewAEEventHandlerUPP((AEEventHandlerProcPtr)
					      OpenAppleEventHandler),
			 false);
}

////////////////////////////////////////////////////////////////

#include <fltk/draw.h>

static void show_drag(bool v, Widget* target, DragReference dragRef) {
  if (v) {
    target->cursor(CURSOR_HAND);
    //ShowDragHilite( dragRef );
  } else {
    target->cursor(CURSOR_DEFAULT);
    //HideDragHilight( dragRef );
  }
  fltk::flush(); // make any fltk drawing get done
}

/*
 * Drag'n'drop tracking handler
 */
static DragReference current_drag;

static pascal OSErr dndTrackingHandler( DragTrackingMessage msg, WindowPtr w, void *userData, DragReference dragRef )
{
  Window *target = (Window*)userData;
  Point mp;
  static int px, py;
  
  switch ( msg )
  {
  case kDragTrackingEnterWindow:
    // check if 'TEXT' is available
    GetDragMouse( dragRef, &mp, 0 );
    e_x_root = px = mp.h;
    e_y_root = py = mp.v;
    e_x = px - target->x();
    e_y = py - target->y();
    show_drag( handle( DND_ENTER, target ), target, dragRef);
    current_drag = dragRef;
    return noErr;
  case kDragTrackingInWindow:
    GetDragMouse( dragRef, &mp, 0 );
    if ( mp.h==px && mp.v==py )
      break;	//+ return previous condition for dnd hiliting
    e_x_root = px = mp.h;
    e_y_root = py = mp.v;
    e_x = px - target->x();
    e_y = py - target->y();
    show_drag ( handle( DND_DRAG, target ), target, dragRef);
    return noErr;
  case kDragTrackingLeaveWindow:
    if (current_drag) { // ignore if release was done
      handle( DND_LEAVE, target );
      show_drag( false, target, dragRef );
    }
    return noErr;
  }
  return noErr;
}


/*
 * Drag'n'drop receive handler
 */
//+++ verify port to FLTK2
static pascal OSErr dndReceiveHandler( WindowPtr w, void *userData, DragReference dragRef )
{
  Point mp;
  OSErr ret;
  
  Window *target = (Window*)userData;
  GetDragMouse( dragRef, &mp, 0 );
  e_x_root = mp.h;
  e_y_root = mp.v;
  e_x = e_x_root - target->x();
  e_y = e_y_root - target->y();

  ret = handle( DND_RELEASE, target );
  Widget* droptarget = belowmouse();
  show_drag( false, target, dragRef );
  current_drag = 0;
  if (!ret || !droptarget)
    return userCanceledErr;

  // get the ASCII text
  UInt16 i, nItem;
  ItemReference itemRef;
  FlavorFlags flags;
  Size itemSize, size = 0;
  CountDragItems( dragRef, &nItem );
  for ( i = 1; i <= nItem; i++ )
  {
    GetDragItemReferenceNumber( dragRef, i, &itemRef );
    ret = GetFlavorFlags( dragRef, itemRef, 'TEXT', &flags );
    if ( ret == noErr )
    {
      GetFlavorDataSize( dragRef, itemRef, 'TEXT', &itemSize );
      size += itemSize;
    }
    ret = GetFlavorFlags( dragRef, itemRef, 'hfs ', &flags );
    if ( ret == noErr )
    {
      size += 1024; //++ ouch! We should create the full pathname and figure out its length
    }
  }
  
  if ( !size )
    return userCanceledErr;
  
  e_length = size + nItem - 1;
  char* buffer = (char*)malloc( size+nItem );;
  e_text = buffer;

  char *dst = buffer;
  for ( i = 1; i <= nItem; i++ ) {
    GetDragItemReferenceNumber( dragRef, i, &itemRef );
    ret = GetFlavorFlags( dragRef, itemRef, 'TEXT', &flags );
    if ( ret == noErr ) {
      GetFlavorDataSize( dragRef, itemRef, 'TEXT', &itemSize );
      GetFlavorData( dragRef, itemRef, 'TEXT', dst, &itemSize, 0L );
      dst += itemSize;
      *dst++ = '\n'; // add our element seperator
    }
    ret = GetFlavorFlags( dragRef, itemRef, 'hfs ', &flags );
    if ( ret == noErr ) {
      HFSFlavor hfs; itemSize = sizeof( hfs );
      GetFlavorData( dragRef, itemRef, 'hfs ', &hfs, &itemSize, 0L );
      itemSize = FSSpec2UnixPath( &hfs.fileSpec, dst );
      dst += itemSize;
      if ( itemSize>1 && ( hfs.fileType=='fold' || hfs.fileType=='disk' ) ) 
        *dst++ = '/';
      *dst++ = '\n'; // add our element seperator
    }
  }
  
  dst[-1] = 0;
//  if ( e_text[e_length-1]==0 ) e_length--; // modify, if trailing 0 is part of string
  e_length = dst - e_text - 1;
  droptarget->handle(PASTE);
  free(buffer);
  
  return noErr;
}

////////////////////////////////////////////////////////////////

void Window::borders( fltk::Rectangle *r ) const {
  if (!this->border() || this->override() || this->parent()) {
    r->set(0,0,0,0);
  } else if (shown()) {
    Rect inside; GetWindowBounds(i->xid, kWindowContentRgn, &inside);
    Rect outside; GetWindowBounds(i->xid, kWindowStructureRgn, &outside);
    r->set(outside.left-inside.left,
	   outside.top-inside.top,
	   (outside.right-outside.left)-(inside.right-inside.left),
	   (outside.bottom-outside.top)-(inside.bottom-inside.top));
  } else if (child_of() && !contains(modal())) {
    r->set(0,-22,0,22);
  } else {
    r->set(0,-22,0,22);
  }
}

void Window::layout() {
  // Unlike X11 and Win32, we do need to propagate xy changes to child windows
  /*if (layout_damage() & ~LAYOUT_XY)*/ Group::layout();
  // Fix the window:
  system_layout();
}

void Window::system_layout() {
  if (parent()) {
    // child windows are done entirely by us
    if (i) for (Widget* p = parent(); ; p = p->parent())
      if (p->is_window()) {
	CreatedWindow::find((Window*)p)->need_new_subRegion = true;
	break;
      }
  } else if (this == resize_from_system) {
    // Ignore changes that came from the system
    resize_from_system = 0;
  } else if ((layout_damage()&LAYOUT_XYWH) && i) { // only for shown windows
    Rect rect;
    rect.left = x();
    rect.top = y();
    rect.right = r();
    rect.bottom = b();
    SetWindowBounds(i->xid, kWindowContentRgn, &rect);
  }
  if (i) i->need_new_subRegion = true;
}

////////////////////////////////////////////////////////////////

/*
 * go ahead, create that (sub)window
 * \todo we should make menu windows slightly transparent for the new Mac look
 */
//+++ verify port to FLTK2
void Window::create()
{
  static WindowGroupRef floatingWindowGroup;
  static WindowGroupRef mainWindowGroup;

  if ( !floatingWindowGroup ) {
    CreateWindowGroup( 0, &floatingWindowGroup );
    CreateWindowGroup( 0, &mainWindowGroup );
    SetWindowGroupParent( floatingWindowGroup, GetWindowGroupOfClass( kDocumentWindowClass ) );
    SetWindowGroupParent( mainWindowGroup, floatingWindowGroup );
    ChangeWindowGroupAttributes( floatingWindowGroup, kWindowGroupAttrSelectAsLayer | kWindowGroupAttrSharedActivation, 0);
  }

  // Create structure to hold the rectangle, initialize the parts that
  // are the same for outer and child windows:
  CreatedWindow* x = new CreatedWindow;
  x->window = this; i = x;
  x->region = 0;
  x->subRegion = 0;
  x->need_new_subRegion = true;
  x->children = x->brother = 0;
  x->overlay = 0;
  x->gc = 0;

  if (parent()) {
    // Apparently Mac does not have child windows, this shows that there
    // really isn't any need for the system to support them as they are
    // easily emulated at the application level. We find the parent
    // window and add ourselves to a list of children. This is then used
    // to create a clipping region that is applied when drawing the
    // parent. We may want to clip brothers, too...
    Window* root;
    for (Widget *o = parent(); ; o = o->parent()) {
      if (o->is_window()) {root = (Window*)o; break;}
    }
    CreatedWindow *rootx = CreatedWindow::find(root);
    x->xid = 0;
    // we need to add it to very end, so overlapping brothers are easy
    // to find:
    CreatedWindow** p = &(rootx->children);
    while (*p) p = &((*p)->brother);
    *p = x;
    x->wait_for_expose = rootx->wait_for_expose;
    x->next = CreatedWindow::first;
    x->need_new_subRegion = true;
    CreatedWindow::first = x;
  } else {
    // create a desktop window
    int winclass, winattr, where;
    WindowGroupRef winGroup = NULL;
    if (!border() || override()) {
      winclass = kHelpWindowClass;
      if (contains(modal()) || override()) {
	// used by menus and tooltips
	winattr = 0;
      } else {
	winattr = 512; // kWindowNoTitleBarAttribute;
      }
      where = kWindowCascadeOnParentWindowScreen;
    } else {
      // a normal window with a border
      if (contains(modal())) {
	winclass = kMovableModalWindowClass;
	winattr = kWindowStandardHandlerAttribute |
	          kWindowCloseBoxAttribute;
	where = kWindowAlertPositionOnParentWindowScreen;
      } else if (child_of()) {
	winclass = kDocumentWindowClass;
	winattr = kWindowStandardHandlerAttribute |
	          kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute;
	where = kWindowCenterOnParentWindowScreen;
        winGroup = floatingWindowGroup;
      } else {
	winclass = kDocumentWindowClass;
	winattr = kWindowStandardHandlerAttribute |
	  kWindowCloseBoxAttribute | kWindowCollapseBoxAttribute;
	where = kWindowCascadeOnParentWindowScreen;
        winGroup = mainWindowGroup;
      }
      if (minw != maxw || minh != maxh)
	winattr |= kWindowFullZoomAttribute |
	  kWindowResizableAttribute | kWindowLiveResizeAttribute;
    }

    bool autoplace = (this->x() == USEDEFAULT || this->y()== USEDEFAULT);
    if (autoplace) {
      // this is a kludge in case system autoplace does not work
      static int xyPos = 50;
      this->x(xyPos); this->y(xyPos);
      xyPos += 25;
      if (xyPos >= 200) xyPos -= (200-44);
    }
    Rect wRect;
    wRect.left   = this->x();
    wRect.top    = this->y(); 
    wRect.right  = wRect.left+w();
    wRect.bottom = wRect.top+h();

    winattr &= GetAvailableWindowAttributes(winclass);	// make sure that the window will open
    CreateNewWindow(winclass, winattr, &wRect, &(x->xid));
    //MoveWindow(x->xid, wRect.left, wRect.top, 1); // avoid Carbon Bug on old OS
    if (child_of() && !contains(modal())) {
      SetWindowActivationScope(x->xid, kWindowActivationScopeAll);
    }
    if ( winGroup )
      SetWindowGroup( x->xid, winGroup );

    label(label(), iconlabel());

    if (autoplace) {
      WindowRef pw =
	(child_of() && child_of()->shown()) ? child_of()->i->xid : 0;
      RepositionWindow(x->xid, pw, where);
      Rect r; GetWindowBounds(x->xid, kWindowContentRgn, &r);
      this->resize(r.left, r.top, r.right-r.left, r.bottom-r.top);
    } else if (border() && !override()) {
      // stop it from putting title bar under the menubar:
      ConstrainWindowToScreen( x->xid, kWindowStructureRgn, kWindowConstrainAllowPartial, NULL, NULL );
    }

    x->wait_for_expose = false;//true;
    x->next = CreatedWindow::first;
    CreatedWindow::first = x;

    { // Install Carbon Event handlers 
      OSStatus ret;
      EventHandlerUPP mousewheelHandler = NewEventHandlerUPP( carbonMousewheelHandler ); // will not be disposed by Carbon...
      static EventTypeSpec mousewheelEvents[] = {
        { kEventClassMouse, kEventMouseWheelMoved } };
      ret = InstallWindowEventHandler( x->xid, mousewheelHandler, 1, mousewheelEvents, this, 0L );
      EventHandlerUPP mouseHandler = NewEventHandlerUPP( carbonMouseHandler ); // will not be disposed by Carbon...
      static EventTypeSpec mouseEvents[] = {
        { kEventClassMouse, kEventMouseDown },
        { kEventClassMouse, kEventMouseUp },
        { kEventClassMouse, kEventMouseMoved },
        { kEventClassMouse, kEventMouseDragged } };
      ret = InstallWindowEventHandler( x->xid, mouseHandler, 4, mouseEvents, this, 0L );
      EventHandlerUPP keyboardHandler = NewEventHandlerUPP( carbonKeyboardHandler ); // will not be disposed by Carbon...
      static EventTypeSpec keyboardEvents[] = {
        { kEventClassKeyboard, kEventRawKeyDown },
        { kEventClassKeyboard, kEventRawKeyRepeat },
        { kEventClassKeyboard, kEventRawKeyUp },
        { kEventClassKeyboard, kEventRawKeyModifiersChanged } };
      ret = InstallWindowEventHandler( x->xid, keyboardHandler, 4, keyboardEvents, this, 0L );
      EventHandlerUPP windowHandler = NewEventHandlerUPP( carbonWindowHandler ); // will not be disposed by Carbon...
      static EventTypeSpec windowEvents[] = {
        { kEventClassWindow, kEventWindowDrawContent },
        { kEventClassWindow, kEventWindowShown },
        { kEventClassWindow, kEventWindowHidden },
        { kEventClassWindow, kEventWindowActivated },
        { kEventClassWindow, kEventWindowDeactivated },
        { kEventClassWindow, kEventWindowClose },
        { kEventClassWindow, kEventWindowBoundsChanging },
        { kEventClassWindow, kEventWindowBoundsChanged } };
      ret = InstallWindowEventHandler( x->xid, windowHandler, 8, windowEvents, this, 0L );
      ret = InstallTrackingHandler( dndTrackingHandler, x->xid, this);
      ret = InstallReceiveHandler( dndReceiveHandler, x->xid, this);
    }
  }
}

//+++ verify port to FLTK2
void fltk::close_display() {}

/*
 * Turn on flag to indicate that user set the min and max size.
 */
void Window::size_range_() {
  size_range_set = 1;
}

/*
 * Returns true if the window is shown but is iconized.
 */
//+++ verify port to FLTK2
bool Window::iconic() const {
  return i && IsWindowCollapsed(i->xid);
}

////////////////////////////////////////////////////////////////

/*
 * set the window title bar
 * \todo make the dock icon work!
 */
//+++ verify port to FLTK2
void Window::label(const char *name, const char * iname) {
  Widget::label(name);
  iconlabel_ = iname;
  if (i && !parent()) {
    if (!name) name = "";
#if 0
    // this probably works, it was in a bug report
    CFStringRef mlabel = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    SetWindowTitleWithCFString(fl_xid(this), mlabel);
    CFRelease(mlabel);
#else
    Str255 pTitle;
    pTitle[0] = strlen(name); memcpy(pTitle+1, name, pTitle[0]);
    SetWTitle(xid(this), pTitle);
#endif
    // if (!iname) iname = filename_name(name);
    // should do something with iname here, it should label the dock icon
  }
}

////////////////////////////////////////////////////////////////
// back buffer gc access (return opaque type for all platforms)
void* Window::backbuffer() const {  return i ? i->gc :0;}

////////////////////////////////////////////////////////////////
// Drawing context

const Window *Window::drawing_window_;
static CGContextRef prev_gc = 0;
static WindowPtr prev_window = 0;
int fl_clip_w, fl_clip_h;

namespace fltk {class Image;}
fltk::Image* fl_current_Image;

void fltk::draw_into(CGContextRef gc, int w, int h) {
  prev_gc = quartz_gc;
  prev_window = quartz_window;
  quartz_window = 0;
  quartz_gc = gc;
  fl_clip_w = w; fl_clip_h = h;
  CGContextSaveGState(quartz_gc);
  fill_quartz_context();
  fl_current_Image = 0;
}

void fltk::stop_drawing(CGImageRef) {
  release_quartz_context();
  quartz_gc = prev_gc;
  quartz_window = prev_window;
}

/*
 * make all drawing go into this window (called by subclass flush() impl.)
 */
//+++ verify port to FLTK2

void Widget::make_current() const {
  int x = 0;
  int y = 0;
  const Widget* widget = this;
  while (widget->parent()) {
    x += widget->x();
    y += widget->y();
    widget = widget->parent();
  }
  const Window* window = (const Window*)widget;
  release_quartz_context();
  Window::drawing_window_ = window;
  quartz_window = xid(window);
  SetPort(GetWindowPort(quartz_window));
//SetOrigin(-X, -Y);
  // We force a clip region to handle child windows. To speed things up
  // we only recalculate the clip region when children are shown, hidden,
  // or moved or resized.
  CreatedWindow* i = CreatedWindow::find(window);
  if (i->need_new_subRegion) {
    i->need_new_subRegion = false;
    if (i->subRegion) DisposeRgn(i->subRegion);
    i->subRegion = NewRgn();
    SetRectRgn(i->subRegion, 0, 0, window->w(), window->h());
  }
  SetPortClipRegion( GetWindowPort(quartz_window), i->subRegion );
  QDBeginCGContext(GetWindowPort(quartz_window), &i->gc);
  quartz_gc = /*CreatedWindow::find(root)->gc =*/ i->gc;
  CGContextSaveGState(quartz_gc);
  fill_quartz_context();
  load_identity();
  translate(x,y);
}

// helper function to manage the current CGContext fl_gc
namespace fltk {
  extern void restore_quartz_line_style();
}

// FLTK has only one global graphics state. This function copies the FLTK 
// state into the current Quartz context
void fltk::fill_quartz_context() {
  if (!quartz_gc) return;
  int hgt = 0,wgt=0;
  if (quartz_window) {
    Rect portRect; 
    GetPortBounds(GetWindowPort(quartz_window), &portRect);
    hgt = portRect.bottom-portRect.top;
    wgt = portRect.right-portRect.left;
  } else {
    hgt = CGBitmapContextGetHeight(quartz_gc);
    wgt = CGBitmapContextGetWidth(quartz_gc);
  }
  CGContextTranslateCTM(quartz_gc, 0.5, hgt-0.5f);
  CGContextScaleCTM(quartz_gc, 1.0f, -1.0f);
  static CGAffineTransform font_mx = { 1, 0, 0, -1, 0, 0 };
  CGContextSetTextMatrix(quartz_gc, font_mx);
#if USE_CAIRO
  if (cr) {
    cairo_destroy(cr);
  }
  cairo_surface_t* s = 
    cairo_quartz_surface_create_for_cg_context(quartz_gc, hgt, wgt);
  cr = cairo_create(s);
  cairo_surface_destroy(s);
#endif
  if (current_font_) setfont(current_font_, current_size_);
  setcolor(current_color_);
  restore_quartz_line_style();
}

// The only way to reset clipping to its original state is to pop the 
// current graphics state and restore the global state.
void fltk::clear_quartz_clipping() {
  if (!quartz_gc) return;
  CGContextRestoreGState(quartz_gc);
  CGContextSaveGState(quartz_gc);
}

// Give the Quartz context back to the system
void fltk::release_quartz_context(CreatedWindow *x) {
  if (x && x->gc!=quartz_gc) return;
  if (!quartz_gc) return;
  CGContextRestoreGState(quartz_gc);
  if (quartz_window) QDEndCGContext(GetWindowPort(quartz_window), &quartz_gc);
  quartz_gc = 0;
}

////////////////////////////////////////////////////////////////
// Window update, double buffering, and overlay:
// Windows are effectively double-buffered at all times. So this
// is much simpler than the X11 and Win32 implementations.

//+++ verify port to FLTK2
void Window::flush() {
  make_current();
  unsigned char damage = this->damage();
  if (damage & ~DAMAGE_EXPOSE) {
    if (damage & DAMAGE_OVERLAY) damage = DAMAGE_ALL;
    set_damage(damage & ~DAMAGE_EXPOSE);
    draw();
    if (i->overlay) draw_overlay();
  }
  if (i->region && !(damage & DAMAGE_ALL)) {
    clip_region(i->region); i->region = 0;
    set_damage(DAMAGE_EXPOSE); draw();
    clip_region(0);
  }
#if 0
  //DrawGrowIcon(i->xid);
  if (border() && (minw != maxw || minh != maxh)) {
    setcolor(color());
    int s = child_of() && !contains(modal()) ? 12 : 15;
    int x = w()-s;
    int y = h()-s;
    fillrect(x,y,s,s);
    setcolor(GRAY15);
    int d = s-12;
    drawline(x+d,y+s-1,x+s-1,y+d); d += 4;
    drawline(x+d,y+s-1,x+s-1,y+d); d += 4;
    drawline(x+d,y+s-1,x+s-1,y+d); d += 4;
  }
#endif
  if (i->gc) CGContextFlush(i->gc);
}

//+++ verify port to FLTK2
void Window::free_backbuffer() {}

bool fltk::enable_tablet_events() {
  return false;
}

////////////////////////////////////////////////////////////////
// Cut & paste.

static char *selection_buffer[2];
static int selection_length[2];
static int selection_buffer_length[2];
static ScrapRef myScrap = 0;

/*
 * create a selection
 * owner: widget that created the selection
 * stuff: pointer to selected data
 * size of selected data
 */
//+++ verify port to FLTK2
void fltk::copy(const char *stuff, int len, bool clipboard) {
  if (!stuff || len<0) return;
  if (len >= selection_buffer_length[clipboard]) {
    delete[] selection_buffer[clipboard];
    int n = selection_buffer_length[clipboard]*2;
    if (!n) n = 1024;
    while (len >= n) n *= 2;
    selection_buffer_length[clipboard] = n;
    selection_buffer[clipboard] = new char[n];
  }
  memcpy(selection_buffer[clipboard], stuff, len);
  selection_buffer[clipboard][len] = 0; // needed for direct paste
  selection_length[clipboard] = len;
  if (clipboard) {
    ClearCurrentScrap();
    if ( GetCurrentScrap( &myScrap ) == noErr ) {
      // Previous version changed \n to \r before sending the text, but I would
      // prefer to leave the local buffer alone, so a copied buffer may be
      // needed. Check to see if this is necessary on OS/X.
      PutScrapFlavor( myScrap, kScrapFlavorTypeText, 0,
		      len, selection_buffer[1] );
    } else {
      myScrap = 0;
    }
  }
}

// Call this when a "paste" operation happens:
//+++ verify port to FLTK2
void fltk::paste(Widget &receiver, bool clipboard) {
  if (clipboard) {
    // see if we own the selection, if not go get it:
    ScrapRef scrap = 0;
    Size len = 0;
    if (GetCurrentScrap(&scrap) == noErr && scrap != myScrap &&
	GetScrapFlavorSize(scrap, kScrapFlavorTypeText, &len) == noErr) {
      if ( len >= selection_buffer_length[1] ) {
	delete[] selection_buffer[clipboard];
	int n = selection_buffer_length[clipboard]*2;
	if (!n) n = 1024;
	while (len >= n) n *= 2;
	selection_buffer_length[clipboard] = n;
	selection_buffer[clipboard] = new char[n];
      }
      GetScrapFlavorData( scrap, kScrapFlavorTypeText, &len,
			  selection_buffer[clipboard] );
      selection_buffer[clipboard][len] = 0;
      selection_length[clipboard] = len;
      // turn all \r characters into \n:
      for (char* p = selection_buffer[clipboard]; *p; ++p)
	if (*p=='\r') *p = '\n';
    }
  }
  e_text = selection_buffer[clipboard];
  e_length = selection_length[clipboard];
  receiver.handle(PASTE);
  return;
}

////////////////////////////////////////////////////////////////

/*
 * drag and drop whatever is in the cut-copy-paste buffer
 * - create a selection first using: 
 *     copy(const char *stuff, int len, 0)
 */
//+++ verify port to FLTK2
bool fltk::dnd()
{
  OSErr result;
  DragReference dragRef;
  result = NewDrag( &dragRef );
  if ( result != noErr ) return false;
  
  result = AddDragItemFlavor( dragRef, 1, 'TEXT', selection_buffer[0], selection_length[0], 0 );
  if ( result != noErr ) { DisposeDrag( dragRef ); return false; }
  
  Point mp;
  GetMouse(&mp);
  LocalToGlobal( &mp );
  RgnHandle region = NewRgn();
  SetRectRgn( region, mp.h-10, mp.v-10, mp.h+10, mp.v+10 );
  RgnHandle r2 = NewRgn();
  SetRectRgn( r2, mp.h-8, mp.v-8, mp.h+8, mp.v+8 );
  DiffRgn( region, r2, region );
  DisposeRgn( r2 );

  EventRecord event;
  ConvertEventRefToEventRecord( os_event, &event );
  result = TrackDrag( dragRef, &event, region );

  Widget *w = pushed();
  if ( w )
  {
    w->handle( RELEASE );
    pushed( 0 );
  }

  if ( result != noErr ) { DisposeRgn( region ); DisposeDrag( dragRef ); return false; }
  
  DisposeRgn( region );
  DisposeDrag( dragRef );
  return true;
}

/*
 * Due to the lack of subwindows, finding the XID is a little bit more involved.
 */
WindowPtr fltk::xid(const Window*w) {
  for (;;) {
    Window *w1 = w->window();
    if (!w1) break;
    w = w1;
  }
  return CreatedWindow::find(w)->xid;
}

FILE* fltk::fltk_fopen(const char* name, const char* flags) {
  return fopen(name, flags);
}

//
// End of "$Id: run.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//

