##     This file is part of the package "parapin".

##     The parapin package is free software; you can redistribute it
##     and/or modify it under the terms of the GNU Library General Public
##     License (LGPL) as published by the Free Software Foundation.

##     The parapin package is distributed in the hope that it will be
##     useful, but WITHOUT ANY WARRANTY; without even the implied
##     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##     See the GNU Library General Public License for more details.

##     You should have received a copy of the GNU Library General Public
##     License along with parapin; if not, write to the Free
##     Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
##     02111-1307 USA


##     For futher information on the parapin package, please refer to the
##     project information hosted on Sourceforge --

##     http://sourceforge.net/projects/parapin/


 


##     setup.py
## 
##    $Id: setup.py,v 1.3 2005/07/23 19:10:16 pwerneck Exp $




from distutils.core import setup, Extension

module1 = Extension('parapin',
                    sources=['parapin_py.c'],
                    include_dirs=['../'],
                    libraries=['parapin'],
                    library_dirs=['../'])

setup(name = 'Parapin',
      ext_modules = [module1])

