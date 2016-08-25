//
// "$Id: checkers.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $"
//
// Checkers game for the Fast Light Tool Kit (FLTK).
//
// Hours of fun: the FLTK checkers game!
// Based on a very old algorithim, but it still works!
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

const char* copyright = 
"Checkers game\n"
"\xa9""2001 Bill Spitzak    spitzak@d2.com\n"
"Original Pascal code:\n"
"\xa9""1978, Oregon Minicomputer Software, Inc.\n"
"2340 SW Canyon Road, Portland, Oregon 97201\n"
"Written by Steve Poulsen 18-Jan-79\n"
"\n"
"This program is free software; you can redistribute it and/or modify "
"it under the terms of the GNU General Public License as published by "
"the Free Software Foundation; either version 2 of the License, or "
"(at your option) any later version.\n"
"\n"
"This program is distributed in the hope that it will be useful, "
"but WITHOUT ANY WARRANTY; without even the implied warranty of "
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the "
"GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU Library General Public "
"License along with this library; if not, write to the Free Software "
"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 "
"USA.";

// Define FLTK to get the fltk interface
// Define VT100 to get the VT100 interface
// Define both to get a program that takes a -t switch

#define FLTK
//#define VT100

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <time.h>

////////////////////////////////////////////////////////////////
// The algorithim:

int maxevaluate=2500;		// max number of moves to examine on a turn
int maxnodes = 2500;		// maximum number of nodes in search tree
int maxply = 20;		// maximum depth to look ahead
char forcejumps = 1;		// is forced jumps rule in effect?

// scoring parameters: (all divided by 5 from original code)
// some signs seem to be backwards, marked them with (-) in comment
const int spiece = 800;		// value of a piece
const int sking = 1200;		// value of a king
const int sadvan = 160;		// value of mypieces/theirpieces-1
// const int smobil = ?		// moves *enemy* can make w/o being jumped
const int sallpin = 80;		// mobil == 0
const int sdeny = 10;		// moves enemy can make that will be jumped
const int spin = 32;		// enemy pieces that have no move except jumped
const int sthreat = -10;	// enemy pieces we can jump if not moved (-)
const int sgrad = 1;		// score of piece positions
const int sback = 10;		// back row occupied so enemy can't make king
const int smoc2 = 200;		// more mobility, more center
const int smoc3 = -8;		// less mobility, less center
const int smoc4 = -80;		// more mobility, less center
const int smode2 = -14;		// less mobility, less denied
const int smode3 = -40;		// more mobility, more denied (-)
const int sdemmo = -20;		// more denied, more moves (-)
const int scent = 10;		// pieces in center
const int skcent = 100;		// kings in center

const int depthpenalty=4;	// guess
const int noise=2;		// values less or eq to this apart are eq

// const int sattackking = 4;	// not used
// const int sattackpiece = 3;

struct node {
  node *father;
  node *son;		// best son
  node *brother;	// next brother
  short int value;	// value of this board position to player making move
  unsigned char from,to; // the move to reach this board
  long int jump;	// bit map of locations jumped
  unsigned char mobil;
  unsigned char deny;
  unsigned char pin;
  unsigned char threat;
  short int gradient;
  unsigned who:1;	// 0 = black's move, 1 = white's move
  unsigned king:1;	// 1 = move causes piece to be kinged
  unsigned back:1;
  unsigned moc2:1;
  unsigned moc3:1;
  unsigned moc4:1;
  unsigned mode2:1;
  unsigned mode3:1;
  unsigned demmo:1;
};

int nodes;		// count of nodes

/*	Board positions:	Border positions:

	      WHITE		  00  01  02  03  04
	  05  06  07  08	04  XX  XX  XX  XX
	09  10  11  12		  XX  XX  XX  XX  13
	  14  15  16  17	13  XX  XX  XX  XX
	18  19  20  21		  XX  XX  XX  XX  22
	  23  24  25  26	22  XX  XX  XX  XX
	27  28  29  30		  XX  XX  XX  XX  31
	  32  33  34  36	31  XX  XX  XX  XX
	36  37  38  39		  XX  XX  XX  XX  40
	      BLACK		40  41  42  43  44

*/

typedef unsigned char piece;

// Piece values so that BLACK and WHITE are bit flags:
enum {
  EMPTY = 0,
  BLACK = 1,
  WHITE = 2,
  KING  = 4,
  BLACKKING = 5,
  WHITEKING = 6,
  BLUE	= 8,
  FRIEND = BLACK,
  FRIENDKING = BLACKKING,
  ENEMY	= WHITE,
  ENEMYKING = WHITEKING
};

const piece flip[9] = {
  EMPTY, WHITE, BLACK, 0, 0, WHITEKING, BLACKKING, 0, BLUE};

const int offset[9][4] = {	// legal move directions
  {0,0,0,0},
  {-5,-4,0,0},
  {4,5,0,0},
  {0,0,0,0},
  {0,0,0,0},
  {4,5,-4,-5},
  {4,5,-4,-5},
  {0,0,0,0},
  {0,0,0,0}
};

piece b[45];		// current board position being considered

int evaluated;		// number of moves evaluated this turn

char centralsquares[45];
char is_protected[45];

piece flipboard[45];	// swapped if enemy is black
piece *tb;		// pointer to real or swapped board

char check(int target,int direction) {
  // see if enemy at target can be jumped from direction by our piece
  int dst = target-direction;
  if (tb[dst]) return(0);
  int src = target+direction;
  if (tb[src] == FRIENDKING);
  else if (direction < 0 || tb[src] != FRIEND) return(0);
  piece a = tb[target]; piece b = tb[src];
  tb[target] = EMPTY; tb[src] = EMPTY;
  int safe =
    ((tb[src-4]&FRIEND && tb[src-8]&ENEMY)
     || (tb[src-5]&FRIEND && tb[src-10]&ENEMY)
     || (tb[dst-4]&ENEMY && !tb[dst+4])
     || (tb[dst-5]&ENEMY && !tb[dst+5])
     || (tb[src+4]&FRIEND && tb[src+8]==ENEMYKING)
     || (tb[src+5]&FRIEND && tb[src+10]==ENEMYKING)
     || (tb[dst+4]==ENEMYKING && !tb[dst-4])
     || (tb[dst+5]==ENEMYKING && !tb[dst-5]));
  tb[target] = a; tb[src] = b;
  return(safe);
}

