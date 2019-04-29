#include <Ecore.h>
#include <Elementary.h>
#include <e_gadget_types.h>
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
on_accelerometer_change(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
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

static void
on_click(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(data);
}

static EAPI_MAIN int initialise_dbus() {
    return eldbus_init();
}

static Eldbus_Proxy* get_dbus_interface() {
    Eldbus_Connection *conn;
    Eldbus_Object *obj;
    Eldbus_Proxy *sensor_proxy = NULL;

    // TODO replace fprintf with DBG() or ERR()
    conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
    if (!conn)
    {
        fprintf(stderr, "Error: could not get system bus\n");
    }
    obj = eldbus_object_get(conn, EFL_DBUS_ACC_BUS, EFL_DBUS_ACC_PATH);
    if (!obj)
    {
        fprintf(stderr, "Error: could not get object\n");
    }
    else {
        fprintf(stdout, "Object fetched.\n");
    }
    sensor_proxy = eldbus_proxy_get(obj, EFL_DBUS_ACC_IFACE);
    if (!sensor_proxy)
    {
        fprintf(stderr, "Error: could not get proxy\n");
    } else {
        fprintf(stdout, "Proxy fetched\n");
    }

    return sensor_proxy;
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
    Evas_Object *win, *btn, *icon;
    int gadget =0, id_num = 0;
    char buf[16];
    Eldbus_Proxy *sensor_proxy;
    Eldbus_Pending *pending_has_orientation, *pending_orientation;

    struct DbusAccelerometer *current_accelerometer_pointer = malloc(sizeof(struct DbusAccelerometer));
    current_accelerometer_pointer->has_accelerometer = 0;
    current_accelerometer_pointer->orientation = "undefined";

    EAPI_MAIN int initialization = initialise_dbus();
    if (initialization == EXIT_FAILURE)
    {
        return initialization;
    }

    sensor_proxy = get_dbus_interface();
    if (sensor_proxy == NULL)
    {
        return EXIT_FAILURE;
    }

    pending_has_orientation = eldbus_proxy_property_get(sensor_proxy, "HasAccelerometer", on_has_accelerometer, &current_accelerometer_pointer);
    pending_orientation = eldbus_proxy_property_get(sensor_proxy, "AccelerometerOrientation", on_accelerometer_change, &current_accelerometer_pointer);
    if (!pending_has_orientation)
    {
        fprintf(stderr, "Error: could not get property HasAccelerometer\n");
        return EXIT_FAILURE;
    }
    if (!pending_orientation)
    {
        fprintf(stderr, "Error: could not get property AccelerometerOrientation\n");
        return EXIT_FAILURE;
    }

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

    if (getenv("E_GADGET_ID"))
    {
        gadget = 1;
        snprintf(buf, sizeof(buf), "%s", getenv("E_GADGET_ID"));
        id_num = atoi(buf);
    }

    win = elm_win_add(NULL, "Main", ELM_WIN_BASIC);
    elm_win_title_set(win, "Hello, World!");
    elm_win_autodel_set(win, EINA_TRUE);

    if (gadget) elm_win_alpha_set(win, EINA_TRUE);

    if (gadget && id_num == -1)
    {
        icon = elm_icon_add(win);
        elm_icon_standard_set(icon, "start-here");
        evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_win_resize_object_add(win, icon);
        evas_object_show(icon);
    }
    else
    {
        btn = elm_button_add(win);
        elm_object_text_set(btn, "Goodbye Cruel World");
        elm_win_resize_object_add(win, btn);
        evas_object_smart_callback_add(btn, "clicked", on_click, win);
        evas_object_show(btn);
    }
    evas_object_show(win);

    elm_run();


    return 0;
}

ELM_MAIN()