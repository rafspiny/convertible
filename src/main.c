//
// Created by raffaele on 28/12/19.
//

#include <X11/extensions/XInput2.h>
#include <stdio.h>

int main(int argc, char** argv)
{
   char str[20];
   scanf("%[^\n]%*c", str);
   char * prop = "Coordinate Transformation Matrix";
   printf("Hello world\n");
   Display* disp = XOpenDisplay(0);
   if (!disp) {
      fprintf(stderr, "[get_prop] Open display failed for\n");
      return;
   }

   Atom prop_id = XInternAtom(disp, prop, True);
   if (prop_id == None) {
      XCloseDisplay(disp);
      fprintf(stderr, "[get_prop] Intern atom %s failed\n", prop);
      return;
   }

   int devive_number = 15;
   Atom a_type = 0;
   int fmt = 0;
   unsigned long num = 0, dummy;
   unsigned char *data = NULL;
   unsigned char *d = NULL;

   int ret = XIGetProperty(disp, devive_number,prop_id, 0, 65536, False, AnyPropertyType, &a_type, &fmt, &num, &dummy, &data);
   printf("Ret: %d\n", ret);
   printf("FMT: %d\n", fmt);
   printf("NUM: %d\n", num);
   printf("Dummy: %f\n", dummy);
   if (XIGetProperty(disp, devive_number,prop_id, 0, 65536, False, AnyPropertyType, &a_type, &fmt, &num, &dummy, &data) != 0) {
      printf("Error\n");
   }
   if (!XIGetProperty(disp, devive_number,prop_id, 0, 65536, False, AnyPropertyType, &a_type, &fmt, &num, &dummy, &data)) {
      printf("Error\n");
   }
   int ndevices_return;
   XIDeviceInfo* device_info = XIQueryDevice(disp, devive_number, &ndevices_return);
   return;
}