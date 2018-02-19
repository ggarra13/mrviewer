/* FLQUBIX

   VERSION:  2.0
   DATE:     20.10.2004
   USING:    FLTK 2.0
   PLATFORM: WINDOWS

   A game of four-in-a-row in 3D

   Adapted for FLTK, (http://www.fltk.org) September 2000, Frank
   Siegert <frank@this.net> More of my workings is available on
   http://www.this.net/~frank

   This code may be used freely as long as it is given away for free
   
   It is hereby placed under the GPL (LGPL if you remove the random
   generator embedded within)
   
   Please do not judge my professional work by the quality of this
   hack, this puppy was written just for fun in about three hours out
   of the DisplayPostscript code from NeXTSTEP. By doing this the code
   comes back from a long journey - as the original author of FLTK
   Bill Spitzak has released it in his famous 'hackkit' for NeXTSTEP
   aeons ago (1991). The code is not written for clarity but *hey* it
   works and now I do not need to boot my trusty NeXT just to
   play. :-)

   Further adapted by Bill Spitzak for fltk2.0 10/20/2004

*/

/* Original comments:

   qubix.c generated from qubic.psw
   by unix pswrap V1.009  Wed Apr 19 17:50:24 PDT 1989

   This is the main file, containing all the game logic

   in ancient times all UI stuff was embedded within too (no AppKit!)
   so do not be too upset with the suboptimal structure

   Original (c) 1991 by Bill Spitzak, Q&D ported to NeXTSTEP/Appkit by
   Frank M. Siegert, 1998
*/

#include <stdio.h>
#include <stdlib.h>
#if !defined (__APPLE__) && !defined (__FreeBSD__)
# include <malloc.h>
#endif
#include <string.h>
#include <math.h>
#include <time.h>

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/Widget.h>
#include <fltk/MenuBar.h>
#include <fltk/Slider.h>
#include <fltk/Window.h>
#include <fltk/ask.h>
#include <fltk/draw.h>
#include <fltk/MenuBuild.h>

using namespace fltk;

Window *mainwin;
Window *infowin;

Slider *y_slider=(Slider *)0;

Slider *x_silder=(Slider *)0;

MenuBar *main_menu=(MenuBar *)0;

void load_menu(MenuBar* menu) {
    menu->begin();
    {MenuSection g("Info"); new Item("About"); }
    {MenuSection g("Game"); 
	new Item("New Game"); 
	new Item("Flip Sides"); 
	new Item("Undo Last Move"); 
    }
    {MenuSection g("System"); new Item("Quit"); }
    menu->end();
};

// taken directly from NeXTSTEP Qubix

const char *remark=NULL;	/* message set by makemove */

int board[65];			/* game field */
bool playerblack;		/* which side I am on */
int xangle=75,yangle=75;	/* angle in range 90 */
double sinx,cosx,siny,cosy;	/* sin/cos of angles */

#define viewdistance 12.0	/* units from origin of "eye" for perspective */
#define MARK 1
#define PLAYER 8
#define COMPUTER 64

#define TRUE 1
#define FALSE 0

#define MAXPATH 32

/* Mersenne Twister random stuff */

/* This library is free software; you can redistribute it and/or   */
/* modify it under the terms of the GNU Library General Public     */
/* License as published by the Free Software Foundation; either    */
/* version 2 of the License, or (at your option) any later         */
/* version.                                                        */
/* This library is distributed in the hope that it will be useful, */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of  */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.            */
/* See the GNU Library General Public License for more details.    */
/* You should have received a copy of the GNU Library General      */
/* Public License along with this library; if not, write to the    */
/* Free Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA   */ 
/* 02111-1307  USA                                                 */

/* Copyright (C) 1997 Makoto Matsumoto and Takuji Nishimura.       */
/* When you use this, send an email to: matumoto@math.keio.ac.jp   */
/* with an appropriate reference to your work.                     */

