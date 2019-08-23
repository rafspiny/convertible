//
// Created by raffaele on 04/05/19.
//
#include <e.h>
#include <e_module.h>
#include "convertible_logging.h"
#include "accelerometer-orientation.h"
#include "e-gadget-convertible.h"
#include "convertible.h"
#include "dbus_acceleration.h"
#include "e_mod_config.h"


// The main module reference
E_Module *convertible_module;

// Configuration
static E_Config_DD *edd;
EINTERN Convertible_Config *convertible_config;

// Logger
int _convertible_log_dom;

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
   eldbus_proxy_property_get(inst->accelerometer->sensor_proxy, "AccelerometerOrientation",
                             on_accelerometer_orientation, inst);
}

/**
 * Free resources when the gadget is removed
 * */
static void
convertible_del(void *data, Evas *e EINA_UNUSED, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
   DBG("CONVERTIBLE convertible_delete");
   Instance *inst = data;

   // Remove callbacks
   DBG("Removing EDJE callbacks");
   evas_object_event_callback_del(inst->o_button, EVAS_CALLBACK_DEL, convertible_del);
   elm_layout_signal_callback_del(inst->o_button, "lock,rotation", "tablet", _rotation_signal_cb);
   elm_layout_signal_callback_del(inst->o_button, "unlock,rotation", "tablet", _rotation_signal_cb);
   elm_layout_signal_callback_del(inst->o_button, "enable,keyboard", "keyboard", _rotation_signal_cb);
   elm_layout_signal_callback_del(inst->o_button, "disable,keyboard", "keyboard", _rotation_signal_cb);

   // dbus related stuff
   DBG("Removing handler for property changed");
   eldbus_signal_handler_unref(inst->accelerometer->dbus_property_changed_sh);

   DBG("Freeing resources");
   // TODO Should to this and wait for the release before continuing
   inst->accelerometer->pending_acc_crelease = eldbus_proxy_call(inst->accelerometer->sensor_proxy, "ReleaseAccelerometer", on_accelerometer_released, inst, -1, "");
   if (inst->accelerometer)
   {
      e_object_del(E_OBJECT(inst->accelerometer));
   }

   DBG("Shutting down ELDBUS");
   eldbus_shutdown();

   // Screen related freeing
   eina_list_free(inst->randr2_ids);

   DBG("Freeing Instance");
   free(inst);
   //    evas_object_smart_callback_del_full(inst->site, "gadget_site_anchor", _anchor_change, inst);
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
 * Gadget creation function. It set the theme, set the icon and register all the callbacks
 * */
EINTERN Evas_Object *
convertible_create(Evas_Object *parent, int *id, E_Gadget_Site_Orient orient EINA_UNUSED)
{
   Evas_Object *o;
   Instance *inst;
   char theme_overlay_path[4096];
   // Screen related part
   E_Zone *zone = NULL;

   DBG("convertible_create entered");
   if (e_gadget_site_is_desklock(parent)) return NULL;
   if (*id == 0) *id = 1;

   DBG("creating instance");
   inst = E_NEW(Instance, 1);
   inst->accelerometer = malloc(sizeof(DbusAccelerometer));
   inst->accelerometer->has_accelerometer = EINA_FALSE;
   inst->accelerometer->monitoring = EINA_FALSE;
   inst->accelerometer->acquired = EINA_FALSE;

   // The next line is probably redundant
   inst->accelerometer->orientation = malloc(sizeof(char) * 20);
   snprintf(inst->accelerometer->orientation, sizeof("undefined"), "undefined");

   inst->accelerometer->sensor_proxy = NULL;
   inst->accelerometer->sensor_proxy_properties = NULL;
   inst->accelerometer->dbus_property_changed_sh = NULL;
   //    TODO Should initialize those as well
   //    Eldbus_Pending *pending_has_orientation, *pending_orientation, *pending_acc_claim, *pending_acc_crelease;
   inst->locked_position = EINA_FALSE;
   inst->disabled_keyboard = EINA_FALSE;

   // TODO Remove these. They are a refuse from the copy and paste of the start (or wireless) module
   //    inst->site = parent;
   //    o = elm_layout_add(parent);

   DBG("setting edje theme layer");
   // Registering the theme in order to get our small custom icon
   snprintf(theme_overlay_path, sizeof(theme_overlay_path), "%s/e-module-convertible.edj", convertible_module->dir);
   DBG(theme_overlay_path);
   elm_theme_extension_add(NULL, theme_overlay_path);
   DBG("theme overlay set");

   // Setting the small icon
   o = elm_layout_add(parent);
   e_theme_edje_object_set(o, "base/theme/modules/convertible",
                           "e/modules/convertible/main");
   //edje_object_signal_emit(o, "e,state,unfocused", "e");

   inst->o_button = o;
   evas_object_size_hint_aspect_set(o, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
   //    evas_object_smart_callback_add(parent, "gadget_site_anchor", _anchor_change, inst);

   // Initialise screen part
   DBG("Looking for the main screen");
   INF("E-comp");
   INF("Zones: %d", eina_list_count(e_comp->zones));
   Eina_List *l;
   inst->randr2_ids = NULL;
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
   evas_object_event_callback_add(o, EVAS_CALLBACK_DEL, convertible_del, inst);
   elm_layout_signal_callback_add(o, "lock,rotation", "tablet", _rotation_signal_cb, inst);
   elm_layout_signal_callback_add(o, "unlock,rotation", "tablet", _rotation_signal_cb, inst);
   elm_layout_signal_callback_add(o, "enable,keyboard", "keyboard", _keyboard_signal_cb, inst);
   elm_layout_signal_callback_add(o, "disable,keyboard", "keyboard", _keyboard_signal_cb, inst);

   // Bringing in the instance ref. It is useful in the delete callback
   convertible_module->data = inst;

   // Initialise DBUS component
   DBG("Before eldbus initialization");
   int initialization = eldbus_init();
   if (initialization == EXIT_FAILURE)
   {
      ERR("Unable to initialise ELDBUS");
   }

   INF("Before get dbus interface");
   inst->accelerometer->sensor_proxy = get_dbus_interface(EFL_DBUS_ACC_IFACE);
   inst->accelerometer->sensor_proxy_properties = get_dbus_interface(ELDBUS_FDO_INTERFACE_PROPERTIES);
   if (inst->accelerometer->sensor_proxy == NULL)
   {
      ERR("Unable to get the proxy for interface %s", EFL_DBUS_ACC_IFACE);
   }

   inst->accelerometer->pending_has_orientation = eldbus_proxy_property_get(inst->accelerometer->sensor_proxy,
                                                                            "HasAccelerometer", on_has_accelerometer,
                                                                            inst);
   inst->accelerometer->pending_orientation = eldbus_proxy_property_get(inst->accelerometer->sensor_proxy,
                                                                        "AccelerometerOrientation",
                                                                        on_accelerometer_orientation, inst);
   if (!inst->accelerometer->pending_has_orientation)
   {
      ERR("Error: could not get property HasAccelerometer");
   }
   if (!inst->accelerometer->pending_orientation)
   {
      ERR("Error: could not get property AccelerometerOrientation");
   }

   // Claim the accelerometer
   inst->accelerometer->pending_acc_claim = eldbus_proxy_call(inst->accelerometer->sensor_proxy, "ClaimAccelerometer",
                                                              on_accelerometer_claimed, inst, -1, "");

   if (!inst->accelerometer->pending_acc_claim)
   {
      ERR("Error: could not call ClaimAccelerometer\n");
   }
   inst->accelerometer->dbus_property_changed_sh = eldbus_proxy_signal_handler_add(inst->accelerometer->sensor_proxy_properties,
                                                               "PropertiesChanged",
                                                               _cb_properties_changed, inst);

   //    do_orient(inst, orient, e_gadget_site_anchor_get(parent));
   DBG("convertible_create end");

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

   econvertible_config_init(NULL);

   INF("Setting the callback for creation");
   e_gadget_type_add("convertible", convertible_create, NULL);

   e_configure_registry_category_add("extensions", 90, "Extensions", NULL,
                                     "preferences-extensions");
   e_configure_registry_item_add("extensions/convertible", 30, "convertible", NULL,
                                 "preferences-desktop-convertible", e_int_config_convertible_module);

//   evas_object_event_callback_add(inst->gadget, EVAS_CALLBACK_MOUSE_DOWN,
//                                  _mouse_down_cb, inst);
   return m;
}

E_API int
e_modapi_shutdown(E_Module *m EINA_UNUSED)
{
   INF("Freing configuration");
   E_CONFIG_DD_FREE(edd);
   E_FREE(convertible_config);

   INF("Shutting down the module");
   convertible_module = NULL;
   //    e_gadcon_provider_unregister(&_gadcon_class);
   e_gadget_type_del("convertible");

   // Removing logger
   DBG("Removing the logger");
   eina_log_domain_unregister(_convertible_log_dom);
   _convertible_log_dom = -1;

   return 1;
}

E_API int
e_modapi_save(E_Module *m EINA_UNUSED)
{
   e_config_domain_save("module.convertible", edd, convertible_config);
   return 1;
}
