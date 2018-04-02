/*  This file is part of the package "parapin".

    The parapin package is free software; you can redistribute it
    and/or modify it under the terms of the GNU Library General Public
    License (LGPL) as published by the Free Software Foundation.

    The parapin package is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with parapin; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
    02111-1307 USA


    For futher information on the parapin package, please refer to the
    project information hosted on Sourceforge --

    http://sourceforge.net/projects/parapin/

*/
 
/*
 * parapin_py.c
 *
 * $Id $
 *
 */
#include <Python.h>
#include <unistd.h>
#include <time.h>
#include "structmember.h"
#include <parapin.h>
#include "parapin_py.h"


/* Exception  */

static PyObject *parapin_error;

/* Pin Object */
typedef struct {

  PyObject_HEAD
  int pin;

} PinObject;

/* Pin dealloc */
static void
Pin_dealloc(PinObject* self){

  self->ob_type->tp_free((PyObject*)self);

};

/* Pin.__init__() */
static int
Pin_init(PinObject *self, PyObject *args){
  
  int pin;
  
  if (!PyArg_ParseTuple(args, "i", &pin))
    return -1;

  self->pin = pin;

  return 0;
};

/* Pin.set() */
static PyObject *
Pin_set(PinObject *self){

  set_pin(self->pin);

  Py_INCREF(Py_None);
  return Py_None;
};

/* Pin.clear() */
static PyObject *
Pin_clear(PinObject *self){

  clear_pin(self->pin);

  Py_INCREF(Py_None);
  return Py_None;
};

/* Pin.change() */
static PyObject *
Pin_change(PinObject *self, PyObject *args){

  int state;

  if (!PyArg_ParseTuple(args, "i", &state))
    return NULL;

  change_pin(self->pin, state);

  Py_INCREF(Py_None);
  return Py_None;
};

/* Pin.is_set() */
static PyObject *
Pin_is_set(PinObject *self){

  return Py_BuildValue("i", pin_is_set(self->pin));
};

/* Pin.__nonzero__() */
static int
Pin_nonzero(PinObject *self){

    return (pin_is_set(self->pin));
};

/* Pin.pulse() */
static PyObject *
Pin_pulse(PinObject *self, PyObject *args){
  
  int pol, secs, msecs, n;

  if (!PyArg_ParseTuple(args, "iiii", &pol, &secs, &msecs, &n))
    return NULL;

  struct timespec sleeptime;
  sleeptime.tv_sec = secs;
  sleeptime.tv_nsec = msecs*1000000;

  while (n > 0) {
    change_pin(self->pin, pol);
    nanosleep(&sleeptime, NULL);
    n --;
    if (n <= 0)
      break;
    change_pin(self->pin, !pol);
    nanosleep(&sleeptime, NULL);
    
    n--;
  }

  Py_INCREF(Py_None);
  return Py_None;
};

/* Pin.__lshift__() */
static PyObject *
Pin_lshift(PinObject *self, PyObject *args){

  PinObject *result;
  int shift;
  if (PyInt_Check(args))
    shift = PyInt_AsLong(args);
  else if (PyTuple_Check(args)){
    if (!PyArg_ParseTuple(args, "i", &shift))
      return NULL;
  }

  else{
    PyErr_SetString(PyExc_ValueError, 
		    "an integer is required");
    return NULL;
  }

  result = PyObject_NEW(PinObject, &PinType);
  result->pin = self->pin << shift;

  Py_INCREF(result);
  return (PyObject *) result;
};

/* Pin.__rshift__() */
static PyObject *
Pin_rshift(PinObject *self, PyObject *args){

  PinObject *result;
  int shift;

  if (PyInt_Check(args))
    shift = PyInt_AsLong(args);

  else if (PyTuple_Check(args)){
    if (!PyArg_ParseTuple(args, "i", &shift))
      return NULL;
  }

  else{
    PyErr_SetString(PyExc_ValueError, 
		    "an integer is required");
    return NULL;
  }

  result = PyObject_NEW(PinObject, &PinType);
  result->pin = self->pin >> shift;

  Py_INCREF(result);
  return (PyObject *) result;
};

