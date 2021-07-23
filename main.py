import os
import sys
import json
import inspect
import importlib.util
import subprocess as subp

from pathlib import Path

import schema_parser as sp


def get_nested_schema(schema: dict, key: str):
    return schema[key]['schema'][key]


def get_device_class_name(schema: dict):
    return schema['name'].replace('_', ' ').title().replace(' ', '')


def get_function_text_from(sub_schema: dict, method_name: str):
    spec = importlib.util.spec_from_file_location("py_code", sub_schema["code"])
    py_code = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(py_code)
    return inspect.getsource(getattr(py_code, method_name)).split('\n')


def get_python_prologue(schema: dict):
    return f'Py_SetProgramName("{schema["name"]}");'


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
