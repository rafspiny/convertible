//
// Created by raffaele on 04/05/19.
//
#include "convertible_logging.h"
#include "e-gadget-convertible.h"
#include "convertible.h"

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
