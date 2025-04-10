# This file was automatically generated by SWIG (https://www.swig.org).
# Version 4.2.1
#
# Do not make changes to this file unless you know what you are doing - modify
# the SWIG interface file instead.

from sys import version_info as _swig_python_version_info
# Import the low-level C/C++ module
if __package__ or "." in __name__:
    from . import _sdrplay
else:
    import _sdrplay

try:
    import builtins as __builtin__
except ImportError:
    import __builtin__

def _swig_repr(self):
    try:
        strthis = "proxy of " + self.this.__repr__()
    except __builtin__.Exception:
        strthis = ""
    return "<%s.%s; %s >" % (self.__class__.__module__, self.__class__.__name__, strthis,)


def _swig_setattr_nondynamic_instance_variable(set):
    def set_instance_attr(self, name, value):
        if name == "this":
            set(self, name, value)
        elif name == "thisown":
            self.this.own(value)
        elif hasattr(self, name) and isinstance(getattr(type(self), name), property):
            set(self, name, value)
        else:
            raise AttributeError("You cannot add instance attributes to %s" % self)
    return set_instance_attr


def _swig_setattr_nondynamic_class_variable(set):
    def set_class_attr(cls, name, value):
        if hasattr(cls, name) and not isinstance(getattr(cls, name), property):
            set(cls, name, value)
        else:
            raise AttributeError("You cannot add class attributes to %s" % cls)
    return set_class_attr


def _swig_add_metaclass(metaclass):
    """Class decorator for adding a metaclass to a SWIG wrapped class - a slimmed down version of six.add_metaclass"""
    def wrapper(cls):
        return metaclass(cls.__name__, cls.__bases__, cls.__dict__.copy())
    return wrapper


class _SwigNonDynamicMeta(type):
    """Meta class to enforce nondynamic attributes (no new attributes) for a class"""
    __setattr__ = _swig_setattr_nondynamic_class_variable(type.__setattr__)


