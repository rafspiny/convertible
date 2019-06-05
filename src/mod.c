//
// Created by raffaele on 04/05/19.
//
#include "convertible_logging.h"
#include "e-gadget-convertible.h"
#include "convertible.h"
#include "dbus_acceleration.h"
#include <Elementary.h>
#include <e_module.h>

// The main module reference
E_Module *convertible_module;

// Logger
int _convertible_log_dom;

/**
 * Free resources when the gadget is removed
 * */
static void
convertible_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    WARN("CONVERTIBLE convertible_delete\n");
    Instance *inst = data;

    if (inst->accelerometer)
    {
    //        e_menu_post_deactivate_callback_set(inst->main_menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->accelerometer));
    }
    // Remove callbacks
    WARN("Removing callbacks");
    evas_object_event_callback_del(inst->o_button, EVAS_CALLBACK_DEL, convertible_del);
    elm_layout_signal_callback_del(inst->o_button, "lock,rotation", "tablet", _rotation_signal_cb);
    elm_layout_signal_callback_del(inst->o_button, "unlock,rotation", "tablet", _rotation_signal_cb);
    elm_layout_signal_callback_del(inst->o_button, "enable,keyboard", "keyboard", _rotation_signal_cb);
    elm_layout_signal_callback_del(inst->o_button, "disable,keyboard", "keyboard", _rotation_signal_cb);

    // Removeing logger
    eina_log_domain_unregister(_convertible_log_dom);
    _convertible_log_dom = -1;
    //    evas_object_smart_callback_del_full(inst->site, "gadget_site_anchor", _anchor_change, inst);
    free(inst);
}

/**
 * Callback for gadget creation
 * */
static void
_gadget_created(void *data, Evas_Object *obj, void *event_info)
{
    WARN("CONVERTIBLE gadget_created\n");
    Instance *inst = data;

    if (event_info != inst->o_button) return;
//    do_orient(inst, e_gadget_site_orient_get(obj), e_gadget_site_anchor_get(obj));
    evas_object_smart_callback_del_full(obj, "gadget_created", _gadget_created, inst);
    WARN("CONVERTIBLE gadget_created END\n");
}

/**
 * Gadget creation function. It set the theme, set the icon and register all the callbacks
 * */
EINTERN Evas_Object *
convertible_create(Evas_Object *parent, int *id, E_Gadget_Site_Orient orient)
{
    Evas_Object *o;
    Instance *inst;
    char theme_overlay_path[4096];

    WARN("convertible_create entered");
    if (e_gadget_site_is_desklock(parent)) return NULL;
    if (*id == 0) *id = 1;

    WARN("creating instance");
    inst = E_NEW(Instance, 1);
//    inst->site = parent;

//    o = elm_layout_add(parent);

    WARN("setting edje theme layer");
    // Registering the theme in order to get our small custom icon
    snprintf(theme_overlay_path, sizeof(theme_overlay_path), "%s/e-module-convertible.edj", convertible_module->dir);
    WARN(theme_overlay_path);
    elm_theme_extension_add(NULL, theme_overlay_path);
    WARN("theme overlay set");

    // Setting the small icon
    o = elm_layout_add(parent);
    WARN("setting icon");
    e_theme_edje_object_set(o, "base/theme/modules/convertible",
                            "e/modules/convertible/main");
    //edje_object_signal_emit(o, "e,state,unfocused", "e");
    WARN("after setting edje icon");

    WARN("saving edje reference in instance");
    inst->o_button = o;
    evas_object_size_hint_aspect_set(o, EVAS_ASPECT_CONTROL_BOTH, 1, 1);

    WARN("Adding callbacks to react to events and gadget removal");
//    evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _button_cb_mouse_down, inst);
    WARN("Adding callback for creation");
//    evas_object_smart_callback_add(parent, "gadget_site_anchor", _anchor_change, inst);

    // Adding callback for EDJE object
    evas_object_smart_callback_add(parent, "gadget_created", _gadget_created, inst);
    evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, convertible_del, inst);
    elm_layout_signal_callback_add(o, "lock,rotation", "tablet", _rotation_signal_cb, inst);
    elm_layout_signal_callback_add(o, "unlock,rotation", "tablet", _rotation_signal_cb, inst);
    elm_layout_signal_callback_add(o, "enable,keyboard", "keyboard", _rotation_signal_cb, inst);
    elm_layout_signal_callback_add(o, "disable,keyboard", "keyboard", _rotation_signal_cb, inst);

    // Bringing in the instance ref. It is useful in the delete callback
    convertible_module->data = inst;

//    do_orient(inst, orient, e_gadget_site_anchor_get(parent));
    WARN("convertible_create end");

    return o;
}

/* module setup */
E_API E_Module_Api e_modapi =
        {
                E_MODULE_API_VERSION,
                "convertible"
        };

E_API void *
e_modapi_init(E_Module *m)
{
    // Initialise the logger
    _convertible_log_dom = eina_log_domain_register("convertible", EINA_COLOR_LIGHTBLUE);

    convertible_module = m;
    // It looks like this is not needed right now
    //    e_gadcon_provider_register(&_gadcon_class);

    INF("Setting the callback for creation");
    e_gadget_type_add("convertible", convertible_create, NULL);

    return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
    INF("Shutting down the module");
    convertible_module = NULL;
    //    e_gadcon_provider_unregister(&_gadcon_class);
    e_gadget_type_del("convertible");
    return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
    return 1;
}


Eldbus_Connection *dbus_conn;
//static E_Config_DD *edd;
struct Convertible_Config *convertible_config;
