#include <Python.h>

#include "ncx.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"

#include "procdefs.h"
#include "dlq.h"
#include "rpc.h"
#include "yangrpc.h"

static int yangrpc_init_done = 0;

static PyObject *
py_yangrpc_connect(PyObject *self, PyObject *args)
{
    int res;
    char* server;
    int port;
    char* user;
    char* password;
    char* private_key;
    char* public_key;
    yangrpc_cb_t *yangrpc_cb;


    if (!PyArg_ParseTuple(args, (char *) "sissss:yangrpc_yangrpc_connect", &server,&port,&user,&password,&public_key,&private_key)) {
        return (NULL);
    }
    if(yangrpc_init_done==0) {
        yangrpc_init();
        yangrpc_init_done=1;
    }

    yangrpc_cb = yangrpc_connect(server, port, user, password, NULL/*public_key*/, NULL/*private_key*/);

    return Py_BuildValue("O", PyCapsule_New(yangrpc_cb, "yangrpc_cb_t_ptr", NULL));
}

static PyObject *
py_yangrpc_exec(PyObject *self, PyObject *args)
{
    PyObject *py_retval;

    PyObject *py_yangrpc_cb;
    PyObject *py_rpc_val;

    int res;
    yangrpc_cb_t *yangrpc_cb;
    val_value_t  *rpc_val;
    val_value_t  *reply_val;

    if (!PyArg_ParseTuple(args, (char *) "OO:yangrpc_yangrpc_exec", &py_yangrpc_cb, &py_rpc_val)) {
        return (NULL);
    }

    yangrpc_cb = (val_value_t*)PyCapsule_GetPointer(py_yangrpc_cb, "yangrpc_cb_t_ptr");
    rpc_val = (val_value_t*)PyCapsule_GetPointer(py_rpc_val, "val_value_t_ptr");

    res = yangrpc_exec(yangrpc_cb, rpc_val, &reply_val);

    py_retval = PyTuple_New(2);
    PyTuple_SetItem(py_retval, 0, Py_BuildValue("i", (int)res));
    PyTuple_SetItem(py_retval, 1, Py_BuildValue("O", PyCapsule_New(reply_val, "val_value_t_ptr", NULL)));
    return py_retval;
}

static PyObject *
py_yangrpc_parse_cli(PyObject *self, PyObject *args)
{
    PyObject *py_retval;

    PyObject *py_yangrpc_cb;
    PyObject *py_rpc_val;

    int res;
    yangrpc_cb_t *yangrpc_cb;
    val_value_t  *rpc_val;
    char* cmd;    

    if (!PyArg_ParseTuple(args, (char *) "Os:yangrpc_yangrpc_parse_cli", &py_yangrpc_cb, &cmd)) {
        return (NULL);
    }

    yangrpc_cb = (val_value_t*)PyCapsule_GetPointer(py_yangrpc_cb, "yangrpc_cb_t_ptr");

    res = yangrpc_parse_cli(yangrpc_cb, cmd, &rpc_val);

    py_retval = PyTuple_New(2);
    PyTuple_SetItem(py_retval, 0, Py_BuildValue("i", (int)res));
    PyTuple_SetItem(py_retval, 1, Py_BuildValue("O", PyCapsule_New(rpc_val, "val_value_t_ptr", NULL)));
    return py_retval;
}

/*  define functions in module */
static PyMethodDef yang_rpc_methods[] =
{
     {"connect", py_yangrpc_connect, METH_VARARGS, "connects to client"},
     {"rpc", py_yangrpc_exec, METH_VARARGS, "executes rpc by sending the rpc data contents and blocking until reply is received"},
     {"parse_cli", py_yangrpc_parse_cli, METH_VARARGS, "converts cli string to rpc data contents"},
     {NULL, NULL, 0, NULL}
};

/* module initialization */
PyMODINIT_FUNC
inityangrpc(void)
{
    (void) Py_InitModule("yangrpc", yang_rpc_methods);
}