int deniedmoves,undeniedmoves;
void analyzemove(int direction,int src) {
  int target = src+direction;
  if (!tb[target]) {
    if (!tb[target+direction]) is_protected[target] = 1;
    piece a = tb[src]; tb[src] = EMPTY;
    if (check(target,4) || check(target,5) ||
	check(target,-4) || check(target,-5) ||
	(tb[src+4]&ENEMY && check(src+4,4)) ||
	(tb[src+5]&ENEMY && check(src+5,5)) ||
	(tb[src-4]&ENEMY && check(src-4,-4)) ||
	(tb[src-5]&ENEMY && check(src-5,-5)))
      deniedmoves++;
    else undeniedmoves++;
    tb[src] = a;
  }
}

void evaluateboard(node *n,int print) {

  if (!n->who) tb = b;	// move was black's
  else {
    for (int i=0; i<45; i++) flipboard[44-i] = flip[b[i]];
    tb = flipboard;
  }

  memset(is_protected,0,sizeof(is_protected));
  int friendpieces = 0;
  int enemypieces = 0;
  int friendkings = 0;
  int enemykings = 0;
  int friendkcent = 0;
  int friendcent = 0;
  int enemykcent = 0;
  int enemycent = 0;
  n->mobil = n->deny = n->pin = n->threat = 0;

  int i;
  for (i=5; i<40; i++) switch(tb[i]) {
  case ENEMYKING:
    enemykings++;
    enemykcent += centralsquares[i];
    deniedmoves = 0;
    undeniedmoves = 0;
    if (i>8) {
      analyzemove(-4,i);
      analyzemove(-5,i);
    }
    goto J1;
  case ENEMY:
    deniedmoves = 0;
    undeniedmoves = 0;
  J1:	enemypieces++;
    enemycent += centralsquares[i];
    if (i<36) {
      analyzemove(4,i);
      analyzemove(5,i);
    }
    if (deniedmoves && !undeniedmoves) n->pin++;
    n->deny += deniedmoves;
    n->mobil += undeniedmoves;
    break;
  case FRIENDKING:
    friendkings++;
    friendkcent += centralsquares[i];
    if (tb[i+4]&ENEMY && !tb[i+8] && !(tb[i+4]==ENEMYKING && !tb[i-4]))
      n->threat++;
    if (tb[i+5]&ENEMY && !tb[i+10] && !(tb[i+5]==ENEMYKING && !tb[i-5]))
      n->threat++;
  case FRIEND:
    friendpieces++;
    friendcent += centralsquares[i];
    if (tb[i-4]&ENEMY && !tb[i-8] && tb[i+4]) n->threat++;
    if (tb[i-5]&ENEMY && !tb[i-10] && tb[i+5]) n->threat++;
    break;
  }

  int gradient[40];
  for (i=4; i<9; i++) gradient[i] = tb[i] ? 0 : 32;
  int total = 0;
  for (i=9; i<40; i++) {
    int x = (gradient[i-4]+gradient[i-5])/2;
    if (tb[i]==FRIEND) total += x;
    gradient[i] = ((tb[i]&FRIEND) || !(tb[i] || is_protected[i])) ? x : 0;
  }
  n->gradient = total;

  n->back = tb[39]==FRIEND && tb[37]==FRIEND && !enemykings;

  node* f = n->father;

  n->moc2 = f->mobil>n->mobil && friendcent>enemycent;
  n->moc3 = f->mobil<=n->mobil && friendcent<enemycent;
  n->moc4 = f->mobil>n->mobil && friendcent<enemycent;
  n->mode2 = f->mobil<=n->mobil && n->deny<f->deny;
  n->mode3 = f->mobil>n->mobil && n->deny>f->deny;
  n->demmo = n->deny>f->deny && f->deny+f->mobil>n->deny+n->mobil;

  total =
    spiece	* (friendpieces - enemypieces) +
    (sking-spiece) * (friendkings	- enemykings) +
    //	mobil?
    sdeny	* (n->deny	- f->deny) +
    spin	* (n->pin	- f->pin) +
    sthreat	* (n->threat	- f->threat) +
    sgrad	* (n->gradient	- f->gradient) +
    sback	* (n->back	- f->back) +
    smoc2	* (n->moc2	- f->moc2) +
    smoc3	* (n->moc3	- f->moc3) +
    smoc4	* (n->moc4	- f->moc4) +
    smode2	* (n->mode2	- f->mode2) +
    smode3	* (n->mode3	- f->mode3) +
    sdemmo	* (n->demmo	- f->demmo) +
    scent	* (friendcent	- enemycent) +
    (skcent-scent) * (friendkcent	- enemykcent);
  if (!n->mobil) total += sallpin;

  if (!enemypieces) total = 30000;
  else if (friendpieces > enemypieces)
    total += (sadvan*friendpieces)/enemypieces-sadvan;
  else total -= (sadvan*enemypieces)/friendpieces-sadvan;

  if (print) {
    printf("\tParent\tNew\tScore\n");
    printf("pieces\t%d\t%d\t%d\n",enemypieces,friendpieces,
	   spiece*(friendpieces-enemypieces));
    printf("kings\t%d\t%d\t%d\n",enemykings,friendkings,
	   (sking-spiece)*(friendkings-enemykings));
    printf("mobil\t%d\t%d\t%d\n",f->mobil,n->mobil,n->mobil?0:sallpin);
    printf("deny\t%d\t%d\t%d\n",f->deny,n->deny,sdeny*(n->deny-f->deny));
    printf("pin\t%d\t%d\t%d\n",f->pin,n->pin,spin*(n->pin-f->pin));
    printf("threat\t%d\t%d\t%d\n",f->threat,n->threat,sthreat*(n->threat-f->threat));
    printf("grad\t%d\t%d\t%d\n",f->gradient,n->gradient,sgrad*(n->gradient-f->gradient));
    printf("back\t%d\t%d\t%d\n",f->back,n->back,sback*(n->back-f->back));
    printf("moc2\t%d\t%d\t%d\n",f->moc2,n->moc2,smoc2*(n->moc2-f->moc2));
    printf("moc3\t%d\t%d\t%d\n",f->moc3,n->moc3,smoc3*(n->moc3-f->moc3));
    printf("moc4\t%d\t%d\t%d\n",f->moc4,n->moc4,smoc4*(n->moc4-f->moc4));
    printf("mode2\t%d\t%d\t%d\n",f->mode2,n->mode2,smode2*(n->mode2-f->mode2));
    printf("mode3\t%d\t%d\t%d\n",f->mode3,n->mode3,smode3*(n->mode3-f->mode3));
    printf("demmo\t%d\t%d\t%d\n",f->demmo,n->demmo,sdemmo*(n->demmo-f->demmo));
    printf("cent\t%d\t%d\t%d\n",enemycent,friendcent,scent*(friendcent-enemycent));
    printf("kcent\t%d\t%d\t%d\n",enemykcent,friendkcent,skcent*(friendkcent-enemykcent));
    printf("total:\t\t\t%d\n",total);
  }
  else {
    n->value = total;
    evaluated++;
  }
}	// end of evaluateboard

