#include <Ecore.h>

#ifndef EFL_DBUS_ACCELERATION
#define EFL_DBUS_ACCELERATION

#define EFL_DBUS_ACC_BUS "net.hadess.SensorProxy"
#define EFL_DBUS_ACC_PATH "/net/hadess/SensorProxy"
#define EFL_DBUS_ACC_IFACE "net.hadess.SensorProxy"

static int _client_log_dom = -1;
#define ERR(...)      EINA_LOG_DOM_ERR(_client_log_dom, __VA_ARGS__)
#define DBG(...)      EINA_LOG_DOM_DBG(_client_log_dom, __VA_ARGS__)


struct DbusAccelerometer {
    Eina_Bool has_accelerometer;
    char *orientation;
};
#endif