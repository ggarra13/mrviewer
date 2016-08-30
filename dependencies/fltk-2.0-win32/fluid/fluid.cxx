//
// "$Id: fluid.cxx 8574 2011-04-11 07:50:08Z bgbnbigben $"
//
// FLUID main entry for the Fast Light Tool Kit (FLTK).
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

const char *copyright =
"The FLTK user interface designer version 2.0\n"
"Copyright 1998-2006 by Bill Spitzak and others.\n"
"\n"
"This library is free software; you can redistribute it and/or "
"modify it under the terms of the GNU Library General Public "
"License as published by the Free Software Foundation; either "
"version 2 of the License, or (at your option) any later version.\n"
"\n"
"This library is distributed in the hope that it will be useful, "
"but WITHOUT ANY WARRANTY; without even the implied warranty of "
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. "
"See the GNU Library General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU Library General Public "
"License along with this library; if not, write to the Free Software "
"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 "
"USA.\n"
"\n"
"Please report bugs to the following page:\n"
"\n"
"    http://www.fltk.org/str.php";

#include <config.h>
#include <fltk/run.h>
#include <fltk/visual.h>
#include <fltk/events.h>
#include <fltk/damage.h>
#include <fltk/Window.h>
#include <fltk/Box.h>
#include <fltk/Button.h>
#include <fltk/Browser.h>
#include <fltk/MenuBar.h>
#include <fltk/StatusBarGroup.h>
#include <fltk/Input.h>
#include <fltk/Tooltip.h>
#include <fltk/ask.h>
#include <fltk/draw.h>
#include <fltk/file_chooser.h>
#include <fltk/ask.h>
#include <fltk/filename.h>
#include <fltk/FileIcon.h>
#include <fltk/Preferences.h>
#include <fltk/MenuBuild.h>
#include <fltk/string.h>
#include <fltk/HelpDialog.h>
#include <fltk/PackedGroup.h>
#include <fltk/Monitor.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#if defined(_WIN32) && !defined(__CYGWIN__)
# undef _POSIX_
# include <io.h>
# define access(a,b) _access(a,b)
# include <fltk/win32.h>	// for MAX_PATH definition
# include <direct.h>
#else
# include <unistd.h>
# include <sys/stat.h>
#endif

#include "about_panel.h"
#include "alignment_panel.h"
#include "function_panel.h"
#include "template_panel.h"
#include "widget_panel.h"

#include "PrefsData.h"
#include "Fluid_Plugins.h"
#include "FluidType.h"
#include "WidgetType.h"
#include "coding_style.h"
#include "fluid_menus.h"
#include "undo.h"

using namespace fltk;

DECL_MENUCBV2(toggle_sourceview_cb,DoubleBufferWindow);


/////////////////////////////////////////
// Read preferences file 
PrefsData prefs(Preferences::USER, "fltk.org", "fluid2");
int modflag=0;

int  read_alignment_prefs() {
    prefs.get("widget_size", WidgetType::default_size, 14.0f);
    Tooltip::enable(prefs.show_tooltip()? true : false);
    return 0;
}

/////////////////////////////////////////

// File history info...
char	absolute_history[MAX_HISTORY][1024];
char	relative_history[MAX_HISTORY][1024];

void    check_history (const char * fname);
void	load_history();
void	update_history(const char *);
void	set_preferences_window();

////////////////////////////////////////////////////////////////

void nyi(Widget *,void *) {
    message("That's not yet implemented, sorry");
}

static char *filename;
static char* file_directory;

void fix_images_dir() {
  static char* pvalue;
  delete[] pvalue;
  pvalue = 0;
  if (!images_dir) {
    SharedImage::set_root_directory(file_directory);
    return;
  }
  if (!file_directory || *images_dir == '/'
#ifdef _WIN32
      || *images_dir && images_dir[1]==':'
#endif
      ) {
    SharedImage::set_root_directory(images_dir);
    return;
  }
  // append them together so images dir is relative to this dir...
  pvalue = new char[strlen(file_directory)+strlen(images_dir)+2];
  sprintf(pvalue, "%s/%s", file_directory, images_dir);
  SharedImage::set_root_directory(pvalue);
}

void set_filename(const char *c) {
  if (!c || !*c) {
    delete[] filename; filename = 0;
    delete[] file_directory; file_directory = 0;
    SharedImage::set_root_directory(images_dir && *images_dir ? images_dir:0);
    return;
  }
  if (c != filename) {
    delete[] filename;
    filename = newstring(c);
#ifdef _WIN32
    for (char* p = filename; *p; p++)
      if (*p == '\\') *p = '/';
#endif
  }
  delete[] file_directory;
  file_directory = newstring(c);
  const char* a = file_directory;
#ifdef _WIN32
  if (*a && a[1]==':') a+=2;
#endif
  if (*a=='/') a++;
  char* p = (char*)(fltk::filename_name(a));
  if (p > a && *(p-1)=='/') p--;
  *p = 0;
  fix_images_dir();
}

Window *main_window;

