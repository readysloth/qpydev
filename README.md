# Qpydev

## What it is?

Qpydev is python program that consumes json description of your
qemu device and creates c source file that can be integrated in qemu.


## How it works?

To ease device prototyping, Qpydev allows creating device logic in python
language and specifying things like:

  + class fields
  + method prototypes
  + device properties

in json files that are called "device schema"

For now it's up to you to integrate in qemu your generated device, but
autointegration is in plans

