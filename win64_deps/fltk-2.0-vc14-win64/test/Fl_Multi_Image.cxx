// Test of the new Fl_MultiImage class for labelling a widget

#include <fltk/Fl.h>
#include <fltk/Fl_Window.h>
#include <fltk/Fl_Button.h>
#include <fltk/Fl_Check_Button.h>
#include <fltk/Fl_Multi_Image.h>

extern Fl_Multi_Image multiimage; // defined below

void inactive_cb(Fl_Widget* w, void*)
{
  if (w->value()) {
    w->window()->child(0)->deactivate();
    w->window()->child(1)->deactivate();
  } else {
    w->window()->child(0)->activate();
    w->window()->child(1)->activate();
  }
}

int main(int argc, char** argv)
{
  Fl_Window window(200,100);
  for (int i = 0; i < 4; i++) {
    Fl_Button* b = new Fl_Button(20+i*40, 20, 32, 32);
    b->image(multiimage);
    // must remove box so it does not blink:
    b->box(FL_NO_BOX);
    // remove focus box because we have an image to show focus:
    b->focus_box(FL_NO_BOX);
    // you have to set highlight color to get it to redraw when the
    // highlighting is changed:
    b->highlight_color(FL_GRAY);
  }
  Fl_Check_Button check(20,60,180,20,"Make two of them inactive");
  check.callback(inactive_cb);
  window.end();
  window.show(argc, argv);
  return Fl::run();
}

////////////////////////////////////////////////////////////////
// Sample images designed and provided by Curtis Edwards

#include <fltk/Fl_Pixmap.h>
#include <fltk/Fl_Bitmap.h>

/* XPM */
static char *clear_tick_up[] = {
/* width height ncolors chars_per_pixel */
"32 32 20 1",
/* colors */
"  c #878E8E",
". c #7A8081",
"X c #8E9797",
"o c #B2B3B3",
"O c #C4CFCB",
"+ c #EAEEED",
"@ c #9A9B9E",
"# c #AAAAAA",
"$ c #94A19E",
"% c #B6BEBC",
"& c #A0A2A3",
"* c #BAC6C3",
"= c #6C7173",
"- c #A8B4B1",
"; c #5C6163",
": c #414C66",
"> c #A6A7A8",
", c #9BAAA7",
"< c #A1B1AC",
"1 c #ACBAB6",
/* pixels */
"################################",
"##########&@  ...  X@>##########",
"########@.;=========. X&########",
"######@=;=.     XX .=;=X@#######",
"#####.;=XXX&O+++++%$XX=; @&#####",
"####.; $$X#++++++++O&$$ ; &>####",
"###.;X$$ -+++++++++++$$$ = >####",
"## ; $$ X++++++++++++* $$ ;@####",
"#&; $$X %+++++++++++++>X$$ ;oo##",
"#.=$,X XO+++++++++++O$- X,X=Xoo#",
"&; $$XX$**OOOOOOOOO,::XXX$$ ;oo#",
" =X$XX$&<-1%%%%%%%X::: $XX$X=&%o",
"=.$$XX$&<-1111111X:::=<$XX$$. %o",
"=.$$X$,<-1.:;%%%X:::=--,$X$$..%%",
"; ,$X$<-1 :::%*X:::=11-<$X$,  %%",
"; ,$X$<-1.:::-X:::=%%%1<$X$,  %*",
"; ,,$,-1%.:::=:::=%**%1-,$,, .%%",
"; ,,,,-1% ::::::;%OO**%-,,,, =%%",
"=.,,,,-%%$:::::=**OOO*%-,,,,.X%%",
".=$,,,-%*<::::=*O**OO*%-,,,$=>%o",
">; ,,,-%** ::.OOOO**O*%-,,, ;#%#",
"#=.,<<<1**O1-OOOOOO*O*1<<<$.=oo#",
"#&;X,<<1%*OOOOOOOOOO**1<<, ;>o##",
"##==X<<-%**OOOOOOOOO*%-<<X==oo##",
"##>;.$<<-***OOOOOOO**-<<$.=#####",
"##>>==X,<1%*********1<< ==>#>>##",
"###>&==.$,1%******%1,$.==&>>>###",
"####>> ==. $,<<<,,$ .== @&&>####",
"#####>&@ ..=========. @@@&>#####",
"######>>&@@XXXXXXXX@@@@&>>######",
"#######>>>&&@@@@@@@@&&>>########",
"##########>>>>>>>>>>>>##########"
};
Fl_Pixmap up(clear_tick_up);

