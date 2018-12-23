//
// "$Id: FileChooser.cxx 8507 2011-03-06 10:18:05Z bgbnbigben $"
//
// FileChooser dialog for the Fast Light Tool Kit (FLTK).
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
//     http://www.fltk.org/str.php
//


#include <fltk/FileChooser.h>
#include <fltk/draw.h>
#include <fltk/run.h>
#include <fltk/Cursor.h>
using namespace fltk;

inline void FileChooser::cb_window_i(fltk::DoubleBufferWindow*, void*) {
  fileName->text("");
  fileList->deselect();
  fltk::remove_timeout((TimeoutHandler)previewCB, this);
  window->hide();
}
void FileChooser::cb_window(fltk::DoubleBufferWindow* o, void* v) {
  ((FileChooser*)(o->user_data()))->cb_window_i(o,v);
}

inline void FileChooser::cb_showChoice_i(fltk::Choice*, void*) {
  showChoiceCB();
}
void FileChooser::cb_showChoice(fltk::Choice* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_showChoice_i(o,v);
}

inline void FileChooser::cb_favoritesButton_i(fltk::PopupMenu*, void*) {
  favoritesButtonCB();
}

void FileChooser::cb_favoritesButton(fltk::PopupMenu* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_favoritesButton_i(o,v);
}

void FileChooser::cb_sortButton_i(fltk::PopupMenu* o, void* v) {
  FileChooser::sort = (FileSortF*)o->item()->user_data();
  update_sort();
}

void FileChooser::cb_sortButton(fltk::PopupMenu* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->parent()->user_data()))->cb_sortButton_i(o,v);
}

inline void FileChooser::cb_newButton_i(fltk::Button*, void*) {
  newdir();
}
void FileChooser::cb_newButton(fltk::Button* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_newButton_i(o,v);
}

#include <fltk/xbmImage.h>
static const unsigned char bits_new[32] = {
0,0,120,0,132,0,2,1,1,254,1,128,49,128,49,128,253,128,253,128,49,128,49,128,
1,128,1,128,255,255,0,0};
static fltk::xbmImage xbmImage_new(bits_new, 16, 16);

inline void FileChooser::cb__i(fltk::TiledGroup*, void*) {
  update_preview();
}
void FileChooser::cb_(fltk::TiledGroup* o, void* v) {
  ((FileChooser*)(o->parent()->user_data()))->cb__i(o,v);
}

inline void FileChooser::cb_fileList_i(fltk::FileBrowser*, void*) {
  fileListCB();
}
void FileChooser::cb_fileList(fltk::FileBrowser* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_fileList_i(o,v);
}

inline void FileChooser::cb_previewButton_i(fltk::CheckButton*, void*) {
  preview(previewButton->value());
}
void FileChooser::cb_previewButton(fltk::CheckButton* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->parent()->user_data()))->cb_previewButton_i(o,v);
}

inline void FileChooser::cb_showHiddenButton_i(fltk::CheckButton*, void*) {
  fileList->show_hidden(showHiddenButton->value() ? true: false);
  fileList->load(fileList->directory());
}
void FileChooser::cb_showHiddenButton(fltk::CheckButton* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->parent()->user_data()))->cb_showHiddenButton_i(o,v);
}

inline void FileChooser::cb_fileName_i(fltk::FileInput*, void*) {
  fileNameCB();
}
void FileChooser::cb_fileName(fltk::FileInput* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_fileName_i(o,v);
}

inline void FileChooser::cb_okButton_i(fltk::ReturnButton*, void*) {
  // Do any callback that is registered...
  if (callback_)
    (*callback_)(this, data_);
  
  window->hide();
}
void FileChooser::cb_okButton(fltk::ReturnButton* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->parent()->user_data()))->cb_okButton_i(o,v);
}

inline void FileChooser::cb_cancelButton_i(fltk::Button*, void*) {
  fileName->text("");
  fileList->deselect();
  fltk::remove_timeout((TimeoutHandler)previewCB, this);
  window->hide();
}
void FileChooser::cb_cancelButton(fltk::Button* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->parent()->user_data()))->cb_cancelButton_i(o,v);
}

inline void FileChooser::cb_favList_i(fltk::FileBrowser*, void*) {
  favoritesCB(favList);
}
void FileChooser::cb_favList(fltk::FileBrowser* o, void* v) {
  ((FileChooser*)(o->parent()->user_data()))->cb_favList_i(o,v);
}

inline void FileChooser::cb_favUpButton_i(fltk::Button*, void*) {
  favoritesCB(favUpButton);
}
void FileChooser::cb_favUpButton(fltk::Button* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_favUpButton_i(o,v);
}

inline void FileChooser::cb_favDeleteButton_i(fltk::Button*, void*) {
  favoritesCB(favDeleteButton);
}
void FileChooser::cb_favDeleteButton(fltk::Button* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_favDeleteButton_i(o,v);
}

inline void FileChooser::cb_favDownButton_i(fltk::Button*, void*) {
  favoritesCB(favDownButton);
}
void FileChooser::cb_favDownButton(fltk::Button* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_favDownButton_i(o,v);
}

