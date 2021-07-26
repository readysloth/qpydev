import os
import sys
import json
import inspect
import importlib.util
import subprocess as subp

from pathlib import Path
from textwrap import dedent

import schema_parser as sp


def get_nested_schema(schema: dict, key: str):
    return schema[key]['schema'][key]


def get_device_class_name(schema: dict, full=False):
    base_class_name = schema["name"].replace('_', ' ').title().replace(' ', '')
    if full:
        return base_class_name + "Class"
    return base_class_name


def get_function_text_from(sub_schema: dict, method_name: str):
    spec = importlib.util.spec_from_file_location("py_code", sub_schema["code"])
    py_code = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(py_code)
    return inspect.getsource(getattr(py_code, method_name)).split('\n')


def get_python_c_api_wrap(schema: dict,
                          _from: str,
                          func_name: str,
                          tuple_size: int,
                          code_for_insert: str):
    c_char_source = [l + '\\n' for l in get_function_text_from(schema[_from], func_name)]

    py_code = f"static char *py_code = "
    py_code_start = len(py_code) + 8

    py_code += f'"{c_char_source[0]}"\n'
    for line in c_char_source[1:-1]:
        py_code += py_code_start*' ' + f'"{line}"\n'
    py_code += py_code_start*' ' + f'"{c_char_source[-1]}";'

    return dedent(f"""
        {py_code}
        PyObject *p_compiled_code     = Py_CompileString(py_code, "{func_name}.py", Py_single_input);
        if (!p_compiled_code) goto err;

        PyObject *p_module            = PyImport_ExecCodeModule("{func_name}_module", p_compiled_code);
        if (!p_module) goto err;

        PyObject *p_func              = PyObject_GetAttrString(p_module, "{func_name}");
        if (!p_func) goto err;

        PyObject *p_func_args         = PyTuple_New({tuple_size});
        if (!p_func_args) goto err;
        {code_for_insert}
        PyObject *p_ret = PyObject_CallObject(p_func, p_func_args);
        if (!p_ret) goto err;
        return;

    err:
        PyErr_Print();
        abort();
    """)


def get_method_name(schema: dict,
                    method_name: str):
    return f"{schema['name']}_{method_name}"


def create_device_method_proto(schema: dict,
                               ret_type: str,
                               method_name: str,
                               args: str,
                               static=True):
    method_proto = f"{ret_type} {get_method_name(schema, method_name)}({args})"
    if static:
        return f"static {method_proto}"
    return method_proto


DEV_SCHEMA_FILE = 'concrete_dev_schema.json'

if __name__ == '__main__':
    ENV = os.environ.copy()
    ENV["PYTHONPATH"] = Path('.').absolute()
    SCHEMA = sp.load_schema(sys.argv[1])

    with open(DEV_SCHEMA_FILE, 'w') as cds:
        json.dump(SCHEMA, cds, indent=2)


    subp.run(['cog', '-d', '-o', SCHEMA["name"] + '.c', sys.argv[2]],
             env=ENV)
