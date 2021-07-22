import os
import sys
import json
import subprocess as subp

from pathlib import Path

import schema_parser as sp


def get_nested_schema(schema: dict, key: str):
    return schema[key]['schema'][key]


def get_device_class_name(schema: dict):
    return schema['name'].replace('_', ' ').title().replace(' ', '')


def get_functions_from_schema(schema: dict):
    pass


DEV_SCHEMA_FILE = 'concrete_dev_schema.json'

if __name__ == '__main__':
    ENV = os.environ.copy()
    ENV["PYTHONPATH"] = Path('.').absolute()
    SCHEMA = sp.load_schema(sys.argv[1])

    with open(DEV_SCHEMA_FILE, 'w') as cds:
        json.dump(SCHEMA, cds)


    subp.run(['cog', '-d', '-o', SCHEMA["name"] + '.c', sys.argv[2]],
             env=ENV)
    os.remove(DEV_SCHEMA_FILE)