/* REFERENCE                                                       */
/* M. Matsumoto and T. Nishimura,                                  */
/* "Mersenne Twister: A 623-Dimensionally Equidistributed Uniform  */
/* Pseudo-Random Number Generator",                                */
/* ACM Transactions on Modeling and Computer Simulation,           */
/* Vol. 8, No. 1, January 1998, pp 3--30.                          */

#include<stdio.h>

/* Period parameters */  
#define N 624
#define M 397
#define MATRIX_A 0x9908b0df   /* constant vector a */
#define UPPER_MASK 0x80000000 /* most significant w-r bits */
#define LOWER_MASK 0x7fffffff /* least significant r bits */

/* Tempering parameters */   
#define TEMPERING_MASK_B 0x9d2c5680
#define TEMPERING_MASK_C 0xefc60000
#define TEMPERING_SHIFT_U(y)  (y >> 11)
#define TEMPERING_SHIFT_S(y)  (y << 7)
#define TEMPERING_SHIFT_T(y)  (y << 15)
#define TEMPERING_SHIFT_L(y)  (y >> 18)

static unsigned long mt[N]; /* the array for the state vector  */
static int mti=N+1; /* mti==N+1 means mt[N] is not initialized */

/* initializing the array with a NONZERO seed */
void sgenrand(unsigned long seed) 
{
    /* setting initial seeds to mt[N] using         */
    /* the generator Line 25 of Table 1 in          */
    /* [KNUTH 1981, The Art of Computer Programming */
    /*    Vol. 2 (2nd Ed.), pp102]                  */
    mt[0]= seed & 0xffffffff;
    for (mti=1; mti<N; mti++)
        mt[mti] = (69069 * mt[mti-1]) & 0xffffffff;
}

unsigned long genrand()
{
    unsigned long y;
    static unsigned long mag01[2]={0x0, MATRIX_A};
    /* mag01[x] = x * MATRIX_A  for x=0,1 */

    if (mti >= N) { /* generate N words at one time */
        int kk;

        if (mti == N+1)   /* if sgenrand() has not been called, */
            sgenrand(4357); /* a default initial seed is used   */

        for (kk=0;kk<N-M;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+M] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        for (;kk<N-1;kk++) {
            y = (mt[kk]&UPPER_MASK)|(mt[kk+1]&LOWER_MASK);
            mt[kk] = mt[kk+(M-N)] ^ (y >> 1) ^ mag01[y & 0x1];
        }
        y = (mt[N-1]&UPPER_MASK)|(mt[0]&LOWER_MASK);
        mt[N-1] = mt[M-1] ^ (y >> 1) ^ mag01[y & 0x1];

        mti = 0;
    }
  
    y = mt[mti++];
    y ^= TEMPERING_SHIFT_U(y);
    y ^= TEMPERING_SHIFT_S(y) & TEMPERING_MASK_B;
    y ^= TEMPERING_SHIFT_T(y) & TEMPERING_MASK_C;
    y ^= TEMPERING_SHIFT_L(y);

    return y; 
}

// Compatibilty stuff
// AKA poor man's DPS routines - we really only do the things the original needs omiting the rest
//

struct tpspoint {
	double x;
	double y;
	bool draw;
};

struct tpspoint PSpath[MAXPATH];
int nPathPtr=0;

double grayvalue=0.0;
double yscale=1.0;

void PSsetgray(double value) {
	int c = int(value * 255+.5);
	setcolor(color(c,c,c));
	grayvalue=value;
}

void PSsetlinewidth(double value) {

}

void PSgsave() {
	push_matrix();
}

void PSgrestore() {
	pop_matrix();
}

void PSsetinstance(int value) {

}

void PSscale(float x, float y) {
	scale(x,y);
	yscale = y;
}


void PStranslate(float x, float y) {
	translate(x,y);
}

void PSshow(char *value) {

}

void PSrectfill(double x1, double y1, double x2, double y2) {

}

