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
    Evas_Object           *o_button;
    Evas_Object           *popup;
    DbusAccelerometer     *accelerometer;
    // I hate to put DBUS related stuff in this struct. Unfortunately, I did not (quickly) find a better way of
    // handling signals across multiple instances sharing one DbusAccelerometer struct.
    Eldbus_Signal_Handler *dbus_property_changed_sh;
    Eina_List             *randr2_ids;

    Eina_Bool             locked_position;
    Eina_Bool             disabled_keyboard;
};

#endif //E_GADGET_CONVERTIBLE_CONVERTIBLE_H