// --------------------- Tree management -----------------

node *freelist;

node *newnode(void) {
  node *n;
  if (freelist) {
    n = freelist;
    freelist = n->brother;
  }
  else n = (node *)malloc(sizeof(node));
  memset(n,0,sizeof(node));
  nodes++;
  return(n);
}

void extract(node *n) {
  node* i = n->father;
  if (i) {
    node* j = i->son;
    if (j==n) i->son = n->brother;
    else while (j) {
      i = j; j = j->brother;
      if (j==n) {i->brother = n->brother; break;}
    }
  }
  n->brother = 0;
}

void killnode(node *x) {
  if (!x) return;
  node *y;
  for (y = x; ; y = y->brother) {
    nodes--;
    killnode(y->son); y->son = 0;
    if (!y->brother) break;
  }
  y->brother = freelist;
  freelist = x;
}

int seed;		// current random number

void insert(node *n) {
  int val = n->value;
  node **pp;
  for (pp = &(n->father->son); *pp; pp = &((*pp)->brother)) {
    int val1 = (*pp)->value;
    if (abs(val-val1) <= noise) {
      seed = (seed*13077+5051)%0100000;
      if ((seed & 070) >= 060) break;
    }
    else if (val > val1) break;
  }
  n->brother = *pp;
  *pp = n;
}

// --------------------------------------------------------------

void movepiece(node* f, int i, node* jnode) {
  static char jumphappened;

  for (int k=0; k<4; k++) {
    int direction = offset[b[i]][k];
    if (!direction) break;
    int j = i+direction;
    if (b[j] == EMPTY) {
      if (!jnode && (!forcejumps || !f->son || !f->son->jump)) {
	node* n = newnode();
	n->father = f;
	n->who = !f->who;
	n->from = i;
	n->to = j;
	piece oldpiece = b[i]; b[i] = EMPTY;
	if (!(oldpiece&KING) && n->who ? (j>=36) : (j<=8)) {
	  n->king = 1;
	  b[j] = oldpiece|KING;
	}
	else b[j] = oldpiece;
	evaluateboard(n,0);
	insert(n);
	b[i] = oldpiece; b[j] = EMPTY;
      }
    } else if (((b[j]^b[i])&(WHITE|BLACK))==(WHITE|BLACK) && !b[j+direction]) {
      if (forcejumps && f->son && !f->son->jump) {
	killnode(f->son);
	f->son = 0;
      }
      int jumploc = j;
      j += direction;
      node* n = newnode();
      n->father = f;
      n->who = !f->who;
      n->from = i;
      n->to = j;
      n->jump = (1<<(jumploc-10));
      piece oldpiece = b[i]; b[i] = EMPTY;
      if (!(oldpiece&KING) && n->who ? (j>=36) : (j<=8)) {
	n->king = 1;
	b[j] = oldpiece|KING;
      }
      else b[j] = oldpiece;
      if (jnode) {
	n->from = jnode->from;
	n->jump |= jnode->jump;
	n->king |= jnode->king;
      }
      piece jumpedpiece = b[jumploc];
      b[jumploc] = EMPTY;
      jumphappened = 0;
      movepiece(f,j,n);
      if (forcejumps && jumphappened) killnode(n);
      else {evaluateboard(n,0); insert(n);}
      b[i] = oldpiece; b[j] = EMPTY;
      b[jumploc] = jumpedpiece;
      jumphappened = 1;
    }
  }
}

void expandnode(node *f) {
  if (f->son || f->value > 28000) return;	// already done
  piece turn = f->who ? BLACK : WHITE;
  for (int i=5; i<40; i++) if (b[i]&turn) movepiece(f,i,0);
  if (f->son) {
    f->value = -f->son->value;
    if (f->brother) f->value -= depthpenalty;
  }
  else f->value = 30000;
}

void makemove(node *n) {
  b[n->to] = b[n->from];
  if (n->king) b[n->to] |= KING;
  b[n->from] = EMPTY;
  if (n->jump) for(int i=0; i<32; i++) {
    if (n->jump & (1<<i)) b[10+i] = EMPTY;
  }
}

int didabort(void);