/* Pin.__and__() */
static PyObject *
Pin_and(PinObject *self, PinObject *other){

  PinObject *result;

  result = PyObject_NEW(PinObject, &PinType);
  result->pin = self->pin & other->pin;

  Py_INCREF(result);
  return (PyObject *) result;
};

/* Pin.__or__() */
static PyObject *
Pin_or(PinObject *self, PinObject *other){

  PinObject *result;

  result = PyObject_NEW(PinObject, &PinType);
  result->pin = self->pin | other->pin;

  Py_INCREF(result);
  return (PyObject *) result;
};

/* Pin.__xor__() */
static PyObject *
Pin_xor(PinObject *self, PinObject *other){

  PinObject *result;

  result = PyObject_NEW(PinObject, &PinType);
  result->pin = self->pin ^ other->pin;

  Py_INCREF(result);
  return (PyObject *) result;
};

/* Pin.__coerce__() */
static int
Pin_coerce(PinObject **self, PyObject **other){
  
  if (PyInt_Check(*other)){
    Py_INCREF(*self);
    return 0;
  }
  return 1;

};

/* Pin attributes */
static PyMemberDef Pin_members[] = {
  {"pin", T_INT, offsetof(PinObject, pin), 0,
   "P.pin -> integer -- pin number"},
  {NULL}  /* Sentinel */
};

/* Pin methods */
static PyMethodDef Pin_methods[] = {
    {"set", (PyCFunction)Pin_set, METH_NOARGS,
     "P.set() -- set this pin"},
    {"clear", (PyCFunction)Pin_clear, METH_NOARGS,
     "P.clear() -- clear this pin"},
    {"change", (PyCFunction)Pin_change, METH_VARARGS,
     "P.change(state) -- change this pin state"},
    {"pulse", (PyCFunction)Pin_pulse, METH_VARARGS,
     "P.pulse(pol, secs, msecs, n) -- send 'n' pulses with 'pol' polarity and 'secs.msecs'"},
    {"is_set", (PyCFunction)Pin_is_set, METH_NOARGS,
     "P.is_set() -> boolean -- get current INPUT pin state"},

    {NULL}  /* Sentinel */
};

/* Pin number methods */
static PyNumberMethods PinAsNumber[] = {
  0,                      /*nb_add*/
  0,                      /*nb_subtract*/
  0,                      /*nb_multiply*/
  0,                      /*nb_divide*/
  0,                      /*nb_remainder*/
  0,                      /*nb_divmod*/
  0,                      /*nb_power*/
  0,                      /*nb_negative*/
  0,                      /*nb_positive*/
  0,                      /*nb_absolute*/
  (inquiry)Pin_nonzero,   /*nb_nonzero*/
  0,                      /*nb_invert*/
  (binaryfunc)Pin_lshift, /*nb_lshift*/
  (binaryfunc)Pin_rshift, /*nb_rshift*/
  (binaryfunc)Pin_and,    /*nb_and*/
  (binaryfunc)Pin_xor,    /*nb_xor*/
  (binaryfunc)Pin_or,     /*nb_or*/
  (coercion)Pin_coerce,   /*nb_coerce*/
  0,                      /*nb_int*/
  0,                      /*nb_long*/
  0,                      /*nb_float*/
  0,                      /*nb_oct*/
  0,                      /*nb_hex*/
  0,                      /*nb_inplace_add*/
  0,                      /*nb_inplace_subtract*/
  0,                      /*nb_inplace_multiply*/
  0,                      /*nb_inplace_divide*/
  0,                      /*nb_inplace_remainder*/
  0,                      /*nb_inplace_power*/
  0,                      /*nb_inplace_lshift*/
  0,                      /*nb_inplace_rshift*/
  0,                      /*nb_inplace_and*/
  0,                      /*nb_inplace_xor*/
  0,                      /*nb_inplace_or*/
  0,                      /* nb_floor_divide */
  0,                      /* nb_true_divide */
  0,                      /* nb_inplace_floor_divide */
  0,                      /* nb_inplace_true_divide */
};


/* Pin Type */
PyTypeObject PinType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "parapin.Pin",             /*tp_name*/
    sizeof(PinObject),         /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Pin_dealloc,   /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    PinAsNumber,              /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Pin(i) -> new pin",       /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    Pin_methods,               /* tp_methods */
    Pin_members,               /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Pin_init,        /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};


