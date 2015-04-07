/*
 * Copyright 2015 Codethink Ltd
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holders not be used in
 * advertising or publicity pertaining to distribution of the software
 * without specific, written prior permission.  The copyright holders make
 * no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
 * SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdlib.h>
#include <string.h>

#include "weston/compositor.h"
#include "ilm_types.h"

#include "ivi-input-server-protocol.h"
#include "ivi-controller-interface.h"

struct seat_ctx {
    struct input_context *input_ctx;
    struct weston_keyboard_grab keyboard_grab;
    struct weston_pointer_grab pointer_grab;
    struct weston_touch_grab touch_grab;
    struct wl_listener updated_caps_listener;
    struct wl_listener destroy_listener;
};

struct surface_ctx {
    struct wl_list link;
    ilmInputDevice focus;
    struct ivi_layout_surface *layout_surface;
    struct wl_array accepted_devices;
    struct input_context *input_context;
};

struct input_controller {
    struct wl_list link;
    struct wl_resource *resource;
    struct wl_client *client;
    uint32_t id;
    struct input_context *input_context;
};

struct input_context {
    struct wl_listener seat_create_listener;
    struct wl_list controller_list;
    struct wl_list surface_list;
    struct weston_compositor *compositor;
    const struct ivi_controller_interface *ivi_controller_interface;
};

static int
get_accepted_seat(struct surface_ctx *surface, const char *seat)
{
    int i;
    char **arr = surface->accepted_devices.data;
    for (i = 0; i < (surface->accepted_devices.size / sizeof(char**)); i++) {
        if (strcmp(arr[i], seat) == 0)
            return i;
    }
    return -1;
}

static int
add_accepted_seat(struct surface_ctx *surface, const char *seat)
{
    char **added_entry;
    const struct ivi_controller_interface *interface =
        surface->input_context->ivi_controller_interface;
    if (get_accepted_seat(surface, seat) >= 0) {
        weston_log("%s: Warning: seat '%s' is already accepted by surface %d\n",
                   __FUNCTION__, seat,
                   interface->get_id_of_surface(surface->layout_surface));
        return 1;
    }
    added_entry = wl_array_add(&surface->accepted_devices, sizeof *added_entry);
    if (added_entry == NULL) {
        weston_log("%s: Failed to expand accepted devices array for "
                   "surface %d\n", __FUNCTION__,
                   interface->get_id_of_surface(surface->layout_surface));
        return 0;
    }
    *added_entry = strdup(seat);
    if (*added_entry == NULL) {
        weston_log("%s: Failed to duplicate seat name '%s'\n",
                   __FUNCTION__, seat);
        return 0;
    }
    return 1;
}

static int
remove_accepted_seat(struct surface_ctx *surface, const char *seat)
{
    int seat_index = get_accepted_seat(surface, seat);
    int i;
    struct wl_array *array = &surface->accepted_devices;
    char **data = array->data;
    const struct ivi_controller_interface *interface =
        surface->input_context->ivi_controller_interface;
    if (seat_index < 0) {
        weston_log("%s: Warning: seat '%s' not found for surface %d\n",
                  __FUNCTION__, seat,
                  interface->get_id_of_surface(surface->layout_surface));
        return 0;
    }
    free(data[seat_index]);
    for (i = seat_index + 1; i < array->size / sizeof(char **); i++)
        data[i - 1] = data[i];
    array->size-= sizeof(char**);

    return 1;
}

static void
send_input_acceptance(struct input_context *ctx, uint32_t surface_id, const char *seat, int32_t accepted)
{
    struct input_controller *controller;
    wl_list_for_each(controller, &ctx->controller_list, link) {
        ivi_input_send_input_acceptance(controller->resource,
                                        surface_id, seat,
                                        accepted);
    }
}

static void
send_input_focus(struct input_context *ctx, t_ilm_surface surface_id,
                 ilmInputDevice device, t_ilm_bool enabled)
{
    struct input_controller *controller;
    wl_list_for_each(controller, &ctx->controller_list, link) {
        ivi_input_send_input_focus(controller->resource, surface_id,
                                   device, enabled);
    }
}

static void
keyboard_grab_key(struct weston_keyboard_grab *grab, uint32_t time,
                  uint32_t key, uint32_t state)
{
}

static void
keyboard_grab_modifiers(struct weston_keyboard_grab *grab, uint32_t serial,
                        uint32_t mods_depressed, uint32_t mods_latched,
                        uint32_t mods_locked, uint32_t group)
{
}

static void
keyboard_grab_cancel(struct weston_keyboard_grab *grab)
{
}

static struct weston_keyboard_grab_interface keyboard_grab_interface = {
    keyboard_grab_key,
    keyboard_grab_modifiers,
    keyboard_grab_cancel
};

static void
pointer_grab_focus(struct weston_pointer_grab *grab)
{
}

static void
pointer_grab_motion(struct weston_pointer_grab *grab, uint32_t time,
                    wl_fixed_t x, wl_fixed_t y)
{
}

static void
pointer_grab_button(struct weston_pointer_grab *grab, uint32_t time,
                    uint32_t button, uint32_t state)
{
}

static void
pointer_grab_cancel(struct weston_pointer_grab *grab)
{
}

static struct weston_pointer_grab_interface pointer_grab_interface = {
    pointer_grab_focus,
    pointer_grab_motion,
    pointer_grab_button,
    pointer_grab_cancel
};

static void
touch_grab_down(struct weston_touch_grab *grab, uint32_t time, int touch_id,
                wl_fixed_t sx, wl_fixed_t sy)
{
}

static void
touch_grab_up(struct weston_touch_grab *grab, uint32_t time, int touch_id)
{
}

static void
touch_grab_motion(struct weston_touch_grab *grab, uint32_t time, int touch_id,
                  wl_fixed_t sx, wl_fixed_t sy)
{
}

static void
touch_grab_frame(struct weston_touch_grab *grab)
{
}

static void
touch_grab_cancel(struct weston_touch_grab *grab)
{
}

static struct weston_touch_grab_interface touch_grab_interface = {
    touch_grab_down,
    touch_grab_up,
    touch_grab_motion,
    touch_grab_frame,
    touch_grab_cancel
};

static uint32_t
get_seat_capabilities(const struct weston_seat *seat)
{
    uint32_t caps = 0;
    if (seat->keyboard_device_count > 0)
        caps |= ILM_INPUT_DEVICE_KEYBOARD;
    if (seat->pointer_device_count > 0)
        caps |= ILM_INPUT_DEVICE_POINTER;
    if (seat->touch_device_count > 0)
        caps |= ILM_INPUT_DEVICE_TOUCH;
    return caps;
}

static void
handle_seat_updated_caps(struct wl_listener *listener, void *data)
{
    struct weston_seat *seat = data;
    struct seat_ctx *ctx = wl_container_of(listener, ctx,
                                           updated_caps_listener);
    struct input_controller *controller;
    if (seat->keyboard && seat->keyboard != ctx->keyboard_grab.keyboard) {
        weston_keyboard_start_grab(seat->keyboard, &ctx->keyboard_grab);
    }
    if (seat->pointer && seat->pointer != ctx->pointer_grab.pointer) {
        weston_pointer_start_grab(seat->pointer, &ctx->pointer_grab);
    }
    if (seat->touch && seat->touch != ctx->touch_grab.touch) {
        weston_touch_start_grab(seat->touch, &ctx->touch_grab);
    }

    wl_list_for_each(controller, &ctx->input_ctx->controller_list, link) {
        ivi_input_send_seat_capabilities(controller->resource,
                                         seat->seat_name,
                                         get_seat_capabilities(seat));
    }
}

static void
handle_seat_destroy(struct wl_listener *listener, void *data)
{
    struct seat_ctx *ctx = wl_container_of(listener, ctx, destroy_listener);
    struct weston_seat *seat = data;
    struct input_controller *controller;

    if (ctx->keyboard_grab.keyboard)
        keyboard_grab_cancel(&ctx->keyboard_grab);
    if (ctx->pointer_grab.pointer)
        pointer_grab_cancel(&ctx->pointer_grab);
    if (ctx->touch_grab.touch)
        touch_grab_cancel(&ctx->touch_grab);

    wl_list_for_each(controller, &ctx->input_ctx->controller_list, link) {
        ivi_input_send_seat_destroyed(controller->resource,
                                      seat->seat_name);
    }

    free(ctx);
}

static void
handle_seat_create(struct wl_listener *listener, void *data)
{
    struct weston_seat *seat = data;
    struct input_context *input_ctx = wl_container_of(listener, input_ctx,
                                                      seat_create_listener);
    struct input_controller *controller;
    struct seat_ctx *ctx = calloc(1, sizeof *ctx);
    if (ctx == NULL) {
        weston_log("%s: Failed to allocate memory\n", __FUNCTION__);
        return;
    }

    ctx->input_ctx = input_ctx;

    ctx->keyboard_grab.interface = &keyboard_grab_interface;
    ctx->pointer_grab.interface = &pointer_grab_interface;
    ctx->touch_grab.interface= &touch_grab_interface;

    ctx->destroy_listener.notify = &handle_seat_destroy;
    wl_signal_add(&seat->destroy_signal, &ctx->destroy_listener);

    ctx->updated_caps_listener.notify = &handle_seat_updated_caps;
    wl_signal_add(&seat->updated_caps_signal, &ctx->updated_caps_listener);

    wl_list_for_each(controller, &input_ctx->controller_list, link) {
        ivi_input_send_seat_created(controller->resource,
                                    seat->seat_name,
                                    get_seat_capabilities(seat));
    }
}

static void
handle_surface_destroy(struct ivi_layout_surface *layout_surface, void *data)
{
    struct input_context *ctx = data;
    struct surface_ctx *surf, *next;
    int surface_removed = 0;
    const struct ivi_controller_interface *interface =
        ctx->ivi_controller_interface;

    wl_list_for_each_safe(surf, next, &ctx->surface_list, link) {
        if (surf->layout_surface == layout_surface) {
            uint32_t i;
            char **data = surf->accepted_devices.data;
            wl_list_remove(&surf->link);
            for (i = 0; i < surf->accepted_devices.size / sizeof(char**); i++) {
                free(data[i]);
	    }
            wl_array_release(&surf->accepted_devices);
            free(surf);
            surface_removed = 1;
            break;
        }
    }

    if (!surface_removed) {
        weston_log("%s: Warning! surface %d already destroyed\n", __FUNCTION__,
                   interface->get_id_of_surface((layout_surface)));
    }
}

static void
handle_surface_create(struct ivi_layout_surface *layout_surface, void *data)
{
    struct input_context *input_ctx = data;
    struct surface_ctx *ctx;
    const struct ivi_controller_interface *interface =
        input_ctx->ivi_controller_interface;

    wl_list_for_each(ctx, &input_ctx->surface_list, link) {
        if (ctx->layout_surface == layout_surface) {
            weston_log("%s: Warning! surface context already created for"
                       " surface %d\n", __FUNCTION__,
                       interface->get_id_of_surface((layout_surface)));
            break;
        }
    }

    ctx = calloc(1, sizeof *ctx);
    if (ctx == NULL) {
        weston_log("%s: Failed to allocate memory\n", __FUNCTION__);
        return;
    }
    ctx->layout_surface = layout_surface;
    ctx->input_context = input_ctx;
    wl_array_init(&ctx->accepted_devices);
    add_accepted_seat(ctx, "default");
    send_input_acceptance(input_ctx,
                          interface->get_id_of_surface(layout_surface),
                          "default", ILM_TRUE);

    wl_list_insert(&input_ctx->surface_list, &ctx->link);
}

static void
unbind_resource_controller(struct wl_resource *resource)
{
    struct input_controller *controller = wl_resource_get_user_data(resource);

    wl_list_remove(&controller->link);

    free(controller);
}

static void
input_set_input_focus(struct wl_client *client,
                                 struct wl_resource *resource,
                                 uint32_t surface, uint32_t device,
                                 int32_t enabled)
{
    struct input_controller *controller = wl_resource_get_user_data(resource);
    struct input_context *ctx = controller->input_context;
    int found_surface = 0;
    struct surface_ctx *surf;
    const struct ivi_controller_interface *interface =
	ctx->ivi_controller_interface;
    /* If focus is enabled for one of these devices, every other surface
     * must have focus unset */
    const ilmInputDevice single_surface_devices =
        ILM_INPUT_DEVICE_POINTER | ILM_INPUT_DEVICE_TOUCH;
    ilmInputDevice single_surface_mask = single_surface_devices & device;

    wl_list_for_each(surf, &ctx->surface_list, link) {
        uint32_t current_surface =
            interface->get_id_of_surface(surf->layout_surface);
        if (current_surface == surface) {
            if (enabled == ILM_TRUE) {
                surf->focus |= device;
            } else {
                surf->focus &= ~device;
            }
            send_input_focus(ctx, current_surface, device, enabled);
            found_surface = 1;
        } else if (single_surface_mask && enabled == ILM_TRUE) {
            surf->focus &= ~(single_surface_mask);
            send_input_focus(ctx, current_surface, single_surface_mask, ILM_FALSE);
        }
    }

    if (!found_surface) {
        weston_log("%s: surface %d was not found\n", __FUNCTION__, surface);
    }
}