char position_window(Window *w, const char *prefsName, int Visible, int X, int Y, int W=0, int H=0 ) {
    Preferences pos(prefs, prefsName);
    if (prevpos_button && prevpos_button->value() ) {
	pos.get("x", X, X);
	pos.get("y", Y, Y);
	if ( W!=0 ) {
	    pos.get("w", W, W);
	    pos.get("h", H, H);
	    w->resize( X, Y, W, H );
	}
	else
	    w->position( X, Y );
    }
    pos.get("visible", Visible, Visible);
    return Visible;
}
void save_position(Window *w, const char *prefsName) {
    Preferences pos(prefs, prefsName);
    pos.set("x", w->x());
    pos.set("y", w->y());
    pos.set("w", w->w());
    pos.set("h", w->h());
    pos.set("visible", (int)(w->shown() && w->visible()));
}

void save_cb(Widget *, void *v) {
    const char *c = filename;
    if (v || !c || !*c) {
	if (!(c=file_chooser("Save to:", "*.f[ld]", c, 0, 0))) return;
	if (v != (void *)2L) set_filename(c);
    }
    if (!write_file(c)) {
	message("Error writing %s: %s", c, strerror(errno));
	return;
    }

    if (v != (void *)2L) {
      modflag = 0;
      Undo::update_saved();
    }
}
void template_browser_cb(Browser* b,void*) {
    if (fltk::event_clicks()) {
      template_panel->hide();
      return;
    }
    fltk::SharedImage *img = (fltk::SharedImage *)template_preview->image();
    if (img) img->remove();
    template_preview->image((fltk::Symbol*)0);
    template_preview->redraw();

    int item = template_browser->value();

    if (item <= 0) template_instance->deactivate();
    else template_instance->activate();

    if (item < 0) {
      template_submit->deactivate();
      template_delete->deactivate();
      return;
    }

    template_submit->activate();

    const char *flfile = (const char *)template_browser->child(item)->label();
    if (!flfile || item<=0) {
      template_delete->deactivate();
      return;
    }
    else
      template_delete->activate();
    template_name->value(template_browser->child(item)->label());


    char pngfile[1024], *ext;

    strlcpy(pngfile, flfile, sizeof(pngfile));
    if ((ext = strrchr(pngfile, '.')) == NULL) return;
    strcpy(ext, ".png");

    img = fltk::SharedImage::get(pngfile);

    if (img) {
      template_preview->image(img);
      template_preview->redraw();
    }
}

// get a filename string from a template name
bool template_filename_from(const char * c, char * filename, size_t size) {
    // Convert template name to filename_with_underscores
  char safename[1024], *safeptr;
  strlcpy(safename, c, sizeof(safename));
  for (safeptr = safename; *safeptr; safeptr ++) {
    if (isspace(*safeptr)) *safeptr = '_';
  }

  // Find the templates directory...
  prefs.getUserdataPath(filename, size);

  strlcat(filename, "templates", size);
#if defined(WIN32) && !defined(__CYGWIN__)
  if (access(filename, 0)) mkdir(filename);
#else
  if (access(filename, 0)) mkdir(filename, 0777);
#endif // WIN32 && !__CYGWIN__

  strlcat(filename, "/", size);
  strlcat(filename, safename, size);

  char *ext = filename + strlen(filename);
  if (ext >= (filename + size - 5)) {
    alert("The template name \"%s\" is too long!", c);
    return false;
  }

  // Save the .fl file...
  strcpy(ext, ".fl");
  return true;
}

void template_delete_cb(fltk::Button *, void *) {
  int item = template_browser->value();
  if (item < 1) return;
  
  char filename[1024];
  const char *name = template_browser->child(item)->label();
  if (!template_filename_from(name, filename, sizeof(filename) )) return;
  
  if (!fltk::choice("Are you sure you want to delete the template \"%s\"?",
                 "Cancel", "Delete", 0, name)) return;
  
  if (unlink(filename)) {
    fltk::alert("Unable to delete template file \"%s\":\n%s", filename, strerror(errno));
    return;
  }
  template_browser->remove(item);
  template_browser->do_callback();
  
}

