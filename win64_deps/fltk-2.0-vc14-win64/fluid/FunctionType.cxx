// "$Id: FunctionType.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// C function type code for the Fast Light Tool Kit (FLTK).
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

#include <fltk/run.h>
#include "FunctionType.h"
#include "WidgetType.h"
#include "Fluid_Image.h"
#include <fltk/ask.h>
#include <fltk/Preferences.h>
#include <fltk/Browser.h>
#include <fltk/string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "coding_style.h"

using namespace fltk;

////////////////////////////////////////////////////////////////
// quick check of any C code for legality, returns an error message
static char buffer[128]; // for error messages
extern fltk::Browser *widget_browser;

const char *strip_default_args(const char *name) {
    if(!strchr(name, '='))
	return name;
    static char *buffer = NULL;
    static size_t bufsize = 0;
    if(!bufsize || bufsize < strlen(name) + 1) {
	if(buffer) delete[] buffer;
	size_t allocsize = strlen(name) * 2;	// the *2 is just arbitrary pad
	buffer = new char[allocsize]; 
	if(buffer) bufsize = allocsize;
    }
    if(buffer) {
	memset(buffer, 0, bufsize);
	char *pbuff = buffer;
	const char *pname = name;
	int skipping = 0;
	int inargs = 0;
	while(*pname) {
	    if(!inargs) {
		if(*pname == '(')
		    inargs = 1;
	    }
	    else if(!skipping) {
		if(*pname == '=') {
		    skipping = 1;
		    ++pname;
		    continue;
		}
	    } else if(*pname == ',' || *pname == ')') {
		skipping = 0;
	    }
	    else if (skipping) {
		++pname;
		continue;
	    }
	    *pbuff++ = *pname++;
	}
	*pbuff = '\0';
    }
    return buffer;
}

// check a quoted string ending in either " or ' or >:
const char *_q_check(const char * & c, int type) {
    for (;;) switch (*c++) {
  case '\0':
      sprintf(buffer,"missing %c",type);
      return buffer;
  case '\\':
      if (*c) c++;
      break;
  default:
      if (*(c-1) == type) return 0;
    }
}

// check normal code, match braces and parenthesis:
const char *_c_check(const char * & c, int type) {
    const char *d;
    for (;;) switch (*c++) {
  case 0:
      if (!type) return 0;
      sprintf(buffer, "missing %c", type);
      return buffer;
  case '/':
      // Skip comments as needed...
      if (*c == '/') {
	  while (*c != '\n' && *c) c++;
      } else if (*c == '*') {
	  c++;
	  while ((*c != '*' || c[1] != '/') && *c) c++;
	  if (*c == '*') c+=2;
	  else {
	      return "missing '*/'";
	  }
      }
      break;
  case '#':
      // treat cpp directives as a comment:
      while (*c != '\n' && *c) c++;
      break;
  case '{':
      if (type==')') goto UNEXPECTED;
      d = _c_check(c,'}');
      if (d) return d;
      break;
  case '(':
      d = _c_check(c,')');
      if (d) return d;
      break;
  case '\"':
      d = _q_check(c,'\"');
      if (d) return d;
      break;
  case '\'':
      d = _q_check(c,'\'');
      if (d) return d;
      break;
  case '}':
  case ')':
UNEXPECTED:
      if (type == *(c-1)) return 0;
      sprintf(buffer, "unexpected %c", *(c-1));
      return buffer;
    }
}

const char *c_check(const char *c, int type) {
    return _c_check(c,type);
}

////////////////////////////////////////////////////////////////

FluidType *FunctionType::make() {
    FluidType *p = FluidType::current;
    while (p && !p->is_decl_block()) p = p->parent;
    FunctionType *o = new FunctionType();
    o->name("make_window()");
    o->attributes = 0;
    o->return_type = 0;
    o->add(p);
    o->factory = this;
    o->public_ = true;
    o->cdecl_ = false;
    return o;
}

void FunctionType::write_properties() {
    FluidType::write_properties();
    if (!public_) write_string("private");
    if (cdecl_) write_string("C");
    if (return_type) {
	write_string("return_type");
	write_word(return_type);
    }
    if (attributes) {
	write_string("attributes");
	write_word(attributes);
    }
}

