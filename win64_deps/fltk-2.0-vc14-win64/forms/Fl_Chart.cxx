//
// "$Id: Fl_Chart.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Forms-compatible chart widget for the Fast Light Tool Kit (FLTK).
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

#include <fltk/math.h>
#include <fltk/Fl.h>
#include <fltk/Fl_Chart.h>
#include <fltk/fl_draw.h>
#include <string.h>
#include <stdlib.h>

#define ARCINC	float(M_PI/180)

static void fl_rectbound(int x, int y, int w, int h, Fl_Color c, Fl_Color l) {
  fl_color(l); fl_rect(x, y, w, h);
  fl_color(c); fl_rectf(x+1, y+1, w-2, h-2);
}

/* Widget specific information */

static void draw_barchart(int x,int y,int w,int h,
			  int numb, FL_CHART_ENTRY entries[],
			  float min, float max, int autosize, int maxnumb,
			  Fl_Color textcolor)
/* Draws a bar chart. x,y,w,h is the bounding box, entries the array of
   numb entries and min and max the boundaries. */
{
  float incr;
  int zeroh;
  float lh = fl_height();
  if (max == min) incr = h;
  else incr = h/(max-min);
  if ( (-min*incr) < lh) {
      incr = (h - lh + min*incr)/(max-min);
      zeroh = int(y+h-lh);
  } else {
      zeroh = int(y+h+min * incr + .5f);
  }
  int bwidth = int(w/float(autosize?numb:maxnumb)+.5f);
  /* Draw base line */
  fl_color(textcolor);
  fl_line(x, zeroh, x+w, zeroh);
  if (min == 0 && max == 0) return; /* Nothing else to draw */
  int i;
  /* Draw the bars */
  for (i=0; i<numb; i++) {
      int h = int(entries[i].val*incr+.5f);
      if (h < 0)
	fl_rectbound(x+i*bwidth,zeroh,bwidth+1,-h+1, (Fl_Color)entries[i].col,
		     textcolor);
      else if (h > 0)
	fl_rectbound(x+i*bwidth,zeroh-h,bwidth+1,h+1,(Fl_Color)entries[i].col,
		     textcolor);
  }
  /* Draw the labels */
  fl_color(textcolor);
  for (i=0; i<numb; i++)
      fl_draw(entries[i].str,
	      x+i*bwidth+bwidth/2,zeroh,0,0,
	      FL_ALIGN_TOP);
}

static void draw_horbarchart(int x,int y,int w,int h,
			     int numb, FL_CHART_ENTRY entries[],
			     float min, float max, int autosize, int maxnumb,
			     Fl_Color textcolor)
/* Draws a horizontal bar chart. x,y,w,h is the bounding box, entries the
   array of numb entries and min and max the boundaries. */
{
  int i;
  float lw = 0;		/* Maximal label width */
  /* Compute maximal label width */
  for (i=0; i<numb; i++) {
      float w1 = fl_width(entries[i].str);
      if (w1 > lw) lw = w1;
  }
  if (lw > 0) lw += 4;
  float incr;
  int zeroh;
  if (max == min) incr = w;
  else incr = w/(max-min);
  if ( (-min*incr) < lw) {
      incr = (w - lw + min*incr)/(max-min);
      zeroh = x+int(lw+.5f);
  } else {
      zeroh = int(x-min * incr + .5f);
  }
  int bwidth = int(h/float(autosize?numb:maxnumb)+.5f);
  /* Draw base line */
  fl_color(textcolor);
  fl_line(zeroh, y, zeroh, y+h);
  if (min == 0 && max == 0) return; /* Nothing else to draw */
  /* Draw the bars */
  for (i=0; i<numb; i++) {
      int w = int(entries[i].val*incr+.5f);
      if (w > 0)
	fl_rectbound(zeroh,y+i*bwidth,w+1,bwidth+1, (Fl_Color)entries[i].col,
		     textcolor);
      else if (w < 0)
	fl_rectbound(zeroh+w,y+i*bwidth,-w+1,bwidth+1,(Fl_Color)entries[i].col,
		     textcolor);
  }
  /* Draw the labels */
  fl_color(textcolor);
  for (i=0; i<numb; i++)
      fl_draw(entries[i].str,
	      zeroh-2,y+i*bwidth+bwidth/2,0,0,
	      FL_ALIGN_RIGHT);
}

static void draw_linechart(int type, int x,int y,int w,int h,
			   int numb, FL_CHART_ENTRY entries[],
			   float min, float max, int autosize, int maxnumb,
			   Fl_Color textcolor)