void save_template_cb(Widget *, void *) {
  // Setup the template panel...
  if (!template_panel) make_template_panel();

  template_clear();
  template_browser->add("New Template");
  template_load();

  template_name->show();
  template_name->value("");

  template_instance->hide();

  template_delete->deactivate();

  template_submit->label("Save");
  template_submit->deactivate();

  template_panel->label("Save Template");

  // Show the panel and wait for the user to do something...
  template_panel->show();
  while (template_panel->visible()) fltk::wait();

  // Get the template name, return if it is empty...
  const char *c = template_name->value();
  if (!c || !*c) return;
  
  char filename[1024];
  // calculate the template filename from it's name
  if(!template_filename_from(c, filename,sizeof(filename))) return;

  if (!access(filename, 0)) {
    if (choice("The template \"%s\" already exists.\n"
                  "Do you want to replace it?", "Cancel",
		  "Replace", NULL, c) == 0) return;
  }

  if (!write_file(filename)) {
    alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

// FIXME : need offscreen capabilities
#if 0
//#if defined(HAVE_LIBPNG) && defined(HAVE_LIBZ)
  // Get the screenshot, if any...
  FluidType *t;

  for (t = FluidType::first; t; t = t->walk(0)) 
      if (t->is_window()) break;  // Find the first window...
  if (!t) return;

  // Grab a screenshot...
  WindowType *wt = (WindowType *)t;
  uchar *pixels;
  int w, h;

  if ((pixels = wt->read_image(w, h)) == NULL) return;

  // Save to a PNG file...
  strcpy(ext, ".png");

  FILE *fp;

  if ((fp = fopen(filename, "wb")) == NULL) {
    delete[] pixels;
    fltk::alert("Error writing %s: %s", filename, strerror(errno));
    return;
  }

  png_structp pptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
  png_infop iptr = png_create_info_struct(pptr);
  png_bytep ptr = (png_bytep)pixels;

  png_init_io(pptr, fp);
  png_set_IHDR(pptr, iptr, w, h, 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_sRGB(pptr, iptr, PNG_sRGB_INTENT_PERCEPTUAL);

  png_write_info(pptr, iptr);

  for (int i = h; i > 0; i --, ptr += w * 3) {
    png_write_row(pptr, ptr);
  }

  png_write_end(pptr, iptr);
  png_destroy_write_struct(&pptr, &iptr);

  fclose(fp);

#  if 0 // The original PPM output code...
  strcpy(ext, ".ppm");
  fp = fopen(filename, "wb");
  fprintf(fp, "P6\n%d %d 255\n", w, h);
  fwrite(pixels, w * h, 3, fp);
  fclose(fp);
#  endif // 0

  delete[] pixels;
#endif // HAVE_LIBPNG && HAVE_LIBZ
}

void revert_cb(Widget *,void *) {
  if (modflag) {
    if (!choice("This user interface has been changed. Really revert?",
                "Cancel", "Revert", NULL)) return;
  }
  Undo::suspend();
  if (!read_file(filename, 0)) {
    Undo::resume();
    message("Can't read %s: %s", filename, strerror(errno));
    return;
  }
  Undo::resume();
  modflag=0;
  Undo::clear();
}

void exit_cb(Widget *,void *) {
    if (modflag)
	switch (choice("Save changes before exiting?", "Yes", "No", "Cancel")) {
      case 2 : /* Cancel */
          return;
      case 0 : /* Yes */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
    
  save_position(main_window,"main_window_pos");

  if (widgetbin_panel) {
    save_position(widgetbin_panel,"widgetbin_pos");
    delete widgetbin_panel;
  }

  if (sourceview_panel) {
    prefs.sv_autorefresh(sv_autorefresh->value());
    prefs.sv_autoposition(sv_autoposition->value());
    prefs.sv_tab(sv_tab->value());
    save_position(sourceview_panel,"sourceview_pos");
    delete sourceview_panel;
  }
    exit(0);
}

void open_cb(Widget *, void *v) {
  if (!v && modflag && !ask("Discard changes?")) return;
  const char *c;
  if (!(c = file_chooser("Open:", "*.f[ld]", filename, 0, 0))) return;
  if (!fltk::filename_exist(c)) {
    message("%s not found", c);
    return;
  }
  Undo::suspend();

  if (!v) set_filename(c);
  if (!read_file(c, v!=0)) {
    message("Can't read %s: %s", c, strerror(errno));
  } else {
    if (!v) modflag = 0;
    else modflag = 1;
  }
  Undo::resume();
  Undo::clear();
}

void open_history_cb(Widget *, void *v) {
  if (modflag) {
    switch (choice("Do you want to save changes to this user\n"
                   "interface before opening another one?", "Cancel",
                   "Save", "Don't Save"))
      {
      case 0 : /* Cancel */
        return;
      case 1 : /* Save */
        save_cb(NULL, NULL);
        if (modflag) return;	// Didn't save!
      }
  }
  char *oldfilename = (char *)v;
  if (!fltk::filename_exist(oldfilename )) {
    message("%s not found", oldfilename );
    check_history(oldfilename);
    return;
  }
  set_filename(oldfilename);
  Undo::suspend(); 
  if (!read_file(filename, 0)) {
    message("Can't read %s: %s", filename, strerror(errno));
  }
  modflag = 0;
  Undo::resume();
  Undo::clear();
}

static char* cutfname(int which = 0) {
  static char name[2][1024];
  static char beenhere = 0;

  if (!beenhere) {
    beenhere = 1;
    prefs.getUserdataPath(name[0], sizeof(name[0]));
    strlcat(name[0], "cut_buffer", sizeof(name[0]));
    prefs.getUserdataPath(name[1], sizeof(name[1]));
    strlcat(name[1], "dup_buffer", sizeof(name[1]));
  }

  return name[which];
}

// Set default widget sizes...
void default_widget_size_cb(RadioButton *b, long size) {
  // Update the "normal" text size of new widgets...
  b->setonly();
  WidgetType::default_size = (float) size;
  prefs.set("widget_size", WidgetType::default_size);
}

// new_cb() : new (possibly template) fulid file creation from File/New menu
void new_cb(Widget *, void *v) {
  // Check if the current file has been modified...
  if (!v && modflag) {
    // Yes, ask the user what to do...
    switch (choice("Do you want to save changes to this user\n"
                      "interface before creating a new one?", "Cancel",
                      "Save", "Don't Save"))
    {
      case 0 : /* Cancel */
          return;
      case 1 : /* Save */
          save_cb(NULL, NULL);
	  if (modflag) return;	// Didn't save!
    }
  }

  // Setup the template panel...
  if (!template_panel) make_template_panel();

  template_clear();
  template_browser->add("Blank");
  template_load();

  template_name->hide();
  template_name->value("");

  template_instance->show();
  template_instance->deactivate();
  template_instance->value("");

  template_submit->label("New");
  template_submit->deactivate();

  template_panel->label("New");

  // Show the panel and wait for the user to do something...
  template_panel->show();
  while (template_panel->visible()) fltk::wait();

  // See if the user chose anything...
  int item = template_browser->value();
  if (item < 0) return;

  // Clear the current data...
  delete_all();
  set_filename(NULL);

  // Load the template, if any...
  const char *tname = (const char *)template_browser->child(item)->user_data();

  if (tname) {
    // Grab the instance name...
    const char *iname = template_instance->value();

    if (iname && *iname) {
      // Copy the template to a temp file, then read it in...
      char line[1024], *ptr, *next;
      FILE *infile, *outfile;

      if ((infile = fopen(tname, "r")) == NULL) {
	alert("Error reading template file \"%s\":\n%s", tname,
        	 strerror(errno));
	modflag=0;
	Undo::clear();
	return;
      }

      if ((outfile = fopen(cutfname(1), "w")) == NULL) {
	alert("Error writing buffer file \"%s\":\n%s", cutfname(1),
        	 strerror(errno));
	fclose(infile);
	modflag = 0;
	Undo::clear();
	return;
      }

      while (fgets(line, sizeof(line), infile)) {
	// Replace @INSTANCE@ with the instance name...
	for (ptr = line; (next = strstr(ptr, "@INSTANCE@")) != NULL; ptr = next + 10) {
	  fwrite(ptr, next - ptr, 1, outfile);
	  fputs(iname, outfile);
	}

	fputs(ptr, outfile);
      }

      fclose(infile);
      fclose(outfile);

      Undo::suspend();
      read_file(cutfname(1), 0);
      unlink(cutfname(1));
      Undo::resume();
    } else {
      // No instance name, so read the template without replacements...
	Undo::suspend();
      read_file(tname, 0);
      Undo::resume();
    }
  }

  modflag =0;
  Undo::clear();
}

/*void new_cb(Widget *, void *v) {
    if (!v && modflag && !ask("Discard changes?")) return;
    const char *c;
    if (!(c = file_chooser("New:", "*.f[ld]", 0, 0, 0))) return;
    delete_all();
    set_filename(c);
    modflag = 0;
}
*/
int compile_only = 0;
int header_file_set = 0;
int code_file_set = 0;
const char* header_file_name = ".h";
const char* code_file_name = ".cxx";

void write_cb(Widget *, void *) {
    if (!filename) {
	save_cb(0,0);
	if (!filename) return;
    }
    char cname[1024];
    char hname[1024];
    if (code_file_name[0] == '.' && isalpha(code_file_name[1])) {
	strlcpy(cname, filename, 1024);
	*filename_ext(cname) = 0;
	strlcat(cname, code_file_name, 1024);
    } else {
	strcpy(cname, code_file_name);
    }
    if (header_file_name[0] == '.' && isalpha(header_file_name[1])) {
	strlcpy(hname, filename, 1024);
	*filename_ext(hname) = 0;
	strlcat(hname, header_file_name, 1024);
    } else {
	strcpy(hname, header_file_name);
    }
    int x = write_code(cname,hname);
    strcat(cname, "/"); strcat(cname,header_file_name);
    if (compile_only) {
	if (!x) {fprintf(stderr,"%s : %s\n",cname,strerror(errno)); exit(1);}
    } else {
	if (!x) {
	    message("Can't write %s: %s", cname, strerror(errno));
	} else {
	    message("Wrote %s", cname, 0);
	}
    }
}

void openwidget_cb(Widget *, void *) {
    if (!FluidType::current) {
	message("Please select a widget");
	return;
    }
    FluidType::current->open();
}

void toggle_overlays(Widget *,void *);

void select_all_cb(Widget *,void *);

void group_cb(Widget *, void *);

void ungroup_cb(Widget *, void *);

extern int pasteoffset;
static int ipasteoffset;

void copy_cb(Widget*, void*) {
    if (!FluidType::current) return;
    Undo::checkpoint();
    ipasteoffset = 10;
    if (!write_file(cutfname(),1)) {
	message("Can't write %s: %s", cutfname(), strerror(errno));
	return;
    }
}

extern void select_only(FluidType *);
void cut_cb(Widget *, void *) {
    if (!FluidType::current) return;
    Undo::checkpoint();
    ipasteoffset = 0;
    FluidType *p = FluidType::current->parent;
    while (p && p->selected()) p = p->parent;
    if (!write_file(cutfname(),1)) {
	message("Can't write %s: %s", cutfname(), strerror(errno));
	return;
    }
    delete_all(1);
    if (p) select_only(p);
}

extern int force_parent;

void paste_cb(Widget*, void*) {
    Undo::checkpoint();
    if (ipasteoffset) force_parent = 1;
    pasteoffset = ipasteoffset;
    if (prefs.gridx()>1) pasteoffset = ((pasteoffset-1)/prefs.gridx()+1)*prefs.gridx();
    if (prefs.gridy()>1) pasteoffset = ((pasteoffset-1)/prefs.gridy()+1)*prefs.gridy();
    if (!read_file(cutfname(), 1)) {
	message("Can't read %s: %s", cutfname(), strerror(errno));
    }
    pasteoffset = 0;
    ipasteoffset += 10;
    force_parent = 0;
}

void earlier_cb(Widget*,void*);
void later_cb(Widget*,void*);
void sort(FluidType *parent); // in WidgetType.cxx

void sort_cb(Widget *,void *) {
    FluidType* f = FluidType::first;
    if(!f) return;
    Undo::checkpoint();
    for (; f; f = f->next_brother) sort(f);
}

void show_preferences_cb(Widget *, void *);

void set_images_dir_cb(Widget *, void *);

//! fluid splashscreen implementation:
//      reuse the about_panel but changes its drawing defaults
Window * splash() {
    if (!about_panel) make_about_panel(copyright);
    Window * o= about_panel;
    o->border(false);
    o->Rectangle::set(fltk::Monitor::find(0,0),o->w(),o->h(),fltk::ALIGN_CENTER);
    o->show();
    about_ok->clear_visible();
    o->flush();
    do {fltk::check(); } while(!o->visible()) ;
    return about_panel;
}

void about_cb(Widget *, void *) {
    if (!about_panel) make_about_panel(copyright);
    Window * o= about_panel;
    //copyright_box->hide();
    //display_group->show();
    o->border(true);
    o->Rectangle::set(fltk::Monitor::find(0,0),o->w(),o->h(),fltk::ALIGN_CENTER);
    about_ok->set_visible();
    o->show();
}

void show_help(const char *name) {
  const char	*docdir;
  char		helpname[1024];
  static HelpDialog *help_dialog = 0;
  
  if (!help_dialog) help_dialog = new HelpDialog();

  if ((docdir = getenv("FLTK_DOCDIR")) == NULL) {
#ifdef __EMX__
    // Doesn't make sense to have a hardcoded fallback
    static char fltk_docdir[1024];

    strlcpy(fltk_docdir, __XOS2RedirRoot("/XFree86/lib/X11/fltk/doc"),
            sizeof(fltk_docdir));

    docdir = fltk_docdir;
#else
# ifdef FLTK_DOCDIR
    docdir = FLTK_DOCDIR;
# else
    docdir = "/usr/local/share/doc/fltk";
# endif
#endif // __EMX__
  }
  snprintf(helpname, sizeof(helpname), "%s/%s", docdir, name);  

  help_dialog->load(helpname);
  help_dialog->show();
}

void help_cb(Widget *, void *) {
  show_help("fluid.html");
}

void manual_cb(Widget *, void *) {
  show_help("index.html");
}

void tt_cb(Widget *w, void *) {
    prefs.show_tooltip(((Button*)w)->value()); 
    Tooltip::enable(prefs.show_tooltip() ? true : false);
}

#include <string.h>
#include <fltk/ask.h>

#include <fltk/StyleSet.h>
StyleSet* fluid_style_set;
StyleSet* style_set;
char* theme;

void set_theme(const char* s) {
#if 0
    if (!s || !*s) {
	if (!theme) return;
	free((void*)theme);
	theme = 0;
    } else {
	if (theme) {
	    if (!strcmp(theme, s)) return;
	    free((void*)theme);
	}
	theme = strdup(s);
    }
    style_set->make_current();
    Theme f = Style::load_theme(theme);
    Style::theme(f);
    // set the scheme...
    Style::reload_theme();
    fluid_style_set->make_current();
    redraw();
    if (!f) alert("Error loading theme \"%s\" (plugin not found?)",
	theme ? theme : "");
#endif
}

void theme_cb(Widget *, void *) {
    const char* s = input("Enter the theme name:", ::theme);
    if (!s) return;
    set_theme(s);
}


void initialize_tab_colors() {
    int c = prefs.tabcolor();
    if (panel_tabs) {
	panel_tabs->child(0)->color((unsigned) c ? prefs.tabcolor1() : fltk::GRAY75);
	panel_tabs->child(1)->color((unsigned) c ? prefs.tabcolor2() : fltk::GRAY75);
	panel_tabs->child(2)->color((unsigned) c ? prefs.tabcolor3() : fltk::GRAY75);
    }
    if (pref_tabs) {
	pref_tabs->child(0)->color((unsigned) c ? prefs.tabcolor1() : fltk::GRAY75);
	pref_tabs->child(1)->color((unsigned) c ? prefs.tabcolor3() : fltk::GRAY75);
	pref_tabs->child(2)->color((unsigned) c ? prefs.tabcolor2() : fltk::GRAY75);
    }
}

void toggle_widgetbin_cb(Widget *o, void * v) {
    if (!widgetbin_panel) {
	make_widgetbin();
	widgetbin_panel->callback(toggle_widgetbin_cb);
	if (!position_window(widgetbin_panel,"widgetbin_pos", 1, 320, 30)) return;
    }
    
    if (widgetbin_panel->visible()) {
	widgetbin_panel->hide();
	iwidget_bin->label("Show Widget &Bin...");
    } else {
	widgetbin_panel->show();
	iwidget_bin->label("Hide Widget &Bin");
    }
}


////////////////////////////////////////////////////////////////
// New fltk2 menu generation
////////////////////////////////////////////////////////////////


#define BROWSERWIDTH 350
#define BROWSERHEIGHT 500
#define WINWIDTH 350
#define MENUHEIGHT 23
#define WINHEIGHT (BROWSERHEIGHT+MENUHEIGHT)


MenuBar* menubar;
Browser* widget_browser;
StatusBarGroup* status_bar;

////////////////////////////////////////////////////////////////
void toggle_statusbar_cb(Widget *,void *) {

    if (status_bar->visible()) {
    status_bar->hide();
    istatusbar->label("Show Status Bar ...");
    prefs.show_statusbar(0);
  } else {
    status_bar->show();
    istatusbar->label("Hide Status Bar ...");
    prefs.show_statusbar(1);
  }
}

////////////////////////////////////////////////////////////////
void toggle_sourceview_cb(DoubleBufferWindow *, void *) {
  if (!sourceview_panel) {
    make_sourceview();
    sourceview_panel->callback((Callback*)toggle_sourceview_cb);
    sv_autorefresh->value(prefs.sv_autorefresh() ? true : false);
    sv_autoposition->value(prefs.sv_autoposition() ? true : false);
    if (prefs.sv_tab()>=0 && prefs.sv_tab()<sv_tab->children()) sv_tab->value(prefs.sv_tab());
    if (!position_window(sourceview_panel,"sourceview_pos", 0, 320, 120, 550, 500)) return;
  }

  if (sourceview_panel->visible()) {
    sourceview_panel->hide();
    isource_view->label("Show Source Code...");
  } else {
    sourceview_panel->show();
    isource_view->label("Hide Source Code...");
    update_sourceview_cb(0,0);
  }
}
////////////////////////////////////////////////////////////////
void toggle_sourceview_b_cb(Button*, void *) {
  toggle_sourceview_cb(0,0);
}
////////////////////////////////////////////////////////////////
void make_main_window() {
    if (!main_window) {
	Widget *o;
	main_window = new Window(WINWIDTH,WINHEIGHT,"fluid2");
	//in_window->size_range(WINWIDTH,100);
	main_window->box(NO_BOX);
	main_window->begin();
	menubar = new MenuBar(0,0,BROWSERWIDTH,MENUHEIGHT);
	menubar->box(FLAT_BOX);
	o = widget_browser = (Browser *) make_widget_browser(0,MENUHEIGHT,BROWSERWIDTH,BROWSERHEIGHT);
	//  o->text_box(FLAT_BOX);
	main_window->resizable(o);
	build_hierarchy(menubar);
	if (prefs.show_tooltip()) itooltip->set_flag(STATE);
	// this is removed because the new ctrl+bindings mess up emacs in
	// the text fields:
	//    menubar->global();
	// create a status bar, only care for h(), other dims are automatically resized
	status_bar = new StatusBarGroup();
	status_bar->child_box(THIN_DOWN_BOX, StatusBarGroup::SBAR_RIGHT);
	status_bar->child_box(FLAT_BOX, StatusBarGroup::SBAR_CENTER);
	
	if (!prefs.show_statusbar()) toggle_statusbar_cb(0, 0);
	main_window->end();
	load_history();
	make_shell_window();
    }
}
////////////////////////////////////////////////////////////////
// Shell command support...
////////////////////////////////////////////////////////////////
#if (!defined(WIN32) || defined(__CYGWIN__)) && !defined(__MWERKS__)
// Support the full piped shell command...
static FILE *shell_pipe = 0;

void
shell_pipe_cb(int, void*) {
  char	line[1024];		// Line from command output...

  if (fgets(line, sizeof(line), shell_pipe) != NULL) {
    // Add the line to the output list...
    shell_run_buffer->append(line);
  } else {
    // End of file; tell the parent...
    fltk::remove_fd(fileno(shell_pipe));

    pclose(shell_pipe);
    shell_pipe = NULL;
    shell_run_buffer->append("... END SHELL COMMAND ...\n");
  }

  shell_run_display->scroll(shell_run_display->total_lines(), 0);
}

////////////////////////////////////////////////////////////////
void do_shell_command(fltk::ReturnButton*, void*) {
    const char	*command;	// Command to run

  shell_window->hide();

  if (shell_pipe) {
    fltk::alert("Previous shell command still running!");
    return;
  }

  if ((command = shell_command_input->value()) == NULL || !*command) {
    fltk::alert("No shell command entered!");
    return;
  }

  if (shell_savefl_button->value()) {
    save_cb(0, 0);
  }

  if (shell_writecode_button->value()) {
    compile_only = 1;
    write_cb(0, 0);
    compile_only = 0;
  }

#if 0 // FIXME intl strings
  if (shell_writemsgs_button->value()) {
    compile_only = 1;
    write_strings_cb(0, 0);
    compile_only = 0;
  }
#endif

  // Show the output window and clear things...
  shell_run_buffer->text("");
  shell_run_buffer->append(command);
  shell_run_buffer->append("\n");
  shell_run_window->label("Shell Command Running...");

  if ((shell_pipe = popen((char *)command, "r")) == NULL) {
    fltk::alert("Unable to run shell command: %s", strerror(errno));
    return;
  }

  shell_run_button->deactivate();
  shell_run_window->show();
  shell_run_window->hotspot(shell_run_display);

  fltk::add_fd(fileno(shell_pipe), shell_pipe_cb);

  while (shell_pipe) fltk::wait();

  shell_run_button->activate();
  shell_run_window->label("Shell Command Complete");
  fltk::beep();

  while (shell_run_window->shown()) fltk::wait();
}
#else
// Just do basic shell command stuff, no status window...
void do_shell_command(fltk::ReturnButton*, void*) {
    const char	*command;	// Command to run
  int		status;		// Status from command...

  shell_window->hide();

  if ((command = shell_command_input->value()) == NULL || !*command) {
    fltk::alert("No shell command entered!");
    return;
  }

  if (shell_savefl_button->value()) {
    save_cb(0, 0);
  }

  if (shell_writecode_button->value()) {
    compile_only = 1;
    write_cb(0, 0);
    compile_only = 0;
  }

/* FIXME : write strings : should it be implemented in fluid 2 as in fltk1 ?
  if (shell_writemsgs_button->value()) {
    compile_only = 1;
    write_strings_cb(0, 0);
    compile_only = 0;
  }
*/
  if ((status = system(command)) != 0) {
    fltk::alert("Shell command returned status %d!", status);
  } else if (completion_button->value()) {
    fltk::message("Shell command completed successfully!");
  }
}
#endif // (!WIN32 || __CYGWIN__) && !__MWERKS__
////////////////////////////////////////////////////////////////
void show_shell_window(Widget*, void*) {
  shell_window->show();
  shell_window->hotspot(shell_command_input);
}
//
// The Source View system offers an immediate preview of the code 
// files that will be generated by FLUID. It also marks the code
// generated for the last selected item in the header and the source
// file.
//
// Can we patent this?  ;-)  - Matt, mm@matthiasm.com
//

//
// Update the header and source code highlighting depending on the
// currently selected object
//
void update_sourceview_position() {
  if (!sourceview_panel || !sourceview_panel->visible()) 
    return;
  if (sv_autoposition->value()==0) 
    return;
  if (sourceview_panel && sourceview_panel->visible() && FluidType::current) {
    int pos0, pos1;
    if (sv_source->visible_r()) {
      pos0 = FluidType::current->code_line;
      pos1 = FluidType::current->code_line_end;
      if (pos0>=0) {
        if (pos1<pos0)
          pos1 = pos0;
        sv_source->buffer()->highlight(pos0, pos1);
        int line = sv_source->buffer()->count_lines(0, pos0);
        sv_source->scroll(line, 0);
      }
    }
    if (sv_header->visible_r()) {
      pos0 = FluidType::current->header_line;
      pos1 = FluidType::current->header_line_end;
      if (pos0>=0) {
        if (pos1<pos0)
          pos1 = pos0;
        sv_header->buffer()->highlight(pos0, pos1);
        int line = sv_header->buffer()->count_lines(0, pos0);
        sv_header->scroll(line, 0);
      }
    }
  }
}

void update_sourceview_position_cb(TabGroup*, void*) {
  update_sourceview_position();
}

static char *sv_source_filename = 0;
static char *sv_header_filename = 0;

//
// Generate a header and source file in a temporary directory and
// load those into the Code Viewer widgets.
//
extern int write_sourceview;

void update_sourceview_cb(Button*, void*) {
  if (!sourceview_panel || !sourceview_panel->visible()) 
    return;
  // generate space for the source and header file filenames
  if (!sv_source_filename) {
    sv_source_filename = (char*)malloc(PATH_MAX);
    prefs.getUserdataPath(sv_source_filename, PATH_MAX);
    strlcat(sv_source_filename, "source_view_tmp.cxx", PATH_MAX);
  }
  if (!sv_header_filename) {
    sv_header_filename = (char*)malloc(PATH_MAX);
    prefs.getUserdataPath(sv_header_filename, PATH_MAX);
    strlcat(sv_header_filename, "source_view_tmp.h", PATH_MAX);
  }

//FIXME
//  strlcpy(i18n_program, fltk::filename_name(sv_source_filename), sizeof(i18n_program));
//  fltk::filename_ext(i18n_program, sizeof(i18n_program), "");
  const char *code_file_name_bak = code_file_name;
  code_file_name = sv_source_filename;
  const char *header_file_name_bak = header_file_name;
  header_file_name = sv_header_filename;

  // generate the code and load the files
  write_sourceview = 1;
  // generate files
  if (write_code(sv_source_filename, sv_header_filename)) 
  {
    // load file into source editor
    int pos = sv_source->top_line();
    sv_source->buffer()->loadfile(sv_source_filename);
    sv_source->scroll(pos, 0);
    // load file into header editor
    pos = sv_header->top_line();
    sv_header->buffer()->loadfile(sv_header_filename);
    sv_header->scroll(pos, 0);
    // update the source code highlighting
    update_sourceview_position();
  }
  write_sourceview = 0;

  code_file_name = code_file_name_bak;
  header_file_name = header_file_name_bak;
}

void update_sourceview_timer(void*)  {
  update_sourceview_cb(0,0);
}

////////////////////////////////////////////////////////////////
// Check that file is valid, remove from history if not
void check_history (const char * fname) {
    int i;
    for (i=0;i<MAX_HISTORY;i++) {
	if (!strcmp(fname,absolute_history[i]) && !fltk::filename_exist(absolute_history[i])) {
	    if (i<MAX_HISTORY-1) {
		for (int p=i;p<MAX_HISTORY-1;p++) {
		    strcpy(absolute_history[p],absolute_history[p+1]);
		    strcpy(relative_history[p],relative_history[p+1]);
		}
		*absolute_history[MAX_HISTORY-1]='\0';
	    }
	    else
		*absolute_history[i]='\0';
	}
    }
    for (i=0;i<MAX_HISTORY; i ++) 
	prefs.set( Preferences::Name("file%d", i), absolute_history[i]);
    for (i=0;i<MAX_HISTORY && *absolute_history[i]; i ++);
    if (i<MAX_HISTORY ) history_item[i]->hide();
}
// Load file history from preferences...
void load_history() {
    int	i;		// Looping var
    int	max_files = prefs.recent_files();
    
    if (max_files > MAX_HISTORY) max_files = MAX_HISTORY;
    
    for (i = 0; i < max_files; i ++) {
	prefs.get( Preferences::Name("file%d", i), absolute_history[i], "", sizeof(absolute_history[i]));
	if (absolute_history[i][0]) {
	    // Make a relative version of the filename for the menu...
	    filename_relative(relative_history[i], sizeof(relative_history[i]),
		absolute_history[i]);
	    history_item[i]->show();
	} else break;
    }
    
    for (; i < MAX_HISTORY; i ++) {
	history_item[i]->hide();
    }
    menubar->redraw();
}

// Update file history from preferences...
void update_history(const char *flname) {
    int	i;		// Looping var
    char	absolute[1024];
    int	max_files=prefs.recent_files();
    
    
    if (max_files > MAX_HISTORY) max_files = MAX_HISTORY;
    
    filename_absolute(absolute, sizeof(absolute), flname);
    
    for (i = 0; i < max_files; i ++)
#if defined(WIN32) || defined(__APPLE__)
	if (!strcasecmp(absolute, absolute_history[i])) break;
#else
	if (!strcmp(absolute, absolute_history[i])) break;
#endif // WIN32 || __APPLE__
	
    if (i == 0) return;
    
    if (i >= max_files) i = max_files - 1;
    
    // Move the other flnames down in the list...
    memmove(absolute_history + 1, absolute_history,
	i * sizeof(absolute_history[0]));
    memmove(relative_history + 1, relative_history,
	i * sizeof(relative_history[0]));
    
    // Put the new file at the top...
    strlcpy(absolute_history[0], absolute, sizeof(absolute_history[0]));
    
    filename_relative(relative_history[0], sizeof(relative_history[0]),
	absolute_history[0]);
    
    // Update the menu items as needed...
    for (i = 0; i < max_files; i ++) {
	prefs.set( Preferences::Name("file%d", i), absolute_history[i]);
	if (!absolute_history[i][0]) break;
	history_item[i]->show();
    }
    
    for (; i < MAX_HISTORY; i ++) {
	prefs.set( Preferences::Name("file%d", i), "");
	history_item[i]->hide();
    }
}
////////////////////////////////////////////////////////////////

static int arg(int argc, char** argv, int& i) {
    if (argv[i][1] == 'c' && !argv[i][2]) {compile_only = 1; i++; return 1;}
    if (argv[i][1] == 'o' && !argv[i][2] && i+1 < argc) {
	code_file_name = argv[i+1];
	code_file_set  = 1;
	i += 2;
	return 2;
    }
    if (argv[i][1] == 'h' && !argv[i][2]) {
	header_file_name = argv[i+1];
	header_file_set  = 1;
	i += 2;
	return 2;
    }
    return 0;
}

#ifndef _WIN32

#include <signal.h>
#ifdef _sigargs
#define SIGARG _sigargs
#else
#ifdef __sigargs
#define SIGARG __sigargs
#else
#define SIGARG int // you may need to fix this for older systems
#endif
#endif

extern "C" {
    static void sigint(SIGARG) {
	signal(SIGINT,sigint);
	exit_cb(0,0);
    }
}
#endif

int main(int argc,char **argv) {
  int i = 1;
  if (!args(argc,argv,i,::arg) || i < argc-1) {
    fprintf(stderr,"usage: %s <switches> name.fl\n"
	    " -c : write .cxx and .h and exit\n"
	    " -o <name> : .cxx output filename, or extension if <name> starts with '.'\n"
	    " -h <name> : .h output filename, or extension if <name> starts with '.'\n"
	    "%s\n", argv[0], help);
    return 1;
  }
  const char *c = argv[i];

  register_images();

  Window * sw = 0;
  double splash_time = fltk::get_time_secs();
  if(!compile_only && prefs.show_splash()) sw = splash();

  fluid_style_set = new StyleSet();
  style_set = new StyleSet();

  read_plugins();
  make_main_window();
  load_coding_style();

  if (c) set_filename(c);
  if (!compile_only) {
    visual(DOUBLE_BUFFER|INDEXED_COLOR);

    FileIcon::load_system_icons();
    main_window->callback(exit_cb);
    main_window->show(argc,argv);
    set_preferences_window();
    position_window(main_window,"main_window_pos", 1, 10, 30, WINWIDTH, WINHEIGHT );
    toggle_widgetbin_cb(0,0);
    toggle_sourceview_cb(0,0);
    if (!c && openlast_button->value() && absolute_history[0][0]) {
      // Open previous file when no file specified...
      open_history_cb(0, absolute_history[0]);
    }
    if(sw) sw->show(); // keep splash screen on top if any
  }
  Undo::suspend();
  if (c && !read_file(c,0)) {
    if (compile_only) {
      Undo::resume();
      fprintf(stderr,"%s : %s\n", c, strerror(errno));
      exit(1);
    }
    message("Can't read %s: %s", c, strerror(errno));
  }
  if(sw) sw->show(); // keep splash screen on top if any
  Undo::resume();
  if (compile_only) {write_cb(0,0); exit(0);}
  modflag = 0;
#ifndef _WIN32
  signal(SIGINT,sigint);
#endif
  if (sw) { // hide splash screen if still visible (no escape have been pressed)
    while(sw->visible() && fltk::get_time_secs()-splash_time<1.0)
      fltk::check();
    sw->hide();
  }
  return run();

}

//
// End of "$Id: fluid.cxx 8574 2011-04-11 07:50:08Z bgbnbigben $".
//