void FunctionType::read_property(const char *c) {
    if (!strcmp(c,"private")) {
	public_ = 0;
    } else if (!strcmp(c,"C")) {
	cdecl_ = true;
    } else if (!strcmp(c,"return_type")) {
	storestring(read_word(),return_type);
    } else if (!strcmp(c,"attributes")) {
	storestring(read_word(),attributes);
    } else {
	FluidType::read_property(c);
    }
}

#include "function_panel.h"
#include <fltk/ask.h>

static void ok_callback(fltk::Widget* w, void*) {
  w->window()->make_exec_return(true);
}

static void cancel_callback(fltk::Widget* w, void*) {
  w->window()->make_exec_return(false);
}

void FunctionType::open() {
    if (!function_panel) {
	make_function_panel();
	f_panel_ok->callback(ok_callback);
	f_panel_cancel->callback(cancel_callback);
    }
    f_return_type_input->static_value(return_type);
    f_attributes_input->static_value(attributes);
    f_name_input->static_value(name());
    f_public_button->value(public_);
    f_c_button->value(cdecl_);
    const char* message = 0;
    for (;;) {
	if (message) fltk::alert(message);
	if (!function_panel->exec()) break;
	const char* c = f_name_input->value();
	while (isspace(*c)) c++;
	message = c_check(c); if (message) continue;
	const char *d = c;
	for (; *d != '('; d++) if (isspace(*d) || !*d) break;
	if (*c && *d != '(') {
	    message = "Name must contain argument list in parenthesis."; continue;
	}
	c = f_return_type_input->value();
	message = c_check(c); if (message) continue;
	name(f_name_input->value());
	storestring(c, return_type);
	storestring(f_attributes_input->value(), attributes);
	public_ = f_public_button->value();
	cdecl_ = f_c_button->value();
	break;
    }
    delete function_panel;
    function_panel = NULL;
}

WidgetType* last_group;

void FunctionType::write_code() {
    const char* rtype = return_type;
    constructor=0;
    havewidgets = 0;
    FluidType *child;
    char attr[256];
    if (attributes) {
      strlcpy(attr, attributes, sizeof(attr));
      strlcat(attr, " ", sizeof(attr));
    } else
	attr[0] = 0;
    for (child = first_child; child; child = child->next_brother)
	if (child->is_widget()) {
	    havewidgets = 1;
	    last_group = (WidgetType*)child;
	}
	write_c("\n");
	if (ismain()) {
	    write_c("#include <fltk/run.h>\n\n");
	    write_c("int main%s(int argc, char **argv)%s\n",
		gno_space_parens ? "" : " ", get_opening_brace(1));
	    // write_c("int main(int argc, char **argv) {\n");
	    if (havewidgets)
		rtype = last_group->subclass();
	    else 
		rtype = "void";
	} else {
	    const char* star = "";
	    // from matt: let the user type "static " at the start of type
	    // in order to declare a static method;
	    int is_static = 0;
	    int is_virtual = 0;
	    if (rtype) {
		if (!strcmp(rtype,"static")) {is_static = 1; rtype = 0;}
		else if (!strncmp(rtype, "static ",7)) {is_static = 1; rtype += 7;}
		if (!strcmp(rtype, "virtual")) {is_virtual = 1; rtype = 0;}
		else if (!strncmp(rtype, "virtual ",8)) {is_virtual = 1; rtype += 8;}
	    }
	    if (!rtype) {
		if (havewidgets) {
		    rtype = last_group->subclass();
		    star = "*";
		} else rtype = "void";
	    }
	    
	    const char* k = member_of(0);
	    if (k) {
		write_public(public_);
		if (name()[0] == '~')
		    constructor = 1;
		else {
		    size_t n = strlen(k);
		    if (!strncmp(name(), k, n) && name()[n] == '(') constructor = 1;
		}
		write_h(get_indent_string(1));
		if (is_static) write_h("static ");
		if (is_virtual) write_h("virtual ");
		if (!constructor) {
		    write_h("%s%s%s ", attr, rtype, star);
		    write_c("%s%s ", rtype, star);
		}
		
		// if this is a subclass, only write_h() the part before the ':'
		char s[1024], *sptr = s;
		char *nptr = (char *)name();
		
		while (*nptr) {
		    if (*nptr == ':') {
			if (nptr[1] != ':') break;
			// Copy extra ":" for "class::member"...
			*sptr++ = *nptr++;
		    }	  
		    *sptr++ = *nptr++;
		}
		*sptr = '\0';
		
		if(constructor)	// already wrote this for constructors.
		    write_h("%s", attr);
		write_h("%s;\n", s);
		write_c("%s::%s%s", k, strip_default_args(name()), get_opening_brace(1));
	    } else {
		if (public_) {
		    if (cdecl_)
			write_h("extern \"C\" { %s%s%s %s; }\n", attr, rtype, star, name());
		    else
			write_h("%s%s%s %s;\n", attr, rtype, star, name());
		}
		else write_c("static ");
		write_c("%s%s %s%s", rtype, star, name(), get_opening_brace(1));
	    }
	}
	indentation += 2;
	if(havewidgets) 
	    write_c("%s%s* w;\n", indent(), last_group->subclass());
	
	for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
	
	if (ismain()) {
	    if (havewidgets) write_c("%sw->show(argc, argv);\n", get_indent_string(1));
	    write_c("%sreturn %s%sfltk::run()%s;\n", get_indent_string(1),
		gno_space_parens ? "" : " ",
		galways_return_parens ? "(" : "", galways_return_parens ? ")" : "");
	} else if (havewidgets && !constructor && !return_type)
	    write_c("%sreturn %s%sw%s;\n", get_indent_string(1), 
	    gno_space_parens ? "" : " ",
	    galways_return_parens ? "(" : "", galways_return_parens ? ")" : "");
	write_c("}\n");
	indentation -= 2;
	if (indentation < 0) indentation = 0;
}

