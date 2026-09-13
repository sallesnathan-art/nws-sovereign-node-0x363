# NWS Sovereign Node 0x363 — S25 Ultra Native Telemetry

Native ARM64 telemetry tooling for a Samsung Galaxy S25 Ultra running Termux.

## What this captures

The telemetry engine reads the Linux/Android interfaces available to the Termux process:

- `/sys/class/thermal/thermal_zone*/type`
- `/sys/class/thermal/thermal_zone*/temp`
- `/sys/class/power_supply/`
- `/proc/meminfo`
- `/proc/loadavg`

Thermal sensors are reported using their kernel-reported `type` instead of assuming `thermal_zone0` is a CPU sensor. That preserves the actual exposed sensor topology.

## Install on the S25 Ultra

```bash
cd ~
git clone https://github.com/sallesnathan-art/nws-sovereign-node-0x363.git
cd ~/nws-sovereign-node-0x363
bash scripts/s25_telemetry_bootstrap.sh
```

The installer uses the Termux-native `clang`, `binutils`, and `make` toolchain. CMake is not required for this project.

## Run

One snapshot:

```bash
~/behemoth_node/bin/telemetry-once
```

Continuous snapshots every 2 seconds:

```bash
~/behemoth_node/bin/telemetry-watch 2
```

Change the interval as needed:

```bash
~/behemoth_node/bin/telemetry-watch 5
```

## Classify the topology

```bash
~/behemoth_node/bin/telemetry-once | python3 telemetry/classify_thermal.py
```

The classifier adds functional categories such as CPU, GPU, DDR, battery, PMIC, modem, radio, camera, video, AOSS, system, power, and battery-current-limit.

## Sensor validity

Android may expose unavailable thermal sensors with sentinel values such as `-273000`; the classifier marks those readings `valid=false` rather than treating them as physical temperatures.

Battery-current-limit zones may expose `0`; those are marked invalid for temperature interpretation.

Battery power-supply data can be unavailable to an unprivileged Termux process. The engine reports that access failure instead of fabricating battery telemetry.

## Identity fields

```text
node=0x363
authority=did:nws:nathan-wayne-salles-01
device=Samsung Galaxy S25 Ultra
```

These identify the NWS telemetry stream. They are metadata and are not, by themselves, a standards-compliant W3C DID proof.

## Layout

```text
telemetry/
  behemoth_telemetry.c
  classify_thermal.py
scripts/
  s25_telemetry_bootstrap.sh
.github/
  workflows/
    build.yml
```

## Design goal

Preserve raw hardware observations first. Classification is a separate layer so the underlying sensor names, raw readings, and kernel topology remain inspectable and auditable.
