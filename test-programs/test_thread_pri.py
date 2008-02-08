#
# test_thread_pri.py
# 
# Copyright 2004 Helsinki Institute for Information Technology (HIIT)
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
import appuifw
import time
import thread
import miso

from miso_test_logging import *
init_logging(FileLogger("c:\\logs\\miso\\testapp.txt"))

def tell(string):
    logwrite(string)
    if e32.is_ui_thread():
        print string
        e32.ao_yield()

# --------------------------------------------------------------------

def thread_func(name, pri):
    """
    The priorities do appear to make some difference,
    even in such a heavily IO-bound case.
    """
    miso.set_thread_priority(pri)
    for i in range(100):
        logwrite("%s: %d" % (name, i))

main_tpri = miso.get_thread_priority()
tell("main thread has priority %d" % main_tpri)
miso.set_thread_priority(main_tpri)
main_tpri = miso.get_thread_priority()
tell("main thread has priority %d" % main_tpri)

main_ppri = miso.get_process_priority()
tell("main process has priority %d" % main_ppri)
miso.set_process_priority(main_ppri)
main_ppri = miso.get_process_priority()
tell("main process has priority %d" % main_ppri)

tids = [thread.start_new_thread(thread_func, ("muchless", -20)),
        thread.start_new_thread(thread_func, ("less", -10)),
        thread.start_new_thread(thread_func, ("normal", 0)),
        thread.start_new_thread(thread_func, ("more", 10))]
tell("threads started")
for tid in tids:
    try:
        thread.ao_waittid(tid)
    except:
        pass
tell("done")
