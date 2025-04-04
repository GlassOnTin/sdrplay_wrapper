/*
 * numpy.i: SWIG interface file for NumPy arrays
 * 
 * This file provides typemaps to convert between C/C++ arrays
 * and NumPy arrays in Python
 */

%{
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>
%}

/*
 * The following code originally comes from
 * https://github.com/numpy/numpy/blob/master/numpy/core/include/numpy/arrayobject.h
 */

#ifdef SWIGPYTHON

%{
/* Macros to extract array attributes.
 */
#define PyArray_NDIM(obj) ((PyArrayObject_fields *)(obj))->nd
#define PyArray_DIMS(obj) ((PyArrayObject_fields *)(obj))->dimensions
#define PyArray_STRIDES(obj) ((PyArrayObject_fields *)(obj))->strides
#define PyArray_DIM(obj,n) (PyArray_DIMS(obj)[n])
#define PyArray_STRIDE(obj,n) (PyArray_STRIDES(obj)[n])
#define PyArray_DATA(obj) ((void*)((PyArrayObject_fields *)(obj))->data)
#define PyArray_DTYPE(obj) ((PyArrayObject_fields *)(obj))->descr
#define PyArray_ITEMSIZE(obj) (PyArray_DTYPE(obj)->elsize)
#define PyArray_SIZE(obj) PyArray_Size((PyObject*)(obj))
%}

%init %{
    import_array();
%}

%define %numpy_typemaps(DATA_TYPE, DATA_TYPECODE)

/* Typemap suite for (DATA_TYPE* INPLACE_ARRAY1, DIM_TYPE DIM1)
 */
%typecheck(SWIG_TYPECHECK_DOUBLE_ARRAY,
           fragment="NumPy_Macros")
  (DATA_TYPE* INPLACE_ARRAY1, DIM_TYPE DIM1)
{
  $1 = is_array($input) && PyArray_EquivTypenums(array_type($input),
                                                 DATA_TYPECODE);
}
%typemap(in,
         fragment="NumPy_Macros")
  (DATA_TYPE* INPLACE_ARRAY1, DIM_TYPE DIM1)
  (PyArrayObject* array=NULL)
{
  array = obj_to_array_no_conversion($input, DATA_TYPECODE);
  if (!array || !require_dimensions(array,1) ||
      !require_contiguous(array) || !require_native(array)) SWIG_fail;
  $1 = (DATA_TYPE*) array_data(array);
  $2 = (DIM_TYPE) array_size(array,0);
}

/* Typemap suite for (DATA_TYPE* INPLACE_ARRAY2, DIM_TYPE DIM1, DIM_TYPE DIM2)
 */
%typecheck(SWIG_TYPECHECK_DOUBLE_ARRAY,
           fragment="NumPy_Macros")
  (DATA_TYPE* INPLACE_ARRAY2, DIM_TYPE DIM1, DIM_TYPE DIM2)
{
  $1 = is_array($input) && PyArray_EquivTypenums(array_type($input),
                                                 DATA_TYPECODE);
}
%typemap(in,
         fragment="NumPy_Macros")
  (DATA_TYPE* INPLACE_ARRAY2, DIM_TYPE DIM1, DIM_TYPE DIM2)
  (PyArrayObject* array=NULL)
{
  array = obj_to_array_no_conversion($input, DATA_TYPECODE);
  if (!array || !require_dimensions(array,2) ||
      !require_contiguous(array) || !require_native(array)) SWIG_fail;
  $1 = (DATA_TYPE*) array_data(array);
  $2 = (DIM_TYPE) array_size(array,0);
  $3 = (DIM_TYPE) array_size(array,1);
}

/* Typemap suite for (DIM_TYPE DIM1, DATA_TYPE* INPLACE_ARRAY1)
 */
%typecheck(SWIG_TYPECHECK_DOUBLE_ARRAY,
           fragment="NumPy_Macros")
  (DIM_TYPE DIM1, DATA_TYPE* INPLACE_ARRAY1)
{
  $1 = is_array($input) && PyArray_EquivTypenums(array_type($input),
                                                 DATA_TYPECODE);
}
%typemap(in,
         fragment="NumPy_Macros")
  (DIM_TYPE DIM1, DATA_TYPE* INPLACE_ARRAY1)
  (PyArrayObject* array=NULL)
{
  array = obj_to_array_no_conversion($input, DATA_TYPECODE);
  if (!array || !require_dimensions(array,1) ||
      !require_contiguous(array) || !require_native(array)) SWIG_fail;
  $1 = (DIM_TYPE) array_size(array,0);
  $2 = (DATA_TYPE*) array_data(array);
}

