# carputer
music computer for car

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
    * "could not configure driver mode" could mean a couple things:
        1. the wireless NIC does not support AP mode
        2. the wireless NIC was not properly brought up before hostapd was run

