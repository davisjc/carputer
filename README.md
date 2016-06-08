# Carputer

Music computer for car.

## About

I love listening to music while driving, and want to be as uninhibited as possible while doing so.

Previously, I had used various mobile streaming apps or would store select music directly on my phone, but I wasn't ever really satisfied with these solutions. I always had to be aware of how much lossy-encoded music I was shovelling over LTE or which songs I'd choose to put on my phone, since space is limited and my music library is large.

This project is my solution to having my entire music library in full fidelity available in my car, while making it simple to keep in sync with my library at home.

Updating my carputer's music library happens like this:
* Plug the transport thumb drive into the desktop machine.
* Run the command [`music_sync_to_transport`](bin/music_sync_to_transport) and wait for the transport drive to finish writing and unmount.
* Later, plug the transport drive into a USB port near the car's cupholder.
* The music library and any code changes sync to the carputer while a status light blinks.
* The status light returns to solid state, indicating the update is done and the drive is now unmounted and safe to yank.

![pic](http://i.imgur.com/1TLI3i6.png)
view from top

![pic](http://i.imgur.com/rzHiFMf.png)
carputer in center console

[Click here to view the entire photo album on imgur.](http://imgur.com/a/3wIdY)

## Approach

This project was a good exercise in reframing a problem.  Many ideas I had upfront were either over-engineered or not really relevant to my priorities.

Earlier, I waffled around with many complex ideas, which included a lot of "why not" thoughts that extended to fabricating an entirely new stereo head unit for my car that would have a screen, controls, amplifier, GPS, WiFi, disc burner, and some database-backed music tracking program that would track offline what music was missing or otherwise out-of-sync with my library at home by calculating and comparing file hashes.

I realized that many of the ideas I was considering were already well made: e.g., I wasn't about to casually make an easier to use interface to Google Maps than was already available on my phone.  Speaking of the phone: there's a quality display that can be mounted on the dash.  My existing stereo head unit already handles analog input well and doesn't need to be redone.

With those considerations in mind, all I really needed was a small computer with large persistent storage that can be controlled by my phone (and more ideally, physical buttons), and a simple strategy for getting music library changes synced between two offline systems using something like a USB thumb drive as a messenger of state.

## Design Priorities

* Provide entire music library in lossless, CD-quality encoding (FLAC).
* Avoid streaming data over my cell phone.
* Make syncing music an easy and non-interfering process.
* Use simple and flexible design that allows for easy customization and extensibility.
* Allow for tactile controls for controlling music (using a touch screen in a car kind of sucks ...and not very safe).

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
*  **Cable Routing**
    * Positioning the Pi in the center console was convenient for easy access to power, the aux audio cable, distance to other controls, and also provides easy access to the Pi itself.http://i.imgur.com/rzHiFMf.png
    * I've been using the PIE FRD04-AUX auxillary audio input harness to get audio fed into the car's stereo.
    * The power supply is tucked away behind the wall of the center console, tapped into the 12V service.
    * A ribbon cable plugs into the top of the Pi and then into the wall of the center console, breaking out into pins on the other side of the wall that can be used for buttons and lights.
    * The ignition wire is spliced and sent towards the center console for use by the power supply.

## The Software

For an OS, I chose [Arch Linux ARM](https://archlinuxarm.org/), because it's familiar and hackable.

At home, I use [Music Player Daemon](https://www.musicpd.org/) (MPD), [ncmpcpp](http://rybczak.net/ncmpcpp/) (an MPD client), and [beets](http://beets.io/) to play and manage my music library.  MPD is great because it's a headless music server that supports a huge array of clients, and is an ideal choice here for its flexibility.  One such MPD client is [MPoD](https://itunes.apple.com/us/app/mpod/id285063020?mt=8) for iOS.

The Pi throws up a WiFi hotspot with [hostapd](https://w1.fi/hostapd/) that mobile phones and other devices can join for controlling the music or otherwise administering the Pi over SSH.

The [udevil](https://ignorantguru.github.io/udevil/) package provides some neat facilities for managing device mounts as well as an automounting daemon, [devmon](https://igurublog.wordpress.com/downloads/script-devmon/), that can execute a program when a new block device is detected.  For example, you can specify a script to be run when a new drive is plugged in:

```bash
ARGS="--always-exec --exec-on-drive '/path/to/carputer/repo/bin/devmon_mount_carputer_transport %f %d'"
```

In the above example, devmon will replace `%f` with the file descriptor of the block device, the `%d` with the path to where the device is now mounted, and then invoke `devmon_mount_carputer_transport` with these arugments.  This provides a really simple way for running code when a USB thumb drive is plugged in.

## Music Library Syncing

In my mind, the design of the project really hinges on how state is kept in sync between two systems that will never talk to each other directly over a network.  A simple way to opt-out of this is by just carrying the hard drive between systems and using rsync to push and pull updates from it.  However, I wanted a way to keep the car's music library in sync without carrying the entire library around.

I wanted to perform a short-lived version control transaction whenever I moved a thumb drive from one system to another.  Specifically:
* The host machine needs to know what files are missing from the carputer, and copy them to the transport device.
* The carputer needs to identify which files have gone stale (removed or modified), and remove them from its own library.
* The carputer needs to copy from the transport device the new files that are currently missing from its library.

After trying a few options out, I went with a simple text representation of state file using `stat` to get the signature of a music file as a line of text in a file.  For example:

```bash
$ stat --format '%n %s %Y' 'Books, The/The Way Out/01 - Group Autogenics I.flac'
```

Produces a line of text with the file's name, size in bytes, and modification time in seconds, which looks like this:

```
Books, The/The Way Out/01 - Group Autogenics I.flac 20528401 1384975911
```

Combined with `find` and `xargs`, we can generate a text file for all music files on that system of the form:
`<path relative to music root> <size in bytes> <mtime in seconds>`

```
Books, The/The Way Out/01 - Group Autogenics I.flac 20528401 1384975911
Books, The/The Way Out/02 - IDKT.flac 9602016 1384975914
Books, The/The Way Out/03 - I Didn't Know That.flac 25711359 1384975911
Books, The/The Way Out/04 - A Cold Freezin' Night.flac 18080242 1384975910
Books, The/The Way Out/05 - Beautiful People.flac 19185859 1384975908
Books, The/The Way Out/06 - I Am Who I Am.flac 22642088 1384975909
Books, The/The Way Out/07 - Chain of Missing Links.flac 26829706 1384975909
Books, The/The Way Out/08 - All You Need Is a Wall.flac 20929658 1384975911
Books, The/The Way Out/09 - Thirty Incoming.flac 31449393 1384975910
Books, The/The Way Out/10 - A Wonderful Phrase by Gandhi.flac 1768568 1384975908
Books, The/The Way Out/11 - We Bought the Flood.flac 25207501 1384975911
Books, The/The Way Out/12 - The Story of Hip Hop.flac 27044550 1384975914
Books, The/The Way Out/13 - Free Translator.flac 18648241 1384975908
Books, The/The Way Out/14 - Group Autogenics II.flac 29314138 1384975914
Books, The/The Way Out/albumart.jpg 37875 1384975914
...
```

Since we're only querying the filesystem for metadata about the files, and not actually reading file contents, this is quite fast even with a large amount of files (see: [`music_list_contents`](bin/music_list_contents)).  This process would be slower if we wanted a more accurate file signature such as a checksum, which must read the contents of the file.  However, I call any difference in size or modififcation date good enough to know it's time to update.

For convenience, the MPD database file is also placed in the root music directory so that it is included in this strategy, since it's just a file.  Typically, MPD rescans the music files to regenerate its database file, but since we're just syncing a library of parallel structure, we can just grab the database file from the source system to avoid unnecessary scans on the carputer.  This means our list of music files includes a line that looks like this:

```
mpd_database 842730 1465326735
```

Now we can generate a representative text file for each system that sufficiently describes the state of the library.  We'll call these files `file_list_pc.txt` and `file_list_car.txt`.  As long as we keep these files up-to-date with the latest state of each system, and store these files on the transport device itself, we can easily answer the following questions by using the `comm` command, which just reports common/uncommon lines between sorted files:

* Which files are missing from the carputer that need to be copied?
```bash
$ comm -2 -3 "file_list_pc.txt" "file_list_car.txt"
```
* Which files need to be removed from the carputer that no longer exist in the master library?
```bash
$ comm -1 -3 "file_list_pc.txt" "file_list_car.txt"
```

Using the results from the above commands, we can copy new files and remove stale files as necessary to stay in sync.  For more clarity, here's a excerpt from the `comm` manpage:

```
-1              suppress column 1 (lines unique to FILE1)
-2              suppress column 2 (lines unique to FILE2)
-3              suppress column 3 (lines that appear in both files)
```

Putting everything together, the syncing does the following:

### Syncing Desktop to Transport Device

1. Plug in transport device to desktop machine.
2. devmon mounts and runs [`devmon_mount_carputer_transport`](bin/devmon_mount_carputer_transport) on the device.
3. If a file named `id` exists on the device whose contents match `$TRANSPORT_ID` as defined in [`vars`](bin/vars), the device is considered a valid transport device and is remounted at `/media/carputer_transport`.
4. Explicitly run `music_sync_to_transport` when a sync is desired.
5. The current music liibrary state is written to `file_list_pc.txt` on the transport device.
6. [`music_sync_content_to_transport`](bin/music_sync_content_to_transport) is invoked.
7. `file_list_car.txt` is read and compared against the now-updated `file_list_pc.txt` to copy the missing files to the transport device und `file_list_pc.txt` and `file_list_car.txt`.  As long as we keep these files up-to-date with the latest state of each system, and store these files on the transport device itself, we can easily answer the following questions by using the `comm` command, which just reports common/uncommon lines between sorted files:

* Which files are missing from the carputer that need to be copied?
```bash
$ comm -2 -3 "file_list_pc.txt" "file_list_car.txt"
```
* Which files need to be removed from the carputer that no longer exist in the master library?
```bash
$ comm -1 -3 "file_list_pc.txt" "file_list_car.txt"
```

Using the results from the above comer the `music/` directory.
8. The most recent version of carputer code is pulled to the transport device under the `code/` directory.
9. The transport device unmounts once the data is flushed.

### Syncing Transport Device to Carputer

1. Plug in transport device.
2. As before, devmon runs and remounts the device at `/media/carputer_transport`.
3. However, this time [`devmon_mount_carputer_transport`](bin/devmon_mount_carputer_transport) also detects that it is running on the carputer and automatically starts syncing.
4. The status LED starts to blink.
5. The most recent carputer code is pulled from the transport device's `code/` directory.
6. [`music_sync_to_carputer`](bin/music_sync_to_carputer) is invoked.
7. [`music_sync_content_to_carputer`](bin/music_sync_content_to_carputer) is invoked.
8. The files still in `file_list_car.txt` that are no longer in `file_list_pc.txt` are deleted.
9. The files in `file_list_pc.txt` that are not in `file_list_car.txt` are copied from the transport device's `music/` directory.
10. The now-updated carputer library is recorded as `file_list_car.txt` on the transport device, effectively recording the completion of the transaction.
11. The transport device unmounts once the data is flushed.
12. The status LED returns to solid, indicating that it's safe to yank.

## Alternatives Considered

### Sync Over WiFi
* One of my first ideas for staying in sync with the world.  Uses a home-base style WiFi connection that background syncs while the car is parked in the garage.
* Not very practical since the Pi drains the car battery in a matter of days.
* Didn't really want to bother with a low-power switching circuit for booting the carputer for a sync.

### git-annex
* This was a tempting "magic" solution.  If you're unfamiliar with git-annex, here's a brief description: it's an extension of git that exploits git for its version control, but side-steps the only-expanding repo problem by committing file metadata to the repo, not the files themselves.  The cool thing that git-annex provides is a tracking system of which files exist in which remote repos.  The files themsleves are stored in a special git-annex blob directory, and made available through symlinks to the working tree of the repo.  After a bit of reading and mucking around with things, I got everything working with git-annex, but it was damn slow...  And wasn't satisfactory for the following reasons:

    1. By default, it computes the hash of the file to determine its uniqueness, which is slow.  Fortunately, this is avoidable by specifying a different ["backend"](https://git-annex.branchable.com/backends/) for git-annex, such as "Write Once, Read Many", which just means the file is determined unique by its filename, size, and modification date.
    2. Files in git-annex can be directly exposed by "unlocking" them, which makes the annexed file modifyable through a hard-link to its stored git-annex blob.  Version 6 of git-annex made this easier to deal with.
    3. The other limitation, which was a more fundamental design constraint, was the way git-annex utilizes git.  In order for git-annex to deal with hard-linked, annexed blobs, git-annex makes use of custom smudge and clean [git filters](https://git-scm.com/book/en/v2/Customizing-Git-Git-Attributes#Keyword-Expansion).  Unfortunately, using git filters means that all file contents are piped through the filter, meaning you must read **all** annexed file content in order to use the filter.  This was painfully slow.  I like the idea of git-annex, but this is way too severe of a limitation for me to consider it seriously for large files.  The creator of git-annex made [a proposal to extend the git's smudge/clean filters](http://thread.gmane.org/gmane.comp.version-control.git/294425) to avoid this hardship, but I haven't heard anything promising yet.

### Carry around USB HDD and update with rsync
* My second choice.  No doubt it's the most reliable.

## Misc.

misc install notes for Arch Linux:

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
    ARGS="--always-exec --exec-on-drive '/path/to/carputer/repo/bin/devmon_mount_carputer_transport %f %d'"
    ```
    * `--always-exec` requires devmon version >= 1.1.9
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

