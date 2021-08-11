# How to qpydev

If you plan to create simple qemu device and don't want to alter generated source file,
then your main point of interest should be [schema](schema) folder, files in which describe
your device, properties (in sense of QOM) it has and etc.

| Files               | Describes                                              | Top members                                      |
|:-------------------:| -------------------------------------------------------|--------------------------------------------------|
| class.json          | Device class fields initialization and methods for QOM | `init` (dict) , `methods` (list of dicts)        |
| device.json         | Device fields                                          | `device` (dict), `MemoryRegionOps` field, if any |
| properties.json     | Device qemu properties, if any                         | `properties` (list of dicts)                     |
| qdev.json           | Scheme as whole, with `*.json` and `*.py` files        | (dict)

## qdev.json

This file should contain:

+ `name` -- it is device name, separate words should be delimeted with `_`
+ `python_version` -- exact version you will link against qemu binary (for example: `3.7m`)
+ `parent` -- dictionary that describes device you inherit
  - `name` -- name of parent device (for example: `SysBus`, `PCI`)
  - `type` -- qemu type of parent device (for example: `SYS_BUS`, `PCI`)
  - `interface` -- interface your device should implement (for example: `CONVENTIONAL_PCI_DEVICE`)
  - `header` -- header with parent device (for example: `hw/sysbus.h`, `hw/pci/pci.h`)
+ `class` -- dictionary that describes class of your device
  - `schema` -- path to schema file (if path is relative, then it is appended to directory where `qdev.json` is lying)
  - `code` -- path to code file (if path is relative, then it is appended to directory where `qdev.json` is lying)
+ `device` -- dictionary that describes class of your device
  - `schema` -- path to schema file (if path is relative, then it is appended to directory where `qdev.json` is lying)
  - `code` -- path to code file (if path is relative, then it is appended to directory where `qdev.json` is lying)
+ `properties` -- dictionary that describes class of your device
  - `schema` -- path to schema file (if path is relative, then it is appended to directory where `qdev.json` is lying)
  - `code` -- path to code file (if path is relative, then it is appended to directory where `qdev.json` is lying)


## class.json

This file should contain:

+ `init` -- dictionary that describes device class initialization (QOM class_init)
  - `class_field_init` -- list of concrete `ObjectClass` casts and field initialization
    * `dev_cast` -- dictionary that describes cast from  `ObjectClass`
      + `type` -- C-type of device cast (for example: `DeviceClass`, `PCIDeviceClass`)
      + `cast` -- device cast macro (for example: `DEVICE_CLASS`, `PCI_DEVICE_CLASS`)
    * `field` -- any field object you casted to with dev_cast contains
+ `methods` -- list of dictionaries that describe class methods and associate it with python functions
  - `c_name` -- function name in generated C source file (will be appended to device name)
  - `c_args` -- string with comma-separated fields `Type argument` for function prototype
    (for example: `void *opaque, hwaddr addr, unsigned size`)
  - `c_to_py_cast` -- string with comma-separated fields `Type` for passing `c_args` to python correctly.
    Field count sould match with `c_args` (for example: `ExampleDevice, hwaddr, unsigned`)
  - `py_name` -- string with comma-separated fields `Type` for passing `c_args` to python correctly.


## device.json

This file should contain:

+ `device` -- dictionary that describes device object
+ variable name of type `MemoryRegionOps` if device object have any and willing to set specific
  fields of `MemoryRegionOps` object

If something is not clear, you can always go to [schema](schema) folder or [examples](examples) to see
how things are done
