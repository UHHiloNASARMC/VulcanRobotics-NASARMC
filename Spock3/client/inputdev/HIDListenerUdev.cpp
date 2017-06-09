#include "IHIDListener.hpp"
#include "DeviceFinder.hpp"
#include <libudev.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <thread>

namespace boo
{

static udev* UDEV_INST = nullptr;
udev* GetUdev()
{
    if (!UDEV_INST)
        UDEV_INST = udev_new();
    return UDEV_INST;
}

udev_device* GetUdevJoystick(udev_device* parent)
{
    udev_device* ret = nullptr;
    udev_enumerate* uenum = udev_enumerate_new(GetUdev());
    udev_enumerate_add_match_parent(uenum, parent);
    udev_enumerate_add_match_subsystem(uenum, "input");
    udev_enumerate_add_match_sysname(uenum, "js[0-9]*");
    udev_enumerate_scan_devices(uenum);
    udev_list_entry* uenumList = udev_enumerate_get_list_entry(uenum);
    udev_list_entry* uenumItem;
    udev_list_entry_foreach(uenumItem, uenumList)
    {
        const char* devPath = udev_list_entry_get_name(uenumItem);
        ret = udev_device_new_from_syspath(UDEV_INST, devPath);
        if (ret)
            break;
    }
    udev_enumerate_unref(uenum);
    return ret;
}

udev_device* GetUdevHidRaw(udev_device* parent)
{
    udev_device* ret = nullptr;
    udev_enumerate* uenum = udev_enumerate_new(GetUdev());
    udev_enumerate_add_match_parent(uenum, parent);
    udev_enumerate_add_match_subsystem(uenum, "hidraw");
    udev_enumerate_scan_devices(uenum);
    udev_list_entry* uenumList = udev_enumerate_get_list_entry(uenum);
    udev_list_entry* uenumItem;
    udev_list_entry_foreach(uenumItem, uenumList)
    {
        const char* devPath = udev_list_entry_get_name(uenumItem);
        ret = udev_device_new_from_syspath(UDEV_INST, devPath);
        if (ret)
            break;
    }
    udev_enumerate_unref(uenum);
    return ret;
}

static void UnescapeString(char* output, size_t outLen, const char* input)
{
    while (*input)
    {
        if (outLen == 1)
        {
            break;
        }
        if (*input == '\\')
        {
            ++input;
            switch (*input++)
            {
            case 'x':
            {
                char num[] = {input[0], input[1], '\0'};
                input += 2;
                *output++ = strtol(num, nullptr, 16);
                --outLen;
                break;
            }
            case 'n':
                *output++ = '\n';
                --outLen;
                break;
            case 't':
                *output++ = '\t';
                --outLen;
                break;
            default: break;
            }
        }
        else
        {
            *output++ = *input++;
            --outLen;
        }
    }
    *output = '\0';
}

class HIDListenerUdev final : public IHIDListener
{
    DeviceFinder& m_finder;

    udev_monitor* m_udevMon;
    std::thread m_udevThread;
    bool m_udevRunning;
    bool m_scanningEnabled;

    static void deviceConnected(HIDListenerUdev* listener,
                                udev_device* device)
    {
        if (!listener->m_scanningEnabled)
            return;

        /* Prevent redundant registration */
        const char* devPath = udev_device_get_syspath(device);
        udev_device* parent = udev_device_get_parent(device);
        if (!parent) /* Reject virtual devices */
            return;
        const char* parentPath = udev_device_get_syspath(parent);

        if (!strcmp("/sys/devices/pci0000:00/0000:00:1d.1/usb3/3-1/3-1:1.0/input/input13/js0", devPath))
            printf("%s\n", devPath);
        if (listener->m_finder._hasToken(devPath) ||
            listener->m_finder._hasToken(parentPath))
            return;

        /* Filter to USB/BT */
        const char* dt = udev_device_get_devtype(device);
        DeviceType type;
        int vid = 0, pid = 0;
        char manufUnesc[256];
        char productUnesc[256];
        const char* manuf = nullptr;
        const char* product = nullptr;
        if (dt)
        {
            if (!strcmp(dt, "usb_device"))
                type = DeviceType::USB;
            else if (!strcmp(dt, "bluetooth_device"))
                type = DeviceType::Bluetooth;
            else
                return;

            udev_list_entry* attrs = udev_device_get_properties_list_entry(device);

            udev_list_entry* vide = udev_list_entry_get_by_name(attrs, "ID_VENDOR_ID");
            if (vide)
                vid = strtol(udev_list_entry_get_value(vide), nullptr, 16);

            udev_list_entry* pide = udev_list_entry_get_by_name(attrs, "ID_MODEL_ID");
            if (pide)
                pid = strtol(udev_list_entry_get_value(pide), nullptr, 16);

            udev_list_entry* manufe = udev_list_entry_get_by_name(attrs, "ID_VENDOR");
            if (manufe)
                manuf = udev_list_entry_get_value(manufe);

            udev_list_entry* producte = udev_list_entry_get_by_name(attrs, "ID_MODEL");
            if (producte)
                product = udev_list_entry_get_value(producte);
        }
        else if (!strcmp(udev_device_get_subsystem(device), "hidraw") ||
                 !strcmp(udev_device_get_subsystem(device), "input"))
        {
            devPath = parentPath;

            if (udev_device* joystick = GetUdevJoystick(parent))
            {
                type = DeviceType::HID;
                udev_list_entry* attrs = udev_device_get_properties_list_entry(joystick);

                udev_list_entry* vide = udev_list_entry_get_by_name(attrs, "ID_VENDOR_ID");
                if (vide)
                    vid = strtol(udev_list_entry_get_value(vide), nullptr, 16);

                udev_list_entry* pide = udev_list_entry_get_by_name(attrs, "ID_MODEL_ID");
                if (pide)
                    pid = strtol(udev_list_entry_get_value(pide), nullptr, 16);

                udev_list_entry* manufe = udev_list_entry_get_by_name(attrs, "ID_VENDOR_ENC");
                if (manufe)
                {
                    manuf = manufUnesc;
                    UnescapeString(manufUnesc, sizeof(manufUnesc), udev_list_entry_get_value(manufe));
                }

                udev_list_entry* producte = udev_list_entry_get_by_name(attrs, "ID_MODEL_ENC");
                if (producte)
                {
                    product = productUnesc;
                    UnescapeString(productUnesc, sizeof(productUnesc), udev_list_entry_get_value(producte));
                }

                udev_device_unref(joystick);
            }
        }
        else
        {
            return;
        }

#if 0
        udev_list_entry* att = nullptr;
        udev_list_entry_foreach(att, attrs)
        {
            const char* name = udev_list_entry_get_name(att);
            const char* val = udev_list_entry_get_value(att);
            fprintf(stderr, "%s %s\n", name, val);
        }
        fprintf(stderr, "\n\n");
#endif

        listener->m_finder._insertToken(
            std::make_unique<DeviceToken>(type, vid, pid, manuf, product, devPath));
    }

