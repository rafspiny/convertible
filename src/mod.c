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

   INF("Setting the callback for gadget creation");
   convertible_gadget_init(accelerometer);

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

   // Shutting down the gadget
   convertible_gadget_shutdown();

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
