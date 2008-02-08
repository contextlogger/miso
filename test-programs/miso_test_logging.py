#
# this is some revision of pdis.lib.logging
#
# Copyright 2004 Helsinki Institute for Information Technology (HIIT)
# and the authors.
#
# Authors: Tero Hasu <tero.hasu@hut.fi>
#          Ken Rimey <rimey@hiit.fi>
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
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

"""
Flexible logging facility

To use the built-in file logger, do this:

    from pdis.lib.logging import *

    init_logging(FileLogger("mylog.txt"))

If the specified file path is not absolute, it is relative to a
predefined base path.  On Symbian OS, this is "c:\logs\pdis".
On other systems, it is the current directory (".").

To actually enable logging, you must create the log file's directory
before running the application.

To implement custom logging functionality, provide a custom logger.
This only has to implement the write(text) and close() methods.

Calling close() is optional unless the particular logger requires it.
"""

import sys
import os
import thread
import time
import traceback

try:
    import threading
    have_threading = True
except:
    have_threading = False

if sys.platform == "symbian_s60":
    base_path = "c:\\logs\\pdis"
else:
    base_path = "."

class EchoLogger:
    """
    Logging to stdout
    """
    def write(self, text):
        print text

    def close(self):
        pass

class FileLogger:
    """
    Logging to a file
    """
    def __init__(self, path, echo = False):
        self.path = os.path.join(base_path, path)
        self.echo = echo
        self.mutex = thread.allocate_lock()
        self.write("logging started", truncate = True)

    def write(self, text, truncate = False):
        self.mutex.acquire()
        try:
            if self.path:
                if truncate:
                    mode = "w"
                else:
                    mode = "a"

                try:
                    f = open(self.path, mode)
                    try:
                        f.write("%s: %s\n" % (time.asctime(), text))
                    finally:
                        f.close()
                except IOError:
                    self.path = None

            if self.echo:
                print text
        finally:
            self.mutex.release()

    def close(self):
        self.write("logging stopped")
        self.path = None
        self.echo = False

_writer = None

def init_logging(writer):
    global _writer
    _writer = writer

def finish_logging():
    global _writer
    _writer.close()
    _writer = None

def thread_finish_logging():
    """
    Loggers supporting Symbian and multiple threads require that
    this method is called by all threads that have used the logger,
    prior to dying.
    """
    if _writer:
        try:
            _writer.close_for_thread()
        except AttributeError:
            pass

def logwrite(text):
    if _writer:
        if have_threading:
            thread_name = threading.currentThread().getName()
            if thread_name:
                text = "[%s] %s" % (thread_name, text)

        if isinstance(text, unicode):
            text = text.encode("utf-8")

        _writer.write(text)

def log_exception(text = None):
    if _writer:
        lines = traceback.format_exception(*sys.exc_info())
        if text:
            lines.insert(0, text + '\n')

        logwrite("".join(lines))

def log_error(text, log_exc = False):
    """deprecated"""
    text = "Error: " + text
    if log_exc:
        log_exception(text)
    else:
        logwrite(text)

def log_warning(text, log_exc = False):
    """deprecated"""
    text = "Warning: " + text
    if log_exc:
        log_exception(text)
    else:
        logwrite(text)