/* Draws a line chart. x,y,w,h is the bounding box, entries the array of
   numb entries and min and max the boundaries. */
{
  int i;
  float lh = fl_height();
  float incr;
  if (max == min) incr = h-2*lh;
  else incr = (h-2*lh)/ (max-min);
  int zeroh = int(y+h-lh+min * incr + .5f);
  float bwidth = w/float(autosize?numb:maxnumb);
  /* Draw the values */
  for (i=0; i<numb; i++) {
      int x0 = x + int((i-.5f)*bwidth+.5f);
      int x1 = x + int((i+.5f)*bwidth+.5f);
      int y0 = i ? zeroh - int(entries[i-1].val*incr+.5f) : 0;
      int y1 = zeroh - int(entries[i].val*incr+.5f);
      if (type == FL_SPIKE_CHART) {
          fl_color((Fl_Color)entries[i].col);
          fl_line(x1, zeroh, x1, y1);
      } else if (type == FL_LINE_CHART && i != 0) {
          fl_color((Fl_Color)entries[i-1].col);
          fl_line(x0,y0,x1,y1);
      } else if (type == FL_FILLED_CHART && i != 0) {
          fl_color((Fl_Color)entries[i-1].col);
          if ((entries[i-1].val>0)!=(entries[i].val>0)) {
            float ttt = entries[i-1].val/(entries[i-1].val-entries[i].val);
            int xt = x + int((i-.5f+ttt)*bwidth+.5f);
            fl_newpath(); fl_vertex(x0, zeroh); fl_vertex(x0, y0);
            fl_vertex(xt, zeroh); fl_closepath(); fl_fill();
            fl_newpath(); fl_vertex(xt, zeroh); fl_vertex(x1, y1);
            fl_vertex(x1, zeroh); fl_closepath(); fl_fill();
          } else {
            fl_newpath(); fl_vertex(x0, zeroh); fl_vertex(x0, y0);
            fl_vertex(x1, y1); fl_vertex(x1, zeroh); fl_closepath(); fl_fill();
          }
          fl_color(textcolor);
          fl_line(x0,y0,x1,y1);
      }
  }
  /* Draw base line */
  fl_color(textcolor);
  fl_line(x,zeroh,x+w,zeroh);
  /* Draw the labels */
  for (i=0; i<numb; i++)
      fl_draw(entries[i].str,
	      x+int((i+.5f)*bwidth+.5f),
	      zeroh - int(entries[i].val*incr+.5f),
	      0, 0,
	      entries[i].val>=0 ? FL_ALIGN_BOTTOM : FL_ALIGN_TOP);
}

static void draw_piechart(int x,int y,int w,int h,
			  int numb, FL_CHART_ENTRY entries[], int special,
			  Fl_Color textcolor)
/* Draws a pie chart. x,y,w,h is the bounding box, entries the array of
   numb entries */
{
  int i;
  float xc,yc,rad;	/* center and radius */
  float tot;		/* sum of values */
  float incr;		/* increment in angle */
  float curang;		/* current angle we are drawing */
  float txc,tyc;	/* temporary center */
  float lh = fl_height();
  /* compute center and radius */
#if 0
  xc = x+w/2.0f; yc = y+h/2.0f;
  rad = h/2.0f - lh;
  if (special) { yc += 0.1f*rad; rad = 0.9f*rad;}
#else // fix suggested by str #780
  double h_denom = (special ? 2.3 : 2.0);
  rad = (h - 2*lh)/h_denom/1.1;
  xc = x+w/2.0; yc = y+h-1.1*rad-lh;
#endif
  /* compute sum of values */
  tot = 0;
  for (i=0; i<numb; i++)
    if (entries[i].val > 0) tot += entries[i].val;
  if (tot == 0) return;
  incr = 360/tot;
  /* Draw the pie */
  curang = 0;
  for (i=0; i<numb; i++)
    if (entries[i].val > 0)
    {
      txc = xc; tyc = yc;
      /* Correct for special pies */
      if (special && i==0)
      {
        txc += 0.3f*rad*cosf(ARCINC*(curang+0.5f*incr*entries[i].val));
        tyc -= 0.3f*rad*sinf(ARCINC*(curang+0.5f*incr*entries[i].val));
      }
      fl_color((Fl_Color)entries[i].col);
      fl_newpath(); fl_vertex(txc,tyc);
      fl_arc(txc,tyc,rad*2,rad*2,curang, curang+incr*entries[i].val);
      fl_closepath(); fl_fill();
      fl_color(textcolor);
      fl_newpath(); fl_vertex(txc,tyc);
      fl_arc(txc,tyc,rad*2,rad*2, curang, curang+incr*entries[i].val);
      fl_closepath(); fl_stroke();
      curang += 0.5f * incr * entries[i].val;
      /* draw the label */
      float xl = txc + 1.1f*rad*cos(ARCINC*curang);
      fl_draw(entries[i].str,
              int(xl+.5f),
              int(tyc - 1.1f*rad*sinf(ARCINC*curang)+.5f),
              0, 0,
              xl<txc ? FL_ALIGN_RIGHT : FL_ALIGN_LEFT);
      curang += 0.5f * incr * entries[i].val;
    }
}

