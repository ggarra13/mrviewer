The relocatex-libintl patch adds builtin relocation for executables to the
libintl dll. With this patch the programs are automatically relocatable. There
is no need anymore to add relocation code to your program when you use this
libintl DLL.

The patch was ported from the GnuWin32 port of libintl, which has also builtin
relacation support.

At the moment the relocation support is only active if you compile with MinGW
for Windows. If you compile for Unix/Linux/Cygwin the functionality is
unchanged. 

See also:
http://waterlan.home.xs4all.nl/libintl.html
http://sourceforge.net/tracker/?func=detail&atid=302435&aid=3003879&group_id=2435
GnuWin32: http://gnuwin32.sourceforge.net/

Great thanks to GnuWin32 maintainer Kees Zeelenberg.

Erwin Waterlander
waterlan@xs4all.nl
http://waterlan.home.xs4all.nl/
+ 
