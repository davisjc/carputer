# Carputer

Music computer for car.

## About

I love listening to music while driving, and want to be as uninhibited as possible while doing so.

Previously, I had used various mobile streaming apps or would store select music directly on my phone, but I wasn't ever really satisfied with these solutions. I always had to be aware of how much lossy-encoded music I was shovelling over LTE or which songs I'd choose to put on my phone, since space is limited and my music collection is large.

This project is my solution to having my entire music collection in full fidelity available in my car, while making it simple to keep in sync with my collection at home.

## Design Priorities

* Provide entire music collection in lossless, CD-quality encoding (FLAC).
* Avoid streaming data over my cell phone.
* Make syncing music an easy and non-interfering process.
* Use simple and flexible design that allows for easy customization and extensibility.
* Allow for tactile controls for controlling music (using a touch screen in a car kind of sucks ...and not to mention is dangerous).

## Approach

This project was a good exercise in reframing a problem.  Many ideas I had upfront were either over-engineered or not really relevant to my priorities.

Earlier, I waffled around with many complex ideas, which included a lot of "why not" thoughts that extended to fabricating an entirely new stereo head unit for my car that would have a screen, controls, GPS, WiFi, disc burner, and some database-backed music tracking program that would track offline what music was missing or otherwise out-of-sync with my collection at home by calculating and comparing file hashes.

I realized that many of the ideas I was considering were already well made: e.g., I wasn't about to casually make an easier to use interface to Google Maps than was already available on my phone.  Speaking of the phone: it's a quality display that can be mounted on the dash.  My existing stereo head unit already handles analog input well and doesn't need to be redone.

With those considerations in mind, all I really needed is a small computer with large persistent storage that can be controlled by my phone (and more ideally, physical buttons), and a simple strategy for getting music collection changes synced between two offline systems using something like a USB thumb drive as a messenger of state.

## The Build

## Alternatives Considered

## Misc.

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

