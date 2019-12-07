//
// Created by raffaele on 04/05/19.
//
#include "convertible_logging.h"
#include "e-gadget-convertible.h"
#include "convertible.h"

DbusAccelerometer* dbus_accelerometer;
static Eina_List *instances;

void _rotation_signal_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED,
                         const char *src EINA_UNUSED)
{
   WARN("Rotation: Signal %s received from %s", sig, src);
   Instance *inst = data;
   if (eina_str_has_prefix(sig, "unlock"))
   {
      inst->locked_position = EINA_FALSE;
   }
   if (eina_str_has_prefix(sig, "lock"))
   {
      inst->locked_position = EINA_TRUE;
   }
}

void _keyboard_signal_cb(void *data EINA_UNUSED, Evas_Object *obj EINA_UNUSED, const char *sig EINA_UNUSED,
                         const char *src EINA_UNUSED)
{
   WARN("Keyboard: Signal %s received from %s", sig, src);
}


/**
 * Callback for gadget creation
 * */
static void
_gadget_created(void *data, Evas_Object *obj, void *event_info)
{
   DBG("Inside gadget created");
   Instance *inst = data;

   if (event_info != inst->o_button) return;
   //    do_orient(inst, e_gadget_site_orient_get(obj), e_gadget_site_anchor_get(obj));
   evas_object_smart_callback_del_full(obj, "gadget_created", _gadget_created, inst);
}


/**
 * Free resources when the gadget is removed
 * */
static void
convertible_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   DBG("CONVERTIBLE convertible_delete");
   Instance *inst = data;

   INF("Removing latest inst %p", inst);
   instances = eina_list_remove(instances, inst);
//   free(inst);

   // Remove callbacks
   DBG("Removing EDJE callbacks %p", inst);
   evas_object_event_callback_del(inst->o_button, EVAS_CALLBACK_DEL, convertible_del);
   elm_layout_signal_callback_del(inst->o_button, "lock,rotation", "tablet", _rotation_signal_cb);
   elm_layout_signal_callback_del(inst->o_button, "unlock,rotation", "tablet", _rotation_signal_cb);
   elm_layout_signal_callback_del(inst->o_button, "enable,keyboard", "keyboard", _keyboard_signal_cb);
   elm_layout_signal_callback_del(inst->o_button, "disable,keyboard", "keyboard", _keyboard_signal_cb);
   INF("Callbacks removed on obj %p on instance %p", inst->o_button, inst);

   // Remove dbus stuff
   INF("Removing signal handler dbus_property_changed_sh");
   eldbus_signal_handler_del(inst->dbus_property_changed_sh);
   INF("Signal handler %p removed from %p", inst->dbus_property_changed_sh, inst);

   if (instances) return;

//   // Screen related freeing
//   eina_list_free(inst->randr2_ids);

//   DBG("Freeing Instance");
//   free(inst);
   //    evas_object_smart_callback_del_full(inst->site, "gadget_site_anchor", _anchor_change, inst);
}

/**
 * Prepare to fetch the new value for the DBUS property that has changed
 * */
static void
_cb_properties_changed(void *data, const Eldbus_Message *msg)
{
   Instance *inst = (Instance *) data;
   Eldbus_Message_Iter *array, *invalidate;
   char *iface;

   if (!eldbus_message_arguments_get(msg, "sa{sv}as", &iface, &array, &invalidate))
   {
      ERR("Error getting data from properties changed signal.");
   }
   // Given that the property changed, let's get the new value
   Eldbus_Pending *pending_operation = eldbus_proxy_property_get(inst->accelerometer->sensor_proxy,
                             "AccelerometerOrientation",
                             on_accelerometer_orientation, inst);
   if (!pending_operation)
   {
      ERR("Error: could not get property AccelerometerOrientation");
   }
   INF("FUCK");
}

/**
 * Gadget creation function. It set the theme, set the icon and register all the callbacks
 * */
