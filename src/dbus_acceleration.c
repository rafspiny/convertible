//
// Created by raffaele on 01/05/19.
//
#include "convertible_logging.h"
#include "accelerometer-orientation.h"
#include "dbus_acceleration.h"
#include "convertible.h"

Eldbus_Proxy* get_dbus_interface(const char* IFACE) {
    Eldbus_Connection *conn;
    Eldbus_Object *obj;
    Eldbus_Proxy *sensor_proxy;

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
        ERR("Error: could not get proxy for interface %s", IFACE);
    } else {
        INF("Proxy fetched");
    }

    return sensor_proxy;
}

Eina_Bool access_string_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, char **string_property_value) {
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
        WARN("It is a complex type, not handle yet.");
        res = EINA_FALSE;
    }
    if (type[0] != 's')
    {
        WARN("Expected type is string(s).");
        res = EINA_FALSE;
    }
    if (!eldbus_message_iter_arguments_get((*variant), "s", string_property_value))
    {
        WARN("error in eldbus_message_iter_arguments_get()");
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
        WARN("It is a complex type, not handle yet.");
        res = EINA_FALSE;
    }
    if (type[0] != 'b')
    {
        WARN("Expected type is int.");
        res = EINA_FALSE;
    }
    if (!eldbus_message_iter_arguments_get((*variant), "b", boolean_property_value))
    {
        WARN("error in eldbus_message_iter_arguments_get()");
        res = EINA_FALSE;
    }
    free((void *) type);
    return res;
}

void
on_has_accelerometer(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
    const char *errname, *errmsg;
    Eina_Bool has_accelerometer = EINA_FALSE;
    Eldbus_Message_Iter *variant = NULL;

    if (eldbus_message_error_get(msg, &errname, &errmsg))
    {
        ERR("Error: %s %s", errname, errmsg);
    }

    access_bool_property(msg, &variant, &has_accelerometer);
    Instance *inst = (Instance *)data;
    inst->accelerometer->has_accelerometer = has_accelerometer;
    WARN("Has Accelerometer: %d", inst->accelerometer->has_accelerometer);
}

void
on_accelerometer_orientation(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
    Instance *inst = (Instance *)data;
    const char *errname, *errmsg;
    char *orientation = malloc(sizeof(char[20]));
    Eldbus_Message_Iter *variant = NULL;

    if (eldbus_message_error_get(msg, &errname, &errmsg))
    {
        ERR("Error: %s %s", errname, errmsg);
        return;
    }

    access_string_property(msg, &variant, &orientation);
    inst->accelerometer->orientation = orientation;
    WARN("Current Orientation: %s", inst->accelerometer->orientation);

    int rotation = 0;

    if (inst->main_screen->info.can_rot_270 && strcmp(ACCELEROMETER_ORIENTATION_RIGHT, orientation) == 0)
        rotation = 270;
    if (inst->main_screen->info.can_rot_90 && strcmp(ACCELEROMETER_ORIENTATION_LEFT, orientation) == 0)
        rotation = 90;
    if (inst->main_screen->info.can_rot_180 && strcmp(ACCELEROMETER_ORIENTATION_BOTTOM, orientation) == 0)
        rotation = 180;
    WARN("Rotation: %d", rotation);

    if (inst->main_screen_cfg == NULL)
        ERR("Screen not set.");
    else {
        WARN("Setting screen rotation to %d", rotation);
        inst->main_screen_cfg->rotation = rotation;
        e_randr2_config_apply();
    }

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
