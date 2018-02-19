#include <fltk/StringList.h>
#include <fltk/Item.h>
using namespace fltk;

/*! \class fltk::StringHierarchy

  This subclass of List allows a Menu or Browser to display strings
  that are generated at run time. This is the most efficient way to
  make hundreds of thousands of items.

  Typically you will create a subclass with the children() and
  label() functions overridden, and make a single static instance
  of this class. It can be used by several menus, but the Menu*
  argument to children() and label() is used to differentiate them.

*/

Widget* StringHierarchy::child(const Menu* group, const int* indexes,int level)
{
  // Construct reusable widget:
  if (!generated_item_) {
    Group::current(0);
    generated_item_ = new Item();
  }
  // cause measure() to be called if width is needed:
  generated_item_->w(0);
  //generated_item_->h(0);
  // We must clear flags so it does not accidentally think the item
  // is selected or (if a parent) is opened:
  generated_item_->clear_flag(fltk::SELECTED|fltk::STATE|fltk::OPENED);
  generated_item_->label(label(group, indexes, level));
  return generated_item_;
}

/*! \fn Widget* StringHierarchy::generated_item()

  Inside label() this points at the widget that will have the label
  set. This allows you to mess with the flags, font, user_data, etc
  of the widget. This widget is reused every time a new label is
  needed, so be sure to set everything on each call and do not
  assumme they have been put back to the default values.

  If your items vary in vertical size, you must do generated_item()->h(0)
  in label(). This is unfortunate but it speeds up the common case where
  they all have the same height.

  Convesely, setting w() will avoid the need to call measure() if the
  width is needed and you happen to know it.
*/

/*! \class fltk::StringList
  This is a simplification of StringHierarchy where there are no subitems.
  It makes the virtual functions easier to write.

  Typically you will create a subclass with the children() and
  label() functions overridden, and make a single static instance
  of this class. It can be used by several menus, but the Menu*
  argument to children() and label() is used to differentiate them.

\code
class MyList : public StringList {
  int children(const Menu* menu) {
    return ((MyClass*)(menu->user_data()))->number_of_items();
  }
  const char* label(const Menu* menu, int index) {
    return ((MyClass*)(menu->user_data()))->label(index);
  }
};
static MyList myList;

Menu themenu;
MyClass myobject;
themenu->list(&mylist);
themenu->callback(mycallback, &myobject);
\endcode
*/

int StringList::children(const Menu* group, const int* indexes, int level) {
  if (!level)
    return children(group);
  else
    return -1;
}

const char* StringList::label(const Menu* group, const int* indexes, int level)
{
  return label(group, indexes[0]);
}

/*! \class fltk::StringArray

  This subclass of List allows a Menu or Browser to display an array
  of const C strings. The array is given to the constructor of can
  be changed with set(). Using this is much more efficient than adding
  an Item() to the menu.

  \code
  const char* names[] = {"alpha", "beta", "gamma"};
  StringArray* list = new StringList(names,3);
  // Identical result can be made with this:
  // StringArray* list = new StringList("alpha\0beta\0gamma\0");
  menu->list(list);
  \endcode
*/
int StringArray::children(const Menu*) {return children_;}

const char* StringArray::label(const Menu*, int index) {
  return array[index];
}

/*! \fn void StringArray::set(const char*const* array, int n)
  Make it return \a n labels from \a array. The array and strings
  are \e not copied, they should be in static memory!
*/

/*!
  Make it return labels from \a array, where the size of the array
  is determined by the first null pointer in it. The array and strings
  are \e not copied, they should be in static memory!
  If \a array is null then children is set to zero.
*/
void StringArray::set(const char*const* array) {
  this->array = array;
  for (children_ = 0; array && array[children_]; children_++);
}

/*!
  Sets the list to return each substring in the nul-seperated
  string.  The list is terminated by two nul characters. If the string
  is null then children is set ot zero.

  Warning: the string is not copied!

  The current implementation leaks the created array when the
  StringList is deleted or if set() is called again. This really
  should be a subclass with a destructor, but I can't figure out a
  good name for it...
*/
void StringArray::set(const char* s) {
  if (!s || !*s) {children_ = 0; return;}
  const char* temp[256];
  int n = 0;
  while (*s && n < 256) {
    temp[n] = s;
    while (*s++);
    n++;
  }
  children_ = n;
  const char** t = new const char* [n]; // this array is never freed...
  array = t;
  for (; n--;) t[n] = temp[n];
}