/* XPM */
static char *clear_tick_hi[] = {
/* width height ncolors chars_per_pixel */
"32 32 20 1",
/* colors */
"  c #9ED2BF",
". c #5D6768",
"X c #A8A9A9",
"o c #A7A7A8",
"O c #A5A5A6",
"+ c #9A9B9E",
"@ c #85A59D",
"# c #B7E0CE",
"$ c #7A8A85",
"% c #AAAAAA",
"& c #B9BCBC",
"* c #697877",
"= c #E6F7F1",
"- c #414C66",
"; c #899492",
": c #A1A1A3",
"> c #8EB3A7",
", c #B3B4B4",
"< c #93C3B2",
"1 c #ADAEAE",
/* pixels */
"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%",
"%%%%%%%%%%:+;$$$$$;;+o%%%%%%%%%%",
"%%%%%%%%+*.********.*;;:%%%%%%%%",
"%%%%%%+..$@@>>>>>>@@$..;+1%%%%%%",
"%%%%%*.$>>><#=====#>>>$.$+:%%%%%",
"%%%%**@<<>&========#<<<@.$:O%%%%",
"%%%**><<>&===========<><>*$O%%%%",
"%%$*><<>>============#><<>*;11%%",
"%:.@<<>@#=============<><<@.11%%",
"%.$><>>>#===========#+ ><<>$$,1%",
"+.><<>><###########<--@>><<@.,,%",
"$$><<><<   #######>---@<><<>$+&,",
"*@<<<<<<    ##   @---* <<<<<@$&,",
".@<<<<<   $-.###@---*  <<<<<@*&&",
"*@<<><   @---##@---*    <><<@$&&",
".><<><   $--- @---*###  <><<>$&&",
".><<<<   $---*---*####  <<<<>*&&",
".@<<<<  #@------.###### <<<<@.&&",
"*@< <   #>-----*#######  < <;;&&",
";$<    ##<----*########    <$X&,",
"O*>  < ###;--*######### < <>*1&1",
"%.;<    ###  #########    <;.,,1",
"%:*>    ##############   <>*O,1%",
"%X.$>   ##############   >$.,11%",
"%Xo.;<   ############   <;.%1XX%",
"%%XO.$>   ##########   >$.o%OX%%",
"%%%oO.$@<  ########  <@$.:oOXX%%",
"%%oXoo$$$@><      <>@$*;+:OoX%%%",
"%%%%XoO+$$$$$$$$$$$$$$+++OoX%%%%",
"%%%%%XoO:++;;;;;;;;++++:OoX%%%%%",
"%%%%%%XXoO::++++++++::OoXX%%%%%%",
"%%%%%%%%%XXooOOOOOOooXXX%%%%%%%%"
};
Fl_Pixmap hi(clear_tick_hi);

/* XPM */
static char *clear_tick_dn[] = {
/* width height ncolors chars_per_pixel */
"32 32 20 1",
/* colors */
"  c #AFAFAF",
". c #A4CBBE",
"X c #8EBBAB",
"o c #8AAEA3",
"O c #8D908F",
"+ c #ADD9C8",
"@ c #606B6C",
"# c #BEBEBE",
"$ c #AAAAAA",
"% c #788E86",
"& c #A4A4A4",
"* c #BBE3D1",
"= c #9A9C9D",
"- c #95C0B2",
"; c #727D7C",
": c #E6F7F1",
"> c #414C66",
", c #95CFBA",
"< c #81A099",
"1 c #B5B5B5",
/* pixels */
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$&=OO%;;OOO=$$$$$$$$$$$",
"$$$$$$$$=;@@;;;;;;;@@OO&$$$$$$$$",
"$$$$$$=@@%<oooooooo<%@@O=$$$$$$$",
"$$$$$;@%oooo*:::::+XXo%@%=&$$$$$",
"$$$$;@oooo.::::::::*---o@%&&$$$$",
"$$$@;oooo.:::::::::::-X-o;%&$$$$",
"$$%;ooooo::::::::::::*o--o@O$ $$",
"$&@ooooo+:::::::::::::.X--<@  $$",
"$@%ooXoo*:::::::::::*=.oX-X%%1 $",
"=@oooXX-+**********->><XX--o@ 1$",
"%%ooXXX-..++++++++o>>><XXX-X%=# ",
"@<oXXX--..+++++++o>>>;.-XX--<;#1",
"@<o-XX-...;>@+++o>>>;..,XX--<;##",
"@o,-XX...<>>>++<>>>;++..-X-,o%##",
"@o,-X-...%>>>.<>>>@+++..XX-,o%##",
"@o,,--.++%>>>;>>>@+*+++.,-,,o;##",
"@o,,,,.++<>>>>>>@+***++.,,,,o@##",
";<,,,,.++o>>>>>@*****++.,,,,<=#1",
"O%X,,,.++->>>>@*******+.,,,X% #1",
"&;o,,,.++*%>>;********+.,,,o;## ",
"$@<,,,,++**..*********+,,,-<@#1 ",
"$&@X,,,++************++,,,o@## $",
"$$@%X,,,+************+,,,X%;#1 $",
"$$$@<-,,,+**********+,,,X<@#1$$$",
"$$$$@%X,,,++++***++++,,o%;#1$$$$",
"$$$$$@%o-,+++++*++++,-<%;#1$$$$$",
"$$$$$ O;%<o,,,,.,,,o<%;&# $$$$$$",
"$$$$$$$$OO%%%%%%%%%%O=11$$$$$$$$",
"$$$$$$$$$$ $$ 11111#1 $$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$",
"$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$"
};
Fl_Pixmap dn(clear_tick_dn);

