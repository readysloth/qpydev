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

#define IF_NULL_GOTO_ERR(VAR, BODY) \
    BODY; \
    if (!VAR){ \
        goto err; \
    }

#define PY_COMPILE_AND_GET_FUNC(FUNC_NAME, PY_CODE, PY_COMPILED, PY_MODULE, PY_FUNC) \
    if (!PY_COMPILED){ \
        IF_NULL_GOTO_ERR(PY_COMPILED, \
                         PY_COMPILED = Py_CompileString(PY_CODE, FUNC_NAME ".py", Py_single_input)) \
    } \
    IF_NULL_GOTO_ERR(PY_MODULE, \
                     PY_MODULE = PyImport_ExecCodeModule(FUNC_NAME "module", PY_COMPILED)) \
    IF_NULL_GOTO_ERR(PY_FUNC, \
                     PY_FUNC = PyObject_GetAttrString(PY_MODULE, FUNC_NAME))


typedef struct {
    char *name;
    size_t size;
} DeviceFieldMetaInfo;


/*[[[cog
    #
    # generating device class
    #

    import json as j
    import main as m
    from textwrap import indent, dedent
    INDENT = lambda s: indent(s, ' '*4)

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)


    device_class    = m.get_device_class_name(SCHEMA, full = True)
    device_instance = m.get_device_class_name(SCHEMA)
    device_parent   = SCHEMA["parent"]["name"]


    cog.outl("typedef struct {")
    cog.outl(INDENT(f"{device_parent}Device parent_obj;"))
    cog.outl(f"}} {device_class};")
    cog.outl()
    cog.outl(f"typedef struct __attribute__((packed)) {{")
    for fname, ftype in m.get_nested_schema(SCHEMA, "device").items():
        cog.outl(INDENT(f"{ftype} {fname};"))
    cog.outl(f"}} {device_instance};")

    cog.outl()
    cog.outl()
    cog.outl(f"DeviceFieldMetaInfo device_fields[] = {{")
    for fname, ftype in m.get_nested_schema(SCHEMA, "device").items():
        cog.outl(INDENT(f'{{ "{fname}", sizeof({ftype}) }},'))
    cog.outl(INDENT('};'))
  ]]]*/
/*[[[end]]]*/

#define DEVICE_FIELDS_COUNT sizeof(device_fields)/sizeof(device_fields[0])

static PyObject* create_dict_for_class_fields(){
    PyObject* p_field_dicts[DEVICE_FIELDS_COUNT];
    PyObject *p_meta_info_aggregation = PyDict_New();
    for(int i = 0; i < DEVICE_FIELDS_COUNT; i++){
        int err = PyDict_Merge(p_meta_info_aggregation,
                               Py_BuildValue("{s:i}",
                                             device_fields[i].name,
                                             device_fields[i].size),
                               1);
        if(err){
            PyErr_Print();
            abort();
        }
    }
    return p_meta_info_aggregation;
}

PyObject *DictWithClassFields;

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
    device_interface   = SCHEMA["parent"]["interface"]
    device_qtype       = SCHEMA["name"].upper()

    cog.outl(f'#define TYPE_{device_qtype} "{device_name}"')
    cog.outl(f"""OBJECT_DEFINE_TYPE_WITH_INTERFACES({device_instance},
                                       {device_name},
                                       {device_qtype},
                                       {device_parent_type}_DEVICE,
                                       {{ INTERFACE_{device_interface} }},
                                       {{ NULL }})""")
  ]]]*/
/*[[[end]]]*/

/*[[[cog
    #
    # generating function prototypes
    #

    import json as j
    import main as m
    from textwrap import indent, dedent
    INDENT = lambda s: indent(s, ' '*4)

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    class_methods = SCHEMA["class"]["schema"]["methods"]
    class_init = m.create_device_method_proto(SCHEMA, "void", "class_init", "ObjectClass *oc, void *data")
    cog.outl(f"{class_init};")

    for method in class_methods:
        cog.outl(f'{m.create_device_method_proto(SCHEMA, method["c_ret"], method["c_name"], method["c_args"])};')
  ]]]*/
/*[[[end]]]*/

/*[[[cog
    #
    # generating device properties
    #

    import json as j
    import main as m
    from textwrap import indent, dedent
    INDENT = lambda s: indent(s, ' '*4)

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

        cog.outl(INDENT(f"DEFINE_PROP_{ptype}({pname}, {dev_name}, {pfield}{default_value}),"))
    cog.outl(INDENT("DEFINE_PROP_END_OF_LIST()"))
    cog.outl("};")
  ]]]*/
/*[[[end]]]*/

/*[[[cog
    #
    # generating device MemoryRegionOps
    #

    import json as j
    import main as m
    from textwrap import indent, dedent
    INDENT = lambda s: indent(s, ' '*4)

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)

    device_name = SCHEMA['name']

    for k,v in m.get_nested_schema(SCHEMA, "device").items():
        if v == "MemoryRegionOps":
            cog.outl(f"static const MemoryRegionOps {device_name}_mem_ops = {{")
            for mem_k, mem_v in SCHEMA["device"]["schema"][k].items():
                cog.outl(INDENT(f".{mem_k} = {mem_v},"))

            cog.outl(INDENT(f".read = {m.get_method_name(SCHEMA, 'read')},"))
            cog.outl(INDENT(f".write = {m.get_method_name(SCHEMA, 'write')},"))

            cog.outl("};")
  ]]]*/
/*[[[end]]]*/