void PSmoveto(double x, double y) {
  PSpath[nPathPtr].x = x;
  PSpath[nPathPtr].y = 0.25 - y;
  PSpath[nPathPtr].draw = FALSE;
  nPathPtr++;
  if (nPathPtr >= MAXPATH) {
    alert("PATH OVERFLOW in PSmoveto()");
    exit(1);
  }
}

void PSlineto(double x, double y) {
  PSpath[nPathPtr].x = x;
  PSpath[nPathPtr].y = 0.25 - y;
  PSpath[nPathPtr].draw = TRUE;
  nPathPtr++;
  if (nPathPtr >= MAXPATH) {
    alert("PATH OVERFLOW in PSlineto()");
    exit(1);
  }
}

void PSstroke() {
  if (nPathPtr<2) return;
  for (int i=0; i<nPathPtr; i++) {
    addvertex(float(PSpath[i].x), float(PSpath[i].y));
  }
  strokepath();
  nPathPtr=0;
}

void PSthickstroke() {
  if (nPathPtr<2) return;
  for (int i=0; i<nPathPtr; i++) {
    addvertex(float(PSpath[i].x), float(PSpath[i].y));
  }
  setcolor(YELLOW);
  float dx = .3f, dy = 0; fltk::transform_distance(dx,dy);
  line_style(JOIN_ROUND|CAP_ROUND,int(sqrt(dx*dx+dy*dy)+.9));
  strokepath();
  line_style(0);
  PSsetgray(grayvalue);
  nPathPtr=0;
}

#define RADIUS 0.25
#define SOLARSIZE 0.2

inline void addcircle(double x, double y, double r) {
  addarc((float) (x-r), (float) (y-r), (float) (2*r), (float) (2*r), 0, 360);
}

void darkblob( void ) {
  if (!nPathPtr) return;
  setcolor(RED);

  addcircle(PSpath[nPathPtr-1].x, PSpath[nPathPtr-1].y, RADIUS);
  fillpath();

  setcolor(color(255,128,128));
  addcircle(PSpath[nPathPtr-1].x-RADIUS/2, PSpath[nPathPtr-1].y-RADIUS/2, RADIUS/5);
  fillpath();

  setcolor(color(255,200,200));
  addcircle(PSpath[nPathPtr-1].x-RADIUS/2, PSpath[nPathPtr-1].y-RADIUS/2, RADIUS/8);
  fillpath();

  setcolor(BLACK);
  addcircle(PSpath[nPathPtr-1].x, PSpath[nPathPtr-1].y, RADIUS);
  strokepath();

  PSsetgray(grayvalue);
  nPathPtr=0;
}

void whiteblob( void ) {
  if (!nPathPtr) return;
  setcolor(BLUE);

  addcircle(PSpath[nPathPtr-1].x, PSpath[nPathPtr-1].y, RADIUS);
  fillpath();

  setcolor(color(128,128,255));
  addcircle(PSpath[nPathPtr-1].x-RADIUS/2, PSpath[nPathPtr-1].y-RADIUS/2, RADIUS/5);
  fillpath();

  setcolor(color(200,200,255));
  addcircle(PSpath[nPathPtr-1].x-RADIUS/2, PSpath[nPathPtr-1].y-RADIUS/2, RADIUS/8);
  fillpath();

  setcolor(BLACK);
  addcircle(PSpath[nPathPtr-1].x, PSpath[nPathPtr-1].y, RADIUS);
  strokepath();
  PSsetgray(grayvalue);
  nPathPtr=0;
}


// not needed, we don't do instance drawing

void clipblob( void ) {

}

// the rest is take directly from the NeXTSTEP qubix again

void moveto3(double x,double y,double z) {
  double zscale;
  zscale = viewdistance/(viewdistance-(siny*y+sinx*cosy*x+cosx*cosy*z));
  PSmoveto((cosx*x-sinx*z)*zscale, (cosy*y-sinx*siny*x-cosx*siny*z)*zscale);
  /*  PSmoveto(cosx*x-sinx*siny*y+sinx*cosy*z, z*siny+y*cosy); */
}

