#ifndef Enumeration_h
#define Enumeration_h

////////////////////////////////////////////////////////////////
// This structure is used to define tables of enumerations:

struct Enumeration {
  const char* menu_entry;	// user-friendly string, null for end of table
  const char* symbol;		// symbol for c++ code and for .fl file
  void* compiled;		// symbol compiled for use by fluid
  const char* subclass;		// For type() of widgets, use this subclass
};

// Use this call to make a menu/Choice from a table. Warning this
// will overwrite the user_data() of the fltk::Menu:
namespace fltk {class Menu;}
void set_menu(fltk::Menu*, const Enumeration*);

// Converters from/to strings and values:
const Enumeration* from_value(void* data, const Enumeration* table);
const Enumeration* from_value(int data, const Enumeration* table);
const Enumeration* from_text(const char* text, const Enumeration* table);
const char* to_text(void* data, const Enumeration* table);
int number_from_text(const char* text, const Enumeration* table);
const char* number_to_text(int number, const Enumeration* table);

////////////////////////////////////////////////////////////////

#endif
