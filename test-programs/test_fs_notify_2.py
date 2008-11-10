# 
# Copyright 2007-2008 Helsinki Institute for Information Technology
# (HIIT) and the authors. All rights reserved.
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
import os
from miso import FsNotifyChange

class GUI:
    def __init__(self):
        self.lock = e32.Ao_lock()
        self.main_title = u"FsChangeTest"
        self.old_title = appuifw.app.title

        app_path = os.path.split(appuifw.app.full_name())[0]
        self.app_drive = app_path[:2]

        main_menu = [
            (u"Exit", self.abort)
            ]

        appuifw.app.title = self.main_title
        appuifw.app.menu = main_menu
        appuifw.app.exit_key_handler = self.abort

        # Yes, it seems that a full pathname is required, including
        # the drive letter. So unfortunately a single invocation
        # cannot request observing the same path on multiple different
        # drives.
        self.dir = u"e:\\images"
        
        self.fs = FsNotifyChange()
        self.fs_observe()

    def fs_observe(self):
        self.fs.notify_change(1, self._dir_changed, self.dir)
        
    def _dir_changed(self, error):
        if error == 0:
            print "got change event"
            self.fs_observe()
        else:
            print "got error %d" % error

    def abort(self):
        self.lock.signal()

    def loop(self):
        self.lock.wait()

    def close(self):
        self.fs.close()
        appuifw.app.menu = []
        appuifw.app.exit_key_handler = None
        appuifw.app.title = self.old_title

def main():
    gui = GUI()
    try:
        gui.loop()
    finally:
        gui.close()

if __name__ == '__main__':
    main()
    print "all done"