int fullexpand(node *f, int level) {
  if (didabort() || nodes > maxnodes-(maxply*10) || evaluated > maxevaluate) return(0);
  expandnode(f);
  if (!f->son) return(1);
  piece oldboard[45];
  memmove(oldboard,b,sizeof(b));
  node* n = f->son;
  if (!n->jump && n->brother) {if (level<1) return(1); level--;}
  int i;
  node* sons[32]; for (i=0; (sons[i++] = n); n = n->brother);
  int ret = 1;
  for (i=0; ret && (n = sons[i++]);) {
    makemove(n);
    ret = fullexpand(n,level);
    memmove(b,oldboard,sizeof(b));
    extract(n);
    insert(n);
  }
  f->value = -f->son->value;
  return(ret);
}

int descend(node *f) {
  static int depth;
  if (didabort() || nodes > maxnodes || depth >= maxply) return(0);
  if (f->son) {
    node* n = f->son;
    makemove(n);
    depth++;
    int ret = descend(n);
    depth--;
    extract(n);
    insert(n);
    f->value = -f->son->value;
    return(ret);
  }
  else {expandnode(f); return(1);}
}

char debug;

node *calcmove(node *root) {	// return best move after root
  expandnode(root);
  if (!root->son) return(0);	// no move due to loss
  if (debug) printf("calcmove() initial nodes = %d\n",nodes);
  evaluated = 0;
  if (root->son->brother) {
    int x;
    for (x = 1; abs(root->value)<28000 && fullexpand(root,x); x++);
    piece saveboard[45]; memmove(saveboard,b,sizeof(b));
    while (abs(root->value)<28000) {
      x = descend(root);
      memmove(b,saveboard,sizeof(b));
      if (!x) break;
    }
  }
  if (debug) printf(" evaluated %d, nodes = %d\n", evaluated, nodes);
  return(root->son);
}

// the actual game state ----------------

node *root,*undoroot;

piece jumpboards[24][45];	// saved boards for undoing jumps
int nextjump;

char user;	// 0 = black, 1 = white
char playing;
char autoplay;

void newgame(void) {

  int n;
  for (n=0; n<5; n++) b[n] = BLUE;
  for (n=5; n<18; n++) b[n] = WHITE;
  for (n=18; n<27; n++) b[n] = EMPTY;
  for (n=27; n<40; n++) b[n] = BLACK;
  for (n=40; n<45; n++) b[n] = BLUE;
  b[13] = b[22] = b[31] = BLUE;

  centralsquares[15] = centralsquares[16] =
    centralsquares[19] = centralsquares[20] =
    centralsquares[24] = centralsquares[25] =
    centralsquares[28] = centralsquares[29] = 1;

  // set up initial search tree:
  nextjump = 0;
  killnode(undoroot);
  undoroot = root = newnode();

  // make it white's move, so first move is black:
  root->who = 1;
  user = 0;
  playing = 1;
}

void domove(node* move) {
  if (move->jump) memmove(jumpboards[nextjump++],b,sizeof(b));
  makemove(move);
  extract(move);
  killnode(root->son);
  root->son = move;
  root = move;
  if (debug) evaluateboard(move,1);
}

node* undomove() {
  node *n = root;
  if (n == undoroot) return 0; // no more undo possible
  if (n->jump) memmove(b,jumpboards[--nextjump],sizeof(b));
  else {
    b[n->from] = b[n->to];
    if (n->king) b[n->from] &= (WHITE|BLACK);
    b[n->to] = EMPTY;
  }
  root = n->father;
  killnode(n);
  root->son = 0;
  root->value = 0;	// prevent it from thinking game is over
  playing = 1;
  if (root == undoroot) user = 0;
  return n;
}

const char _usermoves[] =
"B1D1F1H1A2C2E2G2??B3D3F3H3A4C4E4G4??B5D5F5H5A6C6E6G6??B7D7F7H7A8C8E8G8??";
#define usermoves(x,y) _usermoves[2*((x)-5)+(y)-1]

void dumpnode(node *n, int help) {
  int x = n->from;
  int y = n->to;
  if (help) printf("%c%c %c%c\t- ",
		   usermoves(x,1),usermoves(x,2),
		   usermoves(y,1),usermoves(y,2));
  printf("%s %ss from %c%c to %c%c",
	 n->who ? "White" : "Black",
	 n->jump ? "jump" : "move",
	 usermoves(x,1),usermoves(x,2),
	 usermoves(y,1),usermoves(y,2));
  if (n->jump) {
    for (int i=0; i<32; i++) if (n->jump & (1<<i))
      printf(", %c%c",usermoves(10+i,1),usermoves(10+i,2));
    printf(" removed");
  }
  printf(" (%+d).\n",n->value);
}

int abortflag;

////////////////////////////////////////////////////////////////
// VT100 Interface:
#ifdef VT100

void positioncursor(int i) {
  printf("\033[%d;%dH",
	 usermoves(i,2)-'0'+1,
	 2*(usermoves(i,1)-'A')+1);
}

void outpiecename(piece n) {
  printf(n&BLACK ? "\033[1;7m" : "\033[1m");
  putchar(" BW??BW??"[n]);
  putchar(" BW??KK??"[n]);
  printf("\033[0m");
}

void VT100board(void) {
  printf("\033<\033[H\033[J\033[10r");
  int l = 0;
  puts(" A B C D E F G H");
  for (int i=0; i<4; i++) {
    int j = 9*i+5;
    int k;
    for (k=0; k<4; k++) {
      printf("\033[7m  \033[0m");
      outpiecename(b[j+k]);
    }
    l++;
    printf("%d\n",l);
    j += 4;
    for (k=0; k<4; k++) {
      outpiecename(b[j+k]);
      printf("\033[7m  \033[0m");
    }
    l++;
    printf("%d\n",l);
  }
}

void VT100move(node *n, int) {
  if (!n) return;
  printf("\0337");
  positioncursor(n->from);
  outpiecename(b[n->from]);
  positioncursor(n->to);
  outpiecename(b[n->to]);
  if (n->jump) for(int i=0; i<32; i++) {
    if (n->jump & (1<<i)) {
      positioncursor(10+i);
      outpiecename(b[10+i]);
    }
  }
  printf("\0338");
}