void Fl_Chart::draw() {

    draw_box();
    int xx = 0; int yy = 0; int ww = w(); int hh = h();
    box()->inset(xx, yy, ww, hh); // older versions inset by 9 on all sides
    fl_push_clip(xx, yy, ww, hh);

    ww--; hh--; // adjust for line thickness

    if (min >= max) {
	min = max = 0;
	for (int i=0; i<numb; i++) {
	    if (entries[i].val < min) min = entries[i].val;
	    if (entries[i].val > max) max = entries[i].val;
	}
    }


    fl_font(text_font(),text_size());

    switch (type()) {
    case FL_BAR_CHART:
	draw_barchart(xx,yy,ww,hh, numb, entries, min, max,
			autosize(), maxnumb, text_color());
	break;
    case FL_HORBAR_CHART:
	draw_horbarchart(xx,yy,ww,hh, numb, entries, min, max,
			autosize(), maxnumb, text_color());
	break;
    case FL_PIE_CHART:
	draw_piechart(xx,yy,ww,hh,numb,entries,0, text_color());
	break;
    case FL_SPECIALPIE_CHART:
	draw_piechart(xx,yy,ww,hh,numb,entries,1,text_color());
	break;
    default:
	draw_linechart(type(),xx,yy,ww,hh, numb, entries, min, max,
			autosize(), maxnumb, text_color());
	break;
    }
    draw_inside_label();
    fl_pop_clip();
}

/*------------------------------*/

#define FL_CHART_BOXTYPE	FL_BORDER_BOX
#define FL_CHART_COL1		FL_COL1
#define FL_CHART_LCOL		FL_LCOL
#define FL_CHART_ALIGN		FL_ALIGN_BOTTOM

#include <fltk/Fl_Output.h>

static Fl_Named_Style* style = new Fl_Named_Style("Chart", 0, &style);

Fl_Chart::Fl_Chart(int x,int y,int w,int h,const char *l) :
Fl_Widget(x,y,w,h,l) {
  style(::style);
  clear_flag(FL_ALIGN_MASK);
  set_flag(FL_ALIGN_BOTTOM);
  numb       = 0;
  maxnumb    = 0;
  sizenumb   = FL_CHART_MAX;
  autosize_  = 1;
  min = max  = 0;
  entries    = (FL_CHART_ENTRY *)calloc(sizeof(FL_CHART_ENTRY), FL_CHART_MAX + 1);
}

Fl_Chart::~Fl_Chart() {
  free(entries);
}

void Fl_Chart::clear() {
  numb = 0;
  redraw();
}

void Fl_Chart::add(double val, const char *str, uchar col) {
  /* Allocate more entries if required */
  if (numb >= sizenumb) {
    sizenumb += FL_CHART_MAX;
    entries = (FL_CHART_ENTRY *)realloc(entries, sizeof(FL_CHART_ENTRY) * (sizenumb + 1));
  }
  // Shift entries as needed
  if (numb >= maxnumb && maxnumb > 0) {
    memcpy(entries, entries + 1, sizeof(FL_CHART_ENTRY) * (numb - 1));
    numb --;
  }
  entries[numb].val = float(val);
  entries[numb].col = col;
    if (str) {
	strncpy(entries[numb].str,str,FL_CHART_LABEL_MAX+1);
	entries[numb].str[FL_CHART_LABEL_MAX] = 0;
    } else {
	entries[numb].str[0] = 0;
    }
  numb++;
  redraw();
}

void Fl_Chart::insert(int index, double val, const char *str, uchar col) {
  int i;
  if (index < 1 || index > numb+1) return;
  /* Allocate more entries if required */
  if (numb >= sizenumb) {
    sizenumb += FL_CHART_MAX;
    entries = (FL_CHART_ENTRY *)realloc(entries, sizeof(FL_CHART_ENTRY) * (sizenumb + 1));
  }
  // Shift entries as needed
  for (i=numb; i >= index; i--) entries[i] = entries[i-1];
  if (numb < maxnumb || maxnumb == 0) numb++;
  /* Fill in the new entry */
  entries[index-1].val = float(val);
  entries[index-1].col = col;
  if (str) {
      strncpy(entries[index-1].str,str,FL_CHART_LABEL_MAX+1);
      entries[index-1].str[FL_CHART_LABEL_MAX] = 0;
  } else {
      entries[index-1].str[0] = 0;
  }
  redraw();
}

void Fl_Chart::replace(int index,double val, const char *str, uchar col) {
  if (index < 1 || index > numb) return;
  entries[index-1].val = float(val);
  entries[index-1].col = col;
  if (str) {
      strncpy(entries[index-1].str,str,FL_CHART_LABEL_MAX+1);
      entries[index-1].str[FL_CHART_LABEL_MAX] = 0;
  } else {
      entries[index-1].str[0] = 0;
  }
  redraw();
}

void Fl_Chart::bounds(double min, double max) {
  this->min = min;
  this->max = max;
  redraw();
}

void Fl_Chart::maxsize(int m) {
  int i;
  /* Fill in the new number */
  if (m < 0) return;
  maxnumb = m;
  /* Shift entries if required */
  if (numb > maxnumb) {
      for (i = 0; i<maxnumb; i++)
	  entries[i] = entries[i+numb-maxnumb];
      numb = maxnumb;
      redraw();
  }
}

//
// End of "$Id: Fl_Chart.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