////////////////////////////////////////////////////////////////
FluidType *CodeType::make() {
    FluidType *p = FluidType::current;
    while (p && !p->is_code_block()) p = p->parent;
    if (!p) {
	fltk::message("Please select a function");
	return 0;
    }
    CodeType *o = new CodeType();
    o->name("printf(\"Hello, World!\\n\");");
    o->add(p);
    o->factory = this;
    return o;
}

void CodeType::open() {
    if (!code_panel) {
	make_code_panel();
	code_panel_ok->callback(ok_callback);
	code_panel_cancel->callback(cancel_callback);
    }
    code_input->text(name());
    const char* message = 0;
    for (;;) { // repeat as long as there are errors
	if (message) fltk::alert(message);
	if (!code_panel->exec()) break;
	const char*c = code_input->text();
	message = c_check(c); if (message) continue;
	name(c);
	break;
    }
    code_panel->hide();
    delete code_panel;
    code_panel = NULL;
}


void CodeType::write_code() {
    const char* c = name();
    if (!c) return;
    //write_c("%s%s\n", indent(), c);
    write_code_block((char *)c);
    for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
}

void CodeType::write_static() {
    const char* c = name();
    if (!c) return;
    //write_c("%s%s\n", indent(), c);
    write_includes_from_code((char *)c);
}

////////////////////////////////////////////////////////////////
FluidType *CodeBlockType::make() {
    FluidType *p = FluidType::current;
    while (p && !p->is_code_block()) p = p->parent;
    if (!p) {
	fltk::message("Please select a function");
	return 0;
    }
    CodeBlockType *o = new CodeBlockType();
    o->name("if (test())");
    o->after = 0;
    o->add(p);
    o->factory = this;
    return o;
}

void CodeBlockType::write_properties() {
    FluidType::write_properties();
    if (after) {
	write_string("after");
	write_word(after);
    }
}

void CodeBlockType::read_property(const char *c) {
    if (!strcmp(c,"after")) {
	storestring(read_word(),after);
    } else {
	FluidType::read_property(c);
    }
}

void CodeBlockType::open() {
    if (!codeblock_panel) {
	make_codeblock_panel();
	codeblock_panel_ok->callback(ok_callback);
	codeblock_panel_cancel->callback(cancel_callback);
    }
    code_before_input->static_value(name());
    code_after_input->static_value(after);
    const char* message = 0;
    for (;;) { // repeat as long as there are errors
	if (message) fltk::alert(message);
	if (!codeblock_panel->exec()) break;
	const char*c = code_before_input->value();
	message = c_check(c); if (message) continue;
	name(c);
	c = code_after_input->value();
	message = c_check(c); if (message) continue;
	storestring(c, after);
	break;
    }
    codeblock_panel->hide();
    delete codeblock_panel;
    codeblock_panel = NULL;
}

void CodeBlockType::write_code() {
    const char* c = name();
    write_c("%s%s%s", indent(), c ? c : "", get_opening_brace(0));
    indentation += 2;
    for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
    indentation -= 2;
    if (after) write_c("%s} %s\n", indent(), after);
    else write_c("%s}\n", indent());
}

