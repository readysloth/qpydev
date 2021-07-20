import re
import typing as t
import subprocess as sp

from textwrap import dedent
from pathlib import Path


def get_generators(device_name: str):

    device_class_name = device_name.replace('_', ' ').title().replace(' ', '_')
    head = dedent("""
    #include "qemu/osdep.h"
    #include "qapi/error.h"

    #include <Python.h>

    {body}
    """)

    def indent_body(body_lines: t.List[str]):
        return ('\n' + ' '*12).join(body_lines)


    def generate_function(func_name: str,
                          ret_type: str,
                          args: str,
                          body: t.List[str]):
        return dedent(f"""
        static {ret_type} {device_name}_{func_name}({args}){{
            {indent_body(body)}
        }}
        """)


    def generate_device_struct(fields: dict):
        struct_fields = [f"{type} {field};" for field, type in fields.items()]

        return dedent(f"""
        typedef struct {{
            {indent_body(struct_fields)}
        }} {device_class_name};
        """)


    def generate_properties(properties: t.List[dict]):
        fields = []
        for p in properties:
            ptype = p['type'].upper()
            pname = f"\"{p['name']}\""
            pfield = p['field']
            dev_name = device_class_name
            default_value = ''

            if 'default_value' in p:
                default_value = ', ' + str(p['default_value'])

            fields.append(f"DEFINE_PROP_{ptype}({pname}, {dev_name}, {pfield}{default_value}),")

        return dedent(f"""
        static Property {device_name}_properties[] = {{
            {indent_body(fields)}
            DEFINE_PROP_END_OF_LIST(),
        }};
        """)


    def patch_configure(qemu_path: Path, python_version: str):
        configure_path = qemu_path / Path("configure")
        with open(configure_path, 'r') as f:
            python_config = f"python{python_version}-config"
            cflags = sp.check_output([python_config, '--cflags']).decode().strip()
            ldflags = sp.check_output([python_config, '--ldflags']).decode().strip()

            qemu_flags_alternation = f'QEMU_CFLAGS="$QEMU_CFLAGS {cflags}"; QEMU_LDFLAGS="$QEMU_CFLAGS {ldflags}"'
            configure = f.read()
            configure = re.sub('kvm="auto"',
                               f'\\g<0>\n{device_name}="disabled"',
                               configure)
            configure = re.sub('--enable-kvm.*',
                               f'\\g<0>\n;;\n--enable-{device_name}) {qemu_flags_alternation}',
                               configure)
            configure = re.sub('Advanced options.*',
                               f'\\g<0>\n  --enable-{device_name} enables {device_name} device',
                               configure)


    return {'func' : generate_function,
            'struct' : generate_device_struct,
            'properties' : generate_properties,
            'patch' : {'configure' : patch_configure}}