/* Port Object */

typedef struct {

  PyObject_HEAD
  int base;

} PortObject;

static void
Port_dealloc(PortObject* self){

  self->ob_type->tp_free((PyObject*)self);
};

/* Port.__init__ */
static int
Port_init(PortObject *self, PyObject *args, PyObject *kwds){

  int base;
  int uid = -1;
  int inmode = -1;
  int outmode = -1;

  static char *kwlist[] = {"base", "uid", "inmode", "outmode", NULL};

  if (! PyArg_ParseTupleAndKeywords(args, kwds, "i|iii", kwlist, 
				    &base,
				    &uid,
				    &inmode,
				    &outmode
				    ))
    return -1; 

  self->base = base;
  
  if (pin_init_user(base) < 0){
    PyErr_SetFromErrno(parapin_error);
    return -1;
  }
  
  if (inmode != -1)
    pin_input_mode(inmode);

  if (outmode!= -1)
    pin_output_mode(outmode);

  if (uid != -1){
    if (setuid(uid) < 0){
      PyErr_SetFromErrno(parapin_error);
      return -1;
    }
  }
  
  return 0;
};

/* Port.raw_pin() */
static PyObject *
Port_raw_pin(PortObject *self, PyObject *args){
  
  PinObject *pin_obj;
  
  int rawpin;

  if (!PyArg_ParseTuple(args, "i", &rawpin))
    return NULL;
  
  pin_obj = PyObject_NEW(PinObject, &PinType);
  pin_obj->pin = rawpin;
  
  Py_INCREF(pin_obj);
  return (PyObject *) pin_obj;
};

/* Port.get_pin() */
static PyObject *
Port_get_pin(PortObject *self, PyObject *args){
  
  PinObject *pin_obj;
  PyObject *tmp;
  int ipin, len, itmp;

  assert(PyTuple_Check(args));

  len = PyTuple_Size(args);

  if (!len > 0)
    return NULL;

  if (len == 1){
    if (!PyArg_ParseTuple(args, "i", &ipin))
      return NULL;

    if ((ipin > 25) | (ipin < 0)){
      PyErr_SetString(PyExc_IndexError, 
		      "Pin index out of range.");
      return NULL;
    }

    pin_obj = PyObject_NEW(PinObject, &PinType);
    pin_obj->pin = LP_PIN[ipin];
  }

  else{
    ipin = 0;
    while (len > 0){

      tmp = PyTuple_GetItem(args, len-1);

      if (!PyInt_Check(tmp))
	return NULL;
      itmp = PyInt_AsLong(tmp);

      if ((itmp > 25) | (itmp < 0)){
	PyErr_SetString(PyExc_IndexError, 
			"Pin index out of range.");
	return NULL;
      }
      ipin |= LP_PIN[itmp];
      len --;
    }
    pin_obj = PyObject_NEW(PinObject, &PinType);
    pin_obj->pin = ipin;
  };
    
  Py_INCREF(pin_obj);
  return (PyObject *) pin_obj;

};

/* Port.__getitem__ */
static PyObject *
Port_getitem(PortObject *self, int ipin){
  
  if ((ipin > 25) | (ipin < 0)){
    PyErr_SetString(PyExc_IndexError, 
		    "Pin index out of range.");
    return NULL;
  };

  PinObject *pin_obj;

  pin_obj = PyObject_NEW(PinObject, &PinType);
  pin_obj->pin = LP_PIN[ipin];
  
  Py_INCREF(pin_obj);
  return (PyObject *) pin_obj;
};

/* Port.__getitem__ as slice */
static PyObject *
Port_getslice(PortObject *self, int ilow, int ihigh){

  PyTupleObject *result;
  PyObject **dest;
  int len;
  int i;

  if (ilow < 0)
    ilow = 0;
  if (ihigh > 26)
    ihigh = 26;
      
  len = ihigh - ilow;
  result = (PyTupleObject *)PyTuple_New(len);
  dest = result->ob_item;

  if (result == NULL)
    return NULL;

  for (i = 0; i < len; i++){
    PinObject *pin = PyObject_NEW(PinObject, &PinType);
    pin->pin = LP_PIN[ilow];
    Py_INCREF(pin);
    dest[i] = (PyObject *) pin;
    ilow ++;
  }
  return (PyObject *) result;

};