/* XPM */
static char *clear_tick_up_focus[] = {
/* width height ncolors chars_per_pixel */
"32 32 69 1",
/* colors */
"  c #ADB5AD",
". c #ADADAD",
"X c #A5B5A5",
"o c #A5ADA5",
"O c #A5A5A5",
"+ c #ADC6BD",
"@ c #738C94",
"# c #737373",
"$ c #9CADA5",
"% c #9CA5A5",
"& c #CEDED6",
"* c #C6DECE",
"= c #C6D6CE",
"- c #B5CEBD",
"; c #B5C6BD",
": c #ADC6B5",
"> c #949C9C",
", c #5A6363",
"< c #8C9C94",
"1 c #8C9494",
"2 c #B5CEC6",
"3 c #CEDECE",
"4 c #C6DEC6",
"5 c #C6D6C6",
"6 c #C6CEC6",
"7 c #BDCEBD",
"8 c #BDC6BD",
"9 c #9C9C9C",
"0 c #949C94",
"q c #BDD6C6",
"w c #BDCEC6",
"e c #A5BDB5",
"r c #BDC6C6",
"t c #A5B5B5",
"y c #8C8C8C",
"u c #E7F7EF",
"i c #E7EFEF",
"p c #D6EFDE",
"a c #D6E7DE",
"s c #CEE7D6",
"d c #C6E7CE",
"f c #B5BDBD",
"g c #7B8484",
"h c #ADBDB5",
"j c #ADB5B5",
"k c #A5BDAD",
"l c #A5B5AD",
"z c #A5ADAD",
"x c #A5A5AD",
"c c #E7F7E7",
"v c #E7EFE7",
"b c #DEF7DE",
"n c #DEEFDE",
"m c #94A59C",
"M c #D6EFD6",
"N c #D6E7D6",
"B c #737B7B",
"V c #9CB5AD",
"C c #73737B",
"Z c #9CADAD",
"A c #849C9C",
"S c #B5BDB5",
"D c #6B7373",
"F c #DEF7E7",
"G c #94ADA5",
"H c #B5B5B5",
"J c #DEEFE7",
"K c #94A5A5",
"L c #C6DED6",
/* pixels */
"................................",
".......o..%9y1gggy119O.zO.......",
"........9g,#DB#B#B#Dg1<O........",
"......%#,#g11111>>1g#,D>9.......",
"....og,C>>>o=iiiii8K>>D,19%.....",
"....g,1m$mjivvcvcii=o%$1,1OOO...",
"...g,>K%<hvvcicvciciu$$K1Dyz....",
"..1,1K%<%aapJFvFcvcvuw0$K1,9....",
".%,19Gm>7&&spJbJFJFvci m$%1,HH..",
"og#KV>>$=**NNpnbFnFnLl;0mZ>D0jHO",
"O,yK$m%k=q*sMpppppMqA@l$K$%1,HH.",
"yD>K>KlS-q*Mppnbpp*eGAXk%m%>DOfH",
"Dg%Gm%lSq*sMbFnFnsq+Vl-k$m$%g1fH",
"Dgm%mV:-q3*&pcccp*q--4w:lK$mggff",
",yZ$ml;w*5*aFcucN*qqs*q-l%$Vy1ff",
",yZ$Kl;q*q*pcccJs=qNMsq-k%$Vy1fr",
",1$l$e;q3q*pvFnad=spMsq-h$lZygff",
",1ZZV -q*5=Nnnss3snpss=-hlVZ1Dff",
"DgZVll;5*42&Ns&NbJbppd=-ellZg>ff",
"gDmZll;q*4+q**snFbpps*5;elV%DzfH",
"z,1$Vl+6=*-+23pbppMss*wStlV1,.f.",
".#gZllh+=*&**MpMpNs3s=-helmg#HH.",
".O,>Zle;w=3sssssssss==;htV1,zH..",
"..DD>llhrq=3&s&ss&&3qwhel>#DHH..",
"..z,gKlkhwwq=3*3**=wwhelKg#.....",
".OxoD#>Zl+8wwwwwwwww;ll1#BzoOOzO",
"...zODDg%Z+8wwwwww8fV$gCDOxox...",
"....Oz1DCg1$ZkltlV$1gBD19%Ox....",
".....OO91gg#CB#B#BDBg19%9%O.....",
"......OzO9%0A0A><><9999%Ox.o....",
".......xOz%O9%9%9%99%Oox........",
"..........oxoxoxoxoxox.........."
};
Fl_Pixmap up_focus(clear_tick_up_focus);