/*[[[cog
    #
    # generating device and instance methods
    #

    import re
    import json as j
    import main as m

    from textwrap import indent, dedent
    INDENT = lambda s: indent(s, ' '*4)
    ALIGN_INDENT_BY = lambda s: ' ' * len(s)

    with open(m.DEV_SCHEMA_FILE, 'r') as sf:
        SCHEMA = j.load(sf)


    device_name      = SCHEMA["name"]
    class_init_proto = m.create_device_method_proto(SCHEMA, "void", "class_init", "ObjectClass *oc, void *data")

    cog.outl(f"{class_init_proto}{{")

    class_field_init = SCHEMA["class"]["schema"]["init"]["class_field_init"]

    if not any(filter(lambda cast: cast["dev_cast"]["type"] == "DeviceClass",
                      class_field_init)):
        default_dev_cast = {"dev_cast": {"type" : "DeviceClass", "cast": "DEVICE_CLASS"}}
        class_field_init = [default_dev_cast] + class_field_init

    for entry in class_field_init:
        cog.outl()
        dev_cast   = entry["dev_cast"]
        cast_type  = dev_cast['type']
        cast_macro = dev_cast['cast']

        cog.outl(INDENT(f"{cast_type} *{cast_type}_ptr = {cast_macro}(oc);"))

        for k,v in entry.items():
            if k == "dev_cast":
                continue
            cog.outl(INDENT(f"{cast_type}_ptr->{k} = {v};"))


    cog.outl(INDENT(f"device_class_set_props(DeviceClass_ptr, {device_name}_properties);"))

    cog.outl()
    cog.outl(INDENT(f'Py_SetProgramName("{device_name}");'))
    cog.outl(INDENT(f'Py_Initialize();'))
    cog.outl(INDENT(f'DictWithClassFields = create_dict_for_class_fields();'))
    cog.outl("}")
    cog.outl()
    cog.outl()

    for method in class_methods:
        method_proto = m.create_device_method_proto(SCHEMA, method["c_ret"], method["c_name"], method["c_args"])
        argc = method["c_args"].count(',') + 1
        arguments = [s.strip() for s in method["c_args"].split(',')]
        casts = [s.strip() for s in method["c_to_py_cast"].split(',')]

        argument_type_val = [arg.split() for arg in arguments]
        arguments_and_cast = list(zip(argument_type_val, casts))

        if len(argument_type_val) != len(arguments_and_cast):
            cog.error(f"{argument_type_val} and {casts} length mismatch!")

        cog.outl(f"{method_proto}{{")

        pass_args_to_python = ''
        for i, (arg, cast) in enumerate(arguments_and_cast):
            is_pointer = '*' in arg[0] + arg[1]
            arg_val = arg[1].replace('*', '')
            if is_pointer:
                arg_val_as_buf = arg_val
            else:
                arg_val_as_buf= '&' + arg_val

            if method["c_name"] == "realize" and \
               "MemoryRegionOps" in m.get_nested_schema(SCHEMA, "device").values():
                device_fields = m.get_nested_schema(SCHEMA, "device")
                reverse_device_fields = dict(zip(device_fields.values(), device_fields.keys()))
                mem_io_field = reverse_device_fields["MemoryRegionOps"]
                cog.outl(INDENT(f"memory_region_init_io(&(({cast}*){arg_val_as_buf})->{mem_io_field},"))
                cog.outl(INDENT(f"                      OBJECT({arg_val_as_buf}),"))
                cog.outl(INDENT(f"                      &{device_name}_mem_ops,"))
                cog.outl(INDENT(f"                      {arg_val_as_buf},"))
                cog.outl(INDENT(f'                      "{device_name}-mmio",'))
                cog.outl(INDENT(f"                      1);"))

            pass_args_to_python += INDENT(INDENT(dedent(f"""
                                                 PyObject *p_mem_view_{arg_val} = PyMemoryView_FromMemory((char*){arg_val_as_buf},
                                                                                                 {ALIGN_INDENT_BY(arg_val)}sizeof({cast}),
                                                                                                 {ALIGN_INDENT_BY(arg_val)}PyBUF_WRITE);
                                                 if (!p_mem_view_{arg_val}){{
                                                     goto err;
                                                 }}
                                                 PyTuple_SetItem(p_func_args, {i}, p_mem_view_{arg_val});
                                                 """)))

            py_return_code = ''
            py_return_val_name = ''
            if method["c_ret"] != "void":
                py_return_val_name = 'py_out'
                py_return_code = INDENT(dedent(f"""
                                        char *py_out_buf;
                                        {method["c_ret"]} {py_return_val_name};
                                        int ok = PyArg_ParseTuple(p_ret, "S", &py_out_buf);
                                        {py_return_val_name} = *({method["c_ret"]}*)py_out_buf;
                                        """))

        pass_args_to_python += INDENT(INDENT(f"PyTuple_SetItem(p_func_args, {argc}, DictWithClassFields);\n"))
        c_function_body = m.get_python_c_api_wrap(SCHEMA,
                                                  'class',
                                                  method["py_name"],
                                                  argc + 1,
                                                  pass_args_to_python,
                                                  return_val=py_return_val_name)
        if method["c_name"] == "finalize":
            c_function_body = re.sub("//.*?\n",
                                     "if(Py_FinalizeEx()){\n"
                                     + ' '*8 + "PyErr_Print();\n"
                                     + ' '*8 + "abort();\n"
                                     + ' '*4 + "}\n",
                                     c_function_body)
        else:
            c_function_body = re.sub("//.*?\n", py_return_code + '\n', c_function_body, count=1)
        c_function_body = '\n'.join(filter(lambda l: not re.match("\s+$", l), c_function_body.splitlines()))
        cog.outl(c_function_body)

        cog.outl("}")
        cog.outl()
        cog.outl()
  ]]]*/
/*[[[end]]]*/
