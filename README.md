evdev-right-click-emulation
---

A program that solves the problem of right clicks on Linux devices with a touchscreen. It implements the long-press-to-right-click gesture on Linux touchscreen devices while having no specific requirements on the desktop environment, display server or the distribution, which greatly improves the touch experience for users of Linux distributions.

It directly reads from `evdev` touchscreen input devices, parses the events, and triggers a right click with `uinput` when it detects a long press from a touchscreen. Since `evdev` is lower-level than `libinput` or any other user-space input driver, this program can work regardless of your choice of Xorg or Wayland.

This program started as a Python script that I published on [Gist](https://gist.github.com/PeterCxy/b4e256b6b4a133c93c012b9738c557ca). However, the Python binding of `libevdev` had some mysterious bugs, and I thought it was not the best idea to run a Python interpreter to read every input event just to implement a simple feature. Thus I rewrote it in C, which is what is contained in this repository.

Dependencies
---

- libevdev
- libevdev headers (for development)

And of course `glibc` and your Linux kernel.

Building
---

```bash
make all
```

The compiled program lies in `out/evdev-rce`.

Usage
---

Simply run

```bash
out/evdev-rce
```

Or you may put this executable anywhere on your file system.

Note: this program needs to read from and write to `/dev/input`. Make sure the current user has the permission to do so, or you may use `root`.

There are some tunable options accessible through the following environment variables:

```bash
# Required interval in ms for a touch event to be considered a long press
# default = 1000ms
LONG_CLICK_INTERVAL=1000
# Allowed margin of error (in px) for a long press event
# i.e. the finger is considered still if its movement is within this range
# Note: the px here is not physical pixels on the screen. It's the physical
# pixels of the touch devices, which may have a different resolution.
# default = 100px
LONG_CLICK_FUZZ=100
# Uncomment to set a blacklist of devices that `evdev-rce` will not
# listen on for touch events. The names can be retrived by using `xinput`
# or simply by reading the output of `evdev-rce`
# (it will print out the names of all touch devices when starting,
#  e.g. "Found touch screen at <some_path>: <device_name>)
#TOUCH_DEVICE_BLACKLIST="device1 name|device2 name|..."
# Uncomment to set a whitelist of devices that `evdev-rce` will ONLY
# listen on. This overrides the blacklist - when whitelist is present,
# any device not in this list will be ignored.
#TOUCH_DEVICE_WHITELIST="device1 name|device2 name|..."
```

So you can run the program like

```
LONG_CLICK_INTERVAL=500 LONG_CLICK_FUZZ=50 evdev-rce
```

To customize those options.

Install from GitHub APT repository
---

This repository's GitHub Actions `build.yml` workflow publishes a flat Debian APT repository to the GitHub release tag `apt` whenever a version tag (`v*`) is built.

To install from that repository:

```bash
sudo mkdir -p /etc/apt/sources.list.d
cat <<'EOF' | sudo tee /etc/apt/sources.list.d/evdev-rce-github.list
deb [trusted=yes] https://github.com/bareboat-necessities/evdev-right-click-emulation/releases/download/apt/ ./
EOF

sudo apt-get update
sudo apt-get install evdev-rce
```

Notes:

- The repository is currently published as an unsigned flat repository, so the example uses `trusted=yes`.
- Packages are published to the `apt` release tag and include the generated `Packages`, `Packages.gz`, and `Release` metadata files alongside `.deb` artifacts.
