//
// Created by raffaele on 01/05/19.
//
#include "convertible_logging.h"
#include "accelerometer-orientation.h"
#include "dbus_acceleration.h"
#include "convertible.h"

DbusAccelerometer* sensor_proxy_init() {
   // Initialise DBUS component
   if (accelerometer_dbus != NULL)
   {
      WARN("We already have a struct filled");
      return accelerometer_dbus;
   }
   accelerometer_dbus  = malloc(sizeof(DbusAccelerometer));
   // TODO Double check if we need these initializations
   accelerometer_dbus->has_accelerometer = EINA_FALSE;
   accelerometer_dbus->monitoring = EINA_FALSE;
   accelerometer_dbus->acquired = EINA_FALSE;

   // The next line is probably redundant
   accelerometer_dbus->orientation = malloc(sizeof(char) * 20);
   snprintf(accelerometer_dbus->orientation, sizeof("undefined"), "undefined");

   accelerometer_dbus->sensor_proxy = NULL;
   accelerometer_dbus->sensor_proxy_properties = NULL;

   DBG("Before eldbus initialization");
   int initialization = eldbus_init();
   if (initialization == EXIT_FAILURE)
   {
      ERR("Unable to initialise ELDBUS");
   }

   INF("Getting dbus interfaces");
   accelerometer_dbus->sensor_proxy = get_dbus_interface(EFL_DBUS_ACC_IFACE);
   accelerometer_dbus->sensor_proxy_properties = get_dbus_interface(ELDBUS_FDO_INTERFACE_PROPERTIES);
   if (accelerometer_dbus->sensor_proxy == NULL)
   {
      ERR("Unable to get the proxy for interface %s", EFL_DBUS_ACC_IFACE);
   }

   accelerometer_dbus->pending_has_orientation = eldbus_proxy_property_get(accelerometer_dbus->sensor_proxy,
                                                                            "HasAccelerometer", on_has_accelerometer,
                                                                            accelerometer_dbus);
   if (!accelerometer_dbus->pending_has_orientation)
   {
      ERR("Error: could not get property HasAccelerometer");
   }

   // Claim the accelerometer_dbus
   accelerometer_dbus->pending_acc_claim = eldbus_proxy_call(accelerometer_dbus->sensor_proxy, "ClaimAccelerometer",
                                                              on_accelerometer_claimed, accelerometer_dbus, -1, "");

   if (!accelerometer_dbus->pending_acc_claim)
   {
      ERR("Error: could not call ClaimAccelerometer\n");
   }

   return accelerometer_dbus;
}

void sensor_proxy_shutdown()
{
   INF("Freeing convertible resources");
   // TODO Should to this and wait for the release before continuing
   accelerometer_dbus->pending_acc_crelease = eldbus_proxy_call(accelerometer_dbus->sensor_proxy, "ReleaseAccelerometer", on_accelerometer_released, accelerometer_dbus, -1, "");
   if (accelerometer_dbus)
   {
      e_object_del(E_OBJECT(accelerometer_dbus));
      free(accelerometer_dbus);
      accelerometer_dbus = NULL;
   }

   DBG("Shutting down ELDBUS");
   eldbus_shutdown();
}

int _convertible_rotation_get(const char *orientation);

Eldbus_Proxy *get_dbus_interface(const char *IFACE)
{
   DBG("Working on interface: %s", IFACE);
   Eldbus_Connection *conn;
   Eldbus_Object *obj;
   Eldbus_Proxy *sensor_proxy;

   conn = eldbus_connection_get(ELDBUS_CONNECTION_TYPE_SYSTEM);
   if (!conn)
   {
      ERR("Error: could not get system bus");
   }
   obj = eldbus_object_get(conn, EFL_DBUS_ACC_BUS, EFL_DBUS_ACC_PATH);
   if (!obj)
   {
      ERR("Error: could not get object");
   }
   else
   {
      DBG("Object fetched.");
   }
   sensor_proxy = eldbus_proxy_get(obj, IFACE);
   if (!sensor_proxy)
   {
      ERR("Error: could not get proxy for interface %s", IFACE);
   }
   else
   {
      DBG("Proxy fetched");
   }

   return sensor_proxy;
}

Eina_Bool access_string_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, char **string_property_value)
{
   const char *type;
   Eina_Bool res = EINA_TRUE;

   if (!eldbus_message_arguments_get(msg, "v", variant))
   {
      WARN("Error getting arguments.");
      res = EINA_FALSE;
   }
   type = eldbus_message_iter_signature_get((*variant));
   if (type[1])
   {
      WARN("It is a complex type, not handle yet.");
      res = EINA_FALSE;
   }
   if (type[0] != 's')
   {
      WARN("Expected type is string(s).");
      res = EINA_FALSE;
   }
   if (!eldbus_message_iter_arguments_get((*variant), "s", string_property_value))
   {
      WARN("error in eldbus_message_iter_arguments_get()");
      res = EINA_FALSE;
   }
   free((void *) type);
   return res;
}

