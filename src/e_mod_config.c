//
// Created by raffaele on 01/08/19.
//
#include "convertible_logging.h"
#include "e.h"
#include "e_mod_config.h"

static Convertible_Config *_config = NULL;
static E_Config_DD *cd = NULL;
static E_Config_DD *c_zone;

/**
 * Function to compare data from a Node (Eina_List) with a string
 * @param data1 The Eina_list data (Convertible_Zone_Config)
 * @param data2 The name of the zone. Type is char*
 * @return
 */
static int
_cb_list_zone_comparator(const void *data1, const void *data2)
{
   const Convertible_Zone_Config *d1;
   const char *d2;

   if (!(d1 = data1)) return 1;
   if (!d1->name) return 1;
   if (!(d2 = data2)) return -1;
   return strcmp(d1->name, d2);
}

static void
_econvertible_config_dd_new(void)
{
   c_zone = E_CONFIG_DD_NEW("Econvertible_Zone_Config", Convertible_Zone_Config);
   E_CONFIG_VAL(c_zone, Convertible_Zone_Config, name, STR);
   E_CONFIG_VAL(c_zone, Convertible_Zone_Config, follow_rotation, INT);

   cd = E_CONFIG_DD_NEW("Convertible_Config", Convertible_Config);

   E_CONFIG_VAL(cd, Convertible_Config, monitoring, STR);
   E_CONFIG_VAL(cd, Convertible_Config, disable_keyboard_on_rotation, INT);
   E_CONFIG_LIST(cd, Convertible_Config, rotatable_screen_configuration, c_zone);
}

/**
 * Update the *_config data structure based on the settings coming from the dialog panel
 * @param config The config coming from the Dialog Panel (E_Config_Dialog_data)
 */
static void
_config_set(Convertible_Config *config)
{
   Eina_List *l;
   Convertible_Zone_Config *zone_config;
//   if ((config->disable_keyboard_on_rotation) && (_config->disable_keyboard_on_rotation != config->disable_keyboard_on_rotation))
   _config->disable_keyboard_on_rotation = config->disable_keyboard_on_rotation;
   _config->monitoring = config->monitoring;

   DBG("SAVING CONFIG %d %d", config->disable_keyboard_on_rotation, config->monitoring);
   EINA_LIST_FOREACH(config->rotatable_screen_configuration, l, zone_config)
   {
      DBG("Zone %s with input %d", zone_config->name, zone_config->follow_rotation);
   }
   e_config_domain_save("module.convertible", cd, config);
}

Eina_List* fetch_screen_list()
{
   INF("Looking for the main screen");
   Eina_List *l;
   Eina_List *screens = NULL;
   E_Zone *zone = NULL;
   EINA_LIST_FOREACH(e_comp->zones, l, zone)
   {
      // Get the screen for the zone
      E_Randr2_Screen *screen = e_randr2_screen_id_find(zone->randr2_id);
      // Arbitrarily chosen a condition to check that rotation is enabled
      if (screen->info.can_rot_90 == EINA_TRUE)
      {
         int max_screen_length = 100;
         char *randr2_id =  malloc(sizeof(char) * max_screen_length);
         int copied_chars = eina_strlcpy(randr2_id, zone->randr2_id, max_screen_length);
         if (copied_chars > max_screen_length)
            ERR("Screen name %s has been truncated. Cannot handle screens.", randr2_id);
         if (copied_chars < 0)
            ERR("Can't copy the screen name");

         screens = eina_list_append(screens, randr2_id);
         if (eina_error_get())
         {
            ERR("Memory is low. List allocation failed.");
         }
      }
   }

   return screens;
}

/**
 * Initialise the configuration object
 *
 * @param cfg
 * @return
 */
static void*
_create_data(E_Config_Dialog *cfg EINA_UNUSED)
{
   E_Config_Dialog_Data *dialog_data;

   Eina_List *l;
   Convertible_Zone_Config *zone_config;

   dialog_data = E_NEW(E_Config_Dialog_Data, 1);
   dialog_data->config = malloc(sizeof(Convertible_Config));
   dialog_data->config->monitoring = EINA_TRUE;
   dialog_data->config->disable_keyboard_on_rotation = EINA_TRUE;
   dialog_data->config->rotatable_screen_configuration = NULL;

   // TODO Read from the Instance or the current Configuration object, the list of zones

   Eina_List *screens = fetch_screen_list();
   char *randr2_id = NULL;
   EINA_LIST_FOREACH(screens, l, randr2_id)
   {
      zone_config = E_NEW(Convertible_Zone_Config, 1);
      zone_config->name = randr2_id;
      zone_config->follow_rotation = EINA_TRUE;
      dialog_data->config->rotatable_screen_configuration = eina_list_append(dialog_data->config->rotatable_screen_configuration, zone_config);
   }

   DBG("AFTER create_data");
   DBG("Value %d", eina_list_count(dialog_data->config->rotatable_screen_configuration));
   return dialog_data;
}

