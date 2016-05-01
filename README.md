# carputer

music computer for car

## about

This project is my attempt at building an easy-to-use music system for my car.
Previously, I had used various streaming utilities on my phone that would shovel
lossy-encoded music over LTE, but this wasn't ideal for me.  Also, my phone
isn't nearly large enough to hold all the music I want offline.

## design priorities

* Entire music collection available in lossless, CD-quality encoding (FLAC).
* Avoiding streaming large amounts of data over my cell phone.
* Maintain easy strategy for keeping music in sync with my collection.
* Tactile interface for controlling music (using a touch screen in a car is much
too distracting and dangerous while driving).
* Simple and flexible design that allows for easy customization and
extensibility.

## approach

Earlier on I waffled around with many complex ideas, which extended to
fabricating an entirely new stereo head unit for my car that would have a
screen, controls, GPS, WiFi, disc burner, and some database-backed music
tracking program that would allow me to track offline what music was missing or
otherwise out-of-sync with my collection at home.

I realized that many of the things I was considering to make were already made,
e.g., I wasn't about to make a better or easier to use Google Maps than was
already available through my phone.  My existing stereo head unit already
handles analog input well and doesn't need to be redone.  Also, I hadn't quite
thought out the best way for syncing music with the carputer.

The biggest lesson I learned from this project is eliminating components of the
system that weren't really important, and actually compromised the cleaniness of
the system.

## the build


misc install notes for Arch Linux (will revise):

* verify NIC supports AP mode (using iw list)
* udev rules
    * name host NIC "wlan\_ap"
    * name client NIC "wlan\_slave"
    * edit `/etc/udev/rules.d/10-network.rules`:

    ```
    SUBSYSTEM=="net", ACTION=="add", ATTR{address}=="<MAC_ADDR_HERE>", NAME="wlan_ap"
    SUBSYSTEM=="net", ACTION=="add", ATTR{address}=="<MAC_ADDR_HERE>", NAME="wlan_slave"
    ```
* systemd\_networkd
    * assign static ips of different subnets to eth0 and wlan\_ap
    * create systemd-networkd files, e.g.: `/etc/systemd/network/wlan_ap.network`
    * `systemctl enable systemd-networkd.service`
* dhcpd
    * enable for eth0 and wlan\_ap subnets
    * `systemctl enable dhcpd4.service`
* hostapd
    * enable for wlan\_ap
    * edit wifi settings in `/etc/hostapd/hostapd.conf`
    * `systemctl enable hostapd.service`
* mpd
    * enable mpd system-wide service and set appropriate user in unit file
      (not the per-user service that only starts with login process)
    * `systemctl enable mpd.service`
* carputer repo and $PATH
* devmon (part of udevil package)
    * add hook to transport device mount script by editing `/etc/conf.d/devmon`:

    ```
    ARGS="--exec-on-drive '/path/to/carputer/repo/bin/devmon_mount_carputer_transport %f %d'"
    ```
    * `systemctl enable devmon.service`
* hostapd and dhcpd are particular about start order, force waiting on NIC:
    * `systemctl enable systemd-networkd-wait-online.service`

car notes:

* on rear of "Shaker 500" stereo, the 24-pin connector has a couple easy-to-splice wires:
    * pin 2 is ignition accessory "on" (gray/yellow shielding)
        * logic high is ~12V
        * logic low is ~0V
    * pin 13 is ground (black/pink shielding)

known issues:

* hostapd
    * "could not configure driver mode" could mean a few things:
        1. the wireless NIC does not support AP mode
        2. the wireless NIC was not properly brought up before hostapd was run
        3. a driver for the wireless NIC is missing
            * NOTE: To use the Edimax EW-7811Un as an AP on Arch Linux, the
              [`hostapd-rtl871xdrv`](https://aur.archlinux.org/packages/hostapd-rtl871xdrv/)
              package can be grabbed from the AUR, then specify `driver=rtl871xdrv` in the hostapd
              config file.

