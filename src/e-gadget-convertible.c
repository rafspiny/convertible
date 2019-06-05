//
// Created by raffaele on 04/05/19.
//
#include "convertible_logging.h"
#include "e-gadget-convertible.h"

void _button_cb_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info) {
    WARN("Mouse clicked");
}

void _rotation_signal_cb(void *data, Evas_Object *obj, const char *sig EINA_UNUSED, const char *src EINA_UNUSED) {
    WARN("Rotation: Signal %s received from %s", sig, src);
}
void _keyboard_signal_cb(void *data, Evas_Object *obj, const char *sig EINA_UNUSED, const char *src EINA_UNUSED) {
    WARN("Keyboard: Signal %s received from %s", sig, src);
}
