#ifndef UBUS_UTILS_H
#define UBUS_UTILS_H

#include <stdio.h>

#include <libubox/blobmsg_json.h>
#include <libubus.h>

#include <device_utils.h>

int device_list_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
			  const char *method, struct blob_attr *msg);

int set_on_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
		     const char *method, struct blob_attr *msg);

int set_off_cb(struct ubus_context *ctx, struct ubus_object *obj, struct ubus_request_data *req,
		      const char *method, struct blob_attr *msg);

#endif // UBUS_UTILS_H