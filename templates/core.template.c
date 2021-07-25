#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qom/object.h"
/*[[[cog
    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    device_header = SCHEMA["parent"]["header"]
    cog.outl(f'#include "{device_header}"')
   ]]]*/
/*[[[end]]]*/
#include "exec/address-spaces.h"
#include "hw/qdev-properties-system.h"

#include <Python.h>


/*[[[cog
    #
    # generating device class
    #

    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)


    device_class       = m.get_device_class_name(SCHEMA, full = True)
    device_instance    = m.get_device_class_name(SCHEMA)
    device_parent      = SCHEMA["parent"]["name"]

    cog.outl(f"typedef struct {{}} {device_instance};")

    cog.outl("typedef struct {")
    cog.outl(f"    {device_parent}Device parent_obj;")
    cog.outl()
    for fname, ftype in m.get_nested_schema(SCHEMA, 'device').items():
        cog.outl(f"    {ftype} {fname};")
    cog.outl(f"}} {device_class};")
  ]]]*/
/*[[[end]]]*/


/*[[[cog
    #
    # generating device type
    #

    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    device_instance    = m.get_device_class_name(SCHEMA)
    device_name        = SCHEMA["name"]
    device_parent_type = SCHEMA["parent"]["type"]
    device_qtype       = SCHEMA["name"].upper()

    cog.outl(f'#define TYPE_{device_qtype} "{device_name}"')
    cog.outl(f"OBJECT_DEFINE_TYPE({device_instance}, {device_name}, {device_qtype}, {device_parent_type}_DEVICE)")
  ]]]*/
/*[[[end]]]*/


/*[[[cog
    #
    # generating device properties
    #

    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    device_name = SCHEMA['name']

    cog.outl(f"static Property {device_name}_properties[] = {{")
    for p in m.get_nested_schema(SCHEMA, 'properties'):
        ptype         = p["type"].upper()
        pname         = f"\"{p['name']}\""
        pfield        = p["field"]
        dev_name      = m.get_device_class_name(SCHEMA)
        default_value = ''

        if "default_value" in p:
            default_value = ', ' + str(p["default_value"])

        cog.outl(f"    DEFINE_PROP_{ptype}({pname}, {dev_name}, {pfield}{default_value}),")
    cog.outl("    DEFINE_PROP_END_OF_LIST()")
    cog.outl("};")
  ]]]*/
/*[[[end]]]*/


/*[[[cog
    #
    # generating device and instance constructors and destructors
    #

    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    def offset_text_by(initiator, text):
        return len(initiator)*' ' + text

    device_name              = SCHEMA['name']
    class_init_proto    = m.create_device_method_proto(SCHEMA, "void", "class_init", "ObjectClass *oc, void *data")
    instance_init_proto = m.create_device_method_proto(SCHEMA, "void", "init", "Object *obj")
    instance_finalize_proto = m.create_device_method_proto(SCHEMA, "void", "finalize", "Object *obj")

    cog.outl(f"{class_init_proto}{{")

    for entry in SCHEMA["class"]["schema"]["init"]["class_field_init"]:
        cog.outl()
        dev_cast   = entry["dev_cast"]
        cast_type  = dev_cast['type']
        cast_macro = dev_cast['cast']

        cog.outl(f"    {cast_type} *{cast_type}_ptr = {cast_macro}(oc);")

        for k,v in entry.items():
            if k == "dev_cast":
                continue
            cog.outl(f"    {cast_type}_ptr->{k} = {v};")

    cog.outl()
    cog.outl(f'    Py_SetProgramName("{device_name}");')
    cog.outl(f'    Py_Initialize();')
    cog.outl("}")
    cog.outl()
    cog.outl()

    cog.outl(f"{instance_init_proto}{{")

    init_func_name = SCHEMA["class"]["schema"]["instance_init"]
    instance_init_source = m.get_function_text_from(SCHEMA["class"], init_func_name)

    py_code_start = f"    static char *py_code = "
    cog.out(py_code_start + f'"{instance_init_source[0]}"\n')
    for line in instance_init_source[1:-1]:
        cog.out(offset_text_by(py_code_start, f'"{line}"\n'))
    cog.out(offset_text_by(py_code_start, f'"{instance_init_source[-1]}";'))

    cog.outl()
    cog.outl(f"    PyObject *p_compiled_code     = Py_CompileString(py_code, NULL, 0);")
    cog.outl(f'    PyObject *p_func              = PyObject_GetAttrString(p_compiled_code, "{init_func_name}");')
    cog.outl(f'    PyObject *p_func_args         = PyTuple_New(1);')
    cog.outl(f'    PyObject *p_dev_obj_container = PyBytes_FromStringAndSize(obj, sizeof(Object));')
    cog.outl(f'    PyTuple_SetItem(p_func_args, 0, p_dev_obj_container);')
    cog.outl(f'    PyObject_CallObject(p_func_args, p_func_args);')
    cog.outl("}")

    cog.outl(f"{instance_finalize_proto}{{")
    cog.outl("}")
  ]]]*/
/*[[[end]]]*/


/*[[[cog
    #
    # generating device methods
    #

    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    read_func_proto  = m.create_device_method_proto(SCHEMA, "uint64_t", "read", "void *opaque, hwaddr addr, unsigned size")
    write_func_proto = m.create_device_method_proto(SCHEMA, "void", "write", "void *opaque, hwaddr addr, uint64_t data, unsigned size")

    cog.outl(f"{read_func_proto};")
    cog.outl(f"{write_func_proto};")
    cog.outl()
    cog.outl()
    cog.outl(f"{read_func_proto}{{")
    cog.outl("}")
    cog.outl()
    cog.outl()
    cog.outl(f"{write_func_proto}{{")
    cog.outl("}")
  ]]]*/
/*[[[end]]]*/
