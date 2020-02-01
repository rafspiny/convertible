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
Instance *inst;

// Configuration
extern Convertible_Config *convertible_config;
extern E_Config_DD *edd;

// Logger
int _convertible_log_dom;

/* module setup */
E_API E_Module_Api e_modapi =
     {
                E_MODULE_API_VERSION,
                "convertible"
     };

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
      ERR("Error getting data from properties changed signal.");
   // Given that the property changed, let's get the new value
   Eldbus_Pending *pending_operation = eldbus_proxy_property_get(inst->accelerometer->sensor_proxy,
                                                                 "AccelerometerOrientation",
                                                                 on_accelerometer_orientation, inst);
   if (!pending_operation)
      ERR("Error: could not get property AccelerometerOrientation");
}

E_API void *
e_modapi_init(E_Module *m)
{
   // Initialise the logger
   _convertible_log_dom = eina_log_domain_register("convertible", EINA_COLOR_LIGHTBLUE);

   convertible_module = m;
   // It looks like this is not needed right now
   //    e_gadcon_provider_register(&_gadcon_class);
   char theme_overlay_path[PATH_MAX];
   snprintf(theme_overlay_path, sizeof(theme_overlay_path), "%s/e-module-convertible.edj", convertible_module->dir);
   elm_theme_extension_add(NULL, theme_overlay_path);

   econvertible_config_init(NULL);

   // Config DBus
   DbusAccelerometer* accelerometer = sensor_proxy_init();

   DBG("creating instance");
   inst = E_NEW(Instance, 1);
   inst->accelerometer = accelerometer;

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
      ERR("Error: could not add the signal handler for PropertiesChanged");

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

   // Screen related part
   E_Zone *zone = NULL;

   // Initialise screen part
   DBG("Looking for the main screen");
   Eina_List *l;
   inst->randr2_ids = NULL;
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
      {
      // Get the screen for the zone
      E_Randr2_Screen *screen = e_randr2_screen_id_find(zone->randr2_id);
      DBG("name randr2 id %s", zone->randr2_id);
      DBG("rot_90 %i", screen->info.can_rot_90);
      // Arbitrarily chosen a condition to check that rotation is enabled
      if (screen->info.can_rot_90 == EINA_TRUE)
         {
         int max_screen_length = 300;
         char *randr2_id =  malloc(sizeof(char) * max_screen_length);
         int copied_chars = eina_strlcpy(randr2_id, zone->randr2_id, max_screen_length);
         if (copied_chars > max_screen_length)
            ERR("Screen name %s has been truncated. Cannot handle screens.", randr2_id);
         if (copied_chars < 0)
            ERR("Can't copy the screen name");

         inst->randr2_ids = eina_list_append(inst->randr2_ids, randr2_id);
         if (eina_error_get())
            ERR("Memory is low. List allocation failed.");
         }
      }

   if (inst->randr2_ids == NULL)
      ERR("Unable to find rotatable screens");

   DBG("%d screen(s) has been found", eina_list_count(inst->randr2_ids));

   INF("Setting the callback for gadget creation");
   convertible_gadget_init(inst);

   INF("Creating menu entries for settings");
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
   INF("Freeing configuration");
   econvertible_config_shutdown();

   e_configure_registry_item_del("extensions/convertible");

   // Shutdown Dbus
   sensor_proxy_shutdown();

   // Remove dbus stuff
   INF("Removing signal handler dbus_property_changed_sh");
   eldbus_signal_handler_del(inst->dbus_property_changed_sh);

   // Remove screen info
   char *element;
   EINA_LIST_FREE(inst->randr2_ids, element)
      free(element);

   free(inst);

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
   if (convertible_config)
   {
      e_config_domain_save("module.convertible", edd, convertible_config);
   }
   return 1;
}
