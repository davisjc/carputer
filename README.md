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
* Allow for tactile controls for controlling music (using a touch screen in a car kind of sucks ...and not very safe).

## Approach

This project was a good exercise in reframing a problem.  Many ideas I had upfront were either over-engineered or not really relevant to my priorities.

Earlier, I waffled around with many complex ideas, which included a lot of "why not" thoughts that extended to fabricating an entirely new stereo head unit for my car that would have a screen, controls, amplifier, GPS, WiFi, disc burner, and some database-backed music tracking program that would track offline what music was missing or otherwise out-of-sync with my collection at home by calculating and comparing file hashes.

I realized that many of the ideas I was considering were already well made: e.g., I wasn't about to casually make an easier to use interface to Google Maps than was already available on my phone.  Speaking of the phone: there's a quality display that can be mounted on the dash.  My existing stereo head unit already handles analog input well and doesn't need to be redone.

With those considerations in mind, all I really needed is a small computer with large persistent storage that can be controlled by my phone (and more ideally, physical buttons), and a simple strategy for getting music collection changes synced between two offline systems using something like a USB thumb drive as a messenger of state.

## The Hardware

I went with the [Raspberry Pi 2 Model B](https://www.raspberrypi.org/products/raspberry-pi-2-model-b/) and the following components.

* **Audio**
    * The company, HiFiBerry, makes a pretty neat audio card, the [HifIBerry DAC+](https://www.hifiberry.com/dacplus/), which plugs directly into the Pi's I<sup>2</sup>C bus on the GPIO.  If you're curious, you should check it out - the sound is crisp and full with a high signal-to-noise ratio.  It physically consumes all of the Pi's GPIO pins, but [only uses a fraction of them](https://www.hifiberry.com/guides/gpio-usage-of-the-hifiberry-products/).  An additional pin header can be soldered to the board to access the rest of the pins.
* **Storage**
    * A 1TB, shock-resistant USB hard drive.
    * Ultra High Speed MicroSDHC flash card for OS.
* **Connectivity**
    * Edimax EW-7811Un USB Wifi dongle for serving an access point a mobile phone can connect to.
* **Power**
    * I was about to build a power supply that turned on with the ignition wire and only cut power once the ignition was off and the Pi had completed a normal shutdown sequence by signalling over the GPIO.  However, I found the same circuit for sale at [Mausberry Circuits](https://mausberry-circuits.myshopify.com/collections/car-power-supply-switches/products/3a-car-supply-switch) that provides 3A across two USB ports.  I'm not sure if I'll continue using it or not.  The first PCB's switching logic failed for an unknown reason.  The site is typically out-of-stock and responses by email can be delayed.  However, the owner seemed kind and sent a second PCB for free to replace the defective one.  The supply seems stable now *...fingers crossed*.  If it fails again, I'll probably just build one myself with a DC-DC buck converter such as [this](http://www.amazon.com/dp/B00CEP3A0Q/) and a MOSFET.
    * USB-powered USB hub to power disk and other devices through the power supply and not through the Pi.

## The Software

For an OS, I chose [Arch Linux ARM](https://archlinuxarm.org/), because it's familiar and hackable.

At home, I use [Music Player Daemon](https://www.musicpd.org/) (MPD), [ncmpcpp](http://rybczak.net/ncmpcpp/) (an MPD client), and [beets](http://beets.io/) to play and manage my music library.  MPD is great because it's a headless music server that supports a huge array of clients, and is an ideal choice here for its flexibility.  One such MPD client is [MPoD](https://itunes.apple.com/us/app/mpod/id285063020?mt=8) for iOS.

The Pi throws up a WiFi hotspot with [hostapd](https://w1.fi/hostapd/) that mobile phones and other devices can join for controlling the music or otherwise administering the Pi over SSH.

The [udevil](https://ignorantguru.github.io/udevil/) package provides some neat facilities for managing device mounts as well as auto-mounting when a new block device is detected.  The included script, devmon, provides a simple way to execute code when an auto-mount happens:

```
ARGS="--exec-on-drive '/path/to/carputer/repo/bin/devmon_mount_carputer_transport %f %d'"
```

Simplicity is the focus.  In my mind, the design of the project really hinges on how state is kept in sync between two systems that will never talk to each other directly over a network.  A simple way to opt-out of this is by just carrying the hard drive between systems and using rsync to update the music library.  However, I wanted a way to keep the car's music library in sync without carrying the entire library around.



## Pictures

## Alternatives Considered

* sync over WiFi
* git-annex
* carry disk and update with rsync

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