    static void deviceDisconnected(HIDListenerUdev* listener,
                                   udev_device* device)
    {
        const char* devPath = udev_device_get_syspath(device);
        listener->m_finder._removeToken(devPath);
    }

    /* Empty handler for SIGTERM */
    static void _sigterm(int) {}

    static void _udevProc(HIDListenerUdev* listener)
    {
        /* SIGTERM will be used to terminate thread */
        struct sigaction s;
        s.sa_handler = _sigterm;
        sigemptyset(&s.sa_mask);
        s.sa_flags = 0;
        sigaction(SIGTERM, &s, nullptr);

        sigset_t waitmask, origmask;
        sigemptyset(&waitmask);
        sigaddset(&waitmask, SIGTERM);
        pthread_sigmask(SIG_BLOCK, &waitmask, &origmask);

        udev_monitor_enable_receiving(listener->m_udevMon);
        int fd = udev_monitor_get_fd(listener->m_udevMon);
        while (listener->m_udevRunning)
        {
            fd_set fds;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            if (pselect(fd+1, &fds, nullptr, nullptr, nullptr, &origmask) < 0)
            {
                /* SIGTERM handled here */
                if (errno == EINTR)
                    break;
            }
            udev_device* dev = udev_monitor_receive_device(listener->m_udevMon);
            if (dev)
            {
                const char* action = udev_device_get_action(dev);
                if (!strcmp(action, "add"))
                    deviceConnected(listener, dev);
                else if (!strcmp(action, "remove"))
                    deviceDisconnected(listener, dev);
                udev_device_unref(dev);
            }
        }
    }

public:
    HIDListenerUdev(DeviceFinder& finder)
    : m_finder(finder)
    {
        /* Setup hotplug events */
        m_udevMon = udev_monitor_new_from_netlink(GetUdev(), "udev");
        if (!m_udevMon)
        {
            fprintf(stderr, "unable to init udev_monitor");
            abort();
        }
        udev_monitor_filter_add_match_subsystem_devtype(m_udevMon, "usb", "usb_device");
        udev_monitor_filter_add_match_subsystem_devtype(m_udevMon, "bluetooth", "bluetooth_device");
        udev_monitor_filter_add_match_subsystem_devtype(m_udevMon, "hidraw", nullptr);
        udev_monitor_filter_add_match_subsystem_devtype(m_udevMon, "input", nullptr);
        udev_monitor_filter_update(m_udevMon);

        /* Initial HID Device Add */
        m_scanningEnabled = true;
        scanNow();
        m_scanningEnabled = false;

        /* Start hotplug thread */
        m_udevRunning = true;
        m_udevThread = std::thread(_udevProc, this);
    }

    ~HIDListenerUdev()
    {
        m_udevRunning = false;
        pthread_kill(m_udevThread.native_handle(), SIGTERM);
        m_udevThread.join();
        udev_monitor_unref(m_udevMon);
    }

    /* Automatic device scanning */
    bool startScanning()
    {
        m_scanningEnabled = true;
        return true;
    }
    bool stopScanning()
    {
        m_scanningEnabled = false;
        return true;
    }

    /* Manual device scanning */
    bool scanNow()
    {
        udev_enumerate* uenum = udev_enumerate_new(GetUdev());
        udev_enumerate_add_match_subsystem(uenum, "usb");
        udev_enumerate_add_match_subsystem(uenum, "bluetooth");
        udev_enumerate_add_match_subsystem(uenum, "hidraw");
        udev_enumerate_add_match_subsystem(uenum, "input");
        udev_enumerate_scan_devices(uenum);
        udev_list_entry* uenumList = udev_enumerate_get_list_entry(uenum);
        udev_list_entry* uenumItem;
        udev_list_entry_foreach(uenumItem, uenumList)
        {
            const char* devPath = udev_list_entry_get_name(uenumItem);
            udev_device* dev = udev_device_new_from_syspath(UDEV_INST, devPath);
            if (dev)
                deviceConnected(this, dev);
            udev_device_unref(dev);
        }
        udev_enumerate_unref(uenum);
        return true;
    }

};

std::unique_ptr<IHIDListener> IHIDListenerNew(DeviceFinder& finder)
{
    return std::make_unique<HIDListenerUdev>(finder);
}

}
