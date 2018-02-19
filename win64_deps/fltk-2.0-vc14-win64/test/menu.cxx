// Test of the various subclasses of Menu

#include <fltk/run.h>
#include <fltk/Item.h>
#include <fltk/ItemGroup.h>
#include <fltk/Divider.h>
#include <fltk/Window.h>
#include <fltk/Box.h>
#include <fltk/MenuBar.h>
#include <fltk/PopupMenu.h>
#include <fltk/InputBrowser.h>
#include <fltk/Choice.h>
#include <fltk/CycleButton.h>
#include <fltk/Button.h>
#include <fltk/draw.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fltk/Slider.h>
#include <fltk/StringList.h>
#include <fltk/events.h>
#include <fltk/ask.h>
using namespace fltk;

#define WIDTH 600
#define HEIGHT 23

void callback(Widget* w, void*) {
  Menu* menu = (Menu*)w;
  Widget* item = menu->get_item();
  printf("Callback for %s, item is %s\n",
	 menu->label() ? menu->label() : "menu bar",
	 item ? item->label() ? item->label() : "unnamed" : "none");
  if (item) item->do_callback();
  printf("event_button = %d\n", fltk::event_button());
}

void never_cb(Widget* w, void*) {
  alert("This callback should not be ever executed");
}

const char* const strings[] = {
  "This","is","a","test","of","a","menu","defined","as a","StringArray"
};
StringArray thelist(strings, sizeof(strings)/sizeof(*strings));

void build_hierarchy() {
  Item* ii = new Item("&Alpha");
  ii->when(WHEN_NEVER);
  ii->callback(never_cb);
  new Item("&Beta");
  new Item("&Gamma");
  new Item("&Delta");
  new Item("&Epsilon");
  ItemGroup* g = new ItemGroup("submenu&1");
  g->begin();
  new Item("Item &1");
  new Item("Item &2");
  new Item("Item &3");
  new Item("Item &4");
  g->end();
  // now using MenuSection facility:
  {MenuSection g("submenu&2");
      (new Item("Item &1"))->textsize(10);
      (new Item("Item &2"))->textsize(14);
      (new Item("Item &3"))->textsize(18);
      (new Item("Item &4"))->textsize(22);
      //g->deactivate();
      {MenuSection g1("&nested menu");
	  new Item("Item &1");
	  new Item("Item &2");
	  {MenuSection g2("deeper");
	      (new Item("Very low level items"))->deactivate();
	      (new Item("Are here on this menu"))->deactivate();
	      new Item("In this test");
	      new Item("Program");
	  }
      }
  }
}

void quit_cb(Widget*, void*) {exit(0);}

class ListShortcuts : public ShortcutFunctor {
public:
  bool handle(const Widget* widget, unsigned key) {
    printf("Widget=%s shortcut=%s\n", widget->label() ? widget->label() : "NULL", key_name(key));
    return false;
  }
} listShortcuts;

int main(int argc, char **argv) {
  Window window(WIDTH,400);
  window.begin();
  MenuBar menubar(0,0,WIDTH,HEIGHT);
  menubar.callback(callback);
  menubar.begin();

  ItemGroup file("&File");
  file.begin();
  Item* o = new Item("Quit");
  o->shortcut(COMMAND+'q');
  o->callback(quit_cb);
  new Divider();
  (new Slider(0,0,100,30))->value(.3);
  build_hierarchy();
  file.end();
  ItemGroup edit("&Edit");
  edit.begin();
  (new Item("Undo"))->shortcut(COMMAND+'z');
  (new Item("Cut"))->shortcut(COMMAND+'x');
  (new Item("Copy"))->shortcut(COMMAND+'c');
  (new Item("Paste"))->shortcut(COMMAND+'v');
  build_hierarchy();
  edit.end();
  //edit.deactivate();
  ItemGroup options("&Options");
  options.begin();
  o = new Item("Red"); o->type(Item::RADIO);
  o = new Item("Green"); o->type(Item::RADIO);
  o = new Item("Blue"); o->type(Item::RADIO);
  o = new Item("Aqua"); o->type(Item::RADIO);
  o = new Item("Toggle 1"); o->type(Item::TOGGLE);
  o = new Item("Toggle 2"); o->type(Item::TOGGLE);
  o = new Item("Toggle 3"); o->type(Item::TOGGLE);
  o = new Item("Toggle 4"); o->type(Item::TOGGLE);
  options.end();
  new Divider();
  Item item("&Toggle"); item.type(Item::TOGGLE);
  Item but("&button");
  menubar.end();
  menubar.tooltip("This is a menu bar");

  const int VWIDTH=100;

  MenuBar mb0(0,HEIGHT,VWIDTH,400-HEIGHT);
  mb0.set_vertical();
  mb0.begin();
  build_hierarchy();
  mb0.end();

  Widget box(VWIDTH,HEIGHT,WIDTH-VWIDTH,400-HEIGHT);
  //box.color(WHITE);
  box.box(fltk::DOWN_BOX);
  box.tooltip("Press right button for a pop-up menu");

  PopupMenu mb(VWIDTH,HEIGHT,WIDTH-VWIDTH,400-HEIGHT, "popup");
  mb.callback(callback);
  mb.begin();
  build_hierarchy();
  mb.end();
  mb.type(PopupMenu::POPUP3);

  int x = 180;
  int y = 50;

  PopupMenu mb1(x,y,100,25,"&PopupMenu");
  mb1.callback(callback);
  mb1.begin();
  build_hierarchy();
  mb1.end();
  mb1.tooltip("This is a menu button");
  y += 60;

  Choice ch(x,y,100,25,"&choice:");
  ch.callback(callback);
  ch.begin();
  build_hierarchy();
  ch.end();
  ch.tooltip("This is a choice");
  y += 60;

  CycleButton cb(x,y,100,25,"&cycle:");
  cb.callback(callback);
  cb.list(&thelist);
  cb.tooltip("This is a cyclebutton");
  y += 60;

  InputBrowser ib(x,y,100,25,"Input&Browser:");
  ib.type(InputBrowser::INDENTED);
  ib.callback(callback);
  ib.when(fltk::WHEN_CHANGED);
  ib.begin();
  build_hierarchy();
  ib.end();
  ib.tooltip("This is an InputBrowser");
  y += 60;

  Button button(x+200,y,100,30,"button");
  button.tooltip("This is to test if clicking here to exit menus pushes the button");

  window.resizable(box);
  window.size_range(300,20);
  window.end();
  window.show(argc, argv);

  foreachShortcut(listShortcuts);

  return run();
}