int decode(char *m) {
  int i;
  for(i=5; i<=40; i++)
    if (toupper(m[0])==usermoves(i,1) && m[1]==usermoves(i,2)) return(i);
  return(0);
}

#include <signal.h>

static void sigint(...) {
  abortflag = 1;
  signal(SIGINT,sigint);
}

void fixexit(int x) {
  printf("\0337\033[r\0338");
  exit(x);
}

// Returns a son, or 0 if no move specified, or root to cause "help"
node *getusermove(void) {
  int i,j;
  node *t;
  char line[100],*m1,*m2;

  if (playing)
    printf("\033[1m%s's move?\033[0m ",root->who ? "Black" : "White");
  else
    printf("\033[1mCommand?\033[0m ");
  abortflag = 0;
  if (!gets(line)) {
    putchar('\n');
    if (feof(stdin)) fixexit(0);
    return 0;
  }
  for (m1 = line; *m1 && *m1<=' '; m1++);
  if (!*m1) return(0);
  m2 = m1+1;
  if (*m2) m2++;
  for (; *m2 && *m2<'0'; m2++);
  if (playing && m1[1]>='0' && m1[1]<='9') {
    i = decode(m1);
    j = decode(m2);
    if (i && j) for (t = root->son; t; t = t->brother)
      if (t->from == i && t->to == j) return(t);
    puts("Valid moves are:");
    m1[0] = 'L';
  }
  switch(toupper(m1[0])) {
  case 0: return(0);
  case 'A':
    if (playing) autoplay = 1;
    return(root);
  case 'C':
    puts(copyright);
    break;
  case 'D':
    debug = !debug;
    printf("Debug is now %s.", debug ? "on" : "off");
    break;
  case 'F':
    forcejumps = !forcejumps;
    printf("Forced jumps rule is now %s.",forcejumps ? "on" : "off");
    killnode(root->son); root->son = 0;
    return(0);
  case 'L':
    expandnode(root);
    if (playing) for (t = root->son; t; t = t->brother) dumpnode(t,1);
    break;
  case 'M':
    return(playing ? root : 0);
  case 'N':
    newgame();
    VT100board();
    return(0);
  case 'P':
    printf("I expect the following moves:\n");
    for (t = root->son; t; t = t->son) dumpnode(t,0);
    break;
  case 'Q':
    fixexit(0);
  case 'R':
    VT100board();
    break;
  case 'S':
    user = !user;
    return(root);
  case 'U':
    VT100move(undomove(),1);
    VT100move(undomove(),1);
    return(0);
  case '+':
    maxevaluate = maxnodes = 2*maxevaluate;
    goto J2;
  case '-':
    if (maxevaluate > 1)
      maxevaluate = maxnodes = maxevaluate/2;
  J2: printf("Moves evaluated set to %d.",maxevaluate);
    break;
  default:
    puts(
	 "A(utoplay)\n"
	 "C(opyright)\n"
	 "D(ebug on/off)\n"
	 "F(orce jumps rule on/off)\n"
	 "L(ist legal moves)\n"
	 "M(ake a move for me)\n"
	 "N(ew game)\n"
	 "P(redict next few moves)\n"
	 "Q(uit)\n"
	 "R(edraw screen)\n"
	 "S(witch sides)\n"
	 "U(ndo)\n"
	 "+	- smarter\n"
	 "-	- stupider");
    expandnode(root);
    for (t = root->son; t; t = t->brother) dumpnode(t,1);
  }
  return(0);
}

int VT100main() {
  signal(SIGINT,sigint);
  VT100board();
  for (;;) {
    if (playing) {
      expandnode(root);
      if (!root->son) {
	printf("%s has no move.  Game over.",root->who ? "Black" : "White");
	playing = autoplay = 0;
      }
    }
    node* move;
    if (playing && (autoplay || root->who == user)) {
      move = calcmove(root);
      if (move->value <= -30000) {
 	printf("%s resigns.", move->who ? "White" : "Black");
 	move = 0;
 	playing = autoplay = 0;
      }
    } else {
      move = getusermove();
      if (move == root) move = calcmove(root);
    }
    if (move) {
      dumpnode(move,0);
      domove(move);
      VT100move(move,0);
    }
  }
}

#endif

////////////////////////////////////////////////////////////////
// fltk interface:
#ifdef FLTK

#include <fltk/run.h>
#include <fltk/events.h>
#include <fltk/Window.h>
#include <fltk/xbmImage.h>
#include <fltk/draw.h>
#include <FL/Fl_Menu_Item.H>
#include <fltk/ask.h>
#include <fltk/Cursor.h>

//----------------------------------------------------------------
// old 4-level NeXT images have been seperated into bitmaps so they
// can be drawn with arbitrary colors and real transparency.  This is
// rather tedious and perhaps fltk should provide a direct support
// to do this:

#include "black_1.xbm"
#include "black_2.xbm"
#include "black_3.xbm"
#include "black_4.xbm"
#include "white_1.xbm"
#include "white_2.xbm"
#include "white_3.xbm"
#include "white_4.xbm"
#include "blackking_1.xbm"
#include "blackking_2.xbm"
#include "blackking_3.xbm"
#include "blackking_4.xbm"
#include "whiteking_1.xbm"
#include "whiteking_2.xbm"
#include "whiteking_3.xbm"
#include "whiteking_4.xbm"

fltk::xbmImage *bm[4][4];

