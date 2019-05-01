//
// Created by raffaele on 01/05/19.
//
#include "dbus_acceleration.h"

Eina_Bool access_string_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, Eina_Bool *string_property_value) {
    const char *type;
    Eina_Bool res = EINA_TRUE;

    if (!eldbus_message_arguments_get(msg, "v", variant))
    {
        printf("Error getting arguments.");
        res = EINA_FALSE;
    }
    type = eldbus_message_iter_signature_get((*variant));
    if (type[1])
    {
        printf("It is a complex type, not handle yet.\n\n");
        res = EINA_FALSE;
    }
    if (type[0] != 's')
    {
        printf("Expected type is int.\n\n");
        res = EINA_FALSE;
    }
    if (!eldbus_message_iter_arguments_get((*variant), "s", string_property_value))
    {
        printf("error in eldbus_message_iter_arguments_get()\n\n");
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
        printf("Error getting arguments.");
        res = EINA_FALSE;
    }
    type = eldbus_message_iter_signature_get((*variant));
    if (type[1])
    {
        printf("It is a complex type, not handle yet.\n\n");
        res = EINA_FALSE;
    }
    if (type[0] != 'b')
    {
        printf("Expected type is int.\n\n");
        res = EINA_FALSE;
    }
    if (!eldbus_message_iter_arguments_get((*variant), "b", boolean_property_value))
    {
        printf("error in eldbus_message_iter_arguments_get()\n\n");
        res = EINA_FALSE;
    }
    free((void *) type);
    return res;
}

static void
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
    printf("Has Accelerometer: %d\n", has_accelerometer);
    struct DbusAccelerometer *d1 = (struct DbusAccelerometer *)data;
    d1->has_accelerometer = has_accelerometer;
}

static void
on_accelerometer_orientation(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
    const char *errname, *errmsg, *type;
    char *orientation = malloc(sizeof(char[20]));
    Eina_Bool res = EINA_FALSE;
    Eldbus_Message_Iter *variant = NULL;

    if (eldbus_message_error_get(msg, &errname, &errmsg))
    {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
    }

    access_string_property(msg, &variant, &orientation);
    struct DbusAccelerometer *d1 = (struct DbusAccelerometer *)data;
    d1->orientation = orientation;
    printf("Orientation: %s\n", orientation);
}