void lineto3(double x,double y,double z) {
  double zscale;
  zscale = viewdistance/(viewdistance-(siny*y+sinx*cosy*x+cosx*cosy*z));
  PSlineto((cosx*x-sinx*z)*zscale, (cosy*y-sinx*siny*x-cosx*siny*z)*zscale);
  /*  PSlineto(cosx*x-sinx*siny*y+sinx*cosy*z, z*siny+y*cosy); */
}

double distto(double mx,double my,double x,double y,double z) {
  double zscale;
  zscale = viewdistance/(viewdistance-(siny*y+sinx*cosy*x+cosx*cosy*z));
  return(fabs((cosx*x-sinx*z)*zscale-mx)+
	 fabs((cosy*y-sinx*siny*x-cosx*siny*z)*zscale-my));
}

int inwinrow(int);

static double wx,wy,wz;
void winline(int i,double x,double y, double z) {
  double d=0.0;
  if (!inwinrow(i)) return;
  if (wx) {
    switch ((x!=wx)+(y!=wy)+(z!=wz)) {
    case 1: d = .25; break;
    case 2: d = .25*0.70710678; break;
    case 3: d = .25*0.57735027; break;
    }
    PSsetlinewidth(0.15);
    moveto3(wx+d*(x-wx),wy+d*(y-wy),wz+d*(z-wz));
    lineto3(x,y,z);
    PSthickstroke();
    PSsetlinewidth(0.0);
  }
  wx = x; wy = y; wz = z;
}

/* draw position x and any of the rest of the board that might obscure
   it.  If x is zero, also erase the previous board.  If instance is
   on, draw the new piece using instance drawing. */
void drawfrom(int ifrom, int instance, fltk::Widget* widget) {
  double x,y,z,l1;
  int a,b,c,i,from;
  from = ifrom;
  PSscale(widget->w()/6.0f, widget->h()/6.0f);
  PStranslate(3.0,3.0);
  PSsetlinewidth(0.0);
  if (!from) {
    PSsetgray(.666); 
    PSrectfill(-3,-3,6,6); 
    PSsetgray(0.0);
  } else {
    PSgsave();
    a = (from-1)>>4;
    b = ((from-1)>>2)&3;
    c = (from-1)&3;
    moveto3(-1.5+b,-1.5+a,-1.5+c);
    clipblob();
    if (instance) PSsetinstance(TRUE);
  }
  wx = 0;
  for(a=0,y=-1.5; a<4; a++,y+=1.0) {
    if (xangle<45) {				
      for (b=0,x=-1.5; b<4; b++,x+=1.0) {
	for (c=0,l1=z=-1.5; c<4; c++,z+=1.0) {
	  i = 16*a+4*b+c+1;
	  if (board[i] && (!from || i==from)) {
	    winline(i,x,y,z);
	    if (!from && c) {
	      moveto3(x,y,l1);
	      lineto3(x,y,z);
	      PSstroke();
	    }
	    from = 0;
	    moveto3(x,y,z);
	    if (board[i]==PLAYER)
	      playerblack?darkblob():whiteblob();
	    else playerblack?whiteblob():darkblob();
	    l1 = z+.25;
	  }
	}
	if (from) continue;
	if (l1<1.5) {
	  moveto3(x,y,l1); 
	  lineto3(x,y,1.5); 
	  PSstroke();
	}
	if (b<3) for (c=0,z=-1.5; c<4; c++,z+=1.0) {
	  moveto3(x+(board[16*a+4*b+c+1]?.25:0.0),y,z);
	  lineto3(x+1.0,y,z);
	  PSstroke();
	}
      }
    } else {
      for (c=0,z=-1.5; c<4; c++,z+=1.0) {
	for (b=0,l1=x=-1.5; b<4; b++,x+=1.0) {
	  i = 16*a+4*b+c+1;
	  if (board[i] && (!from || i==from)) {
	    winline(i,x,y,z);
	    if (!from && b) {
	      moveto3(l1,y,z);
	      lineto3(x,y,z);
	      PSstroke();
	    }
	    from = 0;
	    moveto3(x,y,z);
	    if (board[i]==PLAYER)
	      playerblack?darkblob():whiteblob();
	    else playerblack?whiteblob():darkblob();
	    l1 = x+.25;
	  }
	}
	if (from) continue;
	if (l1<1.5) {
	  moveto3(l1,y,z); 
	  lineto3(1.5,y,z); 
	  PSstroke();
	}
	if (c<3) for (b=0,x=-1.5; b<4; b++,x+=1.0) {
	  moveto3(x,y,z+(board[16*a+4*b+c+1]?.25:0.0));
	  lineto3(x,y,z+1.0);
	  PSstroke();
	}
      }
    }
  } 
  if (ifrom) PSgrestore();
}