void make_bitmaps() {
  if (bm[0][0]) return;
  bm[0][0] = new fltk::xbmImage(black_1_bits, black_1_width, black_1_height);
  bm[0][1] = new fltk::xbmImage(black_2_bits, black_1_width, black_1_height);
  bm[0][2] = new fltk::xbmImage(black_3_bits, black_1_width, black_1_height);
  bm[0][3] = new fltk::xbmImage(black_4_bits, black_1_width, black_1_height);
  bm[1][0] = new fltk::xbmImage(white_1_bits, black_1_width, black_1_height);
  bm[1][1] = new fltk::xbmImage(white_2_bits, black_1_width, black_1_height);
  bm[1][2] = new fltk::xbmImage(white_3_bits, black_1_width, black_1_height);
  bm[1][3] = new fltk::xbmImage(white_4_bits, black_1_width, black_1_height);
  bm[2][0] = new fltk::xbmImage(blackking_1_bits, black_1_width, black_1_height);
  bm[2][1] = new fltk::xbmImage(blackking_2_bits, black_1_width, black_1_height);
  bm[2][2] = new fltk::xbmImage(blackking_3_bits, black_1_width, black_1_height);
  bm[2][3] = new fltk::xbmImage(blackking_4_bits, black_1_width, black_1_height);
  bm[3][0] = new fltk::xbmImage(whiteking_1_bits, black_1_width, black_1_height);
  bm[3][1] = new fltk::xbmImage(whiteking_2_bits, black_1_width, black_1_height);
  bm[3][2] = new fltk::xbmImage(whiteking_3_bits, black_1_width, black_1_height);
  bm[3][3] = new fltk::xbmImage(whiteking_4_bits, black_1_width, black_1_height);
}

// Unfortunately, this is necessary on Windows 'cause GDI resources aren't
// automatically released at program exit
// WAS: no, fltk should do this! I guess when a bitmap is drawn the resulting
// image should be put in a list for deletiong on exit.
// void destroy_bitmaps() {
//   for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) delete bm[i][j];
// }

#define ISIZE black_1_width

void draw_piece(int which, int x, int y) {
  if (!fltk::not_clipped(fltk::Rectangle(x,y,ISIZE,ISIZE))) return;
  switch (which) {
  case BLACK: which = 0; break;
  case WHITE: which = 1; break;
  case BLACKKING: which = 2; break;
  case WHITEKING: which = 3; break;
  default: return;
  }
  fltk::setcolor(fltk::GRAY00); bm[which][0]->draw(x, y);
  fltk::setcolor(fltk::GRAY33); bm[which][1]->draw(x, y);
  fltk::setcolor(fltk::GRAY66); bm[which][2]->draw(x, y);
  fltk::setcolor(fltk::GRAY99); bm[which][3]->draw(x, y);
}

//----------------------------------------------------------------

class Board : public fltk::Window {
  void draw();
  int handle(int);
public:
  void drag_piece(int, int, int);
  void drop_piece(int);
  void animate(node* move, int backwards);
  void computer_move(int);
  Board(int w, int h) : fltk::Window(w,h) {color(15); /*set_double_buffer();*/}
};

#define BOXSIZE 52
#define BORDER 4
#define BOARDSIZE (8*BOXSIZE+BORDER)
#define BMOFFSET 5

static int erase_this;  // real location of dragging piece, don't draw it
static int dragging;	// piece being dragged
static int dragx;	// where it is
static int dragy;
static int showlegal;	// show legal moves

int squarex(int i) {return (usermoves(i,1)-'A')*BOXSIZE+BMOFFSET;}
int squarey(int i) {return (usermoves(i,2)-'1')*BOXSIZE+BMOFFSET;}

void Board::draw() {
  make_bitmaps();
  fltk::setcolor(color()); fltk::fillrect(0,0,w(),h());
  fltk::setcolor((fltk::Color)107 /*10*/);
  int x; for (x=0; x<8; x++) for (int y=0; y<8; y++) {
    if (!((x^y)&1)) fltk::fillrect(BORDER+x*BOXSIZE, BORDER+y*BOXSIZE,
			     BOXSIZE-BORDER, BOXSIZE-BORDER);
  }
  fltk::setcolor(fltk::GRAY20);
  for (x=0; x<9; x++) {
    fltk::fillrect(x*BOXSIZE,0,BORDER,h());
    fltk::fillrect(0,x*BOXSIZE,w(),BORDER);
  }
  for (int i = 5; i < 40; i++) if (i != erase_this) {
    draw_piece(::b[i], squarex(i), squarey(i));
  }
  if (showlegal) {
    fltk::setcolor(fltk::WHITE);
    node* n;
    for (n = root->son; n; n = showlegal==2 ? n->son : n->brother) {
      int x1 = squarex(n->from)+BOXSIZE/2-5;
      int y1 = squarey(n->from)+BOXSIZE/2-5;
      int x2 = squarex(n->to)+BOXSIZE/2-5;
      int y2 = squarey(n->to)+BOXSIZE/2-5;
      fltk::drawline(x1,y1,x2,y2);
      fltk::push_matrix();
      fltk::concat(float(x2-x1),float(y2-y1),float(y1-y2),float(x2-x1),float(x2),float(y2));
      fltk::addvertex(0,0);
      fltk::addvertex(-.3f,  .1f);
      fltk::addvertex(-.3f, -.1f);
      fltk::fillpath();
      fltk::pop_matrix();
    }
    int num = 1;
    fltk::setcolor(fltk::BLACK);
    fltk::setfont(labelfont(),10);
    for (n = root->son; n; n = showlegal==2 ? n->son : n->brother) {
      int x1 = squarex(n->from)+BOXSIZE/2-5;
      int y1 = squarey(n->from)+BOXSIZE/2-5;
      int x2 = squarex(n->to)+BOXSIZE/2-5;
      int y2 = squarey(n->to)+BOXSIZE/2-5;
      char buf[20]; sprintf(buf,"%d",num);
      fltk::drawtext(buf, float(x1+int((x2-x1)*.85)-3), float(y1+int((y2-y1)*.85)+5));
      num++;
    }
  }
  if (dragging) draw_piece(dragging, dragx, dragy);
}

