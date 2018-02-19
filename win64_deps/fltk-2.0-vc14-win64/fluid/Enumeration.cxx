// Code to access lists of enuerations used by WidgetType subclasses

#include "Enumeration.h"
#include <fltk/string.h>
#include <fltk/Menu.h>
#include <fltk/Item.h>
#include <stdlib.h>

const Enumeration* from_value(void* data, const Enumeration* table)
{
  for (;table->menu_entry; table++)
    if (table->compiled == data) return table;
  return 0;
}

const Enumeration* from_value(int data, const Enumeration* table)
{
  for (;table->menu_entry; table++)
    if (int((long)(table->compiled)) == data) return table;
  return 0;
}

const Enumeration* from_text(const char* text, const Enumeration* table)
{
  // For back compatability we strip leading fltk:: from symbols:
  if (text[0]=='F' && text[1]=='L' && text[2]=='_') text += 3;
  for (;table->menu_entry; table++) {
    if (table->symbol && !strcmp(table->symbol, text)) return table;
    // also for back compatability we match the menu entry and subclass:
    if (!strcasecmp(table->menu_entry, text)) return table;
    if (table->subclass && !strcmp(table->subclass, text)) return table;
  }
  return 0;
}

const char* to_text(void* data, const Enumeration* table)
{
  for (;table->menu_entry; table++) {
    if (table->compiled == data) {
      if (table->symbol && !table->subclass) return table->symbol;
      return table->menu_entry;
    }
  }
  return 0;
}

int number_from_text(const char* text, const Enumeration* table)
{
  if (table) {
    const Enumeration* t = from_text(text, table);
    if (t) return int((long)(t->compiled));
  }
  return strtol(text, 0, 0);
}

const char* number_to_text(int number, const Enumeration* table)
{
  if (table) {
    const char* t = to_text((void*)number, table);
    if (t) return t;
  }
  static char buffer[20];
  sprintf(buffer, "%d", number);
  return buffer;
}

////////////////////////////////////////////////////////////////
// Code to translate a table of Enumeration entries into fltk::Menu items:

class Enumeration_List : public fltk::List {
  virtual int children(const fltk::Menu*, const int* indexes, int level);
  virtual fltk::Widget* child(const fltk::Menu*, const int* indexes, int level);
  public: 
    virtual ~Enumeration_List() {}
};

static Enumeration_List enumeration_list;

void set_menu(fltk::Menu* menu, const Enumeration* list) {
  menu->list(&enumeration_list);
  menu->user_data((void*)list);
}

int Enumeration_List::children(const fltk::Menu* menu, const int*, int level)
{
  if (level) return -1;
  const Enumeration* e = (const Enumeration*)(menu->user_data());
  int n = 0;
  while (e->menu_entry) {n++; e++;}
  return n;
}

fltk::Widget* Enumeration_List::child(const fltk::Menu* menu, const int* indexes, int)
{
  const Enumeration* e = (const Enumeration*)(menu->user_data());
  int n = *indexes;
  while (n && e->menu_entry) {n--; e++;}
  if (!e->menu_entry) return 0;
  static fltk::Widget* widget;
  if (!widget) {
    fltk::Group::current(0);
    widget = new fltk::Item();
  }
  widget->user_data((void*)e);
  widget->label(e->menu_entry);
  widget->w(0);
  widget->h(0);
  return widget;
}

