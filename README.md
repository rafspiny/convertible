This gadget is meant to improve screen rotation quality by leveraging on [iio-sensor-proxy](https://github.com/hadess/iio-sensor-proxy).
iio-sensor-proxy is a D-Bus proxy for accelerometers and ambient light sensors.

This gadget, merely connect to DBUS and read the information put there by iio-sensor-proxy, provided that the gyroscope is supported.

## Usage
TBD
 

## Installation
To install it please run:
```bash
meson build --prefix /usr
cd build
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
