//
// "$Id: browser.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// browser.cxx
// Maarten de Boer's toggle tree demo program rewritten to use the
// fltk 2.0 browser.  This unfortunately required a bunch of changes.
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

#include <fltk/Browser.h>
#include <fltk/Window.h>
#include <fltk/Button.h>
#include <fltk/CheckButton.h>
#include <fltk/RadioButton.h>
#include <fltk/Input.h>
#include <fltk/ValueSlider.h>
#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/MenuBuild.h>
#include <fltk/ask.h>
#include <fltk/xpmImage.h>
#include <fltk/MultiImage.h>

#include <pixmaps/folder_small.xpm>
#include <pixmaps/folder_small2.xpm>
#include <pixmaps/folder_small3.xpm>
#include <pixmaps/file_small.xpm>
#include <pixmaps/file_small2.xpm>
#include <pixmaps/book.xpm>
#include "porsche.xpm"

#include <stdio.h>
#include <stdlib.h>

using namespace fltk;

xpmImage folderSmall(folder_small);
xpmImage folderSmall2(folder_small2);
xpmImage folderSmall3(folder_small3);
xpmImage fileSmall(file_small);
xpmImage fileSmall2(file_small2);
xpmImage bookImg(book);
xpmImage customImage(porsche_xpm);

void cb_test(Widget* browser, void*) {
  Browser *b = (Browser*)browser;
  Widget* w = b->item();
  printf("Callback, b->item() = '%s'",
	 w && w->label() ? w->label() : "null");
  if (event_clicks()) printf(", Double Click");
  printf("\n");
  if (b->selected_column()!= Browser::NO_COLUMN_SELECTED )
      fltk::message("Column %d selected\n", b->selected_column()+1);
}

void cb_remove(Widget*, void* ptr) {
  Browser* tree = (Browser*) ptr;
  if (tree->type() & Browser::IS_MULTI) {
    Widget* w = tree->goto_top();
    while (w) {
      if (w->selected()) { // test for parent being open
	Group* g = w->parent();
	g->remove(w);
	delete w;
	g->relayout();
	w = tree->goto_top();
      } else {
	w = tree->next();
      }
    }
  } else {
    Widget* w = tree->goto_focus();
    if (w) {
      Group* g = w->parent();
      g->remove(w);
      delete w;
      g->relayout();
    }
  }
}

void cb_multi(Button* w, void* ptr) {
  Browser* tree = (Browser*) ptr;
  tree->type(w->value() ? Browser::MULTI : Browser::NORMAL);
  tree->relayout();
}

static Group* current_group(Browser* tree) {
  Widget* w = tree->goto_focus();
  if (!w) return tree;
  if (w->is_group() && w->flag(fltk::OPENED)) return (Group*)w;
  return w->parent() ? w->parent() : tree;
}

void cb_add_folder(Widget*, void* ptr) {
  Browser* tree = (Browser*) ptr;
  tree->add_group("Added folder", current_group(tree));
  tree->relayout();
}

void cb_add_paper(Widget*, void* ptr) {
  Browser* tree = (Browser*) ptr;
  tree->add_leaf("New paper\t@rt2.col\t3.col", current_group(tree));
  tree->relayout();
}

void cb_when_changed(Button* b, void* ptr) {
  Browser* tree = (Browser*)ptr;
  if (b->value())
    tree->when(tree->when()|fltk::WHEN_CHANGED);
  else
    tree->when(tree->when()&~fltk::WHEN_CHANGED);
}

void cb_when_release(Button* b, void* ptr) {
  Browser* tree = (Browser*)ptr;
  if (b->value())
    tree->when(tree->when()|fltk::WHEN_RELEASE);
  else
    tree->when(tree->when()&~fltk::WHEN_RELEASE);
}

void cb_when_not_changed(Button* b, void* ptr) {
  Browser* tree = (Browser*)ptr;
  if (b->value())
    tree->when(tree->when()|fltk::WHEN_NOT_CHANGED);
  else
    tree->when(tree->when()&~fltk::WHEN_NOT_CHANGED);
}