/*=========================== Original NS User Interface ===========================*/

void maindisplay() {
  mainwin->redraw();
  mainwin->flush();
}

int winrow;	/* n-1 out of lines table of winning line */
char moves[64];	/* undo history */
int movenum;
int initedfltk=0;

void newgame(void) {
  memset(board,0,sizeof(board));
  winrow = 0;
  movenum = 0;
  playerblack = 0;
  remark = NULL;
  if (initedfltk) maindisplay();
}

void erasemarks();
int makemove();

void computermove(void) {
  int i = makemove(); erasemarks();
  if (i) board[i] = COMPUTER;
  moves[movenum++] = i;
  maindisplay();
}

void flipboard(void) {
  int i;
  for (i=1; i<65; i++)
    if (board[i] == PLAYER) board[i] = COMPUTER;
    else if (board[i] == COMPUTER) board[i] = PLAYER;
}

void flipsides(void) {
  flipboard();
  playerblack = !playerblack;
  if (movenum && moves[movenum-1]==0) movenum--;
  else computermove();
  maindisplay();
}

void help(void) {
  int i;
  flipboard();
  i = makemove(); erasemarks();
  flipboard();
  if (i) {
    board[i] = PLAYER;
    moves[movenum++] = i;
    maindisplay();
    computermove();
  }
}

void undo(void) {
  if (movenum<2) return;
  board[(int)moves[--movenum]] = 0;
  board[(int)moves[--movenum]] = 0;
  winrow = 0;
  maindisplay();
}

int setAPiece(double pntx, double pnty, int for_real, fltk::Widget* widget)
{
  static int needsReset=FALSE;
  static int pieceReset=0;
  double x,y,z,l1,mx,my,idist;
  int a,b,c,i,pi;
  int e;

  if (needsReset == TRUE) {
    board[pieceReset] = 0;
    needsReset=FALSE;
  }

  if (winrow) return 1;
  //printf("setAPiece %d: %f %f\n",for_real, pntx,pnty);

  pi = 0;
  do {
    mx = (6.0*(pntx))/widget->w() - 3; 
    my = (6.0*(pnty))/widget->h() - 3;
    idist = .25; i = 0;
    for(a=0,y=-1.5; a<4; a++,y+=1.0) {
      for (c=0,z=-1.5; c<4; c++,z+=1.0) {
	for (b=0,x=-1.5; b<4; b++,x+=1.0) {
	  if (board[16*a+4*b+c+1]) continue;
	  l1 = distto(mx,my,x,y,z);
	  if (l1 <= idist) {
	    idist=l1;
	    i = 16*a+4*b+c+1;
	  }
	}
      }
    }
    if (i!=pi) {
      if (i && !for_real) {
	board[i] = PLAYER;
	maindisplay();
	needsReset=TRUE;
	pieceReset=i;
      }
      pi = i;
    }
    e = 0;
  } while (e != 0);
  if (!i) {
    maindisplay();
    return 2;
  }
  if (!for_real) {
    maindisplay();
    return 3;
  }
  board[i] = PLAYER; 
  moves[movenum++] = i;
  maindisplay();
  computermove();
  return 0;
}