////////////////////////////////////////////////////////////////
FluidType *DeclType::make() {
    FluidType *p = FluidType::current;
    while (p && !p->is_decl_block()) p = p->parent;
    DeclType *o = new DeclType();
    o->public_ = 0;
    o->name("int x;");
    o->add(p);
    o->factory = this;
    return o;
}

void DeclType::write_properties() {
    FluidType::write_properties();
    if (public_) write_string("public");
}

void DeclType::read_property(const char *c) {
    if (!strcmp(c,"public")) {
	public_ = 1;
    } else {
	FluidType::read_property(c);
    }
}

void DeclType::open() {
    if (!decl_panel) {
	make_decl_panel();
	decl_panel_ok->callback(ok_callback);
	decl_panel_cancel->callback(cancel_callback);
    }
    decl_input->static_value(name());
    decl_public_button->value(public_);
    const char* message = 0;
    for (;;) { // repeat as long as there are errors
	if (message) fltk::alert(message);
	if (!decl_panel->exec()) break;
	const char*c = decl_input->value();
	while (isspace(*c)) c++;
	message = c_check(c&&c[0]=='#' ? c+1 : c);
	if (message) continue;
	name(c);
	public_ = decl_public_button->value();
	break;
    }
    decl_panel->hide();
    delete decl_panel;
    decl_panel = NULL;
}


void DeclType::write_code() {
    const char* c = name();
    if (!c) return;
    // handle putting #include or extern or typedef into decl:
    if ((!isalpha(*c) && *c != '~')
	|| (!strncmp(c,"extern",6) && isspace(c[6]))
	|| (!strncmp(c,"class",5) && isspace(c[5]))
	|| (!strncmp(c,"typedef",7) && isspace(c[7]))
	//    || (!strncmp(c,"struct",6) && isspace(c[6]))
	) {
	if (public_) {
	    char * s = strdup(c);
	    char * p = (char*) strrchr(c,'=');
	    if (p) {*p++=';';*p='\0';}
	    write_h("%s\n", s);
	    free(s);
	}
	else
	    write_c("%s\n", c);
	return;
    }
    // lose all trailing semicolons so I can add one:
    const char* e = c+strlen(c);
    while (e>c && e[-1]==';') e--;
    if (member_of()) {
	write_public(public_);
	write_h("%s%.*s;\n", get_indent_string(1), e-c, c);
    } else {
	if (public_) {
	    char * s = strdup(c);
	    char * p = (char*) strrchr(s,'=');
	    if (p) {*p++=';';*p='\0';}
	    //write_h("extern %.*s;\n", e-c, c);
	    write_h("extern %s\n", s);
	    free(s);
	    write_c("%.*s;\n", e-c, c);
	} else {
	    write_c("static %.*s;\n", e-c, c);
	}
    }
    
    for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
}

////////////////////////////////////////////////////////////////

FluidType *DeclBlockType::make() {
    FluidType *p = FluidType::current;
    while (p && !p->is_decl_block()) p = p->parent;
    DeclBlockType *o = new DeclBlockType();
    o->name("#if 1");
    o->after = strdup("#endif");
    o->add(p);
    o->factory = this;
    return o;
}

void DeclBlockType::write_properties() {
    FluidType::write_properties();
    write_string("after");
    write_word(after);
}

void DeclBlockType::read_property(const char *c) {
    if (!strcmp(c,"after")) {
	storestring(read_word(),after);
    } else {
	FluidType::read_property(c);
    }
}

void DeclBlockType::open() {
    if (!declblock_panel) {
	make_declblock_panel();
	declblock_panel_ok->callback(ok_callback);
	declblock_panel_cancel->callback(cancel_callback);
    }
    decl_before_input->static_value(name());
    decl_after_input->static_value(after);
    const char* message = 0;
    for (;;) { // repeat as long as there are errors
	if (message) fltk::alert(message);
	if (!declblock_panel->exec()) break;
	const char*c = decl_before_input->value();
	while (isspace(*c)) c++;
	message = c_check(c&&c[0]=='#' ? c+1 : c);
	if (message) continue;
	name(c);
	c = decl_after_input->value();
	while (isspace(*c)) c++;
	message = c_check(c&&c[0]=='#' ? c+1 : c);
	if (message) continue;
	storestring(c,after);
	break;
    }
    declblock_panel->hide();
    delete declblock_panel;
    declblock_panel = NULL;
}