void cb_when_enter_key(Button* b, void* ptr) {
  Browser* tree = (Browser*)ptr;
  if (b->value())
    tree->when(tree->when()|fltk::WHEN_ENTER_KEY);
  else
    tree->when(tree->when()&~fltk::WHEN_ENTER_KEY);
}

void button_cb(Widget* b, void *) {
  printf("Button %s pushed\n", b->label());
}

const char *labels[] = {"Column 1", "Column 2\nwith new\nlines", "Column 3", 0};
int widths[]   = {100, 70, 70, 0};

Browser *browser=0;
bool flip = false;
bool bm = true;
bool dlines = true;

MultiImage g1(fileSmall, HIGHLIGHT, fileSmall2);
MultiImage g2(folderSmall, HIGHLIGHT, folderSmall3);
MultiImage g3(fileSmall2, HIGHLIGHT, fileSmall);
MultiImage g4(folderSmall2, OPENED, folderSmall3, HIGHLIGHT, folderSmall);
MultiImage g5(folderSmall2, OPENED, folderSmall3);

void update_look() {
  if (bm) {
    if (flip) {
      browser->leaf_symbol(&g1);
      browser->group_symbol(&g2);
    } else {
      browser->leaf_symbol(&g3);
      browser->group_symbol(&g4);
    }
  } else {
    if (flip) {
      browser->leaf_symbol(&fileSmall);
      browser->group_symbol(&folderSmall);
    } else {
      browser->leaf_symbol(&fileSmall2);
      browser->group_symbol(&g5);
    }
  }
  browser->relayout();
}

// callback for changing dynamically the look of the tree browser 
void cb_change_look(Widget*, void* ptr) {
  flip = !flip;
  update_look();
}

// callback for deactivate/activate the belowmouse img change
void below_mouse_cb(Button *w, long arg) {
  bm = w->value();
  update_look();
}

void display_lines_cb(Widget* w, void* ptr_arg) {
  Browser* tree = reinterpret_cast<Browser*>(ptr_arg);
  dlines = !dlines;
  tree->display_lines(dlines);
  tree->relayout();
}

void change_resize(Button *w, long arg) {
  if (w->value()) 
    widths[1] = -1;
  else
    widths[1] = 70; 
  browser->column_widths(widths);
}

