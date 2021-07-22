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
        ptype = p['type'].upper()
        pname = f"\"{p['name']}\""
        pfield = p['field']
        dev_name = m.get_device_class_name(SCHEMA)
        default_value = ''

        if 'default_value' in p:
            default_value = ', ' + str(p['default_value'])

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
