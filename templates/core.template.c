#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qom/object.h"

#include <Python.h>

/*[[[cog
    #
    # generating device type
    #

    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    cog.outl(f"OBJECT_DECLARE_SIMPLE_TYPE({m.get_device_class_name(SCHEMA)}, {SCHEMA['name'].upper()})")
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

    cog.outl(f"static Property {SCHEMA['name']}_properties[]{{")
    for p in m.get_nested_schema(SCHEMA, 'properties'):
        ptype = p["type"].upper()
        pname = f"\"{p['name']}\""
        pfield = p["field"]
        dev_name = m.get_device_class_name(SCHEMA)
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
    # generating device class
    #

    import json as j
    import main as m

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    cog.outl("typedef struct {")
    for fname, ftype in m.get_nested_schema(SCHEMA, 'device').items():
        cog.outl(f"    {ftype} {fname};")
    cog.outl(f"}} {m.get_device_class_name(SCHEMA)};")
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

    class_init_func_proto = m.create_device_method_proto(SCHEMA, "void", "class_init", "ObjectClass *oc, void *data")
    instance_init_func_proto = m.create_device_method_proto(SCHEMA, "void", "instance_init", "Object *obj")

    cog.outl(f"{class_init_func_proto};")
    cog.outl(f"{instance_init_func_proto};")
    cog.outl()
    cog.outl()

    cog.outl(f"{class_init_func_proto}{{")

    for entry in SCHEMA["class"]["schema"]["init"]["class_field_init"]:
        cog.outl()
        dev_cast = entry["dev_cast"]
        cog.outl(f"    {dev_cast} *{dev_cast}_ptr = {dev_cast}(oc);")

        for k,v in entry.items():
            if k == "dev_cast":
                continue
            cog.outl(f"    {dev_cast}_ptr->{k} = {v};")
    cog.outl("}")
    cog.outl()
    cog.outl()

    cog.outl(f"{instance_init_func_proto}{{")

    instance_init_source = m.get_function_text_from(SCHEMA['class'],
                                                    SCHEMA['class']['schema']['instance_init'])

    py_code_start = f"    char *py_code = "

    cog.out(py_code_start + f'"{instance_init_source[0]}"\n')
    for line in instance_init_source[1:-1]:
        cog.out(offset_text_by(py_code_start, f'"{line}"\n'))
    cog.out(offset_text_by(py_code_start, f'"{instance_init_source[-1]}";'))
    cog.outl()

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

    read_func_proto = m.create_device_method_proto(SCHEMA, "uint64_t", "read", "void *opaque, hwaddr addr, unsigned size")
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