void updateSliderPos(double xangle, double yangle)
{
  double x,y;
  //alert("%f %f",xangle, yangle);
  x = (90-xangle)*(3.14159/180);
  sinx = sin(x); cosx = cos(x);
  y = (90-yangle)*(3.14159/180);
  siny = sin(y); cosy = cos(y);
}

void setupGFX() {
  double x,y;

  x = (90-xangle)*(3.14159/180);
  sinx = sin(x); cosx = cos(x);
  y = (90-yangle)*(3.14159/180);
  siny = sin(y); cosy = cos(y);
}

/*=============================== Game logic =============================*/

const int lines[304] = {
	1,2,3,4,	8,7,6,5,	12,11,10,9,	13,14,15,16,
	20,19,18,17,	21,22,23,24,	25,26,27,28,	32,31,30,29,
	36,35,34,33,	37,38,39,40,	41,42,43,44,	48,47,46,45,
	49,50,51,52,	56,55,54,53,	60,59,58,57,	61,62,63,64,

	1,5,9,13,	14,10,6,2,	15,11,7,3,	4,8,12,16,
	29,25,21,17,	18,22,26,30,	19,23,27,31,	32,28,24,20,
	45,41,37,33,	34,38,42,46,	35,39,43,47,	48,44,40,36,
	49,53,57,61,	62,58,54,50,	63,59,55,51,	52,56,60,64,

	1,17,33,49,	53,37,21,5,	57,41,25,9,	13,29,45,61,
	50,34,18,2,	6,22,38,54,	10,26,42,58,	62,46,30,14,
	51,35,19,3,	7,23,39,55,	11,27,43,59,	63,47,31,15,
	4,20,36,52,	56,40,24,8,	60,44,28,12,	16,32,48,64,

	1,6,11,16,	32,27,22,17,	48,43,38,33,	49,54,59,64,
	13,10,7,4,	20,23,26,29,	36,39,42,45,	61,58,55,52,
	1,21,41,61,	62,42,22,2,	63,43,23,3,	4,24,44,64,
	49,37,25,13,	14,26,38,50,	15,27,39,51,	52,40,28,16,
	1,18,35,52,	56,39,22,5,	60,43,26,9,	13,30,47,64,
	49,34,19,4,	8,23,38,53,	12,27,42,57,	61,46,31,16,

	1,22,43,64,	49,38,27,16,	61,42,23,4,	13,26,39,52};

int nummarks;

void erasemarks() {
  int *point;
  for (point = &board[64]; point > board; point--)
    if (*point == MARK) *point = 0;
  nummarks = 0;
}

int linesum(int i) {	/* add the values in a line on the board */
  int sum,j;
  const int *p;
  if (i > 75) return(-1);
  for (sum = 0, j = 0, p = lines+4*i; j < 4; j++) sum += board[*p++];
  return(sum);
}

void markline(int i) {
  int j;
  const int *p;
  for (j = 0, p = lines+4*i; j < 4; j++,p++) if (!board[*p]) {
    board[*p] = MARK;
    nummarks++;
  }
}

int find(int n, int i) {	/* locate n in a line */
  const int *p;
  p = lines+4*i;
  if (board[*(p+1)] == n) return(*(p+1));
  if (board[*(p+2)] == n) return(*(p+2));
  if (board[*(p+0)] == n) return(*(p+0));
  if (board[*(p+3)] == n) return(*(p+3));
  return(0);
}

int inwinrow(int n) {
  const int *p;
  if (!winrow) return(FALSE);
  p = lines+4*(winrow-1);
  if (*p++ == n) return(TRUE);
  if (*p++ == n) return(TRUE);
  if (*p++ == n) return(TRUE);
  if (*p == n) return(TRUE);
  return(FALSE);
}

