//
// Created by raffaele on 04/05/19.
//

#include "convertible.h"
#include <Ecore.h>
#include <Elementary.h>
#include "e.h"

EINTERN void
convertible_gadget_init(void)
{
    e_gadget_type_add("Convertible", convertible_create, NULL);
    // TODO re-enable this
//    handler = ecore_event_handler_add(E_EVENT_CONFIG_MODE_CHANGED, _convertible_mode_change, NULL);
}

EINTERN void
convertible_gadget_shutdown(void)
{
    e_gadget_type_del("Convertible");
    E_FREE_FUNC(handler, ecore_event_handler_del);
}

static void
_convertible_created_cb(void *data, Evas_Object *obj, void *event_data EINA_UNUSED)
{
//    Instance *inst = data;
//    evas_object_data_set(inst->box, "Instance", inst);
//    e_gadget_configure_cb_set(inst->box, _wireless_gadget_configure_cb);
//    e_gadget_menu_populate_cb_set(inst->box, _wireless_gadget_menu_populate_cb);
//    evas_object_smart_callback_del_full(obj, "gadget_created", _wireless_created_cb, data);
}

void convertible_init(void) {

}
void convertible_shutdown(void) {

}

EINTERN void convertible_gadget_init(void) {
    fprintf(stdout, "In gadget init");
}

EINTERN void convertible_gadget_shutdown(void) {
    fprintf(stdout, "In gadget shutdown");
}

static void
convertible_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
    Instance *inst = data;

    if (inst->accelerometer)
    {
//        e_menu_post_deactivate_callback_set(inst->main_menu, NULL, NULL);
        e_object_del(E_OBJECT(inst->accelerometer));
    }
//    evas_object_smart_callback_del_full(inst->site, "gadget_site_anchor", _anchor_change, inst);
    free(inst);
}

static void
_gadget_created(void *data, Evas_Object *obj, void *event_info)
{
    Instance *inst = data;

    if (event_info != inst->o_button) return;
//    do_orient(inst, e_gadget_site_orient_get(obj), e_gadget_site_anchor_get(obj));
    evas_object_smart_callback_del_full(obj, "gadget_created", _gadget_created, inst);
}