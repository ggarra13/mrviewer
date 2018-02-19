//
// "$Id: FileChooser.h 8507 2011-03-06 10:18:05Z bgbnbigben $"
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

#ifndef fltk_FileChooser_h
#define fltk_FileChooser_h

#include <fltk/DoubleBufferWindow.h>
#include <fltk/Choice.h>
#include <fltk/PopupMenu.h>
#include <fltk/Button.h>
#include <fltk/Preferences.h>
#include <fltk/TiledGroup.h>
#include <fltk/FileBrowser.h>
#include <fltk/InvisibleBox.h>
#include <fltk/CheckButton.h>
#include <fltk/FileInput.h>
#include <fltk/ReturnButton.h>
#include <fltk/ask.h>

namespace fltk  {

class FL_API FileChooser  {
private:
  static fltk::Preferences prefs_;
  void (*callback_)(FileChooser*, void *);
  void *data_;
  char directory_[1024];
  char pattern_[1024];
  char preview_text_[2048];
  int type_;
  void favoritesButtonCB();
  void favoritesCB(fltk::Widget *w);
  void fileListCB();
  void fileNameCB();
  void newdir();
  static void previewCB(FileChooser *fc);
  void showChoiceCB();
  void update_favorites();
  void update_preview();
  void update_sort();
  int favorites_showing;
  void activate_okButton_if_file();
  void directory(const char *d, bool);

  // FileChooser window
  fltk::DoubleBufferWindow *window;
  inline void cb_window_i(fltk::DoubleBufferWindow*, void*);
  static void cb_window(fltk::DoubleBufferWindow*, void*);

  // Show button
  fltk::Choice *showChoice;
  inline void cb_showChoice_i(fltk::Choice*, void*);
  static void cb_showChoice(fltk::Choice*, void*);

  // Favorites button
  fltk::PopupMenu *favoritesButton;
  inline void cb_favoritesButton_i(fltk::PopupMenu*, void*);
  static void cb_favoritesButton(fltk::PopupMenu*, void*);

  // "New" button
  inline void cb_newButton_i(fltk::Button*, void*);
  static void cb_newButton(fltk::Button*, void*);

  // Default callback (update_preview())
  inline void cb__i(fltk::TiledGroup*, void*);
  static void cb_(fltk::TiledGroup*, void*);

  // List of files in the directory
  fltk::FileBrowser *fileList;
  inline void cb_fileList_i(fltk::FileBrowser*, void*);
  static void cb_fileList(fltk::FileBrowser*, void*);

  // Preview box
  fltk::InvisibleBox *previewBox;
  inline void cb_previewButton_i(fltk::CheckButton*, void*);
  static void cb_previewButton(fltk::CheckButton*, void*);

  // Show "hidden" files
  inline void cb_showHiddenButton_i(fltk::CheckButton*, void*);
  static void cb_showHiddenButton(fltk::CheckButton*, void*);

  // Filename (input section)
  fltk::FileInput *fileName;
  inline void cb_fileName_i(fltk::FileInput*, void*);
  static void cb_fileName(fltk::FileInput*, void*);

  // "OK" button
  fltk::ReturnButton *okButton;
  inline void cb_okButton_i(fltk::ReturnButton*, void*);
  static void cb_okButton(fltk::ReturnButton*, void*);

  // "Cancel" button
  fltk::Button *cancelButton;
  inline void cb_cancelButton_i(fltk::Button*, void*);
  static void cb_cancelButton(fltk::Button*, void*);

  // Window for favorite directories
  fltk::DoubleBufferWindow *favWindow;

  // List of favorite directories.
  fltk::FileBrowser *favList;
  inline void cb_favList_i(fltk::FileBrowser*, void*);
  static void cb_favList(fltk::FileBrowser*, void*);

  // Favorites menu "Up" button
  fltk::Button *favUpButton;
  inline void cb_favUpButton_i(fltk::Button*, void*);
  static void cb_favUpButton(fltk::Button*, void*);

