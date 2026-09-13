# Telemetry layer

`behemoth_telemetry.c` is the native collector. It emits raw thermal-zone observations and available system metrics.

`classify_thermal.py` is intentionally downstream from collection. Pipe collector output into it when a functional classification is useful:

```bash
~/behemoth_node/bin/telemetry-once | python3 telemetry/classify_thermal.py
```

This separation keeps observation lossless at the text-record level: the kernel sensor `type`, thermal-zone identifier, raw value, and converted Celsius value remain present after classification.

## Important readings from the S25 Ultra sample

Observed kernel-reported types included:

```text
cpu-* / cpuss-*
gpuss-*
ddr
battery
pm8550*
pmr735d_tz
mdmss-*
nsphmx-*
nsphvx-*
mmw*
sdr*
camera-*
video
aoss-*
sys-therm-*
ac
pm8550-bcl-lvl*
```

A negative sentinel such as `raw=-273000` is retained but classified as `valid=false` rather than being interpreted as a literal physical temperature.
