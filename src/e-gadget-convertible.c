//
// Created by raffaele on 04/05/19.
//
#include "convertible_logging.h"
#include "e-gadget-convertible.h"
#include "convertible.h"

Instance* instance;

void _rotation_signal_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED,
                         const char *src EINA_UNUSED)
{
   DBG("Rotation: Signal %s received from %s", sig, src);
   Instance *inst = data;
   if (eina_str_has_prefix(sig, "unlock"))
      inst->locked_position = EINA_FALSE;
   if (eina_str_has_prefix(sig, "lock"))
      inst->locked_position = EINA_TRUE;
}

void _keyboard_signal_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED,
                         const char *src EINA_UNUSED)
{
   DBG("Keyboard: Signal %s received from %s", sig, src);
}


/**
 * Callback for gadget creation
 * */
static void
_gadget_created(void *data EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
   DBG("Inside gadget created");
   //    do_orient(inst, e_gadget_site_orient_get(obj), e_gadget_site_anchor_get(obj));
   evas_object_smart_callback_del_full(obj, "gadget_created", _gadget_created, NULL);
}


/**
 * Free resources when the gadget is removed
 * */
static void
convertible_del(void *data EINA_UNUSED, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   DBG("CONVERTIBLE convertible_delete");

   // Remove callbacks
   DBG("Removing EDJE callbacks");
   evas_object_event_callback_del(obj, EVAS_CALLBACK_DEL, convertible_del);
   elm_layout_signal_callback_del(obj, "lock,rotation", "tablet", _rotation_signal_cb);
   elm_layout_signal_callback_del(obj, "unlock,rotation", "tablet", _rotation_signal_cb);
   elm_layout_signal_callback_del(obj, "enable,keyboard", "keyboard", _keyboard_signal_cb);
   elm_layout_signal_callback_del(obj, "disable,keyboard", "keyboard", _keyboard_signal_cb);
}


/**
 * Gadget creation function. It set the theme, set the icon and register all the callbacks
 * */
EINTERN Evas_Object *
convertible_create(Evas_Object *parent, int *id, E_Gadget_Site_Orient orient EINA_UNUSED)
{
   Evas_Object *o;

   DBG("convertible_create entered");
   if (e_gadget_site_is_desklock(parent)) return NULL;
   if (*id == 0) *id = 1;

   // Setting the small icon
   o = elm_layout_add(parent);
   e_theme_edje_object_set(o, "base/theme/modules/convertible",
                           "e/modules/convertible/main");
   //edje_object_signal_emit(o, "e,state,unfocused", "e");

   evas_object_size_hint_aspect_set(o, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
   //    evas_object_smart_callback_add(parent, "gadget_site_anchor", _anchor_change, inst);

   // Adding callback for EDJE object
   INF("Adding callback for creation and other events from EDJE");
   evas_object_smart_callback_add(parent, "gadget_created", _gadget_created, NULL);
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, convertible_del, NULL);
   elm_layout_signal_callback_add(o, "lock,rotation", "tablet", _rotation_signal_cb, instance);
   elm_layout_signal_callback_add(o, "unlock,rotation", "tablet", _rotation_signal_cb, instance);
   elm_layout_signal_callback_add(o, "enable,keyboard", "keyboard", _keyboard_signal_cb, instance);
   elm_layout_signal_callback_add(o, "disable,keyboard", "keyboard", _keyboard_signal_cb, instance);

   //    do_orient(inst, orient, e_gadget_site_anchor_get(parent));
   DBG("convertible_create end");

   return o;
}

void convertible_gadget_init(Instance* inst) {
   instance = inst;
   e_gadget_type_add("convertible", convertible_create, NULL);
}