/* XPM */
static char *clear_tick_dn_focus[] = {
/* width height ncolors chars_per_pixel */
"32 32 94 2",
/* colors */
"   c #ADB5AD",
" . c #636B6B",
" X c #ADADAD",
" o c #9CD6BD",
" O c #9CCEBD",
" + c #A5ADA5",
" @ c #A5A5A5",
" # c #9CA59C",
" $ c #ADD6BD",
" % c #ADCEBD",
" & c #A5CEB5",
" * c #A5C6B5",
" = c #B5DECE",
" - c #ADDEC6",
" ; c #849C94",
" : c #9CA5A5",
" > c #ADD6C6",
" , c #ADCEC6",
" < c #C6DECE",
" 1 c #9CCEB5",
" 2 c #9CC6B5",
" 3 c #94C6AD",
" 4 c #B5CEBD",
" 5 c #BDDECE",
" 6 c #949C9C",
" 7 c #B5DEC6",
" 8 c #8C9C94",
" 9 c #B5D6C6",
" 0 c #8C9494",
" q c #849C8C",
" w c #84948C",
" e c #848C8C",
" r c #73847B",
" t c #BDC6BD",
" y c #CEF7DE",
" u c #CEEFDE",
" i c #84A59C",
" p c #C6EFD6",
" a c #C6E7D6",
" s c #7B948C",
" d c #9C9C9C",
" f c #BDDEC6",
" g c #949C94",
" h c #BDD6C6",
" j c #738C84",
" k c #949494",
" l c #738484",
" z c #A5BDB5",
" x c #E7F7EF",
" c c #BDEFD6",
" v c #BDE7D6",
" b c #D6F7DE",
" n c #B5E7CE",
" m c #D6EFDE",
" M c #8CA59C",
" N c #CEF7D6",
" B c #D6E7DE",
" V c #CEEFD6",
" C c #84A594",
" Z c #CEE7D6",
" A c #C6EFCE",
" S c #C6E7CE",
" D c #94BDAD",
" F c #6B7B7B",
" G c #94B5AD",
" H c #7B8C84",
" J c #7B8484",
" K c #8CB5A5",
" L c #637373",
" P c #D6F7E7",
" I c #ADB5B5",
" U c #636B73",
" Y c #EFF7EF",
" T c #E7F7E7",
" R c #DEF7DE",
" E c #A5DEBD",
" W c #BDE7CE",
" Q c #DEEFDE",
" ! c #A5D6BD",
" ~ c #D6F7D6",
" ^ c #A5CEBD",
" / c #D6EFD6",
" ( c #8CA594",
" ) c #D6E7D6",
" _ c #9CBDAD",
" ` c #737B7B",
" ' c #9CB5AD",
" ] c #BDBDBD",
" [ c #6B7B73",
" { c #6B7373",
" } c #DEF7E7",
" | c #94ADA5",
".  c #B5B5B5",
".. c #A5DEC6",
/* pixels */
" X X X X X X X X X X       I  .    I     X X X X X X X X X X X X",
" X X X X X X X X     X + # # ( w w # d 6 :     X X X X X X X X X",
" X X X X X X X   : e r j 8 8 8 ( 8 8 q l r 6 6 + X X X X X X X X",
" X X X X X X : F r ( | 2 * * & & & * * D M r F g : X X X X X X X",
" X X X X X J [ ; D & ^ $ p } } } } } W ! & _ C F q : @ X X X X X",
" X + X X r [ D O ^ ^ 7 } } b P b P R } p.. ^ ^ G [ s + @ X + X X",
" X X X L J G O ^ 2 7 b P R b b b b b } } } $ 1 ^ G H s @ X X X X",
" X X s l G 2 ^ 2 ^ u b b b b b b b b b P } p * ^ 1 G { k X X X X",
" X @ . K 2 1 2 * 7 p m b y b b b b b b b b } 7 1 ^ 2 i L.  X X X",
" X L s 3 O 2 _ & W p N V ~ b b b b b N V p 4 7 2 2 1 2 s H.  X X",
" # . K 2 2 1 & > A p V N b b b R b b y A z | * ^ 1 O 2 K L  .  X",
" s s K 2 2 1 ^ > W p N b b R R R R b V 9 * ' & ^ & 2 1 D s # ] X",
" . i 2 3 2 1 $ 9 W p V b } } } T } R S 9 4 4 W > & 2 2 2 i l ]. ",
" U i 2 2 2 2 $ 5 S V S ) R T T T } ) < h f S W - 1 2 2 2 i ` ] ]",
" . K o 2 2 & 7 W a S Z m } x T x Q Z Z Z N V W = ! 2 2 o K s ] ]",
" . K o 2 2 ^ = W a < Z Q } Y T } m Z / b b V W = & 2 2 o K s ] ]",
" . K o o 1 ^ 7 W p f < Z R T Q ) Z m R } m V p =.. ^ o O K ` ] ]",
" . K O o o.. 9 v p < h Z B ) Z Z Q } } b b p W 7 ! o o O K . t ]",
" ` i o o o ! 7 W p 5 h < Z Z V Q T T } b b p W 7.. o o o i d ]. ",
" 0 s D o o ! > W a W % 5 < < m } } } R b y V W 9 E o o D s   ]. ",
" @ ` K O o o > n W V 4 % h S R R b b m V p p W > ! o o K ` ] ] X",
" X . i o O o o = 5 p p S p / u b u V N p p a = ! o o 2 i . ].  X",
" X @ . D o o o = = p p p V V u N u V V p p W = o o o K L ] ] X X",
" X X . s D o o o = v p p p p p V p p p a p = o o o D s r ].  X X",
" X X X . i 2 o o o = v a p a p a p a c a = o o o D i . t.  X X X",
" X X X X . s D O o o 7 = = = W v W = 7 7 = o O K s ` ].  X X X X",
" X X X X X . s K 2 o = = = = = v = = = = o 2 i s ` ].  X X X X X",
" X X X X X   k r s i K o o o o , o o o K i s r @ ] X X X X X X X",
" X X X X X X X X 0 k s s s s s s s s s s k d. .  X X X X X X X X",
" X X X X X X X X X +   X X. . . . . .  t.  I X X + X X X X X X X",
" X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X",
" X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X X"
};
Fl_Pixmap dn_focus(clear_tick_dn_focus);