int makemove() {
  int i,line,move;

  for (i=0; (line=linesum(i))>=0; i++) if (line == 4*PLAYER) {
    remark = "Rats.  You win.";
    winrow = i+1;
    return(0);
  }
  else if (line == 4*COMPUTER) {
    remark = "I win!";
    winrow = i+1;
    return(0);
  }

  for (i=0; (line=linesum(i))>=0; i++) if (line == 3*COMPUTER) {
    remark = "I win!";
    winrow = i+1;
    return(find(0,i));
  }

  for (i=0; (line=linesum(i))>=0; i++) if (line == 3*PLAYER) {
    remark = "I'll block that.";
    return(find(0,i));
  }

  move = 0;

  for (i=0; (line=linesum(i))>=0; i++) {
    /* computer tries to make crossed triples */
    if (line == 2*COMPUTER) markline(i);
    else if (line == 2*COMPUTER+MARK || line == 2*COMPUTER+2*MARK) {
      remark = "Let's see you get out of this...";
      move = find(MARK,i);
      goto DONE;
    }
  }

  if (nummarks) for (i=0; (line=linesum(i))>=0; i++) {
    /* computer tries to force a crossed triple */
    if (line == COMPUTER+2*MARK || line == COMPUTER+3*MARK) {
      remark = "You've had it now...";
      move = find(MARK,i);
      goto DONE;
    }
    else if (line == 3*MARK || line == 4*MARK) {
      remark = "You have to block my triple...";
      move = find(MARK,i);
      goto DONE;
    }
    else if (line == 2*MARK) {
      remark = "Look carefully or you will lose...";
      move = find(0,i);
    }
  }

  erasemarks();
  for (i=0; (line=linesum(i))>=0; i++)
    if (line == 2*PLAYER) markline(i);
    else if (line == 2*PLAYER+MARK || line == 2*PLAYER+2*MARK) {
      remark = "You'll have to be more clever than that.";
      move = find(MARK,i);
      goto DONE;
    }

  if (nummarks) for (i=0; (line=linesum(i))>=0; i++) {
    /* try to block good players */
    if (line == PLAYER+2*MARK || line == PLAYER+3*MARK) {
      remark = "This should mess up your plans.";
      move = find(MARK,i);
      goto DONE;
    }
    else if (line == 2*MARK || line == 3*MARK || line == 4*MARK) {
      remark = "Are you trying something?";
      move = find(MARK,i);
    }
    else if (!move && line>COMPUTER && line<COMPUTER+4*MARK) {
      remark = 0;
      move = find(MARK,i);
    }
  }
  if (move) goto DONE;

  /* random move, as long as it is in non-dead line */
  erasemarks();
  for (i=1; (line=linesum(i))>=0; i++)
    if (!((line & 7*PLAYER) && (line & 7*COMPUTER)))
      /* line does not contain both types of pieces */
      markline(i);
  if (nummarks) {
    remark = 0;
    do move = (genrand()&63)+1; while (board[move] != MARK);
  }
  else remark = "Tie game!";

 DONE:
  erasemarks();
  return(move);
}

// FLTK specific stuff 

// the game board view, also the event handler

class QubixView : public Widget {
  void draw() {
    push_clip(0, 0, w(),h());
    setcolor(WHITE);
    fillrect(0,0,w(),h());
    push_matrix();
    drawfrom(0,0, this);
    pop_matrix();
    if (remark) {
      setfont(HELVETICA,12);
      drawtext(remark, 3, (float) h()-13);
    }
    pop_clip();
  }
  int handle(int event) {
    int x2,y2;
    switch (event) {
    case PUSH:
      /* fall throu */
    case DRAG:
      x2 = event_x();
      y2 = event_y();
      setAPiece(x2-6,(h()-y2)+18, 0, this);
      return 1;
      /* not reached */
      break;
    case RELEASE:
      x2 = event_x();
      y2 = event_y();
      setAPiece(x2-6,(h()-y2)+18, 1, this);
      return 1;
      /* not reached */
      break;
    }
    return 0;
  }
public:
  QubixView(int X,int Y,int W,int H) : Widget(X,Y,W,H) {}
};