Eina_Bool
access_bool_property(const Eldbus_Message *msg, Eldbus_Message_Iter **variant, Eina_Bool *boolean_property_value)
{
   const char *type;
   Eina_Bool res = EINA_TRUE;

   if (!eldbus_message_arguments_get(msg, "v", variant))
   {
      WARN("Error getting arguments.");
      res = EINA_FALSE;
   }
   type = eldbus_message_iter_signature_get((*variant));
   if (type[1])
   {
      WARN("It is a complex type, not handle yet.");
      res = EINA_FALSE;
   }
   if (type[0] != 'b')
   {
      WARN("Expected type is int.");
      res = EINA_FALSE;
   }
   if (!eldbus_message_iter_arguments_get((*variant), "b", boolean_property_value))
   {
      WARN("error in eldbus_message_iter_arguments_get()");
      res = EINA_FALSE;
   }
   free((void *) type);
   return res;
}

void
on_has_accelerometer(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;
   Eina_Bool has_accelerometer = EINA_FALSE;
   Eldbus_Message_Iter *variant = NULL;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
   {
      ERR("Error: %s %s", errname, errmsg);
   }

   access_bool_property(msg, &variant, &has_accelerometer);
   DbusAccelerometer *accelerometer = (DbusAccelerometer *) data;
   accelerometer->has_accelerometer = has_accelerometer;
   DBG("Has Accelerometer: %d", accelerometer->has_accelerometer);
}

void
on_accelerometer_orientation(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   INF("New orientation received");
   Instance *inst = (Instance *) data;

   if (inst->locked_position == EINA_TRUE)
   {
      WARN("Locked position. Ignoring rotation");
      return;
   }


   const char *errname, *errmsg;
   char *orientation = malloc(sizeof(char[20]));
   Eldbus_Message_Iter *variant = NULL;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
   {
      ERR("Error: %s %s", errname, errmsg);
      return;
   }

   access_string_property(msg, &variant, &orientation);
   inst->accelerometer->orientation = orientation;
   DBG("Current Orientation: %s", inst->accelerometer->orientation);
   int rotation = _convertible_rotation_get(orientation);

   if (inst->randr2_ids == NULL)
      ERR("Screen not set.");
   else
   {
      WARN("Setting screen(s) rotation to %d", rotation);


      Eina_List *l;
      const char *randr_id = NULL;
      EINA_LIST_FOREACH(inst->randr2_ids, l, randr_id)
      {
         _fetch_and_rotate_screen(randr_id, rotation);
      }
      free((void *)randr_id);
   }
}

void _fetch_and_rotate_screen(const char* randr_id, int rotation) {
   DBG("Working on screen %s", randr_id);
   E_Randr2_Screen *rotatable_screen = e_randr2_screen_id_find(randr_id);
   E_Config_Randr2_Screen *screen_randr_cfg = e_randr2_config_screen_find(rotatable_screen, e_randr2_cfg);
   DBG("Screen %s is going to be rotated to %d", randr_id, rotation);

   if (rotation == screen_randr_cfg->rotation)
   {
      WARN("Screen %s is already rotated to %d degrees", randr_id, rotation);
   } else
   {
      screen_randr_cfg->rotation = rotation;
      e_randr2_config_apply();
      DBG("Screen %s rotated to %d", randr_id, rotation);
   }
}

int _convertible_rotation_get(const char *orientation)
   {
   int rotation = 0;
   // TODO Should really check for inst->main_screen->info.can_rot_x
   if (strcmp(ACCELEROMETER_ORIENTATION_RIGHT, orientation) == 0)
         rotation = 270;
   if (strcmp(ACCELEROMETER_ORIENTATION_LEFT, orientation) == 0)
         rotation = 90;
   if (strcmp(ACCELEROMETER_ORIENTATION_BOTTOM, orientation) == 0)
         rotation = 180;
   DBG("Rotation: %d", rotation);
   return rotation;
   }

void
on_accelerometer_claimed(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   INF("Going to claim the accelerometer_dbus");
   if (eldbus_message_error_get(msg, &errname, &errmsg))
   {
      ERR("Error: %s %s", errname, errmsg);
      return;
   }
   INF("Accelerometer claimed");

   // set the acquired field
   DbusAccelerometer *accelerometer = (DbusAccelerometer *) data;
   accelerometer->acquired = EINA_TRUE;
}

void
on_accelerometer_released(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   INF("Going to release the accelerometer_dbus");
   if (eldbus_message_error_get(msg, &errname, &errmsg))
   {
      ERR("Error: %s %s", errname, errmsg);
      return;
   }
   INF("Accelerometer released");
   // unset the acquired field
   DbusAccelerometer *accelerometer = (DbusAccelerometer *) data;
   if (accelerometer)
   {
      accelerometer->acquired = EINA_FALSE;
   }

}
