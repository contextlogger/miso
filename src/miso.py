def miso_version():
    """
    Returns the version number of this library, as an decimal integer
    of the form xyy, where x is the major version, and yy is the minor
    version.
    """
    pass

def get_thread_priority():
    """
    The return values to expect are: EPriorityNull (-30, cannot be set
    by user code), EPriorityMuchLess (-20), EPriorityLess (-10),
    EPriorityNormal (0), EPriorityMore (10), EPriorityMuchMore (20),
    EPriorityRealTime (30), EPriorityAbsoluteVeryLow (100),
    EPriorityAbsoluteLow (200), EPriorityAbsoluteBackground (300),
    EPriorityAbsoluteForeground (400), EPriorityAbsoluteHigh (500,
    highest possible apart from the kernel).
    """
    pass

def set_thread_priority(priority):
    """
    priority:: An integer value. See get_thread_priority()
               documentation for a list of legal values.
    """
    pass

def get_process_priority():
    """
    The return values to expect are: EPriorityLow (150),
    EPriorityBackground (250), EPriorityForeground (350),
    EPriorityHigh (450), EPriorityWindowServer (650),
    EPriorityFileServer (750), EPriorityRealTimeServer (850),
    EPrioritySupervisor (950).
    """
    pass

def set_process_priority(priority):
    """
    priority:: An integer value. See get_process_priority()
               documentation for a list of legal values.
    """
    pass

def have_process(spec):
    """
    Returns a true or false value indicating whether there are any
    processes running that match the given process name pattern.
    spec:: As for kill_process.
    """
    pass

def kill_process(spec, reason):
    """
    Ends the specified process(es). On S60 3rd Edition, the PowerMgmt
    capability is required to successfully execute this function.
    spec:: A Unicode string value. A pattern to match against either
           the process name, which would typically consist of the
           executable name and the UID. Examples are u"daemon.exe*" or
           u"*[12345678]*".
    reason:: An integer value. The reason code with which the process
             should exit.
    """
    pass

def num_alloc_heap_cells():
    """
    Returns the total number of cells allocated on the calling
    thread's heap.
    """
    pass

def num_free_heap_cells():
    """
    Returns the total number of free cells on the calling thread's
    heap.
    """
    pass

def alloc_heap_cells_size():
    """
    Returns the total size of the cells allocated on the calling
    thread's heap.
    """
    pass

def heap_biggest_avail():
    """
    Returns the space available in the largest free block an the
    calling thread's heap.
    """
    pass

def heap_total_avail():
    """
    Returns the total free space available on the calling thread's
    heap.
    """
    pass

def check_heap():
    """
    Checks the validity of the calling threads heap, raising a panic
    if any corruption is found.
    """
    pass

def compress_all_heaps():
    """
    Compresses memory chunks containing heaps.
    """
    pass

def heap_base_address():
    """
    Returns the base address of the heap of the running thread as an
    integer.
    """
    pass

def alloc_heap_cell(size):
    """
    Allocates a cell of at least the specified size from the current
    thread's heap.

    size:: An integer value.
    """
    pass

def free_heap_cell(address):
    """
    Frees the heap cell at the specified address. Passing an invalid
    address will result in a panic.
    
    address:: An integer value.
    """
    pass

def stack_info():
    """
    Returns a tuple of three integers (free, used, total). "free" and
    "used" indicate the amount of stack remaining and used up at the
    point where this function is called. "total" gives the total stack
    size allocated for the calling thread.
    
    Only supported on S60 3rd Edition and higher.
    """
    pass

def get_subst_path(drive_num):
    """
    Returns the path assigned to the specified substed drive.
    
    drive_num:: An integer specifying a drive number.
    """
    pass
    
def create_drive_subst(drive_num, path):
    """
    Assigns a path to a drive letter. To clear an existing
    substitution, specify an empty string as the path.

    drive_num:: An integer specifying a drive number.
    path:: A unicode string specifying the path.
    """
    pass

def local_bt_name():
    """
    Returns the local Bluetooth name.

    Only works with Bluetooth enabled.
    """
    pass

def local_bt_address():
    """
    Returns the local Bluetooth address.
    
    Only works with Bluetooth enabled.
    """
    pass

def set_hal_attr(attr, value):
    """
    Sets the specified HAL attribute to the specified value.
    attr:: An integer specifying the attribute.
    value:: An integer specifying its value.
    """
    pass

def get_hal_attr(attr):
    """
    Gets the value of the specified HAL attribute.
    
    attr:: An integer specifying the attribute.
    """
    pass

def tick_count():
    """
    Returns the current tick count.
    """
    pass

def reset_inactivity_time():
    """
    Resets all inactivity timers. May be used to keep the backlight
    on, for instance.
    
    Deprecated. PyS60 1.3.17 onwards has reset_inactivity(), which
    should be used instead.
    """
    pass

def restart_phone():
    """
    Restarts the device.
    """
    pass

def vibrate(duration, intensity):
    """
    Runs the vibra motor for the specified duration of time at the
    specified intensity. The duration is specified in milliseconds,
    and must be greater than zero. The intensity is a value between
    -100 and 100 (inclusive), and it specifies percentage of full
    rotation speed. Positive and negative values cause rotation in
    opposite directions; a zero value indicates no rotation, and thus
    it makes little sense to call this function with 0 intensity
    argument.

    Vibration will not work unless enabled in the profile. Only
    supported on S60 2nd Ed FP2 and higher.

    duration:: An integer.
    intensity:: An integer.
    """
    pass

class FsNotifyChange:

    def __init__(self):
        """
        Constructs an instance of FsNotifyChange. After finishing
        using a instance, you should call close() on it to free the
        associated resources.
        """
        pass

    def notify_change(self, type, callback, pathname = None):
        """
        Asynchronously requests notification of changes either
        anywhere in the filesystem, or only in locations matching the
        optionally specified Unicode pathname (which may contain
        wildcards).

        The type parameter indicates the kind of change that should
        result in notification -- its value should be one of:
        ENotifyEntry=0x00 (directory addition or deletion, or change
        of disk in a drive), ENotifyAll=0x01 (any change),
        ENotifyFile=0x04 (file creation, rename, or deletion),
        ENotifyDir=0x08 (directory creation, rename, or deletion),
        ENotifyAttributes=0x10 (attribute change), ENotifyWrite=0x20
        (change resulting from a write to a file), ENotifyDisk=0x40
        (change resulting from a raw write to a disk).

        If the registration to observe for changes fails, this method
        throws an exception, and no change notification will be
        delivered. Otherwise a change notification request will remain
        pending until either delivered, or cancel or close is called,
        in which case the request gets cancelled without notification.

        Change notification is delivered by calling the callable
        object provided as a parameter. It should accept a single
        integer argument, which will be passed as 0 to indicate that a
        filesystem change has taken place. If change observation
        fails, the callback will be called with a negative argument,
        which is a Symbian error code that should give some indication
        of the cause of the failure. The callback is not allowed to
        throw an exception; if it does, the calling thread will be
        panicked.

        Only one request per FsNotifyChange object may be outstanding
        at any one time, and it is an error to call this method when
        there is an outstanding request. Once a request completes, it
        is necessary to make a new request to observe for subsequent
        changes -- it is okay to call this method from within the
        callback.
        """
        pass

    def cancel(self):
        """
        Cancels any outstanding request.
        """
        pass

    def close(self):
        """
        Frees the resources associated with the object instance. The
        object may no longer be used after this method has been
        called. It is okay to call this method more than once, though.
        """
        pass
