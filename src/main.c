#include <Ecore.h>
#include <Elementary.h>
#include <e_gadget_types.h>
#include "dbus_acceleration.h"
#include "accelerometer-orientation.h"

/**
 * Callback definition to handle the execution of the ClaimAccelerometer() method of DBUS
 * interface net.hadess.SensorProxy
 * @param data not used
 * @param msg The message
 * @param pending
 */
static void
on_accelerometer_claimed(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    Eldbus_Message_Iter *array, *entry;
    const char *errname, *errmsg;

    if (eldbus_message_error_get(msg, &errname, &errmsg)) {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
    }
}

/**
 * Callback definition to handle the execution of the ReleaseAccelerometer() method of DBUS
 * interface net.hadess.SensorProxy
 * @param data not used
 * @param msg The message
 * @param pending
 */
static void
on_accelerometer_released(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED) {
    Eldbus_Message_Iter *array, *entry;
    const char *errname, *errmsg;

    if (eldbus_message_error_get(msg, &errname, &errmsg)) {
        fprintf(stderr, "Error: %s %s\n", errname, errmsg);
        return;
    }
}

static void
_cb_properties_changed(void *data, const Eldbus_Message *msg) {
    struct DbusAccelerometer *structure = data;
    Eldbus_Proxy *proxy = structure->proxy;
    Eldbus_Message_Iter *array, *invalidate;
    char *iface;

    if (!eldbus_message_arguments_get(msg, "sa{sv}as", &iface, &array, &invalidate))
    {
        ERR("Error getting data from properties changed signal.");
        return;
    }
    eldbus_proxy_property_get(proxy, "AccelerometerOrientation", on_accelerometer_orientation, &structure);
}

static void
on_click(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(data);
}

static Eldbus_Proxy* get_dbus_interface(const char* IFACE) {
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
    sensor_proxy = eldbus_proxy_get(obj, IFACE);
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
    Eldbus_Proxy *sensor_proxy, *sensor_proxy_properties;
    Eldbus_Pending *pending_has_orientation, *pending_orientation, *pending_acc_claim;

    struct DbusAccelerometer *current_accelerometer_pointer = malloc(sizeof(struct DbusAccelerometer));
    current_accelerometer_pointer->has_accelerometer = 0;
    current_accelerometer_pointer->orientation = "undefined";
    current_accelerometer_pointer->proxy = NULL;

    int initialization = eldbus_init();
    if (initialization == EXIT_FAILURE)
    {
        return initialization;
    }

    sensor_proxy = get_dbus_interface(EFL_DBUS_ACC_IFACE);
    sensor_proxy_properties = get_dbus_interface(ELDBUS_FDO_INTERFACE_PROPERTIES);
    if (sensor_proxy == NULL)
    {
        return EXIT_FAILURE;
    }

    current_accelerometer_pointer->proxy = sensor_proxy;

    pending_has_orientation = eldbus_proxy_property_get(sensor_proxy, "HasAccelerometer", on_has_accelerometer, &current_accelerometer_pointer);
    pending_orientation = eldbus_proxy_property_get(sensor_proxy, "AccelerometerOrientation", on_accelerometer_orientation, &current_accelerometer_pointer);
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

    // Claim the accelerometer
    pending_acc_claim = eldbus_proxy_call(sensor_proxy, "ClaimAccelerometer", on_accelerometer_claimed, NULL, -1, "");

   if (!pending_acc_claim)
     {
        fprintf(stderr, "Error: could not call ClaimAccelerometer\n");
        return EXIT_FAILURE;
     }
    Eldbus_Signal_Handler *sh = eldbus_proxy_signal_handler_add(sensor_proxy_properties, "PropertiesChanged",
                                                                _cb_properties_changed, current_accelerometer_pointer);
//    Eldbus_Signal_Handler *sh = eldbus_proxy_properties_changed_callback_add(sensor_proxy_properties, _cb_properties_changed, sensor_proxy_properties);

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