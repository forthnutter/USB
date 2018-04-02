#    This file is part of the package "parapin".
#
#    The parapin package is free software; you can redistribute it
#    and/or modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation.
#
#    The parapin package is distributed in the hope that it will be
#    useful, but WITHOUT ANY WARRANTY; without even the implied
#    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#    See the GNU Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with parapindriver; if not, write to the Free
#    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#    02111-1307 USA
#
#
#    For futher information on the parapin package, please refer to the
#    project information hosted on Sourceforge --
#    http://parapin.sourceforge.net/
#
default: parapin.ps parapin.txt html

parapin.ps: parapin.tex
	latex parapin.tex
	latex parapin.tex
	dvips -o parapin.ps parapin.dvi

parapin.txt: parapin.tex
	rm -rf /tmp/parapintemp
	mkdir /tmp/parapintemp
	latex2html -split 0 -ascii_mode -noauto_link -rootdir /tmp/parapintemp parapin
	lynx -dump --nolist /tmp/parapintemp/parapin.html > parapin.txt
	cp /tmp/parapintemp/parapin.html .
	rm -rf /tmp/parapintemp

html:
	latex2html -split 4 -local_icons parapin

clean:
	rm -f parapin.ps parapin.txt parapin.html