//
// Created by raffaele on 04/05/19.
//
#include "e-gadget-convertible.h"
#include "convertible.h"
#include <Elementary.h>
#include <e_module.h>

// The main module reference
E_Module *convertible_module;

/**
 * Free resource when the gadget is removed
 * */
static void
convertible_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    fprintf(stdout, "CONVERTIBLE convertible_delete\n");
    Instance *inst = data;

    if (inst->accelerometer)
    {
//        e_menu_post_deactivate_callback_set(inst->main_menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->accelerometer));
    }
//    evas_object_smart_callback_del_full(inst->site, "gadget_site_anchor", _anchor_change, inst);
    free(inst);
}

/**
 * Callback for gadget creation
 * */
static void
_gadget_created(void *data, Evas_Object *obj, void *event_info)
{
    fprintf(stdout, "CONVERTIBLE gadget_created\n");
    Instance *inst = data;

    if (event_info != inst->o_button) return;
//    do_orient(inst, e_gadget_site_orient_get(obj), e_gadget_site_anchor_get(obj));
    evas_object_smart_callback_del_full(obj, "gadget_created", _gadget_created, inst);
    fprintf(stdout, "CONVERTIBLE gadget_created END\n");
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

    fprintf(stdout, "CREATE FUNCTION\n");
    if (e_gadget_site_is_desklock(parent)) return NULL;
    if (*id == 0) *id = 1;
    fprintf(stdout, "CREATE FUNCTION 2\n");
    inst = E_NEW(Instance, 1);
//    inst->site = parent;

//    o = elm_layout_add(parent);

    fprintf(stdout, "before setting edje\n");
    // Registering the theme in order to get our small custom icon#include <Elementary.h>
    snprintf(&theme_overlay_path, sizeof(theme_overlay_path), "%s/e-module-convertible.edj", convertible_module->dir);
    fprintf(stdout, theme_overlay_path);
    fprintf(stdout, "\n");
    fprintf(stdout, "after setting edje theme\n");
    elm_theme_extension_add(NULL, theme_overlay_path);
    fprintf(stdout, "after setting edje extension\n");
    // Setting the small icon
    o = elm_layout_add(parent);
    fprintf(stdout, "before settin icon edje\n");
    e_theme_edje_object_set(o, "base/theme/modules/convertible",
                            "e/modules/convertible/main");
    //edje_object_signal_emit(o, "e,state,unfocused", "e");
    fprintf(stdout, "after setting edje\n");

    fprintf(stdout, "CREATE FUNCTION 3\n");
    e_theme_edje_object_set(o, NULL, "e/gadget/convertible/main");
    elm_layout_signal_emit(o, "e,state,unfocused", "e");
    fprintf(stdout, "CREATE FUNCTION 4\n");

    inst->o_button = o;
    evas_object_size_hint_aspect_set(o, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
    fprintf(stdout, "CREATE FUNCTION 5\n");

    evas_object_event_callback_add(o, EVAS_CALLBACK_MOUSE_DOWN, _button_cb_mouse_down, inst);
    evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, convertible_del, inst);
    fprintf(stdout, "CREATE FUNCTION 6\n");
//    evas_object_smart_callback_add(parent, "gadget_site_anchor", _anchor_change, inst);
    evas_object_smart_callback_add(parent, "gadget_created", _gadget_created, inst);
    fprintf(stdout, "CREATE FUNCTION 7\n");
//    do_orient(inst, orient, e_gadget_site_anchor_get(parent));
    fprintf(stdout, "CREATE FUNCTION 8\n");

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
    fprintf(stdout, "CONVERTIBLE: In module init");
    EINTERN Evas_Object *start_create(Evas_Object *parent, int *id, E_Gadget_Site_Orient orient);

//    _e_gconvertible_log_domain = eina_log_domain_register("convertible_gadget", EINA_COLOR_RED);
    convertible_module = m;
//    e_gadcon_provider_register(&_gadcon_class);
    e_gadget_type_add("convertible", convertible_create, NULL);
    fprintf(stdout, "CONVERTIBLE: In module init END");
    return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
    fprintf(stdout, "CONVERTIBLE: In module shutdown");
    convertible_module = NULL;
//    e_gadcon_provider_unregister(&_gadcon_class);
    e_gadget_type_del("convertible");
    fprintf(stdout, "CONVERTIBLE: In module shutdown END");
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