inline void FileChooser::cb_favCancelButton_i(fltk::Button*, void*) {
  favWindow->hide();
}
void FileChooser::cb_favCancelButton(fltk::Button* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_favCancelButton_i(o,v);
}

inline void FileChooser::cb_favOkButton_i(fltk::ReturnButton*, void*) {
  favoritesCB(favOkButton);
}
void FileChooser::cb_favOkButton(fltk::ReturnButton* o, void* v) {
  ((FileChooser*)(o->parent()->parent()->user_data()))->cb_favOkButton_i(o,v);
}

FileChooser::FileChooser(const char *d, const char *p, int t, const char *title) {
  fltk::DoubleBufferWindow* w;
   {fltk::DoubleBufferWindow* o = window = new fltk::DoubleBufferWindow(490, 380, "Choose File");
    o->type(241);
    o->shortcut(0xff1b);
    o->callback((fltk::Callback*)cb_window, (void*)(this));
    o->begin();
     {fltk::Group* o = new fltk::Group(65, 10, 415, 25);
      o->begin();
       {fltk::Choice* o = showChoice = new fltk::Choice(0, 0, 215, 25, "Show:");
        o->labelfont(fltk::HELVETICA_BOLD);
        o->callback((fltk::Callback*)cb_showChoice);
        fltk::Group::current()->resizable(o);
        showChoice->label(show_label);
        fltk::Group::current()->resizable(o);
      }
       {fltk::PopupMenu* o = favoritesButton = new fltk::PopupMenu(225, 0, 155, 25, "Favorites");
        o->callback((fltk::Callback*)cb_favoritesButton);
        o->align(fltk::ALIGN_CENTER|fltk::ALIGN_INSIDE);
        favoritesButton->label(favorites_label);
        favorites_showing = 1;
      }
       {fltk::Button* o = newButton = new fltk::Button(390, 0, 25, 25);
        o->image(xbmImage_new);
        o->labelsize(8);
        o->callback((fltk::Callback*)cb_newButton);
        o->tooltip(new_directory_tooltip);
      }
      o->end();
    }
     {fltk::TiledGroup* o = new fltk::TiledGroup(10, 45, 470, 225);
      o->callback((fltk::Callback*)cb_);
      o->begin();
       {fltk::FileBrowser* o = fileList = new fltk::FileBrowser(0, 0, 295, 225);
        o->labelsize(14);
        o->callback((fltk::Callback*)cb_fileList);
        ((fltk::Window*)(o->parent()->parent()))->hotspot(o);
        fileList->type(0);fileList->when(fltk::WHEN_CHANGED);
      }
       {fltk::InvisibleBox* o = previewBox = new fltk::InvisibleBox(295, 0, 175, 225, "?");
        o->set_vertical();
        o->box(fltk::DOWN_BOX);
        o->labelsize(100);
        o->align(fltk::ALIGN_CENTER|fltk::ALIGN_CLIP);
      }
      o->end();
      //window->resizable(o);
      fltk::Group::current()->resizable(o);
    }
     {fltk::Group* o = new fltk::Group(10, 275, 470, 99);
      o->begin();
       {fltk::Group* o = new fltk::Group(0, 0, 470, 40);
        o->begin();
         {fltk::CheckButton* o = previewButton = new fltk::CheckButton(0, 0, 73, 20, "Preview");
          o->set_flag(fltk::STATE);
          o->shortcut(0x80070);
          o->callback((fltk::Callback*)cb_previewButton);
          previewButton->label(preview_label);
        }
         {fltk::CheckButton* o = showHiddenButton = new fltk::CheckButton(80, 0, 100, 20, "Show &Hidden");
          o->set_flag(fltk::STATE);
          o->shortcut(0x80070);
          o->callback((fltk::Callback*)cb_showHiddenButton);
          o->value(fileList->show_hidden() ? 1 : 0);
        }
         {fltk::PopupMenu* o = sortButton = new fltk::PopupMenu(270, 0, 200, 20, "Sort method");
          o->callback((fltk::Callback*)cb_sortButton);
          o->align(fltk::ALIGN_CENTER|fltk::ALIGN_INSIDE);
          sortButton->label(sort_menu_label);
        }
        //new fltk::InvisibleBox(180, 0, 290, 20);
        o->end();
      }
       {fltk::FileInput* o = fileName = new fltk::FileInput(105, 25, 365, 35);
        o->labelfont(fltk::HELVETICA_BOLD);
        o->labelsize(14);
        o->textsize(14);
        o->callback((fltk::Callback*)cb_fileName);
        o->when(fltk::WHEN_ENTER_KEY);
        fileName->when(fltk::WHEN_CHANGED | fltk::WHEN_ENTER_KEY_ALWAYS);
      }
       {fltk::InvisibleBox* o = new fltk::InvisibleBox(0, 35, 95, 25, "Filename");
        o->labelfont(fltk::HELVETICA_BOLD);
        o->align(fltk::ALIGN_RIGHT|fltk::ALIGN_CENTER|fltk::ALIGN_INSIDE);
        o->label(filename_label);
      }
       {fltk::Group* o = new fltk::Group(0, 70, 470, 29);
        o->begin();
         {fltk::ReturnButton* o = okButton = new fltk::ReturnButton(315, 0, 73, 25, "OK");
          o->shortcut(0xff0d);
          o->callback((fltk::Callback*)cb_okButton);
          okButton->label("Ok");
        }
         {fltk::Button* o = cancelButton = new fltk::Button(398, 0, 72, 25, "Cancel");
          o->callback((fltk::Callback*)cb_cancelButton);
          o->label("Cancel");
        }
        o->end();
      }
      o->end();
    }
    o->end();
    if (title) window->label(title);
    o->set_modal();
  }
   {fltk::DoubleBufferWindow* o = favWindow = new fltk::DoubleBufferWindow(355, 150, "Manage Favorites");
    w = o;
    o->type(241);
    o->shortcut(0xff1b);
    o->user_data((void*)(this));
    o->begin();
     {fltk::FileBrowser* o = favList = new fltk::FileBrowser(10, 10, 300, 95);
      o->callback((fltk::Callback*)cb_favList);
      o->when(fltk::WHEN_CHANGED);
      o->type(0);
    }
     {fltk::Group* o = new fltk::Group(320, 10, 25, 95);
      o->set_vertical();
      o->begin();
       {fltk::Button* o = favUpButton = new fltk::Button(0, 0, 25, 25, "@8>");
        o->callback((fltk::Callback*)cb_favUpButton);
      }
       {fltk::Button* o = favDeleteButton = new fltk::Button(0, 35, 25, 25, "X");
        o->labelfont(fltk::HELVETICA_BOLD);
        o->callback((fltk::Callback*)cb_favDeleteButton);
        fltk::Group::current()->resizable(o);
      }
       {fltk::Button* o = favDownButton = new fltk::Button(0, 70, 25, 25, "@2>");
        o->callback((fltk::Callback*)cb_favDownButton);
      }
      o->end();
      o->resizable(0);
    }
     {fltk::Group* o = new fltk::Group(0, 113, 355, 37);
      o->begin();
       {fltk::Button* o = favCancelButton = new fltk::Button(272, 2, 72, 25, "Cancel");
        o->callback((fltk::Callback*)cb_favCancelButton);
        favCancelButton->label("Cancel");
      }
       {fltk::ReturnButton* o = favOkButton = new fltk::ReturnButton(180, 2, 79, 25, "Save");
        o->shortcut(0xff0d);
        o->callback((fltk::Callback*)cb_favOkButton);
        favOkButton->label(save_label);
      }
       {fltk::InvisibleBox* o = new fltk::InvisibleBox(0, 2, 175, 25);
        fltk::Group::current()->resizable(o);
      }
      o->end();
      o->resizable(0);
    }
    o->end();
    favWindow->label(manage_favorites_label);
    favWindow->resizable(fileList);
    o->shortcut(0xff1b);
    o->set_modal();
    o->resizable(o);
  }
  callback_ = 0;
  data_ = 0;
  directory_[0] = 0;
  window->size_range(window->w(), window->h(), 0, 0);
  type(t);
  filter(p);
  update_favorites();
  sortButton->hide();
  value(d);
  type(t);
  int e;
  prefs_.get("preview", e, 1);
  preview(e);
}