/* Port.set_input_mode() */
static PyObject *
Port_set_input_mode(PortObject *self, PyObject *args){

  int mode;
  if (!PyArg_ParseTuple(args, "i", &mode))
    return NULL;

  pin_input_mode(mode);

  Py_INCREF(Py_None);
  return Py_None;
};

/* Port.set_output_mode() */
static PyObject *
Port_set_output_mode(PortObject *self, PyObject *args){

  int mode;
  if (!PyArg_ParseTuple(args, "i", &mode))
    return NULL;

  pin_output_mode(mode);

  Py_INCREF(Py_None);
  return Py_None;
};

/* Port attributes */
static PyMemberDef Port_members[] = {
  {"base", T_INT, offsetof(PortObject, base), 0,
   "P.base -> integer -- port base address"},
  {NULL}  /* Sentinel */
};

/* Port methods */
static PyMethodDef Port_methods[] = {
  {"raw_pin", (PyCFunction)Port_raw_pin, METH_VARARGS,
   "P.raw_pin(i) -> Pin(i) -- return Pin object with i value(a single object)"},
  {"get_pin", (PyCFunction)Port_get_pin, METH_VARARGS,
   "P.get_pin(i0, i1, ... ) -> Pin(LP_PIN[i0] | LP_PIN[i1] | ...)"},
  {"set_input_mode", (PyCFunction)Port_set_input_mode, METH_VARARGS,
   "P.set_input_mode(mode) -- set port input mode"},
  {"set_output_mode", (PyCFunction)Port_set_output_mode, METH_VARARGS,
   "P.set_output_mode(mode) -- set port output mode"},
  {NULL}  /* Sentinel */
};

/* Port sequence methods */
static PySequenceMethods PortAsSequence[] = {
        0,                           /* sq_length */
        0,                           /* sq_concat */
        0,                           /* sq_repeat */
        (intargfunc)Port_getitem,    /* sq_item */
        (intintargfunc)Port_getslice,/* sq_slice */
        0,                           /* sq_ass_item */
        0,                           /* sq_ass_slice */
        0,                           /* sq_contains */
        0,                           /* sq_inplace_concat */
        0                            /* sq_inplace_repeat */
};

/* Port table */
PyTypeObject PortType = {
  PyObject_HEAD_INIT(NULL)
  0,                         /*ob_size*/
  "parapin.Port",             /*tp_name*/
  sizeof(PortObject),        /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  (destructor)Port_dealloc, /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  0,                         /*tp_compare*/
  0,                         /*tp_repr*/
  0,                         /*tp_as_number*/
  PortAsSequence,           /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  0,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "Port(base, uid=-1, inmode=-1, outmode=-1) -> new port with 'base' address. If uid is given, will drop permissions after port initialization.",           
                             /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  Port_methods,             /* tp_methods */
  Port_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)Port_init,      /* tp_init */
  0                         /* tp_alloc */
};


/* Module functions */

/* pin_init_user */
static char parapin_pin_init_user__doc__[] = "int pin_init_user(int lp_base);";

static PyObject *
parapin_pin_init_user(PyObject *self, PyObject *args) {
  int lp_base;

  if (!PyArg_ParseTuple(args, "i", &lp_base))
    return NULL;

  if (pin_init_user(lp_base) < 0)
    return PyErr_SetFromErrno(parapin_error);
  
  Py_INCREF(Py_None);
  return Py_None;
};

/* set_pin */
static char parapin_set_pin__doc__[] = "void set_pin(int pins);";

static PyObject *
parapin_set_pin(PyObject *self, PyObject *args) {
  int pins;

  if (!PyArg_ParseTuple(args, "i", &pins))
    return NULL;

  set_pin(pins);
  
  Py_INCREF(Py_None);
  return Py_None;
};

/* clear_pin */
static char parapin_clear_pin__doc__[] = "void clear_pin(int pins);";

static PyObject *
parapin_clear_pin(PyObject *self, PyObject *args) {
  int pins;

  if (!PyArg_ParseTuple(args, "i", &pins))
    return NULL;

  clear_pin(pins);
  
  Py_INCREF(Py_None);
  return Py_None;
};

