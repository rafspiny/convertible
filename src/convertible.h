//
// Created by raffaele on 04/05/19.
//

#include "dbus_acceleration.h"

#ifndef E_GADGET_CONVERTIBLE_CONVERTIBLE_H
#define E_GADGET_CONVERTIBLE_CONVERTIBLE_H

typedef struct _Instance Instance;

struct _Instance
{
    E_Gadcon_Client     *gcc;
    Evas_Object         *o_button;
    Evas_Object         *box;
    struct DbusAccelerometer   *accelerometer;
};

void convertible_gadget_init(void);
void convertible_gadget_shutdown(void);

struct Convertible_Config {
    Eina_Bool monitoring;
    Eina_Bool disable_keykboard_on_non_standard_position;
    E_Screen *main_screen;
};

#endif //E_GADGET_CONVERTIBLE_CONVERTIBLE_H