#define clear_tick_mask_width 32
#define clear_tick_mask_height 32
static char clear_tick_mask_bits[] = {
 0x00,0x00,0x00,0x00,0x00,0xfd,0xaf,0x00,0x00,0x55,0x55,0x00,0x40,0xab,0x6a,
 0x01,0x40,0x0a,0x48,0x01,0xb0,0xa8,0x92,0x01,0x28,0x01,0x20,0x05,0x98,0x54,
 0x55,0x0a,0x92,0x82,0x80,0x2a,0x26,0x18,0x25,0x14,0x4a,0x42,0x70,0x12,0x4a,
 0x20,0x78,0x17,0x4a,0x49,0x5a,0x12,0x11,0x1a,0x97,0x34,0x23,0x1c,0x0f,0x25,
 0xab,0x9d,0x47,0x36,0x25,0xbd,0xa1,0x24,0x49,0x6a,0x05,0x29,0x92,0xfc,0xa0,
 0x22,0x05,0x7c,0x0a,0x10,0xaa,0x18,0x40,0x15,0x42,0x82,0x24,0x28,0x0a,0x12,
 0x80,0x08,0x94,0x44,0x95,0x0a,0x54,0x10,0x20,0x0a,0xb0,0x4a,0xca,0x02,0x40,
 0x81,0x40,0x01,0x80,0x2a,0xaa,0x02,0x80,0xfa,0xab,0x02,0x00,0x00,0x00,0x00,
 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
Fl_Bitmap up_inactive(clear_tick_mask_bits, clear_tick_mask_width, clear_tick_mask_height);
#define dn_inactive up_inactive

Fl_Multi_Image multiimage(up,
			 FL_HIGHLIGHT, hi,
  			 FL_SELECTED, up_focus,
  			 FL_SELECTED|FL_HIGHLIGHT, dn_focus,
			 FL_VALUE, dn,
			 FL_INACTIVE, up_inactive,
			 FL_INACTIVE|FL_VALUE, dn_inactive);

// end of Fl_MultiImage test


