#include <Python.h>

#include "ncx.h"
#include "ncxmod.h"
#include "ncxtypes.h"
#include "status.h"

#include "procdefs.h"
#include "agt.h"
#include "agt_cb.h"
#include "agt_timer.h"
#include "agt_util.h"
#include "agt_rpc.h"
#include "dlq.h"
#include "rpc.h"


static PyObject *
yuma_init(PyObject *self, PyObject *args)
{
    status_t res;
    int argc=1;
    char* argv[] = {"hello"};
    char buff[] = "yuma for python";
    res = ncx_init(FALSE, LOG_DEBUG_INFO, TRUE, buff, argc, argv);

    res = ncxmod_load_module( NCXMOD_YUMA_NETCONF, NULL, NULL, NULL );
    assert(res == 0);            
    res = ncxmod_load_module( NCXMOD_NETCONFD, NULL, NULL, NULL );
    assert(res == 0);

    return Py_BuildValue("i", (int)res);
}

static PyObject *
yuma_schema_module_load(PyObject *self, PyObject *args)
{
    status_t res;
    PyObject *py_retval;
    ncx_module_t* mod;
    const char* mod_name_str;
    if (!PyArg_ParseTuple(args, (char *) "s:yuma_schema_module_load", &mod_name_str)) {
        return (NULL);
    }

    res = ncxmod_load_module( (const xmlChar*)mod_name_str, NULL, NULL, &mod);

    py_retval = PyTuple_New(2);
    PyTuple_SetItem(py_retval, 0, Py_BuildValue("i", (int)res));
    PyTuple_SetItem(py_retval, 1, Py_BuildValue("O", PyCapsule_New(mod, "ncx_module_t_ptr", NULL)));
    return py_retval;
}

static PyObject *
yuma_cfg_load(PyObject *self, PyObject *args)
{
    status_t res;
    PyObject *py_retval;
    int argc=1;
    char* argv[] = {"hello"};
    char* cfg_filename_str;
    char* startup_arg;
    boolean showver;
    help_mode_t showhelpmode;
    agt_profile_t *profile;
    cfg_template_t  *runningcfg;

    if (!PyArg_ParseTuple(args, (char *) "s:yuma_cfg_load", &cfg_filename_str)) {
        return (NULL);
    }

    startup_arg = malloc(strlen(cfg_filename_str)+1);
    sprintf(startup_arg,"%s",cfg_filename_str);

    res = agt_init1(argc, argv, &showver, &showhelpmode);
    assert(res == 0);

    if (showver || showhelpmode != HELP_MODE_NONE) {
        printf("ver 1.0\n");
    }

    profile = agt_get_profile();
    res = ncxmod_load_module( NCXMOD_WITH_DEFAULTS, NULL, 
                                      &profile->agt_savedevQ, NULL );

    assert(res == 0);

    profile->agt_has_startup=TRUE;
    profile->agt_startup=startup_arg;
    
    res = agt_init2();
    assert(res == 0);

    runningcfg = cfg_get_config(NCX_CFG_RUNNING);
    assert(runningcfg!=NULL);

    py_retval = PyTuple_New(2);
    PyTuple_SetItem(py_retval, 0, Py_BuildValue("i", (int)res));
    PyTuple_SetItem(py_retval, 1, Py_BuildValue("O", PyCapsule_New(runningcfg->root, "val_value_t_ptr", NULL)));
    return py_retval;
}

static PyObject *
yuma_val_find_child(PyObject *self, PyObject *args)
{
    PyObject *py_retval;
    PyObject *py_parent_val;
    int res;
    val_value_t* parent_val;
    val_value_t* child_val;
    char* namespace;
    char* name;

    if (!PyArg_ParseTuple(args, (char *) "Oss:yuma_val_find_child", &py_parent_val,&namespace,&name)) {
        return (NULL);
    }
    parent_val = (val_value_t*)PyCapsule_GetPointer(py_parent_val, "val_value_t_ptr");
    child_val=val_find_child(parent_val, namespace, name);
    if(child_val==NULL) {
        res=-1;
    } else {
        res=0;
    }
    py_retval = PyTuple_New(2);
    PyTuple_SetItem(py_retval, 0, Py_BuildValue("i", (int)res));
    PyTuple_SetItem(py_retval, 1, Py_BuildValue("O", PyCapsule_New(child_val, "val_value_t_ptr", NULL)));
    return py_retval;
}

static PyObject *
yuma_val_string(PyObject *self, PyObject *args)
{
    PyObject *py_val;
    val_value_t* val;
    if (!PyArg_ParseTuple(args, (char *) "O:yuma_val_string", &py_val)) {
        return (NULL);
    }
    val = (val_value_t*)PyCapsule_GetPointer(py_val, "val_value_t_ptr");
    
    return Py_BuildValue("s", VAL_STRING(val));
}

static PyObject *
yuma_val_dump_value(PyObject *self, PyObject *args)
{
    PyObject *py_val;
    int res;
    val_value_t* val;
    int flag;

    if (!PyArg_ParseTuple(args, (char *) "Oi:yuma_val_dump_value", &py_val,&flag)) {
        return (NULL);
    }
    val = (val_value_t*)PyCapsule_GetPointer(py_val, "val_value_t_ptr");
    val_dump_value(val,flag);
    Py_RETURN_NONE;
}

/*  define functions in module */
static PyMethodDef YumaMethods[] =
{
     {"init", yuma_init, METH_VARARGS, "initialization"},
     {"schema_module_load", yuma_schema_module_load, METH_VARARGS, "load schema module"},
     {"cfg_load", yuma_cfg_load, METH_VARARGS, "load configuration"},
     {"val_find_child", yuma_val_find_child, METH_VARARGS, "find child of parent val"},
     {"val_string", yuma_val_string, METH_VARARGS, "get value of val represented as string"},
     {"val_dump_value", yuma_val_dump_value, METH_VARARGS, "dump the value of the provided variable to stdout"},
     {NULL, NULL, 0, NULL}
};

/* module initialization */
PyMODINIT_FUNC
inityuma(void)
{
    (void) Py_InitModule("yuma", YumaMethods);
}

