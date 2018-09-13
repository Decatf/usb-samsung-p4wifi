#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <asm/errno.h>
#include <linux/gpio.h>
#include <sys/ioctl.h>

#define DEV_NAME "gpiochip0"

enum usb_sel_gpios {
    USB_SEL1 = 0,
    USB_SEL2,
    NUM_GPIOS,
};

struct usb_sel_gpio {
    enum usb_sel_gpios idx;
    unsigned int offset;
    const char *name;
} static const gpio_table[] = {
    { .idx = USB_SEL1, .offset = 10, .name = "usb-sel1" },
    { .idx = USB_SEL2, .offset = 113, .name = "usb-sel2" },
};

int request_lines(int fd, struct gpiohandle_request *req,
    struct gpiohandle_data *data, const char *consumer_label)
{
    int ret;
    strcpy(req->consumer_label, consumer_label);

    ret = ioctl(fd, GPIO_GET_LINEHANDLE_IOCTL, req);
    if (ret == -1) {
        ret = -errno;
        fprintf(stderr, "get linehandle error (%d)\n", ret);
        return ret;
    }

    return 0;
}


int get_line_values(int fd, struct gpiohandle_request *req,
    struct gpiohandle_data *data)
{
    int ret;

    ret = ioctl(req->fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, data);
    if (ret == -1) {
        ret = -errno;
        fprintf(stderr, "get line values error (%d)\n", ret);
        return ret;
    }

    return 0;
}


int set_line_values(int fd, struct gpiohandle_request *req,
    struct gpiohandle_data *data)
{
    int ret;

    ret = ioctl(req->fd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &data);

    if (ret == -1) {
        ret = -errno;
        fprintf(stderr, "set line values error (%d)\n", ret);
        return ret;
    }

    return 0;
}

void print_line_values(struct gpiohandle_request *req,
    struct gpiohandle_data *data)
{
    unsigned int i;
    for (i = 0; i < req->lines; ++i)
    {
        fprintf(stdout, "%s gpio %u is %s\n",
            gpio_table[i].name, gpio_table[i].offset,
            data->values[0] ? "high" : "low");        
    }
}

int init_lines(int fd)
{
    struct gpioline_info linfo;
    int i, ret;

    for (i = 0; i < NUM_GPIOS; ++i)
    {
        linfo.line_offset = gpio_table[i].offset;
        ret = ioctl(fd, GPIO_GET_LINEINFO_IOCTL, &linfo);
        if (ret == -1) {
            ret = -errno;
            fprintf(stderr, "Could not get info for gpio %u"
                " error (%d)\n", linfo.line_offset, ret);
            return ret;
        }

        fprintf(stdout, "line %2d: %s %s 0x%X\n",
            linfo.line_offset, linfo.name, linfo.consumer, linfo.flags);
    }

    return 0;
}

int main()
{
    int fd;
    struct gpiochip_info cinfo;
    struct gpiohandle_request req;
    struct gpiohandle_data data;
    int ret;

    fd = open("/dev/" DEV_NAME, 0);
    if (fd < 0) {
        fprintf(stderr, "Could not open " DEV_NAME "\n");
        return fd;
    }


    ret = ioctl(fd, GPIO_GET_CHIPINFO_IOCTL, &cinfo);
    if (ret == -1) {
        fprintf(stderr, "Get chipinfo error (%d)\n", -errno);
        goto exit;
    }
    fprintf(stdout, "GPIO chip: %s, \"%s\", %u GPIO lines\n",
        cinfo.name, cinfo.label, cinfo.lines);
    
    ret = init_lines(fd);
    if (ret)
        goto exit;

    req.lineoffsets[USB_SEL1] = gpio_table[USB_SEL1].offset;
    req.lineoffsets[USB_SEL2] = gpio_table[USB_SEL2].offset;
    req.lines = NUM_GPIOS;
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;

    ret = request_lines(fd, &req, &data, "usb-sel");
    if (ret)
        goto exit;
    
    ret = get_line_values(fd, &req, &data);
    if (ret)
        goto exit;

    print_line_values(&req, &data);

    data.values[USB_SEL1] = 1;
    data.values[USB_SEL2] = 1;
    ret = set_line_values(fd, &req, &data);
    if (ret)
        goto exit;

    ret = get_line_values(fd, &req, &data);
    if (ret)
        goto exit;

    print_line_values(&req, &data);

exit:
    close(fd);
    return ret;
}
