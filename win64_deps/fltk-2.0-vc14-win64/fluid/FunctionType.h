//
// "$Id$"
//
// function type decl for the Fast Light Tool Kit (FLTK).
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

#ifndef FunctionType_h
#define FunctionType_h

#include "FluidType.h"

class FunctionType : public FluidType {
  const char* return_type;
  const char* attributes;
  bool public_, cdecl_, constructor, havewidgets;
 public:
  FluidType *make();
  void write_code();
  void open();
  int ismain() {return name_ == 0;}
  virtual const char *type_name() const {return "Function";}
  virtual const char *title() {
    return name() ? name() : "main()";
  }
  int is_parent() const {return 1;}
  int is_code_block() const {return 1;}
  void write_properties();
  void read_property(const char *);
  int pixmapID() { return 7; }
};

class CodeType : public FluidType {
 public:
  FluidType *make();
  void write_code();
  void write_static();
  void open();
  virtual const char *type_name() const {return "code";}
  int is_code_block() const {return 0;}
  int pixmapID() { return 8; }
};

class CodeBlockType : public FluidType {
  const char* after;
 public:
  FluidType *make();
  void write_code();
  void open();
  virtual const char *type_name() const {return "codeblock";}
  int is_code_block() const {return 1;}
  int is_parent() const {return 1;}
  void write_properties();
  void read_property(const char *);
  int pixmapID() { return 9; }
};

class DeclType : public FluidType {
  bool public_;
 public:
  FluidType *make();
  void write_code();
  void open();
  virtual const char *type_name() const {return "decl";}
  void write_properties();
  void read_property(const char *);
  int pixmapID() { return 10; }
};

class DeclBlockType : public FluidType {
  const char* after;
 public:
  FluidType *make();
  void write_code();
  void open();
  virtual const char *type_name() const {return "declblock";}
  void write_properties();
  void read_property(const char *);
  int is_parent() const {return 1;}
  int is_decl_block() const {return 1;}
  int pixmapID() { return 11; }
};

class CommentType : public FluidType {
  char title_buf[64];
 public:
  bool in_c_, in_h_;
  FluidType *make();
  void write_code1();
  void write_code2();
  void open();
  virtual const char *type_name() const {return "comment";}
  virtual const char *title(); // string for browser
  void write_properties();
  void read_property(const char *);
  virtual int is_public() const { return 1; }
  virtual int is_comment() const { return 1; }
  int pixmapID() { return 46; }
};

class ClassType : public FluidType {
  const char* subclass_of;
  const char* class_prefix;
  bool public_;
 public:
  // state variables for output:
  char write_public_state; // true when public: has been printed
  ClassType* parent_class; // save class if nested
  //
  FluidType *make();
  void write_code();
  void open();
  virtual const char *type_name() const {return "class";}
  int is_parent() const {return 1;}
  int is_decl_block() const {return 1;}
  int is_class() const {return 1;}
  void write_properties();
  void read_property(const char *);

  // fc: added for FL_API prefixing and others prefix capability
  // class prefix attribute access
  void prefix(const char* p);
  const char*  prefix() const {return class_prefix;}
  int pixmapID() { return 12; }
};

class NamespaceType : public FluidType {
 protected:
  const char * get_full_string() const ;
 public:
  // state variables for output:
  NamespaceType* parent_namespace; // save namespace if nested
  //
  FluidType *make();
  void write_code();
  void write_static(); // for c file "using namespace .." gen.
  void open();
  virtual const char *type_name() const {return "namespace";}
  int is_parent() const {return 1;}
  int is_decl_block() const {return 1;} // namespace can contain namespace(s) | class(es)
  int is_class() const {return 0;}
  void write_properties();
  void read_property(const char *);

  int pixmapID() { return 49; }
};

#endif