FileChooser::~FileChooser() {
  fltk::remove_timeout((TimeoutHandler)previewCB, this);
  delete window;
  delete favWindow;
}

void FileChooser::filter_value(int f) {
  showChoice->value(f);
  showChoiceCB();
}

void FileChooser::ok_label(const char *l) {
  okButton->label(l);
  int w=0, h=0;
  okButton->measure_label(w, h);
  w+=70;
  okButton->resize(cancelButton->x()- w, cancelButton->y(),w-10, 25);
  okButton->parent()->init_sizes();
}

void FileChooser::show() {
  window->hotspot(fileList);
  window->show();
  fltk::flush();
  fltk::cursor(fltk::CURSOR_WAIT);
  rescan();
  fltk::cursor(fltk::CURSOR_DEFAULT);
  fileName->take_focus();
}

void FileChooser::show(int x, int y) {
  window->show();
  window->resize(x, y,window->w(),window->h());
  fltk::flush();
  fltk::cursor(fltk::CURSOR_WAIT);
  rescan();
  fltk::cursor(fltk::CURSOR_DEFAULT);
  fileName->take_focus();
}

bool FileChooser::exec(Window* p, bool grab) {
  if (shown()) window->hide();
  return window->exec(p, grab);
}

void FileChooser::type(int t) {
  type_ = t;
  if (t & MULTI)
    fileList->type(Browser::MULTI);
  else
    fileList->type(Browser::NORMAL);
  if (t & CREATE)
    newButton->activate();
  else
    newButton->deactivate();
  if (t & DIRECTORY)
    fileList->filetype(FileBrowser::DIRECTORIES);
  else
    fileList->filetype(FileBrowser::FILES);
}

void FileChooser::favorites(int e) {
  favorites_showing = e;
  update_favorites();
}

//  End of "$Id: FileChooser.cxx 8507 2011-03-06 10:18:05Z bgbnbigben $"