/* turn the game board by using the sliders */
void updatedSlider(Widget *wid, void *value) {
  double xv, yv;

  xv = x_silder->value();
  yv = y_slider->value();

  updateSliderPos(xv,yv);

  mainwin->redraw();
}

/* handle the menu */
void menu_callback(Widget* w, void*) {
  Menu* mw = (Menu*)w;
  const Widget* m = mw->item();
  const char* acTmp = m->label();

  if (!strcmp("About",acTmp)) {
    infowin->show();
  }
  if (!strcmp("New Game",acTmp)) {
    newgame();
  }
  if (!strcmp("Flip Sides",acTmp)) {
    flipsides();
  }
  if (!strcmp("Undo Last Move",acTmp)) {
    undo();
  }
  if (!strcmp("Quit",acTmp)) {
    alert("  Thanks for playing FlQubix... Have a nice day!");
    exit(0);
  }

}

/* make our windows */

Window* make_infowin() {
  Window* w;
  { Window* o = new Window(364, 299);
    w = o;
    o->begin();
    { Widget* o = new Widget(20, 0, 325, 95, "Qubix");
      o->labelfont(TIMES_BOLD_ITALIC);
      o->labelsize(80);
    }
    { Widget* o = new Widget(30, 105, 305, 40, "A game of 4-in-a-row in 3D");
      o->labelfont(TIMES);
      o->labelsize(24);
    }
    { Widget* o = new Widget(20, 150, 335, 35, "written by Frank Siegert <frank@this.net>");
      o->labelfont(TIMES);
      o->labelsize(18);
    }
    { Widget* o = new Widget(15, 190, 340, 35, "Based on Qubix by Bill Spitzak");
      o->labelfont(TIMES);
      o->labelsize(18);
    }
    { Widget* o = new Widget(85, 215, 195, 25, "(c) 1991 on NeXTSTEP");
      o->labelfont(TIMES);
      o->labelsize(18);
    }
    { Widget* o = new Widget(25, 260, 320, 25, "This program may be distributed freely (GPL)");
      o->labelfont(TIMES);
      o->labelsize(18);
    }
    { Widget* o = new Widget(10, 75, 320, 20, "in FLTK");
      o->labelfont(TIMES);
      o->labelsize(18);
    }
    o->end();
  }
  return w;
}


Window* make_gamewin() {
  Window* w;
  { Window* o = new Window(403, 426,"FLTK Qubix");
    o->set_double_buffer();
    o->begin();
    w = o;
    y_slider = new Slider(380, 35, 20, 370);
    y_slider->set_vertical();
	y_slider->minimum(0.0);
	y_slider->maximum(90.0);
	y_slider->value(75.0);
	y_slider->callback(updatedSlider);

	// oh well... if this the way of fluid....

    { Slider* o = x_silder = new Slider(0, 405, 380, 20);
	  o->minimum(0.0);
	  o->maximum(90.0);
	  o->value(75.0);
	  o->callback(updatedSlider);
    }
    { MenuBar* o = main_menu = new MenuBar(10, 5, 170, 25);
      load_menu(main_menu);
	  o->callback(menu_callback);
    }
    { QubixView* o = new QubixView(10, 35, 370, 370);
    w->resizable(o);
    }
    o->end();
  }
  return w;
}

/* main routine */

int main(int argc, char **argv) {

  sgenrand(time(0));

  mainwin = make_gamewin();
  infowin = make_infowin();

  setupGFX();
  newgame();

  mainwin->show();

  initedfltk = 1;

  return run();
}