void DeclBlockType::write_code() {
    const char* c = name();
    if (c) write_c("%s\n", c);
    for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
    if (after) write_c("%s\n", after);
}

////////////////////////////////////////////////////////////////

static CommentType * current_comment=0;

FluidType *CommentType::make() {
    FluidType *p = FluidType::current;
    while (p && !p->is_code_block()) p = p->parent;
    CommentType *o = new CommentType();
    o->in_c_ = 1;
    o->in_h_ = 1;
    o->name("my comment");
    o->add(p);
    o->factory = this;
    o->title_buf[0] = 0;
    current_comment = o;
    return o;
}

void CommentType::write_properties() {
    FluidType::write_properties();
    if (in_c_) write_string("in_source"); else write_string("not_in_source"); 
    if (in_h_) write_string("in_header"); else write_string("not_in_header");
}

void CommentType::read_property(const char *c) {
    if (!strcmp(c,"in_source")) {
	in_c_ = 1;
    } else if (!strcmp(c,"not_in_source")) {
	in_c_ = 0;
    } else if (!strcmp(c,"in_header")) {
	in_h_ = 1;
    } else if (!strcmp(c,"not_in_header")) {
	in_h_ = 0;
    } else {
	FluidType::read_property(c);
    }
}

#include "comments.h"

static void load_comments_preset(fltk::Preferences &menu) 
{
  static const char * const predefined_comment[] = {
    "GNU Public License/GPL Header",  "GNU Public License/GPL Footer",
    "GNU Public License/LGPL Header", "GNU Public License/LGPL Footer",
    "FLTK/Header", "FLTK/Footer" };
  menu.set("n", 6);
  fltk::Preferences db(fltk::Preferences::USER, "fltk.org", "fluid_comments");
  for (int i=0; i<6; i++) {
    menu.set(fltk::Preferences::Name(i), predefined_comment[i]);
    db.set(predefined_comment[i], comment_text[i]);
  }
}


static void comment_predefined_cb(fltk::Widget * w, void * data) {
  // TODO : handle predefined  comments menus cb
}


static void comment_ok_cb(fltk::Widget * w, void * data) {
  // TODO : handle predefined  comments menus cb
  const char * c = comment_input->value();
  current_comment->name(c);
  int mod = 0;
  if (current_comment->in_c_ != comment_in_source->value()) {
    current_comment->in_c_ = comment_in_source->value();
    mod = 1;
  }
  if (current_comment->in_h_ != comment_in_header->value()) {
    current_comment->in_h_ = comment_in_header->value();
    mod = 1;
  }
  if (mod) modflag=1;
  if (modflag  && widget_browser) {
    widget_browser->relayout();
    refresh_browser_views();
  }
  comment_panel->hide();
}

void CommentType::open() {
#if 1
    if (!comment_panel) make_comment_panel();
    const char *text = name();
    {
	int i=0, n=0;
	fltk::Preferences menu(fltk::Preferences::USER, "fltk.org", "fluid_comments_menu");
	comment_predefined->clear();
	comment_predefined->add("_Edit/Add current comment...");
	comment_predefined->add("_Edit/Remove last selection...");
	menu.get("n", n, -1);
	if (n==-1) load_comments_preset(menu);
	menu.get("n", n, 0);
	for (i=0;i<n;i++) {
	    char *text;
	    menu.get(fltk::Preferences::Name(i), text, "");
	    comment_predefined->add(text);
	    free(text);
	}
	comment_predefined->callback (comment_predefined_cb);
    }
    comment_input->value( text ? text : "" );
    comment_in_source->value(in_c_ ? true : false);
    comment_in_header->value(in_h_ ? true : false);
    comment_panel_ok->callback(comment_ok_cb);
    char itempath[256]; itempath[0] = 0;
    comment_panel->exec(0,false);
 
    title_buf[0] = 0;
    comment_panel->hide();
#endif
}

const char * CommentType::title() {
    const char* n = name(); 
    if (!n || !*n) return type_name();

    const char *s = n;
    char *d = title_buf;
	int i = 50;
	while (--i > 0) {
	    char n = *s++;
	    if (n==0) break;
	    if (n=='\r') { *d++ = '\\'; *d++ = 'r'; i--; }
	    else if (n=='\n') { *d++ = '\\'; *d++ = 'n'; i--; }
	    else if (n<32) { *d++ = '^'; *d++ = 'A'+n; i--; }
	    else *d++ = n;
	}
	if (i<=0) { *d++ = '.'; *d++ = '.'; *d++ = '.'; }
	*d++ = 0;
    return title_buf;
}