static void
input_set_input_acceptance(struct wl_client *client,
                                      struct wl_resource *resource,
                                      uint32_t surface, const char *seat,
                                      int32_t accepted)
{
    struct input_controller *controller = wl_resource_get_user_data(resource);
    struct input_context *ctx = controller->input_context;
    struct surface_ctx *surface_ctx;
    int found_seat = 0;
    const struct ivi_controller_interface *interface =
        ctx->ivi_controller_interface;

    wl_list_for_each(surface_ctx, &ctx->surface_list, link) {
        if (interface->get_id_of_surface(surface_ctx->layout_surface) == surface) {
            if (accepted == ILM_TRUE)
                found_seat = add_accepted_seat(surface_ctx, seat);
            else
                found_seat = remove_accepted_seat(surface_ctx, seat);
            break;
        }
    }

    if (found_seat)
        send_input_acceptance(ctx, surface, seat, accepted);
}

static const struct ivi_input_interface input_implementation = {
    input_set_input_focus,
    input_set_input_acceptance
};

static void
bind_ivi_input(struct wl_client *client, void *data,
               uint32_t version, uint32_t id)
{
    struct input_context *ctx = data;
    struct input_controller *controller;
    struct weston_seat *seat;
    struct surface_ctx *surface_ctx;
    const struct ivi_controller_interface *interface =
        ctx->ivi_controller_interface;
    controller = calloc(1, sizeof *controller);
    if (controller == NULL) {
        weston_log("%s: Failed to allocate memory for controller\n",
                   __FUNCTION__);
        return;
    }

    controller->input_context = ctx;
    controller->resource =
        wl_resource_create(client, &ivi_input_interface, 1, id);
    wl_resource_set_implementation(controller->resource, &input_implementation,
                                   controller, unbind_resource_controller);

    controller->client = client;
    controller->id = id;

    wl_list_insert(&ctx->controller_list, &controller->link);

    /* Send seat events for all known seats to the client */
    wl_list_for_each(seat, &ctx->compositor->seat_list, link) {
        ivi_input_send_seat_created(controller->resource,
                                               seat->seat_name,
                                               get_seat_capabilities(seat));
    }
    /* Send focus events for all known surfaces to the client */
    wl_list_for_each(surface_ctx, &ctx->surface_list, link) {
        ivi_input_send_input_focus(controller->resource,
            interface->get_id_of_surface(surface_ctx->layout_surface),
            surface_ctx->focus, ILM_TRUE);
    }
    /* Send acceptance events for all known surfaces to the client */
    wl_list_for_each(surface_ctx, &ctx->surface_list, link) {
        char **name;
        wl_array_for_each(name, &surface_ctx->accepted_devices) {
            ivi_input_send_input_acceptance(controller->resource,
                    interface->get_id_of_surface(surface_ctx->layout_surface),
                    *name, ILM_TRUE);
        }
    }
}

