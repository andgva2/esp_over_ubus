#include <device_utils.h>

struct Device *create_device(char *port, char *vid, char *pid)
{
	struct Device *new_device = NULL;
	new_device		  = (struct Device *)malloc(sizeof(struct Device));
	if (new_device == NULL) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}

	strcpy(new_device->port, port);
	strcpy(new_device->vid, vid);
	strcpy(new_device->pid, pid);
	new_device->next = NULL;

	return new_device;
}

int add_to_list(struct Device **device_list, char *port, char *vid, char *pid)
{
	struct Device *new_device = create_device(port, vid, pid);
	if (new_device == NULL) {
		return 1;
	}

	if (*device_list == NULL) {
		*device_list = new_device;
		return 0;
	}

	struct Device *current_device = *device_list;

	while (current_device->next != NULL) {
		current_device = current_device->next;
	}
	current_device->next = new_device;

	return 0;
}

char *get_device_name(struct Device **device)
{
	if ((*device) == NULL) {
		return "";
	}

	struct sp_port *port;
	enum sp_return result = sp_get_port_by_name((*device)->port, &port);

	if (result != SP_OK) {
		fprintf(stderr, "Failed to parse port!\n");
		return "";
	}

	char *device_name = sp_get_port_usb_product(port);
	return device_name;
}

struct Device *get_device_list(void)
{
	struct sp_port **port_list;
	enum sp_return result = sp_list_ports(&port_list);
	if (result != SP_OK) {
		fprintf(stderr, "Failed to parse ports!\n");
		return NULL;
	}

	struct Device *device_list = NULL;
	int i			   = 0;
	for (i = 0; port_list[i] != NULL; i++) {
		struct sp_port *port	    = port_list[i];
		enum sp_transport transport = sp_get_port_transport(port);

		if (transport != SP_TRANSPORT_USB) {
			continue;
		}

		int usb_vid, usb_pid;
		sp_get_port_usb_vid_pid(port, &usb_vid, &usb_pid);
		char usb_vid_str[30];
		char usb_pid_str[30];
		snprintf(usb_vid_str, sizeof(usb_vid_str), "%04X", usb_vid);
		snprintf(usb_pid_str, sizeof(usb_pid_str), "%04X", usb_pid);

		if (strcmp(usb_pid_str, PRODUCT_ID) != 0 || strcmp(usb_vid_str, VENDOR_ID) != 0) {
			continue;
		}

		char *port_name = sp_get_port_name(port);

		add_to_list(&device_list, port_name, usb_vid_str, usb_pid_str);

		struct Device *current_device = device_list;

		while (current_device != NULL) {
			current_device = current_device->next;
		}
	}

	return device_list;
}

int check(enum sp_return result)
{
	char *error_message;

	switch (result) {
	case SP_ERR_ARG:
		fprintf(stderr, "Error: Invalid argument.\n");
		break;
	case SP_ERR_FAIL:
		error_message = sp_last_error_message();
		fprintf(stderr, "Error: Failed: %s\n", error_message);
		sp_free_error_message(error_message);
		break;
	case SP_ERR_SUPP:
		fprintf(stderr, "Error: Not supported.\n");
		break;
	case SP_ERR_MEM:
		fprintf(stderr, "Error: Couldn't allocate memory.\n");
		break;
	case SP_OK:
	default:
		return result;
	}
	return result;
}

int send_to_device(struct Device **device, char *message)
{
	if ((*device) == NULL) {
		return 1;
	}

	int ret = 0;
	struct sp_port *port;
	ret = check(sp_get_port_by_name((*device)->port, &port));
	ret = check(sp_open(port, SP_MODE_WRITE));
	ret = check(sp_set_baudrate(port, 9600));
	ret = check(sp_set_bits(port, 8));
	ret = check(sp_set_parity(port, SP_PARITY_NONE));
	ret = check(sp_set_stopbits(port, 1));
	ret = check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));

	if (ret != SP_OK) {
		fprintf(stderr, "Error: Failed to configure port.\n");
		return ret;
	}	

	unsigned int timeout = 1000;
	int size	     = strlen(message);

	int result = 0;
	result	   = check(sp_blocking_write(port, message, size, timeout));
	if (result < SP_OK) {
		fprintf(stderr, "Error: Failed to write to port.\n");
		return result;
	}


	if (result != size) {
		fprintf(stderr, "Timed out, %d/%d bytes sent.\n", result, size);
	}

	ret = sp_close(port);
	sp_free_port(port);
	return ret;
}

char *receive_from_device(struct Device **device, int size)
{
	if ((*device) == NULL) {
		return "";
	}

	int ret = 0;
	struct sp_port *port;

	ret = check(sp_get_port_by_name((*device)->port, &port));
	ret = check(sp_open(port, SP_MODE_READ));
	ret = check(sp_set_baudrate(port, 9600));
	ret = check(sp_set_bits(port, 8));
	ret = check(sp_set_parity(port, SP_PARITY_NONE));
	ret = check(sp_set_stopbits(port, 1));
	ret = check(sp_set_flowcontrol(port, SP_FLOWCONTROL_NONE));
	if (ret != SP_OK) {
		fprintf(stderr, "Error: Failed to configure port.\n");
		return "";
	}

	char *buf = malloc(sizeof(char) * (size));

	unsigned int timeout = 1000;
	int result	     = check(sp_blocking_read(port, buf, size, timeout));

	if (ret < SP_OK) {
		fprintf(stderr, "Error: Failed to read from port.\n");
		return "";
	}

	if (result > size) {
		fprintf(stderr, "Result out of bounds (%d/%d).\n", result, size);
	}

	buf[result] = '\0';

	ret = check(sp_close(port));
	sp_free_port(port);
	return buf;
}

void free_device_list(struct Device **device_list)
{
	if (*device_list == NULL) {
		return;
	}

	struct Device *current_device = *device_list;
	struct Device *next_device    = NULL;

	while (current_device != NULL) {
		next_device = current_device->next;
		free(current_device);
		current_device = next_device;
	}
	*device_list = NULL;
}
