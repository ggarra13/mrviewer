#
# "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $"
#
# Copyright 1998-2004 by Bill Spitzak and others.
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA.
#
# Please report all bugs and problems to the following page:
#
#    http://www.fltk.org/str.php
#

include ../makeinclude

# The only target that probably works now is to run doxygen for html:
doxygen_html:
	@-mkdir -p html
	@-rm -rf html/*.*
	doxygen
	cp tiny.gif fltk_shadow.png doxygen.png license.html html

clean:
	-rm -rf html/*.*

depend:

# Install is not yet implemented, this is from the fltk1 version:

install:
# 	echo "Installing HTML documentation in $(docdir)"
# 	-$(MKDIR) -p $(docdir)
# 	$(CP) $(HTMLFILES) *.gif *.jpg index.html $(docdir)
# 	$(CHMOD) 644 $(docdir)/*
	echo "Installing man pages in $(mandir)"
	-$(MKDIR) -p $(DESTDIR)$(mandir)/man1
	for file in $(MAN1FILES); do \
		$(CP) $$file $(DESTDIR)$(mandir)/man1/`basename $$file .man`.1; \
	done
	-$(MKDIR) -p $(DESTDIR)$(mandir)/man3
	for file in $(MAN3FILES); do \
		$(CP) $$file $(DESTDIR)$(mandir)/man3/`basename $$file .man`.3; \
	done

uninstall:
# 	echo "Removing HTML documentation from $(docdir)"
# 	$(RM) -r $(docdir)
	echo "Removing man pages from $(mandir)"
	for file in $(MAN1FILES); do \
		$(RM) $(mandir)/man1/`basename $$file .man`.1; \
	done
	for file in $(MAN3FILES); do \
		$(RM) $(mandir)/man3/`basename $$file .man`.3; \
	done

fltk.ps: $(HTMLFILES)
	@echo "Generating PostScript documentation..."
	@$(HTMLDOC) -f fltk.ps $(MEDIA) -t ps2 --duplex --verbose --toclevels 2 --titleimage fltk.gif $(HTMLFILES)

fltk.pdf: $(HTMLFILES)
	@echo "Generating PDF documentation..."
	@rm -f fltk.pdf
	@$(HTMLDOC) -f fltk.pdf $(MEDIA) --jpeg --compression=9 --duplex --verbose --toclevels 2 --titleimage fltk.gif $(HTMLFILES)

# End of "$Id: Makefile 8500 2011-03-03 09:20:46Z bgbnbigben $".