static struct input_context *
create_input_context(struct weston_compositor *ec,
                     const struct ivi_controller_interface *interface)
{
    struct input_context *ctx = NULL;
    struct weston_seat *seat;
    ctx = calloc(1, sizeof *ctx);
    if (ctx == NULL) {
        weston_log("%s: Failed to allocate memory for input context\n",
                   __FUNCTION__);
        return NULL;
    }

    ctx->compositor = ec;
    ctx->ivi_controller_interface = interface;
    wl_list_init(&ctx->controller_list);
    wl_list_init(&ctx->surface_list);

    /* Add signal handlers for ivi surfaces. Warning: these functions leak
     * memory. */
    interface->add_notification_create_surface(handle_surface_create, ctx);
    interface->add_notification_remove_surface(handle_surface_destroy, ctx);

    ctx->seat_create_listener.notify = &handle_seat_create;
    wl_signal_add(&ec->seat_created_signal, &ctx->seat_create_listener);

    wl_list_for_each(seat, &ec->seat_list, link) {
        handle_seat_create(&ctx->seat_create_listener, seat);
        wl_signal_emit(&seat->updated_caps_signal, seat);
    }

    return ctx;
}

WL_EXPORT int
controller_module_init(struct weston_compositor *ec, int* argc, char *argv[],
                       const struct ivi_controller_interface *interface,
                       size_t interface_version)
{
    struct input_context *ctx = create_input_context(ec, interface);
    if (ctx == NULL) {
        weston_log("%s: Failed to create input context\n", __FUNCTION__);
        return -1;
    }

    if (wl_global_create(ec->wl_display, &ivi_input_interface, 1,
                         ctx, bind_ivi_input) == NULL) {
        return -1;
    }
    weston_log("ivi-input-controller module loaded successfully!\n");
    return 0;
}
