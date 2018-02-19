#include <windows.h>
#include <process.h>

// these pointers are in run.cxx:
extern void (*fl_lock_function)();
extern void (*fl_unlock_function)();
static void init_function();
static void (*init_or_lock_function)() = init_function;

static CRITICAL_SECTION cs;

static void unlock_function() {
  LeaveCriticalSection(&cs);
}

static void lock_function() {
  EnterCriticalSection(&cs);
}

static DWORD main_thread_id;

static void init_function() {
  InitializeCriticalSection(&cs);
  lock_function();
  fl_lock_function = init_or_lock_function = lock_function;
  fl_unlock_function = unlock_function;
  main_thread_id = GetCurrentThreadId();
}

void fltk::lock() {init_or_lock_function();}

void fltk::unlock() {fl_unlock_function();}

bool fltk::in_main_thread() {
  return init_or_lock_function == init_function || GetCurrentThreadId() == main_thread_id;
}

#define WM_MAKEWAITRETURN (WM_USER+0x401)
void fltk::awake(void* msg) {
  PostThreadMessage( main_thread_id, WM_MAKEWAITRETURN, (WPARAM)msg, 0);
}
