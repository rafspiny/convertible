//
// Created by raffaele on 04/05/19.
//
#include <e.h>
#include "dbus_acceleration.h"


#ifndef E_GADGET_CONVERTIBLE_CONVERTIBLE_H
#define E_GADGET_CONVERTIBLE_CONVERTIBLE_H

typedef struct _Instance Instance;

struct _Instance
{
    Evas_Object         *o_button;
    Evas_Object         *box;
    DbusAccelerometer   *accelerometer;
    Eina_List           *randr2_ids;

    Eina_Bool locked_position;
    Eina_Bool disabled_keyboard;
};

struct Convertible_Config {
    Eina_Bool monitoring;
    Eina_Bool disable_keyboard_on_non_standard_position;
};

#endif //E_GADGET_CONVERTIBLE_CONVERTIBLE_H