/* Typemap suite for (DIM_TYPE DIM1, DIM_TYPE DIM2, DATA_TYPE* INPLACE_ARRAY2)
 */
%typecheck(SWIG_TYPECHECK_DOUBLE_ARRAY,
           fragment="NumPy_Macros")
  (DIM_TYPE DIM1, DIM_TYPE DIM2, DATA_TYPE* INPLACE_ARRAY2)
{
  $1 = is_array($input) && PyArray_EquivTypenums(array_type($input),
                                                 DATA_TYPECODE);
}
%typemap(in,
         fragment="NumPy_Macros")
  (DIM_TYPE DIM1, DIM_TYPE DIM2, DATA_TYPE* INPLACE_ARRAY2)
  (PyArrayObject* array=NULL)
{
  array = obj_to_array_no_conversion($input, DATA_TYPECODE);
  if (!array || !require_dimensions(array,2) ||
      !require_contiguous(array) || !require_native(array)) SWIG_fail;
  $1 = (DIM_TYPE) array_size(array,0);
  $2 = (DIM_TYPE) array_size(array,1);
  $3 = (DATA_TYPE*) array_data(array);
}

/* Typemap suite for (DATA_TYPE INPLACE_ARRAY3[ANY][ANY][ANY])
 */
%typecheck(SWIG_TYPECHECK_DOUBLE_ARRAY,
           fragment="NumPy_Macros")
  (DATA_TYPE INPLACE_ARRAY3[ANY][ANY][ANY])
{
  $1 = is_array($input) && PyArray_EquivTypenums(array_type($input),
                                                 DATA_TYPECODE);
}
%typemap(in,
         fragment="NumPy_Macros")
  (DATA_TYPE INPLACE_ARRAY3[ANY][ANY][ANY])
  (PyArrayObject* array=NULL)
{
  array = obj_to_array_no_conversion($input, DATA_TYPECODE);
  if (!array || !require_dimensions(array,3) ||
      !require_contiguous(array) || !require_native(array)) SWIG_fail;
  $1 = (DATA_TYPE*) array_data(array);
}

/* Typemap suite for (DATA_TYPE* ARGOUT_ARRAY1, DIM_TYPE DIM1)
 */
%typemap(in,numinputs=0,
         fragment="NumPy_Macros")
  (DATA_TYPE* ARGOUT_ARRAY1, DIM_TYPE DIM1)
  (PyObject* array = NULL)
{
  $1 = (DATA_TYPE*) malloc($2*sizeof(DATA_TYPE));
  if ($1 == NULL) SWIG_fail;
}
%typemap(argout,fragment="NumPy_Macros")
  (DATA_TYPE* ARGOUT_ARRAY1, DIM_TYPE DIM1)
{
  npy_intp dims[1] = { $2 };
  PyObject* obj = PyArray_SimpleNewFromData(1, dims, DATA_TYPECODE, (void*)$1);
  PyArrayObject* array = (PyArrayObject*) obj;
  if (!array) SWIG_fail;
  $result = SWIG_Python_AppendOutput($result,obj);
}
%typemap(freearg)
  (DATA_TYPE* ARGOUT_ARRAY1, DIM_TYPE DIM1)
{
  if ($1) free($1);
}

/* Typemap suite for (DATA_TYPE* ARGOUT_ARRAY2, DIM_TYPE DIM1, DIM_TYPE DIM2)
 */
%typemap(in,numinputs=0,
         fragment="NumPy_Macros")
  (DATA_TYPE* ARGOUT_ARRAY2, DIM_TYPE DIM1, DIM_TYPE DIM2)
  (PyObject* array = NULL)
{
  $1 = (DATA_TYPE*) malloc($2*$3*sizeof(DATA_TYPE));
  if ($1 == NULL) SWIG_fail;
}
%typemap(argout,fragment="NumPy_Macros")
  (DATA_TYPE* ARGOUT_ARRAY2, DIM_TYPE DIM1, DIM_TYPE DIM2)
{
  npy_intp dims[2] = { $2, $3 };
  PyObject* obj = PyArray_SimpleNewFromData(2, dims, DATA_TYPECODE, (void*)$1);
  PyArrayObject* array = (PyArrayObject*) obj;
  if (!array) SWIG_fail;
  $result = SWIG_Python_AppendOutput($result,obj);
}
%typemap(freearg)
  (DATA_TYPE* ARGOUT_ARRAY2, DIM_TYPE DIM1, DIM_TYPE DIM2)
{
  if ($1) free($1);
}