class SwigPyIterator(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")

    def __init__(self, *args, **kwargs):
        raise AttributeError("No constructor defined - class is abstract")
    __repr__ = _swig_repr
    __swig_destroy__ = _sdrplay.delete_SwigPyIterator

    def value(self):
        return _sdrplay.SwigPyIterator_value(self)

    def incr(self, n=1):
        return _sdrplay.SwigPyIterator_incr(self, n)

    def decr(self, n=1):
        return _sdrplay.SwigPyIterator_decr(self, n)

    def distance(self, x):
        return _sdrplay.SwigPyIterator_distance(self, x)

    def equal(self, x):
        return _sdrplay.SwigPyIterator_equal(self, x)

    def copy(self):
        return _sdrplay.SwigPyIterator_copy(self)

    def next(self):
        return _sdrplay.SwigPyIterator_next(self)

    def __next__(self):
        return _sdrplay.SwigPyIterator___next__(self)

    def previous(self):
        return _sdrplay.SwigPyIterator_previous(self)

    def advance(self, n):
        return _sdrplay.SwigPyIterator_advance(self, n)

    def __eq__(self, x):
        return _sdrplay.SwigPyIterator___eq__(self, x)

    def __ne__(self, x):
        return _sdrplay.SwigPyIterator___ne__(self, x)

    def __iadd__(self, n):
        return _sdrplay.SwigPyIterator___iadd__(self, n)

    def __isub__(self, n):
        return _sdrplay.SwigPyIterator___isub__(self, n)

    def __add__(self, n):
        return _sdrplay.SwigPyIterator___add__(self, n)

    def __sub__(self, *args):
        return _sdrplay.SwigPyIterator___sub__(self, *args)
    def __iter__(self):
        return self

# Register SwigPyIterator in _sdrplay:
_sdrplay.SwigPyIterator_swigregister(SwigPyIterator)
class DeviceInfoVector(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr

    def iterator(self):
        return _sdrplay.DeviceInfoVector_iterator(self)
    def __iter__(self):
        return self.iterator()

    def __nonzero__(self):
        return _sdrplay.DeviceInfoVector___nonzero__(self)

    def __bool__(self):
        return _sdrplay.DeviceInfoVector___bool__(self)

    def __len__(self):
        return _sdrplay.DeviceInfoVector___len__(self)

    def __getslice__(self, i, j):
        return _sdrplay.DeviceInfoVector___getslice__(self, i, j)

    def __setslice__(self, *args):
        return _sdrplay.DeviceInfoVector___setslice__(self, *args)

    def __delslice__(self, i, j):
        return _sdrplay.DeviceInfoVector___delslice__(self, i, j)

    def __delitem__(self, *args):
        return _sdrplay.DeviceInfoVector___delitem__(self, *args)

    def __getitem__(self, *args):
        return _sdrplay.DeviceInfoVector___getitem__(self, *args)

    def __setitem__(self, *args):
        return _sdrplay.DeviceInfoVector___setitem__(self, *args)

    def pop(self):
        return _sdrplay.DeviceInfoVector_pop(self)

    def append(self, x):
        return _sdrplay.DeviceInfoVector_append(self, x)

    def empty(self):
        return _sdrplay.DeviceInfoVector_empty(self)

    def size(self):
        return _sdrplay.DeviceInfoVector_size(self)

    def swap(self, v):
        return _sdrplay.DeviceInfoVector_swap(self, v)

    def begin(self):
        return _sdrplay.DeviceInfoVector_begin(self)

    def end(self):
        return _sdrplay.DeviceInfoVector_end(self)

    def rbegin(self):
        return _sdrplay.DeviceInfoVector_rbegin(self)

    def rend(self):
        return _sdrplay.DeviceInfoVector_rend(self)

    def clear(self):
        return _sdrplay.DeviceInfoVector_clear(self)

    def get_allocator(self):
        return _sdrplay.DeviceInfoVector_get_allocator(self)

    def pop_back(self):
        return _sdrplay.DeviceInfoVector_pop_back(self)

    def erase(self, *args):
        return _sdrplay.DeviceInfoVector_erase(self, *args)

    def __init__(self, *args):
        _sdrplay.DeviceInfoVector_swiginit(self, _sdrplay.new_DeviceInfoVector(*args))

    def push_back(self, x):
        return _sdrplay.DeviceInfoVector_push_back(self, x)

    def front(self):
        return _sdrplay.DeviceInfoVector_front(self)

    def back(self):
        return _sdrplay.DeviceInfoVector_back(self)

    def assign(self, n, x):
        return _sdrplay.DeviceInfoVector_assign(self, n, x)

    def resize(self, *args):
        return _sdrplay.DeviceInfoVector_resize(self, *args)

    def insert(self, *args):
        return _sdrplay.DeviceInfoVector_insert(self, *args)

    def reserve(self, n):
        return _sdrplay.DeviceInfoVector_reserve(self, n)

    def capacity(self):
        return _sdrplay.DeviceInfoVector_capacity(self)
    __swig_destroy__ = _sdrplay.delete_DeviceInfoVector

# Register DeviceInfoVector in _sdrplay:
_sdrplay.DeviceInfoVector_swigregister(DeviceInfoVector)

def initializeDeviceRegistry():
    return _sdrplay.initializeDeviceRegistry()
TunerSelect_Neither = _sdrplay.TunerSelect_Neither
TunerSelect_A = _sdrplay.TunerSelect_A
TunerSelect_B = _sdrplay.TunerSelect_B
TunerSelect_Both = _sdrplay.TunerSelect_Both
RspDuoMode_Unknown = _sdrplay.RspDuoMode_Unknown
RspDuoMode_Single_Tuner = _sdrplay.RspDuoMode_Single_Tuner
RspDuoMode_Dual_Tuner = _sdrplay.RspDuoMode_Dual_Tuner
RspDuoMode_Master = _sdrplay.RspDuoMode_Master
RspDuoMode_Slave = _sdrplay.RspDuoMode_Slave
class DeviceInfo(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr
    serialNumber = property(_sdrplay.DeviceInfo_serialNumber_get, _sdrplay.DeviceInfo_serialNumber_set)
    hwVer = property(_sdrplay.DeviceInfo_hwVer_get, _sdrplay.DeviceInfo_hwVer_set)
    tuner = property(_sdrplay.DeviceInfo_tuner_get, _sdrplay.DeviceInfo_tuner_set)
    rspDuoMode = property(_sdrplay.DeviceInfo_rspDuoMode_get, _sdrplay.DeviceInfo_rspDuoMode_set)
    valid = property(_sdrplay.DeviceInfo_valid_get, _sdrplay.DeviceInfo_valid_set)
    rspDuoSampleFreq = property(_sdrplay.DeviceInfo_rspDuoSampleFreq_get, _sdrplay.DeviceInfo_rspDuoSampleFreq_set)
    dev = property(_sdrplay.DeviceInfo_dev_get, _sdrplay.DeviceInfo_dev_set)

    def __init__(self):
        _sdrplay.DeviceInfo_swiginit(self, _sdrplay.new_DeviceInfo())
    __swig_destroy__ = _sdrplay.delete_DeviceInfo

# Register DeviceInfo in _sdrplay:
_sdrplay.DeviceInfo_swigregister(DeviceInfo)
class BasicParams(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr

    def __init__(self, deviceControl):
        _sdrplay.BasicParams_swiginit(self, _sdrplay.new_BasicParams(deviceControl))
    __swig_destroy__ = _sdrplay.delete_BasicParams

    def setSampleRate(self, sampleRateHz):
        return _sdrplay.BasicParams_setSampleRate(self, sampleRateHz)

    def setRfFrequency(self, frequencyHz):
        return _sdrplay.BasicParams_setRfFrequency(self, frequencyHz)

    def setBandwidth(self, bandwidthKHz):
        return _sdrplay.BasicParams_setBandwidth(self, bandwidthKHz)

    def setIfType(self, ifkHz):
        return _sdrplay.BasicParams_setIfType(self, ifkHz)

    def setGain(self, gainReduction, lnaState):
        return _sdrplay.BasicParams_setGain(self, gainReduction, lnaState)

    def update(self):
        return _sdrplay.BasicParams_update(self)

# Register BasicParams in _sdrplay:
_sdrplay.BasicParams_swigregister(BasicParams)
cvar = _sdrplay.cvar
RSP1_HWVER = cvar.RSP1_HWVER
RSP1A_HWVER = cvar.RSP1A_HWVER
RSP2_HWVER = cvar.RSP2_HWVER
RSPDUO_HWVER = cvar.RSPDUO_HWVER
RSPDX_HWVER = cvar.RSPDX_HWVER
RSP1B_HWVER = cvar.RSP1B_HWVER
RSPDXR2_HWVER = cvar.RSPDXR2_HWVER

class ControlParams(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr

    def __init__(self, deviceControl):
        _sdrplay.ControlParams_swiginit(self, _sdrplay.new_ControlParams(deviceControl))
    __swig_destroy__ = _sdrplay.delete_ControlParams

    def setAgcControl(self, enable, setPoint=-60):
        return _sdrplay.ControlParams_setAgcControl(self, enable, setPoint)

    def setDcOffset(self, dcEnable, iqEnable):
        return _sdrplay.ControlParams_setDcOffset(self, dcEnable, iqEnable)

    def setDecimation(self, enable, decimationFactor, wideBandSignal):
        return _sdrplay.ControlParams_setDecimation(self, enable, decimationFactor, wideBandSignal)

    def update(self):
        return _sdrplay.ControlParams_update(self)

# Register ControlParams in _sdrplay:
_sdrplay.ControlParams_swigregister(ControlParams)
class RSP1AParameters(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr

    def __init__(self, control=None):
        _sdrplay.RSP1AParameters_swiginit(self, _sdrplay.new_RSP1AParameters(control))

    def getDeviceName(self):
        return _sdrplay.RSP1AParameters_getDeviceName(self)

    def applyDefaults(self):
        return _sdrplay.RSP1AParameters_applyDefaults(self)

    def setFrequency(self, freq):
        return _sdrplay.RSP1AParameters_setFrequency(self, freq)

    def getFrequency(self):
        return _sdrplay.RSP1AParameters_getFrequency(self)

    def setSampleRate(self, rate):
        return _sdrplay.RSP1AParameters_setSampleRate(self, rate)

    def getSampleRate(self):
        return _sdrplay.RSP1AParameters_getSampleRate(self)

    def setGainReduction(self, gain):
        return _sdrplay.RSP1AParameters_setGainReduction(self, gain)

    def setLNAState(self, state):
        return _sdrplay.RSP1AParameters_setLNAState(self, state)
    __swig_destroy__ = _sdrplay.delete_RSP1AParameters

# Register RSP1AParameters in _sdrplay:
_sdrplay.RSP1AParameters_swigregister(RSP1AParameters)
class RSPdxR2Parameters(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr

    def __init__(self, control=None):
        _sdrplay.RSPdxR2Parameters_swiginit(self, _sdrplay.new_RSPdxR2Parameters(control))

    def getDeviceName(self):
        return _sdrplay.RSPdxR2Parameters_getDeviceName(self)

    def applyDefaults(self):
        return _sdrplay.RSPdxR2Parameters_applyDefaults(self)

    def setFrequency(self, freq):
        return _sdrplay.RSPdxR2Parameters_setFrequency(self, freq)

    def getFrequency(self):
        return _sdrplay.RSPdxR2Parameters_getFrequency(self)

    def setSampleRate(self, rate):
        return _sdrplay.RSPdxR2Parameters_setSampleRate(self, rate)

    def getSampleRate(self):
        return _sdrplay.RSPdxR2Parameters_getSampleRate(self)

    def setHDRMode(self, enable):
        return _sdrplay.RSPdxR2Parameters_setHDRMode(self, enable)

    def setBiasTEnabled(self, enable):
        return _sdrplay.RSPdxR2Parameters_setBiasTEnabled(self, enable)
    __swig_destroy__ = _sdrplay.delete_RSPdxR2Parameters

# Register RSPdxR2Parameters in _sdrplay:
_sdrplay.RSPdxR2Parameters_swigregister(RSPdxR2Parameters)
class Device(object):
    thisown = property(lambda x: x.this.own(), lambda x, v: x.this.own(v), doc="The membership flag")
    __repr__ = _swig_repr

    def __init__(self):
        _sdrplay.Device_swiginit(self, _sdrplay.new_Device())
    __swig_destroy__ = _sdrplay.delete_Device

    def selectDevice(self, deviceInfo):
        return _sdrplay.Device_selectDevice(self, deviceInfo)

    def releaseDevice(self):
        return _sdrplay.Device_releaseDevice(self)

    def getAvailableDevices(self):
        return _sdrplay.Device_getAvailableDevices(self)

    def setFrequency(self, freq):
        return _sdrplay.Device_setFrequency(self, freq)

    def getFrequency(self):
        return _sdrplay.Device_getFrequency(self)

    def setSampleRate(self, rate):
        return _sdrplay.Device_setSampleRate(self, rate)

    def getSampleRate(self):
        return _sdrplay.Device_getSampleRate(self)

    def getRsp1aParams(self):
        return _sdrplay.Device_getRsp1aParams(self)

    def getRspDxR2Params(self):
        return _sdrplay.Device_getRspDxR2Params(self)

# Register Device in _sdrplay:
_sdrplay.Device_swigregister(Device)