int main(int argc,char** argv) {

  Window win(280, 340, "Browser Example");
  win.begin();

  Browser tree(10, 10, 260, 180);
  tree.indented(1);
  tree.callback(cb_test);

  browser = &tree;
  tree.column_widths(widths);
  tree.column_labels(labels);
  
  Button remove_button(5, 200, 80, 22, "Remove");
  remove_button.callback((Callback*)cb_remove, (void *)&tree);

  Button add_paper_button(5, 224, 80, 22, "Add Paper");
  add_paper_button.callback((Callback*)cb_add_paper, (void *)&tree);

  Button add_folder_button(5, 248, 80, 22, "Add Folder");
  add_folder_button.callback((Callback*)cb_add_folder, (void *)&tree);

  Button change_look_button(5, 272, 80, 22, "Change Look!");
  change_look_button.callback((Callback*)cb_change_look , (void *)&tree);

  CheckButton multi_button(88, 200, 160, 20, "Browser::MULTI");
  multi_button.callback((Callback*)cb_multi, (void *)&tree);

  CheckButton when_changed_button(88, 220, 160, 20, "WHEN_CHANGED");
  when_changed_button.callback((Callback*)cb_when_changed, (void *)&tree);

  CheckButton when_not_changed_button(88, 240, 160, 20, "WHEN_NOT_CHANGED");
  when_not_changed_button.callback((Callback*)cb_when_not_changed, (void *)&tree);

  CheckButton when_release_button(88, 260, 160, 20, "WHEN_RELEASE");
  when_release_button.callback((Callback*)cb_when_release, (void *)&tree);
  when_release_button.set_flag(fltk::STATE);

  CheckButton when_enter_key_button(88, 280, 160, 20, "WHEN_ENTER_KEY");
  when_enter_key_button.callback((Callback*)cb_when_enter_key, (void *)&tree);

  // Let's show how to show/hide lines in the Browser (tree) widget
  CheckButton display_lines(88, 300, 160, 20, "Display lines");
  display_lines.value(true);
  display_lines.callback((Callback*)display_lines_cb, (void *)&tree);

  CheckButton resize(108, 320, 160, 20, "Make 2. column flexible");
  resize.callback((Callback*)change_resize);

  CheckButton bm(5, 320, 82, 20, "below mouse");
  bm.set();
  bm.callback((Callback*)below_mouse_cb);

  win.resizable(tree);
  win.end();

#if USE_STRING_LIST
  //tree.list(new String_List("alpha\0beta\0ceta\0delta\0red\0green\0blue\0"));
  tree.list(new String_List(strings, sizeof(strings)/sizeof(*strings)));
  //tree.list(new String_List(strings));
#else

  // defining default images for nodes
  cb_change_look(0, &tree);

  // Add some nodes with icons -- some open, some closed.
  Group* g;
  g = tree.add_group ("aaa\t2.col\t3.col", &tree);
  tree.add_group ("bbb TWO\t2.col\t3.col", g);

  g = tree.add_group ("bbb", &tree);
  tree.add_leaf("ccc\t789", g); 
  tree.add_leaf("ddd\t012", g); 

  
  g = tree.add_group("eee", &tree);
  tree.add_leaf("fff", &tree)->image(customImage); // let's have fun with a custom image
					    // while demonstrating different height nodes
  g = tree.add_group("ggg", g);
  tree.add_leaf("hhh", g); // g decl is not even necessary for next children
  tree.add_leaf("iii", g); // I let it for keeping API orthogonality

  g = tree.add_group("jjj",&tree);
  tree.add_leaf("kkk",g);

  tree.add_leaf("lll");
 tree.add_leaf("zut");

  g = tree.add_group("mmm", &tree, 0); // let this node closed
  tree.add_leaf("nnn",g);
  tree.add_leaf("ooo",g);

  g = tree.add_group("ppp", g);
  tree.add_leaf("qqq",g);

  g = tree.add_group("rrr", g); // more imbricated groups 
  g = tree.add_group("sss", g);
  g = tree.add_group("ttt", g);

  g = tree.add_group("uuu", &tree);
  tree.add_leaf("vvv", g);
  tree.add_leaf("www", g);

  tree.add_leaf("yyy", g);
  tree.add_leaf("zzz", g);


  // add some widgets:
  Button * b = new Button(0,0,100,23,"button");
  b->callback(button_cb);
#if 0
  // fabien: creating the CheckButton below will still mess up the drawing ... 
  //   have to be fixed, but not in the browser i think.
  b = new CheckButton(0,0,100,23,"CheckButton");

  printf("b->type = %d, group = %d, is_group = %d\n",
	 b->type(), Widget::GROUP_TYPE, b->is_group());
  b->callback(button_cb);
#endif
  new Input(0,0,200,23,"Input:");
  new ValueSlider(0,0,200,23,"Input1:");
  new ValueSlider(0,0,200,23,"Input2:");
  new ValueSlider(0,0,200,23,"Input3:");
  tree.end();


#if 0
  // Examples of removing items (successfully, and unsuccessfully)
  // by label name:
  if (&tree.remove("xxx"))
    printf("Successfully deleted \"xxx\"\n");
  else
    printf("Could not delete \"xxx\"\n");

  if (&tree.remove("nonexistant"))
    printf("Successfully deleted \"nonexistant\"\n");
  else
    printf("Could not delete \"nonexistant\"\n");
#endif
#endif

  tree.select(tree.find( "fff" ),1);

  win.show(argc,argv);
 
  run();
  return 0;
}

// End of "$Id: browser.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