%enddef /* %numpy_typemaps */

/*
 * Templates for array conversion
 */
%numpy_typemaps(signed char, NPY_BYTE)
%numpy_typemaps(unsigned char, NPY_UBYTE)
%numpy_typemaps(short, NPY_SHORT)
%numpy_typemaps(unsigned short, NPY_USHORT)
%numpy_typemaps(int, NPY_INT)
%numpy_typemaps(unsigned int, NPY_UINT)
%numpy_typemaps(long, NPY_LONG)
%numpy_typemaps(unsigned long, NPY_ULONG)
%numpy_typemaps(long long, NPY_LONGLONG)
%numpy_typemaps(unsigned long long, NPY_ULONGLONG)
%numpy_typemaps(float, NPY_FLOAT)
%numpy_typemaps(double, NPY_DOUBLE)

/* Array fragments */
%fragment("NumPy_Macros",
          "header")
{
/* Macros to extract array attributes -- adopted from numpy/arrayobject.h */
%#define PyArray_NDIM(obj) ((PyArrayObject_fields *)(obj))->nd
%#define PyArray_DIMS(obj) ((PyArrayObject_fields *)(obj))->dimensions
%#define PyArray_STRIDES(obj) ((PyArrayObject_fields *)(obj))->strides
%#define PyArray_DIM(obj,n) (PyArray_DIMS(obj)[n])
%#define PyArray_STRIDE(obj,n) (PyArray_STRIDES(obj)[n])
%#define PyArray_DATA(obj) ((void*)((PyArrayObject_fields *)(obj))->data)
%#define PyArray_DTYPE(obj) ((PyArrayObject_fields *)(obj))->descr
%#define PyArray_ITEMSIZE(obj) (PyArray_DTYPE(obj)->elsize)
%#define PyArray_SIZE(obj) PyArray_Size((PyObject*)(obj))

/* Utilities for checking arrays */
static int is_array(PyObject* obj) {
  return PyArray_Check(obj);
}
static int array_dimensions(PyArrayObject* arr) {
  return PyArray_NDIM(arr);
}
static int array_size(PyArrayObject* arr, int i) {
  return PyArray_DIM(arr, i);
}
static int array_is_contiguous(PyArrayObject* arr) {
  return PyArray_ISCONTIGUOUS(arr);
}
static int array_is_native(PyArrayObject* arr) {
  return PyArray_ISNOTSWAPPED(arr);
}
static void* array_data(PyArrayObject* arr) {
  return PyArray_DATA(arr);
}
static int array_type(PyArrayObject* arr) {
  return PyArray_TYPE(arr);
}

/* Check if the given PyObject is an array and has the given type and rank
   Return 1 if it is a match, 0 otherwise */
static int require_dimensions(PyArrayObject* ary, int rank) {
  if (array_dimensions(ary) != rank) {
    PyErr_Format(PyExc_TypeError,
                 "Array has wrong dimension (expected %d, got %d)",
                 rank, array_dimensions(ary));
    return 0;
  }
  return 1;
}

/* Check if the given PyObject is an array of the given type.
   Return a borrowed reference to the array if successful, NULL otherwise. */
static PyArrayObject* obj_to_array_no_conversion(PyObject* obj, int typecode) {
  if (!PyArray_Check(obj)) {
    PyErr_SetString(PyExc_TypeError, "Expected an array");
    return NULL;
  }
  if (array_type((PyArrayObject*)obj) != typecode) {
    PyErr_Format(PyExc_TypeError,
                 "Array has wrong data type (expected %d)",
                 typecode);
    return NULL;
  }
  return (PyArrayObject*)obj;
}

/* Require that a numpy array is contiguous.
   Return 1 if array is contiguous, 0 otherwise. */
static int require_contiguous(PyArrayObject* ary) {
  if (!array_is_contiguous(ary)) {
    PyErr_SetString(PyExc_TypeError,
                    "Array must be contiguous");
    return 0;
  }
  return 1;
}

/* Require the given PyArrayObject to have native byte order.
   Return 1 if it is native, 0 otherwise. */
static int require_native(PyArrayObject* ary) {
  if (!array_is_native(ary)) {
    PyErr_SetString(PyExc_TypeError,
                    "Array must have native byte order");
    return 0;
  }
  return 1;
}
}

#endif /* SWIGPYTHON */