/* change_pin */
static char parapin_change_pin__doc__[] = "void change_pin(int pins, int state);";

static PyObject *
parapin_change_pin(PyObject *self, PyObject *args) {
  int pins, state;

  if (!PyArg_ParseTuple(args, "ii", &pins, &state))
    return NULL;

  change_pin(pins, state);
  
  Py_INCREF(Py_None);
  return Py_None;
};

/* pin_is_set */
static char parapin_pin_is_set__doc__[] = "void pin_is_set(int pins);";

static PyObject *
parapin_pin_is_set(PyObject *self, PyObject *args) {
  int pins;

  if (!PyArg_ParseTuple(args, "i", &pins))
    return NULL;

  return Py_BuildValue("b", pin_is_set(pins));

};

/* pulse_pin */
static char parapin_pulse_pin__doc__[] = "void pin_pulse(int pins, int pol, int secs, int nsecs, int n);";

static PyObject *
parapin_pulse_pin(PyObject *self, PyObject *args) {
  int pins, pol, secs, msecs, n;

  if (!PyArg_ParseTuple(args, "iiiii", &pins, &pol, &secs, &msecs, &n))
    return NULL;

  struct timespec sleeptime;

  sleeptime.tv_sec = secs;
  sleeptime.tv_nsec = msecs * 1000000;

  while (n > 0){
    change_pin(pins, pol);
    nanosleep(&sleeptime, NULL);
    n --;
    if (n <= 0)
      break;
    change_pin(pins, !pol);
    nanosleep(&sleeptime, NULL);
    n--;
  }

  Py_INCREF(Py_None);
  return Py_None;
};

/* pin_input_mode */
static char parapin_pin_input_mode__doc__[] = "void pin_input_mode(int pins);";

static PyObject *
parapin_pin_input_mode(PyObject *self, PyObject *args) {
  int pins;

  if (!PyArg_ParseTuple(args, "i", &pins))
    return NULL;

  pin_input_mode(pins);
  
  Py_INCREF(Py_None);
  return Py_None;
};

/* pin_output_mode */
static char parapin_pin_output_mode__doc__[] = "void pin_output_mode(int pins);";

static PyObject *
parapin_pin_output_mode(PyObject *self, PyObject *args) {
  int pins;

  if (!PyArg_ParseTuple(args, "i", &pins))
    return NULL;

  pin_output_mode(pins);
  
  Py_INCREF(Py_None);
  return Py_None;
};

/* pin_mode */
static char parapin_pin_mode__doc__[] = "void pin_mode(int pins, int mode);";

static PyObject *
parapin_pin_mode(PyObject *self, PyObject *args) {
  int pins, mode;

  if (!PyArg_ParseTuple(args, "ii", &pins, &mode))
    return NULL;

  pin_mode(pins, mode);
  
  Py_INCREF(Py_None);
  return Py_None;
};


/* Functions listing */
static PyMethodDef module_methods[] = {
  {"pin_init_user", parapin_pin_init_user, METH_VARARGS, parapin_pin_init_user__doc__},
  {"set_pin", parapin_set_pin, METH_VARARGS, parapin_set_pin__doc__},
  {"clear_pin", parapin_clear_pin, METH_VARARGS, parapin_clear_pin__doc__},
  {"change_pin", parapin_change_pin, METH_VARARGS, parapin_change_pin__doc__},
  {"pulse_pin", parapin_pulse_pin, METH_VARARGS, parapin_pulse_pin__doc__},
  {"pin_is_set", parapin_pin_is_set, METH_VARARGS, parapin_pin_is_set__doc__},
  {"pin_input_mode", parapin_pin_input_mode, METH_VARARGS, parapin_pin_input_mode__doc__},
  {"pin_output_mode", parapin_pin_output_mode, METH_VARARGS, parapin_pin_output_mode__doc__},
  {"pin_mode", parapin_pin_mode, METH_VARARGS, parapin_pin_mode__doc__},
  
  /* Sentinel */
  {NULL, NULL, 0, NULL}
};


