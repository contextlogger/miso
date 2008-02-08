#
# test_fs_notify.py
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

import e32
import miso

lock = e32.Ao_lock()

def cb(error):
    print "error %d" % error
    lock.signal()

fs = miso.FsNotifyChange()

print "observing"
fs.notify_change(1, cb)
print "cancelling"
fs.cancel()

print "observing all"
fs.notify_change(1, cb)
print "waiting"
lock.wait()

path = u"c:\\logs\\"
print "observing " + path
fs.notify_change(1, cb, path)
print "waiting"
lock.wait()

fs.close()
print "all done"
