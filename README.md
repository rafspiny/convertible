This gadget is meant to help report screen rotation based on iio-sensor-proxy.
iio-sensor-proxy is providing data from the accelerometer through dbus.

## 

## Installation
To install it please run:
```bash
meson build --prefix /usr
ninja 
sudo ninja install
``` 

This will create a gadget in the <prefix>/<lib_folder>/enlightenment/modules
The gadget structure is like the following:

```
modules
|   
|
└───convertible
|   |
|   └───module.desktop
|   └───linux-gnu-x86_64-ver-0.23
|   |   |   # The gadget you compiled
|   |   └───e_gadget_convertible
|   └───e-gadget-covnertible.edj
└───... # Other gadgets
```