// drag the piece on square i to dx dy, or undo drag if i is zero:
void Board::drag_piece(int i, int dx, int dy) {
  dy = (dy&-2) | (dx&1); // make halftone shadows line up
  if (i != erase_this) drop_piece(erase_this); // should not happen
  if (!erase_this) { // pick up old piece
    dragx = squarex(i); dragy = squarey(i);
    erase_this = i;
    dragging = ::b[i];
  }
  if (dx != dragx || dy != dragy) {
    redraw(Rectangle(dragx, dragy, ISIZE, ISIZE));
    redraw(Rectangle(dx, dy, ISIZE, ISIZE));
  }
  dragx = dx;
  dragy = dy;
}

// drop currently dragged piece on square i
void Board::drop_piece(int i) {
  if (!erase_this) return; // should not happen!
  erase_this = 0;
  dragging = 0;
  int x = squarex(i);
  int y = squarey(i);
  if (x != dragx || y != dragy) {
    redraw(Rectangle(dragx, dragy, ISIZE, ISIZE));
    redraw(Rectangle(x, y, ISIZE, ISIZE));
  }
}

#ifndef _WIN32
# include <unistd.h>
#endif

// show move (call this *before* the move, *after* undo):
void Board::animate(node* move, int backwards) {
  if (showlegal) {showlegal = 0; redraw();}
  if (!move) return;
  int f = move->from;
  int t = move->to;
  if (backwards) {int x = f; f = t; t = x;}
  int x1 = squarex(f);
  int y1 = squarey(f);
  int x2 = squarex(t);
  int y2 = squarey(t);
  const int STEPS=20; // was 35;
  for (int i=0; i<STEPS; i++) {
    int x = x1+(x2-x1)*i/STEPS;
    int y = y1+(y2-y1)*i/STEPS;
    drag_piece(move->from,x,y);
    fltk::flush();
#ifndef _WIN32
    usleep(0);
#endif
  }
  drop_piece(t);
  if (move->jump) redraw();
}

int busy; // causes pop-up abort menu

void message(const char* m, ...) {
  char buffer[2048];
  va_list a;
  va_start(a,m);
  vsprintf(buffer, m, a);
  va_end(a);
  fltk::message(buffer);
}

void Board::computer_move(int help) {
  if (!playing) return;
  cursor(fltk::CURSOR_WAIT);
  fltk::flush();
  busy = 1; abortflag = 0;
  node* move = calcmove(root);
  busy = 0;
  if (move) {
    if (!help && move->value <= -30000) {
      message("%s resigns", move->who ? "White" : "Black");
      playing = autoplay = 0;
      cursor(fltk::CURSOR_DEFAULT);
      return;
    }
    animate(move,0);
    domove(move);
  }
  expandnode(root);
  if (!root->son) {
    message("%s has no move", root->who ? "Black" : "White");
    playing = autoplay = 0;
  }
  if (!autoplay) cursor(fltk::CURSOR_DEFAULT);
}

extern Fl_Menu_Item menu[];
extern Fl_Menu_Item busymenu[];
static Board* board;

int Board::handle(int e) {
  board = this;
  if (busy) {
    switch(e) {
    case fltk::PUSH:
      busymenu->popup(fltk::event_x(), fltk::event_y());
      return 1;
    case fltk::SHORTCUT:
      return busymenu->test_shortcut() != 0;
    default:
      return Window::handle(e);
    }
  }
  node *t, *n;
  static int deltax, deltay;
  int dist;
  switch (e) {
  case fltk::PUSH:
    if (fltk::event_button() > 1) {
      static const Fl_Menu_Item* previous;
      const Fl_Menu_Item* i =
	menu->popup(fltk::event_x(), fltk::event_y(), 0, previous);
      if (i) previous = i;
      return 1;
    }
    if (playing) {
      expandnode(root);
      for (t = root->son; t; t = t->brother) {
	int x = squarex(t->from);
	int y = squarey(t->from);
	if (fltk::event_inside(Rectangle(x,y,BOXSIZE,BOXSIZE))) {
	  deltax = fltk::event_x()-x;
	  deltay = fltk::event_y()-y;
	  drag_piece(t->from,x,y);
	  return 1;
	}
      }
    }
    return 0;
  case fltk::SHORTCUT:
    return menu->test_shortcut() != 0;
  case fltk::DRAG:
    drag_piece(erase_this, fltk::event_x()-deltax, fltk::event_y()-deltay);
    return 1;
  case fltk::RELEASE:
    // find the closest legal move he dropped it on:
    dist = 50*50; n = 0;
    for (t = root->son; t; t = t->brother) if (t->from==erase_this) {
      int d1 = fltk::event_x()-deltax-squarex(t->to);
      int d = d1*d1;
      d1 = fltk::event_y()-deltay-squarey(t->to);
      d += d1*d1;
      if (d < dist) {dist = d; n = t;}
    }
    if (!n) {drop_piece(erase_this); return 1;} // none found
    drop_piece(n->to);
    domove(n);
    if (showlegal) {showlegal = 0; redraw();}
    if (n->jump) redraw();
    computer_move(0);
    return 1;
  default:
    return Window::handle(e);
  }
}

void quit_cb(fltk::Widget*, void*) { exit(0); }

#include <fltk/visual.h>
int FLTKmain(int argc, char** argv) {
  fltk::visual(fltk::DOUBLE_BUFFER|fltk::INDEXED_COLOR);
  Board b(BOARDSIZE,BOARDSIZE);
  b.callback(quit_cb);
  b.show(argc,argv);
  return fltk::run();
} 

void autoplay_cb(fltk::Widget*, void*) {
  if (autoplay) {autoplay = 0; return;}
  if (!playing) return;
  autoplay = 1;
  while (autoplay) {board->computer_move(0); board->computer_move(0);}
}