void CommentType::write_code1() {
    const char* c = name();
    if (!c) return;
    if (!in_c_ && !in_h_) return;
    // find out if there is already a valid comment:
    const char *s = c;
    while (isspace(*s)) s++;
    // if this seems to be a C style comment, copy the block as is
    // (it's up to the user to correctly close the comment)
    if (s[0]=='/' && s[1]=='*') {
	if (in_h_) write_h("%s\n", c);
	if (in_c_) write_c("%s\n", c);
	return;
    }
    // copy the comment line by line, add the double slash if needed
    char *txt = strdup(c);
    char *b = txt, *e = txt;
    for (;;) {
	// find the end of the line and set it to NUL
	while (*e && *e!='\n') e++;
	char eol = *e;
	*e = 0;
	// check if there is a C++ style comment at the beginning of the line
	char *s = b;
	while (isspace(*s)) s++;
	if (s!=e && ( s[0]!='/' || s[1]!='/') ) {
	    // if no comment marker was found, we add one ourselves
	    if (in_h_) write_h("// ");
	    if (in_c_) write_c("// ");
	}
	// now copy the rest of the line
	if (in_h_) write_h("%s\n", b);
	if (in_c_) write_c("%s\n", b);
	if (eol==0) break;
	*e++ = eol;
	b = e;
    }
}

void CommentType::write_code2() {}

////////////////////////////////////////////////////////////////

// Return the class that this is a member of, or null if this is not
// a member of a class. If need_nest is true then a fully-qualified
// name (ie foo::bar::baz) of nested classes is returned, you need this
// if you actually want to print the class.
const char* FluidType::member_of(bool need_nest) const {
    FluidType* p = parent;
    while (p) {
	if (p->is_class()) {
	    if (!need_nest) return p->name();
	    // see if we are nested in another class, we must fully-qualify name:
	    // this is lame but works...
	    const char* q = p->member_of(true);
	    if (!q) return p->name();
	    static char buffer[256];
	    if (q != buffer) strcpy(buffer, q);
	    strcat(buffer, "::");
	    strcat(buffer, p->name());
	    return buffer;
	}
	p = p->parent;
    }
    return 0;
}

void ClassType::prefix(const char*p) {
    free((void*) class_prefix);
    class_prefix=strdup(p ? p : "" );
}

FluidType *ClassType::make() {
    FluidType *p = FluidType::current;
    while (p && !p->is_decl_block()) p = p->parent;
    ClassType *o = new ClassType();
    o->name("UserInterface");
    o->class_prefix = 0;
    o->subclass_of = 0;
    o->public_ = true;
    o->add(p);
    o->factory = this;
    return o;
}

void ClassType::write_properties() {
    FluidType::write_properties();
    if (subclass_of) {
	write_string(":");
	write_word(subclass_of);
    }
    if (!public_) write_string("private");
}

void ClassType::read_property(const char *c) {
    if (!strcmp(c,"private")) {
	public_ = false;
    } else if (!strcmp(c,":")) {
	storestring(read_word(), subclass_of);
    } else {
	FluidType::read_property(c);
    }
}

void ClassType::open() {
    if (!class_panel) {
	make_class_panel();
	c_panel_ok->callback(ok_callback);
	c_panel_cancel->callback(cancel_callback);
    }
    char fullname[1024]="";
    if (prefix() && strlen(prefix())) 
	sprintf(fullname,"%s %s",prefix(),name());
    else 
	strcpy(fullname, name());
    c_name_input->static_value(fullname);
    
    c_subclass_input->static_value(subclass_of);
    c_public_button->value(public_);
    const char* message = 0;
    char *na=0,*pr=0,*p=0; // name and prefix substrings
    
    for (;;) { // repeat as long as there are errors
	if (message) fltk::alert(message);
	if (!class_panel->exec()) break;
	const char*c = c_name_input->value();
	char *s = strdup(c);
	size_t len = strlen(s);
	if (!*s) goto OOPS;
	p = (char*) (s+len-1);
	while (p>=s && isspace(*p)) *(p--)='\0';
	if (p<s) goto OOPS;
	while (p>=s && is_id(*p)) p--;
	if ( (p<s && !is_id(*(p+1))) || !*(p+1) ) {
OOPS: message = "class name must be C++ identifier";
      free((void*)s);
      continue;
	}
	na=p+1; // now we have the name
	if(p>s) *p--='\0';
	while (p>=s && isspace(*p)) *(p--)='\0';
	while (p>=s && is_id(*p))   p--;
	if (p<s)                    p++;
	if (is_id(*p) && p<na)      pr=p; // prefix detected
	c = c_subclass_input->value();
	message = c_check(c); 
	if (message) { free((void*)s);continue;}
	name(na);
	prefix(pr);
	free((void*)s);
	storestring(c, subclass_of);
	if (public_ != c_public_button->value()) {
	    public_ = c_public_button->value();
	}
	break;
    }
    class_panel->hide();
    delete class_panel;
    class_panel = NULL;
}

