#include <Ecore.h>
#include <Elementary.h>

#ifndef EFL_DBUS_ACCELERATION
#define EFL_DBUS_ACCELERATION

#define EFL_DBUS_ACC_BUS "net.hadess.SensorProxy"
#define EFL_DBUS_ACC_PATH "/net/hadess/SensorProxy"
#define EFL_DBUS_ACC_IFACE "net.hadess.SensorProxy"

struct DbusAccelerometer {
    Eina_Bool has_accelerometer;
    char *orientation;
    Eina_Bool monitoring;
    Eldbus_Proxy *proxy;
};


/**
 * Helper function to extract ta string property from the message
 * @param msg The message coming from the get property invocation
 * @param variant
 * @param string_property_value The string property pointer where the value should be stored, if read
 * @return
 */
Eina_Bool
access_string_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, Eina_Bool *string_property_value);

/**
 * Helper function to extract ta boolean property from the message
 * @param msg The message coming from the get property invocation
 * @param variant
 * @param boolean_property_value The boolean property pointer where the value should be stored, if read
 * @return
 */
Eina_Bool
access_bool_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, Eina_Bool *boolean_property_value);

/**
 * Callback definition to handle the request of the hasAccelerometer property of DBUS interface net.hadess.SensorProxy
 * @param data DbusAccelerometer
 * @param msg The message
 * @param pending
 */
void
on_has_accelerometer(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED);

/**
 * Callback definition to handle the request of the accelerometer property of DBUS interface net.hadess.SensorProxy
 * @param data DbusAccelerometer
 * @param msg The message
 * @param pending
 */
void
on_accelerometer_orientation(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED);
#endif