fltk::Window *copyright_window;
void copyright_cb(fltk::Widget*, void*) {
  if (!copyright_window) {
    copyright_window = new fltk::Window(400,270,"About checkers");
    //copyright_window->color(fltk::WHITE);
    copyright_window->begin();
    fltk::Widget *b = new fltk::Widget(20,0,380,270,copyright);
    b->labelsize(10);
    b->clear_flag(fltk::ALIGN_MASK);
    b->set_flag(fltk::ALIGN_LEFT|fltk::ALIGN_INSIDE|fltk::ALIGN_WRAP);
    copyright_window->end();
  }
  copyright_window->hotspot(copyright_window);
  copyright_window->set_non_modal();
  copyright_window->show();
}

void debug_cb(fltk::Widget* v, void*) {
  debug = !debug;
  // LAME! Fltk2 does not emulate Fl_Menu_Item well enough to allow any
  // changes to be remembered. Go change the table directly:
  Fl_Menu_Item* item = menu+8;
  if (debug) item->set(); else item->clear();
}

void forced_cb(fltk::Widget* v, void*) {
  forcejumps = !forcejumps;
  // LAME! Fltk2 does not emulate Fl_Menu_Item well enough to allow any
  // changes to be remembered. Go change the table directly:
  Fl_Menu_Item* item = menu+7;
  if (forcejumps) item->set(); else item->clear();
  killnode(root->son); root->son = 0;
  if (showlegal) {expandnode(root); board->redraw();}
}

void move_cb(fltk::Widget*, void*) {
  if (playing) board->computer_move(1);
  if (playing) board->computer_move(0);
}

void newgame_cb(fltk::Widget*, void*) {
  showlegal = 0;
  newgame();
  board->redraw();
}

void legal_cb(fltk::Widget*, void*) {
  if (showlegal == 1) {showlegal = 0; board->redraw(); return;}
  if (!playing) return;
  expandnode(root);
  showlegal = 1; board->redraw();
}

void predict_cb(fltk::Widget*, void*) {
  if (showlegal == 2) {showlegal = 0; board->redraw(); return;}
  if (playing) expandnode(root);
  showlegal = 2; board->redraw();
}

void switch_cb(fltk::Widget*, void* pb) {
  user = !user;
  board->computer_move(0);
}

void undo_cb(fltk::Widget*, void* pb) {
  board->animate(undomove(),1);
  board->animate(undomove(),1);
}

//--------------------------

#include <fltk/Slider.h>
#include <fltk/ValueInput.h>

fltk::Window *intel_window;
fltk::ValueInput *intel_output;

void intel_slider_cb(fltk::Widget*w, void*) {
  double v = ((fltk::Slider*)w)->value();
  int n = int(v*v);
  intel_output->value(n);
  maxevaluate = maxnodes = n;
}

void intel_cb(fltk::Widget*, void*) {
  if (!intel_window) {
    intel_window = new fltk::Window(200,25,"Checkers Intelligence");
    intel_window->begin();
    fltk::Slider* s = new fltk::Slider(60,0,140,25);
    s->minimum(1); s->maximum(500); s->value(50);
    s->callback(intel_slider_cb);
    intel_output = new fltk::ValueInput(0,0,60,25);
    intel_output->value(maxevaluate);
    intel_window->resizable(s);
    intel_window->end();
  }
  intel_window->hotspot(intel_window);
  intel_window->set_non_modal();
  intel_window->show();
}

//---------------------------

void stop_cb(fltk::Widget*, void*) {abortflag = 1;}

void continue_cb(fltk::Widget*, void*) {}

Fl_Menu_Item menu[] = {
  {"Autoplay", 'a', autoplay_cb},
  {"Legal moves", 'l', legal_cb},
  {"Move for me", 'm', move_cb},
  {"New game", 'n', newgame_cb},
  {"Predict", 'p', predict_cb},
  {"Switch sides", 's', switch_cb},
  {"Undo", 'u', undo_cb, 0, FL_MENU_DIVIDER},
  {"Forced jumps rule", 'f', forced_cb, 0, FL_MENU_TOGGLE|FL_MENU_VALUE},
  {"Debug", 'd', debug_cb, 0, FL_MENU_TOGGLE},
  {"Intelligence...", 'i', intel_cb, 0, FL_MENU_DIVIDER},
  {"About...", 'c', copyright_cb},
  {"Quit", 'q', quit_cb},
  {(const char *)0}};

Fl_Menu_Item busymenu[] = {
  {"Stop", '.', stop_cb},
  {"Autoplay", 'a', autoplay_cb},
  {"Continue", 0, continue_cb},
  {"Debug", 'd', debug_cb, 0, FL_MENU_TOGGLE},
  {"Intelligence...", 'i', intel_cb},
  {"Copyright", 'c', copyright_cb},
  {"Quit", 'q', quit_cb},
  {(const char *)0}};

#endif

////////////////////////////////////////////////////////////////
// parts shared by both interface:

#ifdef FLTK
#ifdef VT100
#define BOTH
#endif
#endif

#ifdef BOTH
int terminal;
int arg(int, char **argv, int &i) {
  if (argv[i][1] == 't') {terminal = 1; i++; return 1;}
  return 0;
}
#endif

int didabort(void) {
#ifdef FLTK
#ifdef BOTH
  if (!terminal)
#endif
    fltk::check();
#endif
  if (abortflag) {
    autoplay = 0;
    abortflag = 0;
    return 1;
  }
  return(0);
}

int main(int argc, char **argv) {
  seed = time(0);
  newgame();
#ifdef BOTH
  int i = 1;
  if (fltk::args(argc, argv, i, arg) < argc) {
    fprintf(stderr," -t : use VT100 display\n", fltk::help);
    exit(1);
  }
  if (!getenv("DISPLAY")) terminal = 1;
  if (!terminal)
#endif
#ifdef FLTK
    return FLTKmain(argc,argv);
#endif
#ifdef VT100
  return VT100main();
#endif
}

//
// End of "$Id: checkers.cxx 8500 2011-03-03 09:20:46Z bgbnbigben $".
//
