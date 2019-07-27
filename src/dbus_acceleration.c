//
// Created by raffaele on 01/05/19.
//
#include "convertible_logging.h"
#include "accelerometer-orientation.h"
#include "dbus_acceleration.h"
#include "convertible.h"

int _convertible_rotation_get(const char *orientation);

Eldbus_Proxy *get_dbus_interface(const char *IFACE)
{
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
      INF("Object fetched.");
   }
   sensor_proxy = eldbus_proxy_get(obj, IFACE);
   if (!sensor_proxy)
   {
      ERR("Error: could not get proxy for interface %s", IFACE);
   }
   else
   {
      INF("Proxy fetched");
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
   Instance *inst = (Instance *) data;
   inst->accelerometer->has_accelerometer = has_accelerometer;
   WARN("Has Accelerometer: %d", inst->accelerometer->has_accelerometer);
}

void
on_accelerometer_orientation(void *data, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
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
   WARN("Current Orientation: %s", inst->accelerometer->orientation);
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
   screen_randr_cfg->rotation = rotation;
   e_randr2_config_apply();
   DBG("Screen %s rotated to %d", randr_id, rotation);
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
   WARN("Rotation: %d", rotation);
   return rotation;
   }

void
on_accelerometer_claimed(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
   {
      ERR("Error: %s %s", errname, errmsg);
      return;
   }

   // set the acquired field
   Instance *inst = (Instance *) data;
   inst->accelerometer->acquired = EINA_TRUE;
}

void
on_accelerometer_released(void *data EINA_UNUSED, const Eldbus_Message *msg, Eldbus_Pending *pending EINA_UNUSED)
{
   const char *errname, *errmsg;

   if (eldbus_message_error_get(msg, &errname, &errmsg))
   {
      ERR("Error: %s %s", errname, errmsg);
      return;
   }
   // unset the acquired field
   Instance *inst = (Instance *) data;
   inst->accelerometer->acquired = EINA_FALSE;
}