#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initparapin(void) 
{
  PyObject *module, *lp_pin, *constants;
  
  PortType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PortType) < 0)
    return;

  PinType.tp_new = PyType_GenericNew;
  if (PyType_Ready(&PinType) < 0)
    return;
  
  module = Py_InitModule3("parapin", module_methods,
			  "parapin module.");
  
  constants = Py_InitModule3("parapin.CONST", NULL, "Parapin constants");

  if ((module == NULL) | (constants == NULL))
      return;
  

  /* ParapinError exception */
  parapin_error = PyErr_NewException("parapin.ParapinError", NULL, NULL);
  Py_INCREF(parapin_error);
  PyModule_AddObject(module, "ParapinError", parapin_error);

  /* Port type */
  Py_INCREF(&PortType);
  PyModule_AddObject(module, "Port", (PyObject *)&PortType);

  /* Pin type */
  Py_INCREF(&PinType);
  PyModule_AddObject(module, "Pin", (PyObject *)&PinType);

  /* constants */
  Py_INCREF(constants);
  PyModule_AddObject(module, "CONST", (PyObject *)constants);

  /* constants */
  PyModule_AddIntConstant(constants, "LPT1", LPT1);
  PyModule_AddIntConstant(constants, "LPT2", LPT2);
  PyModule_AddIntConstant(constants, "LP_CLEAR", LP_CLEAR);
  PyModule_AddIntConstant(constants, "LP_SET", LP_SET);
  PyModule_AddIntConstant(constants, "LP_PIN01", LP_PIN01);
  PyModule_AddIntConstant(constants, "LP_PIN02", LP_PIN02);
  PyModule_AddIntConstant(constants, "LP_PIN03", LP_PIN03);
  PyModule_AddIntConstant(constants, "LP_PIN04", LP_PIN04);
  PyModule_AddIntConstant(constants, "LP_PIN05", LP_PIN05);
  PyModule_AddIntConstant(constants, "LP_PIN06", LP_PIN06);
  PyModule_AddIntConstant(constants, "LP_PIN07", LP_PIN07);
  PyModule_AddIntConstant(constants, "LP_PIN08", LP_PIN08);
  PyModule_AddIntConstant(constants, "LP_PIN09", LP_PIN09);
  PyModule_AddIntConstant(constants, "LP_PIN10", LP_PIN10);
  PyModule_AddIntConstant(constants, "LP_PIN11", LP_PIN11);
  PyModule_AddIntConstant(constants, "LP_PIN12", LP_PIN12);
  PyModule_AddIntConstant(constants, "LP_PIN13", LP_PIN13);
  PyModule_AddIntConstant(constants, "LP_PIN14", LP_PIN14);
  PyModule_AddIntConstant(constants, "LP_PIN15", LP_PIN15);
  PyModule_AddIntConstant(constants, "LP_PIN16", LP_PIN16);
  PyModule_AddIntConstant(constants, "LP_PIN17", LP_PIN17);
  PyModule_AddIntConstant(constants, "LP_IRQ_MODE", LP_IRQ_MODE);
  PyModule_AddIntConstant(constants, "LP_INPUT_MODE", LP_INPUT_MODE);
  PyModule_AddIntConstant(constants, "LP_ALWAYS_INPUT_PINS",LP_ALWAYS_INPUT_PINS);
  PyModule_AddIntConstant(constants, "LP_DATA_PINS", LP_DATA_PINS);
  PyModule_AddIntConstant(constants, "LP_SWITCHABLE_PINS", LP_SWITCHABLE_PINS);

  
  lp_pin = Py_BuildValue("(i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i, i)", 
                         0, 
                         LP_PIN01, 
                         LP_PIN02, 
                         LP_PIN03, 
                         LP_PIN04, 
                         LP_PIN05, 
                         LP_PIN06, 
                         LP_PIN07, 
                         LP_PIN08, 
                         LP_PIN09, 
                         LP_PIN10, 
                         LP_PIN11, 
                         LP_PIN12, 
                         LP_PIN13, 
                         LP_PIN14, 
                         LP_PIN15, 
                         LP_PIN16, 
                         LP_PIN17,
                         0,
                         0,
                         0,
                         0,
                         0,
                         0,
                         0,
                         0);
  Py_INCREF(lp_pin);
  PyModule_AddObject(constants, "LP_PIN", lp_pin);


  /* Check for errors */
  if (PyErr_Occurred())
    Py_FatalError("can't initialize module parapin");

}

