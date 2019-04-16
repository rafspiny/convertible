#include <Ecore.h>
#include <Elementary.h>
#include <e_gadget_types.h>


static E_Gadget_Site_Orient gorient;
static E_Gadget_Site_Anchor ganchor;

static void
update_anchor_orient(void *data, E_Gadget_Site_Orient orient, E_Gadget_Site_Anchor anchor)
{
    char buf[4096];
    const char *s = "float";
    Evas_Object *lbl = data;

    if (anchor & E_GADGET_SITE_ANCHOR_LEFT)
    {
        if (anchor & E_GADGET_SITE_ANCHOR_TOP)
        {
            switch (orient)
            {
                case E_GADGET_SITE_ORIENT_HORIZONTAL:
                    s = "top_left";
                    break;
                case E_GADGET_SITE_ORIENT_VERTICAL:
                    s = "left_top";
                    break;
                case E_GADGET_SITE_ORIENT_NONE:
                    s = "left_top";
                    break;
            }
        }
        else if (anchor & E_GADGET_SITE_ANCHOR_BOTTOM)
        {
            switch (orient)
            {
                case E_GADGET_SITE_ORIENT_HORIZONTAL:
                    s = "bottom_left";
                    break;
                case E_GADGET_SITE_ORIENT_VERTICAL:
                    s = "left_bottom";
                    break;
                case E_GADGET_SITE_ORIENT_NONE:
                    s = "left_bottom";
                    break;
            }
        }
        else
            s = "left";
    }
    else if (anchor & E_GADGET_SITE_ANCHOR_RIGHT)
    {
        if (anchor & E_GADGET_SITE_ANCHOR_TOP)
        {
            switch (orient)
            {
                case E_GADGET_SITE_ORIENT_HORIZONTAL:
                    s = "top_right";
                    break;
                case E_GADGET_SITE_ORIENT_VERTICAL:
                    s = "right_top";
                    break;
                case E_GADGET_SITE_ORIENT_NONE:
                    s = "right_top";
                    break;
            }
        }
        else if (anchor & E_GADGET_SITE_ANCHOR_BOTTOM)
        {
            switch (orient)
            {
                case E_GADGET_SITE_ORIENT_HORIZONTAL:
                    s = "bottom_right";
                    break;
                case E_GADGET_SITE_ORIENT_VERTICAL:
                    s = "right_bottom";
                    break;
                case E_GADGET_SITE_ORIENT_NONE:
                    s = "right_bottom";
                    break;
            }
        }
        else
            s = "right";
    }
    else if (anchor & E_GADGET_SITE_ANCHOR_TOP)
        s = "top";
    else if (anchor & E_GADGET_SITE_ANCHOR_BOTTOM)
        s = "bottom";
    else
    {
        switch (orient)
        {
            case E_GADGET_SITE_ORIENT_HORIZONTAL:
                s = "horizontal";
                break;
            case E_GADGET_SITE_ORIENT_VERTICAL:
                s = "vertical";
                break;
            default: break;
        }
    }
    snprintf(buf, sizeof(buf), "e,state,orientation,%s", s);
    elm_object_text_set(lbl, buf);
}

static void
anchor_change(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    ganchor = (uintptr_t)event_info;
    update_anchor_orient(data, gorient, ganchor);
}

static void
orient_change(void *data, Evas_Object *obj EINA_UNUSED, void *event_info)
{
    gorient = (uintptr_t)event_info;
    update_anchor_orient(data, gorient, ganchor);
}

static void
on_click(void *data, Evas_Object *obj, void *event_info)
{
    evas_object_del(data);
}

EAPI_MAIN int
elm_main(int argc, char **argv)
{
    Evas_Object *win, *btn, *icon;
    int gadget =0, id_num = 0;
    char buf[16];

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

    if (getenv("E_GADGET_ID"))
    {
        gadget = 1;
        snprintf(buf, sizeof(buf), "%s", getenv("E_GADGET_ID"));
        id_num = atoi(buf);
    }

    win = elm_win_add(NULL, "Main", ELM_WIN_BASIC);
    elm_win_title_set(win, "Hello, World!");
    elm_win_autodel_set(win, EINA_TRUE);

    if (gadget) elm_win_alpha_set(win, EINA_TRUE);

    if (gadget && id_num == -1)
    {
        icon = elm_icon_add(win);
        elm_icon_standard_set(icon, "start-here");
        evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
        evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
        elm_win_resize_object_add(win, icon);
        evas_object_show(icon);
    }
    else
    {
        btn = elm_button_add(win);
        elm_object_text_set(btn, "Goodbye Cruel World");
        elm_win_resize_object_add(win, btn);
        evas_object_smart_callback_add(btn, "clicked", on_click, win);
        evas_object_show(btn);
    }
    evas_object_show(win);

    elm_run();

    return 0;
}