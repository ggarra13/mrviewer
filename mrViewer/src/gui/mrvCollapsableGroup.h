/**
 * @file   mrvCollapsableGroup.h
 * @author gga
 * @date   Tue Aug  7 04:48:52 2007
 * 
 * @brief  A group widget that has a button allowing it to collapse
 *         the contents.
 * 
 * 
 */

#ifndef mrvCollapsableGroup_h
#define mrvCollapsableGroup_h


#include <fltk/PackedGroup.h>

namespace fltk {
  class Button;
}


namespace mrv {

  class CollapsableGroup : public fltk::Group
  {
  public:
    CollapsableGroup( const int x, const int y, const int w, 
		      const int h, const char* l = 0 );
    ~CollapsableGroup();

    void add( fltk::Widget* w );

    void remove_all();

    void spacing( int x );

    fltk::PackedGroup* contents() { return _contents; }

    virtual void layout();

  protected:
    fltk::PackedGroup* _contents;

    static void toggle_tab( fltk::Button* w, void* data );
  };


} // namespace mrv


#endif // mrvCollapsableGroup_h
