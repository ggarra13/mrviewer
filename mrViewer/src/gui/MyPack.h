//
// This is a nabbed copy of fltk1.3.x Fl_Pack, modified to allow the
// layout code to be called directly, instead of hidden in draw(). -erco 03/19/19
//

/* \file
   MyPack widget . */

#ifndef MyPack_H
#define MyPack_H

#include <FL/Fl_Group.H>
#include <FL/Fl_Scrollbar.H>

/**
  This widget was designed to add the functionality of compressing and
  aligning widgets.
  <P>If type() is MyPack::HORIZONTAL all the children are
  resized to the height of the MyPack, and are moved next to
  each other horizontally. If type() is not MyPack::HORIZONTAL
  then the children are resized to the width and are stacked below each
  other.  Then the MyPack resizes itself to surround the child
  widgets.
  <P>This widget is needed for the Fl_Tabs.
  In addition you may want to put the MyPack inside an 
  Fl_Scroll.

  <P>The resizable for MyPack is set to NULL by default.</p>
  <P>See also: Fl_Group::resizable()
*/
class FL_EXPORT MyPack : public Fl_Group {
  int spacing_;
  
public:
  enum { // values for type(int)
    VERTICAL = 0,
    HORIZONTAL = 1
  };

protected:
  void draw();

public:
  MyPack(int x,int y,int w ,int h,const char *l = 0);

  void end()
  {
      Fl_Group::end();
      layout();
  }

  /**
    Gets the number of extra pixels of blank space that are added
    between the children.
  */
  int spacing() const {return spacing_;}
  /**
    Sets the number of extra pixels of blank space that are added
    between the children.
  */
  void spacing(int i) {spacing_ = i;}
  /** Same as Fl_Group::type() */
  uchar horizontal() const {return type();}
  void layout();
};

#endif
