#
# test_subst.py
# 
# Copyright 2005 Helsinki Institute for Information Technology (HIIT)
# and the authors.  All rights reserved.
# 
# Authors: Tero Hasu <tero.hasu@hut.fi>
#

# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import appuifw
import e32
import miso

actions = [u"subst", u"clear", u"show"]

drive = 10

i = True
while i is not None:
    i = appuifw.popup_menu(actions, u"Choose:")
    if i == 0:
        miso.create_drive_subst(drive, u"e:\\system\\apps\\python\\")
    elif i == 1:
        miso.create_drive_subst(drive, u"")
    elif i == 2:
        print miso.get_subst_path(drive)
    e32.ao_sleep(2)