  // Favorites menu "Delete" button
  fltk::Button *favDeleteButton;
  inline void cb_favDeleteButton_i(fltk::Button*, void*);
  static void cb_favDeleteButton(fltk::Button*, void*);

  // Favorites menu "Down" button
  fltk::Button *favDownButton;
  inline void cb_favDownButton_i(fltk::Button*, void*);
  static void cb_favDownButton(fltk::Button*, void*);

  // Favorites menu "Cancel" button
  fltk::Button *favCancelButton;
  inline void cb_favCancelButton_i(fltk::Button*, void*);
  static void cb_favCancelButton(fltk::Button*, void*);

  // Favorites menu "OK" button
  fltk::ReturnButton *favOkButton;
  inline void cb_favOkButton_i(fltk::ReturnButton*, void*);
  static void cb_favOkButton(fltk::ReturnButton*, void*);

  // Sort button
  fltk::PopupMenu *sortButton;
  void cb_sortButton_i(fltk::PopupMenu*, void*);
  static void cb_sortButton(fltk::PopupMenu*, void*);

public:
  enum { SINGLE = 0, MULTI = 1, CREATE = 2, DIRECTORY = 4 };
  FileChooser(const char *d, const char *p, int t, const char *title);
  ~FileChooser();
  inline void add_sort_fn(const char* label, FileSortF func) { sortButton->add(label, (void*)func); }
  inline void callback(void (*cb)(FileChooser *, void *), void *d = 0) { callback_ = cb, data_ = d; }
  inline void color(Color c) { fileList->color(c); }
  inline Color color() const { return fileList->color(); }
  int count();
  void directory(const char *d);
  inline char * directory() const { return (char*)directory_ ; }
  bool exec(Window* p, bool grab);
  void favorites(int e);
  inline int favorites() const { return favorites_showing; }
  void filter(const char *p);
  inline const char * filter() const { return fileList->filter(); }
  inline int filter_value() const { return showChoice->value(); }
  void filter_value(int f);
  inline void hide() { return window->hide(); }
  void icon_size(uchar s) { fileList->icon_size(s); }
  inline uchar icon_size() const { return (uchar)fileList->icon_size(); }
  inline void label(const char *l) { window->label(l); }
  inline const char * label() const { return window->label(); }
  void ok_label(const char *l);
  inline const char * ok_label() const { return okButton->label(); }
  void preview(bool e);
  inline int preview() const { return previewButton->value(); };
  inline void remove_sort_fn(const char* label) { sortButton->remove(label); }
  void rescan();
  void show();
  void show(int x, int y);
  inline int shown() const { return window->shown(); }
  void sort_visible(int e);
  int sort_visible() const;
  inline void textcolor(Color c) { fileList->textcolor(c); }
  inline Color textcolor() const { return fileList->textcolor(); }
  inline void textfont(Font* f) { fileList->textfont(f); }
  inline Font* textfont() const { return fileList->textfont(); }
  inline void textsize(float s) { fileList->textsize(s); }
  inline float textsize() const { return fileList->textsize(); }
  void type(int t);
  inline int type() const { return type_; }
  inline void * user_data() const { return data_; }
  inline void user_data(void *d) { data_ = d; }
  const char *value(int f = 1);
  void value(const char *filename);
  inline int visible() const { return window->visible(); }
  static const char *add_favorites_label;
  static const char *all_files_label;
  static const char *custom_filter_label;
  static const char *existing_file_label;
  static const char *favorites_label;
  static const char *sort_menu_label;
  static const char *filename_label;
  static const char *filesystems_label;
  static const char *manage_favorites_label;
  static const char *new_directory_label;
  static const char *new_directory_tooltip;
  static const char *preview_label;
  static const char *save_label;
  static const char *show_label;
  static FileSortF *sort;

  fltk::Button *newButton;
  fltk::CheckButton *previewButton;
  fltk::CheckButton *showHiddenButton;
};

}

#endif

//
// End of "$Id: FileChooser.h 8507 2011-03-06 10:18:05Z bgbnbigben $".
//