EINTERN Evas_Object *
convertible_create(Evas_Object *parent, int *id, E_Gadget_Site_Orient orient EINA_UNUSED)
{
   Evas_Object *o;
   // Screen related part
   E_Zone *zone = NULL;

   DBG("convertible_create entered");
   if (e_gadget_site_is_desklock(parent)) return NULL;
   if (*id == 0) *id = 1;

   DBG("creating instance");
   Instance *inst = E_NEW(Instance, 1);
   DBG("Created instance %p", inst);
   DBG("Created instance with dbus %p", dbus_accelerometer);
   inst->accelerometer = dbus_accelerometer;

//   inst->accelerometer->pending_orientation = eldbus_proxy_property_get(inst->accelerometer->sensor_proxy,
//                                                                  "AccelerometerOrientation",
//                                                                  on_accelerometer_orientation, inst);
//   if (!inst->accelerometer->pending_orientation)
//   {
//      ERR("Error: could not get property AccelerometerOrientation");
//   }

   inst->dbus_property_changed_sh = eldbus_proxy_signal_handler_add(inst->accelerometer->sensor_proxy_properties,
                                                                             "PropertiesChanged",
                                                                             _cb_properties_changed, inst);
   if (!inst->dbus_property_changed_sh)
   {
      ERR("Error: could not add the signal handler for PropertiesChanged");
   }
   INF("Signal handler %p added on %p", inst->dbus_property_changed_sh, inst);

   // TODO Should initialize those as well
   // Eldbus_Pending *pending_has_orientation, *pending_orientation, *pending_acc_claim, *pending_acc_crelease;
   inst->locked_position = EINA_FALSE;
   inst->disabled_keyboard = EINA_FALSE;

   // TODO Remove these. They are a refuse from the copy and paste of the start (or wireless) module
   //    inst->site = parent;
   //    o = elm_layout_add(parent);

   DBG("setting edje theme layer");
   // Registering the theme in order to get our small custom icon
   // TODO Consider replacing the folowing block with the commented line below
   // e_theme_edje_object_set(g, NULL, "e/gadget/wireless/wifi");

   // Setting the small icon
   o = elm_layout_add(parent);
   e_theme_edje_object_set(o, "base/theme/modules/convertible",
                           "e/modules/convertible/main");
   //edje_object_signal_emit(o, "e,state,unfocused", "e");

   inst->o_button = o;
   evas_object_size_hint_aspect_set(inst->o_button, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
   //    evas_object_smart_callback_add(parent, "gadget_site_anchor", _anchor_change, inst);

   // Initialise screen part
   DBG("Looking for the main screen");
   INF("E-comp");
   INF("Zones: %d", eina_list_count(e_comp->zones));
   Eina_List *l;
   inst->randr2_ids = NULL;
   eina_list_free(inst->randr2_ids);
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
      {
      DBG("ID: %d", zone->id);
      DBG("NAME: %s", zone->name);
      DBG("RANDR2_ID: %s", zone->randr2_id);
      DBG("W: %d", zone->w);
      DBG("H: %d", zone->h);

      // Get the screen for the zone
      E_Randr2_Screen *screen = e_randr2_screen_id_find(zone->randr2_id);
      DBG("rot_90 %i", screen->info.can_rot_90);
      DBG("namrandr2 id %s", zone->randr2_id);
      // Arbitrarily chosen a condition to check that rotation is enabled
      if (screen->info.can_rot_90 == EINA_TRUE)
         {
         DBG("Rotatable");
         DBG(zone->randr2_id);
         int max_screen_length = 100;
         char *randr2_id =  malloc(sizeof(char) * max_screen_length);
         int copied_chars = eina_strlcpy(randr2_id, zone->randr2_id, max_screen_length);
         if (copied_chars > max_screen_length)
            ERR("Screen name %s has been truncated. Cannot handle screens.", randr2_id);
         if (copied_chars < 0)
            ERR("Can't copy the screen name");

         INF("Screen name: %s", randr2_id);

         inst->randr2_ids = eina_list_append(inst->randr2_ids, randr2_id);
         if (eina_error_get())
            {
            ERR("Memory is low. List allocation failed.");
            }
         }
      }

   if (inst->randr2_ids == NULL)
      {
      ERR("Unable to find rotatable screens");
      }

   DBG("%d screen(s) has been found", eina_list_count(inst->randr2_ids));

   // Adding callback for EDJE object
   INF("Adding callback for creation and other events from EDJE");
   evas_object_smart_callback_add(parent, "gadget_created", _gadget_created, inst);
   evas_object_event_callback_add(inst->o_button, EVAS_CALLBACK_DEL, convertible_del, inst);
   elm_layout_signal_callback_add(inst->o_button, "lock,rotation", "tablet", _rotation_signal_cb, inst);
   elm_layout_signal_callback_add(inst->o_button, "unlock,rotation", "tablet", _rotation_signal_cb, inst);
   elm_layout_signal_callback_add(inst->o_button, "enable,keyboard", "keyboard", _keyboard_signal_cb, inst);
   elm_layout_signal_callback_add(inst->o_button, "disable,keyboard", "keyboard", _keyboard_signal_cb, inst);
   INF("Callbacks added to obj %p on instance %p", inst->o_button, inst);

   // Bringing in the inst ref. It is useful in the delete callback
   // TODO we may remove this one. The delete is done here. There should be no need to kep this reference.
//   convertible_module->data = inst;

   //    do_orient(inst, orient, e_gadget_site_anchor_get(parent));
   DBG("convertible_create end");

   instances = eina_list_append(instances, inst);

   return inst->o_button;
}

void convertible_gadget_init(DbusAccelerometer* accelerometer) {
   e_gadget_type_add("convertible", convertible_create, NULL);
   dbus_accelerometer = accelerometer;
}

void covnertible_gadget_shutdown()
{
   // dbus related stuff
//   DBG("Removing handler for property changed");
//   eldbus_signal_handler_unref(inst->accelerometer->dbus_property_changed_sh);
}