static ClassType *current_class;
extern int varused_test;

void write_public(int state) {
    if (!current_class || varused_test) return;
    if (current_class->write_public_state == state) return;
    current_class->write_public_state = state;
    write_h(state ? "public:\n" : "private:\n");
}

void ClassType::write_code() {
    parent_class = current_class;
    current_class = this;
    write_public_state = 0;
    if (prefix() && strlen(prefix())) {
	if (strcmp(prefix(),"FL_EXPORT")==0) // convert it to FL_API
	    prefix("FL_API");
	write_h("\nclass %s %s ", prefix(), name());
    }
    else
	write_h("\nclass %s ", name());
    if (subclass_of) write_h(": %s ", subclass_of);
    write_h("%s", get_opening_brace(1));
    for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
    write_h("};\n");
    current_class = parent_class;
}

////////////////////////////////////////////////////////////////
// Return the class that this is a member of, or null if this is not
// a member of a class. If need_nest is true then a fully-qualified
// name (ie foo::bar::baz) of nested classes is returned, you need this
// if you actually want to print the class.
FluidType *NamespaceType::make() {
    FluidType *p = FluidType::current;
    while (p && (!p->is_decl_block() || p->is_class())) p = p->parent;
    NamespaceType *o = new NamespaceType();
    o->parent_namespace = 0;
    o->name("fltk");
    o->add(p);
    o->factory = this;
    return o;
}

void NamespaceType::write_properties() {
    FluidType::write_properties();
    //TODO
}

// read declaration property when loading the file
void NamespaceType::read_property(const char *c) {
    FluidType::read_property(c);
}

void NamespaceType::open() {
    if (!namespace_panel) {
	make_namespace_panel();
	namespace_panel_ok->callback(ok_callback);
	namespace_panel_cancel->callback(cancel_callback);
    }
    namespace_input->static_value(name());
    
    // scan the input field for namespace string and check it
    const char* message = 0;
    for (;;) { // repeat as long as there are errors
	if (message) fltk::alert(message);
	if (!namespace_panel->exec()) break;
	const char*c = namespace_input->value();
	while (isspace(*c)) c++;
	message = c_check(c);
	if (message) continue;
	name(c && *c ? c : "fltk");
	break;
    }
    
    namespace_panel->hide();
    delete namespace_panel;
    namespace_panel = NULL;
}


static NamespaceType *current_namespace;
extern int varused_test;

// get the full path from root to most imbricated namespace
const char * NamespaceType::get_full_string() const {
    static char sFull[256];
    const int cMaxImbNs=16;
    const char *list[cMaxImbNs];
    int nlist=0,i;
    
    if (name()) {
	const NamespaceType * parent=this;
	do {
	    if (nlist<cMaxImbNs) list[nlist++]=parent->name();
	    parent=parent->parent_namespace;
	} while (parent);
	
	strlcpy(sFull, list[nlist-1], sizeof(sFull));
	for (i=nlist-2; i>=0;i--) {
	    if (i<nlist-1) strlcat(sFull, "::", sizeof(sFull));
	    strlcat(sFull, list[i], sizeof(sFull));
	}
    }
    return sFull;
}

void NamespaceType::write_static() {
    write_c("using namespace %s;\n",get_full_string());
}
void NamespaceType::write_code() {
    parent_namespace = current_namespace;
    current_namespace = this;
    
    write_h("\nnamespace %s ", name());
    write_h("%s", get_opening_brace(1));
    for (FluidType* q = first_child; q; q = q->next_brother) q->write_code();
    write_h("}\n");
    current_namespace = parent_namespace;
}

//
// End of "$Id: FunctionType.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