/**
 * Release memory for the data structure holding the configuration
 *
 * @param c
 * @param cf
 */
static void
_free_data(E_Config_Dialog *c EINA_UNUSED, E_Config_Dialog_Data *cf)
{
   DBG("BEFORE free_data");
   eina_list_free(cf->config->rotatable_screen_configuration);
   free(cf->config);
   free(cf);
   DBG("AFTER free_data");
}

/**
 * This function should store the modified settings into the data structure referred by the pointer _config
 * @param cfd
 * @param cfdata
 * @return
 */
static int
_basic_apply_data(E_Config_Dialog *cfd EINA_UNUSED, E_Config_Dialog_Data *cfdata)
{
   // TODO Finish it
   _config_set(cfdata->config);
   return 1;
}

/**
 * Create the panel by adding all the items like labels, checkbox and lists
 *
 * @param cfd
 * @param evas
 * @param cfdata
 * @return
 */
static Evas_Object *
_basic_create_widgets(E_Config_Dialog *cfd EINA_UNUSED, Evas *evas,
                      E_Config_Dialog_Data *cfdata)
{
   DBG("BEFORE basci_create_widget");
   Evas_Object *o, *list_item_screen, *evas_option_input_disable, *evas_label_section_screens;
   Evas_Object *screen_list;

   o = e_widget_list_add(evas, 0, 0);

   evas_option_input_disable = e_widget_check_add(evas, "Disable input when rotated", &cfdata->config->disable_keyboard_on_rotation);
   e_widget_list_object_append(o, evas_option_input_disable, 0, 0, 0);

   evas_label_section_screens = e_widget_framelist_add(evas, "Rotatable zones", 0);

   Convertible_Zone_Config *zone_config;
   char *screen_name;
   Eina_List *l;
   DBG("Value %d", eina_list_count(cfdata->config->rotatable_screen_configuration));
   EINA_LIST_FOREACH(cfdata->config->rotatable_screen_configuration, l, zone_config)
   {
      screen_name = zone_config->name;
      DBG("Zone %s", screen_name);
      // Get the configuration for the current zone
      Eina_List *current_node = eina_list_search_unsorted_list(cfdata->config->rotatable_screen_configuration, _cb_list_zone_comparator, screen_name);
      if (current_node == NULL)
      {
         ERR("It looks like there is no node for zone '%s'", screen_name);
      }
      Convertible_Zone_Config *current_zone_config = (Convertible_Zone_Config*) eina_list_data_get(current_node);

      list_item_screen = e_widget_check_add(evas, screen_name, &(current_zone_config->follow_rotation));
      e_widget_ilist_append(screen_list, NULL, screen_name, NULL, NULL, NULL);
      e_widget_framelist_object_append(evas_label_section_screens, list_item_screen);
   }

   e_widget_list_object_append(o, evas_label_section_screens, 1, 0, 0.5);

   DBG("After basic_create_widget");
   return o;
   }

/**
 * This function initialise the config dialog for the module
 * @param comp
 * @param p
 * @return
 */
E_Config_Dialog*
e_int_config_convertible_module(Evas_Object *comp, const char *p EINA_UNUSED)
{
   E_Config_Dialog *cfd;
   E_Config_Dialog_View *v;

   DBG("BEFORE finding dialog");

   if (e_config_dialog_find("E", "windows/convertible"))
      return NULL;

   DBG("AFTER finding dialog");

   v = E_NEW(E_Config_Dialog_View, 1);
   v->create_cfdata = _create_data;
   v->free_cfdata = _free_data;
   v->basic.apply_cfdata = _basic_apply_data;
   v->basic.create_widgets = _basic_create_widgets;

   cfd = e_config_dialog_new(comp,
                             "Convertible Configuration",
                             "E", "windows/convertible",
                             NULL,
                             0, v, NULL);
   return cfd;
}

void
econvertible_config_init(const void *userdata)
{
   // TODO Not sure what his line does. Apparently, it is needed to specify the type of the configuration data structure
   _econvertible_config_dd_new();
   _config = e_config_domain_load("module.econvertible", cd);
   if (!_config)
   {
      _config = E_NEW(Convertible_Config, 1);
      _config->disable_keyboard_on_rotation = EINA_TRUE;
      _config->monitoring = EINA_TRUE;
      _config->rotatable_screen_configuration = NULL;
   }

//   if (_config->save == 0) _config->save = 1;
   DBG("Config loaded");
}
