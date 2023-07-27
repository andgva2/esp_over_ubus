#include <stdio.h>

#include <libubus.h>

#include <ubus_utils.h>

extern struct ubus_object esp_over_ubus_object;

int main(void)
{
	struct ubus_context *ctx;

	uloop_init();

	ctx = ubus_connect(NULL);
	if (!ctx) {
		fprintf(stderr, "Failed to connect to ubus\n");
		return -1;
	}

	ubus_add_uloop(ctx);

	int ret = ubus_add_object(ctx, &esp_over_ubus_object);
	if (ret) {
		fprintf(stderr, "Failed to add object: %s\n", ubus_strerror(ret));
	}

	uloop_run();

	ubus_free(ctx);
	uloop_done();

	return 0;
}