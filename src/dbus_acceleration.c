//
// Created by raffaele on 01/05/19.
//
#include "convertible_logging.h"
#include "dbus_acceleration.h"

Eldbus_Proxy* get_dbus_interface(const char* IFACE) {
    WARN("Inside get_dbus_interface");
    Eldbus_Connection *conn;
    Eldbus_Object *obj;
    Eldbus_Proxy *sensor_proxy = NULL;

    conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
    if (!conn)
    {
        ERR("Error: could not get system bus");
    }
    obj = eldbus_object_get(conn, EFL_DBUS_ACC_BUS, EFL_DBUS_ACC_PATH);
    if (!obj)
    {
        ERR("Error: could not get object");
    }
    else {
        INF("Object fetched.");
    }
    sensor_proxy = eldbus_proxy_get(obj, IFACE);
    if (!sensor_proxy)
    {
        ERR("Error: could not get proxy");
    } else {
        INF("Proxy fetched");
    }

    return sensor_proxy;
}

Eina_Bool access_string_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, Eina_Bool *string_property_value) {
    const char *type;
    Eina_Bool res = EINA_TRUE;

    if (!eldbus_message_arguments_get(msg, "v", variant))
    {
        WARN("Error getting arguments.");
        res = EINA_FALSE;
    }
    type = eldbus_message_iter_signature_get((*variant));
    if (type[1])
    {
        WARN("It is a complex type, not handle yet.\n\n");
        res = EINA_FALSE;
    }
    if (type[0] != 's')
    {
        WARN("Expected type is int.\n\n");
        res = EINA_FALSE;
    }
    if (!eldbus_message_iter_arguments_get((*variant), "s", string_property_value))
    {
        WARN("error in eldbus_message_iter_arguments_get()\n\n");
        res = EINA_FALSE;
    }
    free((void *) type);
    return res;
}

Eina_Bool access_bool_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, Eina_Bool *boolean_property_value) {
    const char *type;
    Eina_Bool res = EINA_TRUE;

    if (!eldbus_message_arguments_get(msg, "v", variant))
    {
        WARN("Error getting arguments.");
        res = EINA_FALSE;
    }
    type = eldbus_message_iter_signature_get((*variant));
    if (type[1])
    {
        WARN("It is a complex type, not handle yet.\n\n");
        res = EINA_FALSE;
    }
    if (type[0] != 'b')
    {
        WARN("Expected type is int.\n\n");
        res = EINA_FALSE;
    }
    if (!eldbus_message_iter_arguments_get((*variant), "b", boolean_property_value))
    {
        WARN("error in eldbus_message_iter_arguments_get()\n\n");
        res = EINA_FALSE;
    }
    free((void *) type);
    return res;
}

void
on_has_accelerometer(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
    const char *errname, *errmsg;
    Eina_Bool has_accelerometer = EINA_FALSE;
    Eldbus_Message_Iter *variant = NULL;

    if (eldbus_message_error_get(msg, &errname, &errmsg))
    {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
    }

    access_bool_property(msg, &variant, &has_accelerometer);
    WARN("Has Accelerometer: %d\n", has_accelerometer);
    struct DbusAccelerometer *d1 = (struct DbusAccelerometer *)data;
    d1->has_accelerometer = has_accelerometer;
}

void
on_accelerometer_orientation(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
    const char *errname, *errmsg, *type;
    char *orientation = malloc(sizeof(char[20]));
    Eldbus_Message_Iter *variant = NULL;

    if (eldbus_message_error_get(msg, &errname, &errmsg))
    {
        ERR("Error: %s %s", errname, errmsg);
        return;
    }

    access_string_property(msg, &variant, &orientation);
    struct DbusAccelerometer *d1 = (struct DbusAccelerometer *)data;
    d1->orientation = orientation;
    WARN("Orientation: %s\n", orientation);
}

void
on_accelerometer_claimed(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    const char *errname, *errmsg;

    if (eldbus_message_error_get(msg, &errname, &errmsg)) {
        ERR("Error: %s %s", errname, errmsg);
        return;
    }
    // TODO set the acquired field
}

void
on_accelerometer_released(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    const char *errname, *errmsg;

    if (eldbus_message_error_get(msg, &errname, &errmsg)) {
        ERR("Error: %s %s", errname, errmsg);
        return;
    }
    // TODO unset the acquired field
}
