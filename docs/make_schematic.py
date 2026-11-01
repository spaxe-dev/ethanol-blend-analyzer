#!/usr/bin/env python3
# Generates the EthanolBlend measurement schematic as SVG + PNG.
# Every pin/net here is taken from src/config.h and the physical rig.
# Output: docs/ethanolblend-schematic.svg / .png

import os
import cairosvg

W, H = 1700, 1280
INK = "#111111"
GREY = "#555555"
LIGHT = "#8a8a8a"
P = []


# ---------------------------------------------------------------- primitives
def text(x, y, s, size=15, weight="400", anchor="start", fill=INK, spacing=None):
    extra = f' letter-spacing="{spacing}"' if spacing else ""
    P.append(
        f'<text x="{x}" y="{y}" font-size="{size}" font-weight="{weight}" '
        f'text-anchor="{anchor}" fill="{fill}"{extra}>{s}</text>'
    )


def line(x1, y1, x2, y2, w=2.5, dash=None):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    P.append(
        f'<line x1="{x1}" y1="{y1}" x2="{x2}" y2="{y2}" stroke="{INK}" '
        f'stroke-width="{w}"{d} />'
    )


def rect(x, y, w, h, stroke=INK, sw=3, fill="#ffffff", rx=5, dash=None):
    d = f' stroke-dasharray="{dash}"' if dash else ""
    P.append(
        f'<rect x="{x}" y="{y}" width="{w}" height="{h}" rx="{rx}" ry="{rx}" '
        f'fill="{fill}" stroke="{stroke}" stroke-width="{sw}"{d} />'
    )


def wire(pts, w=2.5):
    d = " ".join(f"{'M' if i == 0 else 'L'}{x} {y}" for i, (x, y) in enumerate(pts))
    P.append(f'<path d="{d}" fill="none" stroke="{INK}" stroke-width="{w}" '
             f'stroke-linejoin="round" stroke-linecap="round" />')


def junction(x, y):
    P.append(f'<circle cx="{x}" cy="{y}" r="5.5" fill="{INK}" />')


def ground(x, y):
    """Ground symbol; the wire attaches at (x, y), symbol hangs below."""
    line(x, y, x, y + 12)
    line(x - 19, y + 12, x + 19, y + 12)
    line(x - 12, y + 18, x + 12, y + 18)
    line(x - 5, y + 24, x + 5, y + 24)


def power(x, y, label):
    """Power rail symbol; the wire attaches at (x, y), symbol points up."""
    line(x, y, x, y - 16)
    line(x - 13, y - 16, x + 13, y - 16)
    text(x, y - 24, label, 13, "700", "middle")


def res_h(x0, y, length, amp=9, n=6, leaf=10):
    seg = (length - 2 * leaf) / n
    pts = [(x0, y), (x0 + leaf, y)]
    for i in range(n):
        pts.append((x0 + leaf + seg * (i + 0.5), y + (amp if i % 2 == 0 else -amp)))
    pts += [(x0 + length - leaf, y), (x0 + length, y)]
    wire(pts)


def res_v(x, y0, length, amp=8, n=4, leaf=8):
    seg = (length - 2 * leaf) / n
    pts = [(x, y0), (x, y0 + leaf)]
    for i in range(n):
        pts.append((x + (amp if i % 2 == 0 else -amp), y0 + leaf + seg * (i + 0.5)))
    pts += [(x, y0 + length - leaf), (x, y0 + length)]
    wire(pts)


def nc(x, y):
    line(x - 5, y - 5, x + 5, y + 5)
    line(x - 5, y + 5, x + 5, y - 5)


# ---------------------------------------------------------------- sheet frame
rect(16, 16, W - 32, H - 32, sw=2.5, fill="none", rx=0)

# ---------------------------------------------------------------- title block
text(70, 96, "ETHANOLBLEND", 38, "700")
text(70, 126, "Measurement schematic &#8212; ESP32 + ADS1115 conductivity probe", 19)
text(70, 150, "20 Hz square excitation &#183; single-ended ADC &#183; 6-point empirical calibration", 15, fill=GREY)
text(70, 170, "Rev feat/esp32-excitation @ c335b5f &#183; 2026-11-01 &#183; sheet 1/1", 13, fill=LIGHT)
line(70, 184, 1630, 184, 2)

# ---------------------------------------------------------------- ESP32
rect(90, 195, 270, 660)
text(112, 232, "ESP32", 24, "700")
text(112, 254, "DevKit V1 &#183; 30-pin &#183; 3.3 V logic", 13, fill=GREY)
text(112, 806, "Powered from USB 5 V", 12, fill=GREY)
text(112, 824, "EN pin: not connected", 12, fill=GREY)

esp_pins = [
    (300, "3V3", "pwr"),
    (340, "GND", "gnd"),
    (410, "GPIO21", "SDA"),
    (450, "GPIO22", "SCL"),
    (520, "GPIO4", "DQ"),
    (600, "GPIO26", "SIG"),
    (700, "GPIO13", "BTN"),
]
for y, pin, net in esp_pins:
    line(360, y, 400, y)
    text(348, y + 5, pin, 15, anchor="end")
    if net == "pwr":
        power(400, y, "3V3")
    elif net == "gnd":
        ground(400, y)
    else:
        text(410, y + 5, net, 16, "700")

# ---------------------------------------------------------------- OLED
rect(900, 195, 350, 210)
text(920, 227, "OLED 0.96&#8243; SSD1306", 19, "700")
text(920, 248, "128 &#215; 64 &#183; I&#178;C 0x3C", 13, fill=GREY)

# screen preview
rect(1140, 268, 88, 52, sw=2.5, fill="#111111", rx=3)
text(1184, 289, "LIKELY", 10, "700", "middle", "#ffffff")
text(1184, 309, "E20", 16, "700", "middle", "#ffffff")

oled_pins = [(280, "VCC", "pwr"), (314, "GND", "gnd"),
             (348, "SDA", "SDA"), (382, "SCL", "SCL")]
for y, pin, net in oled_pins:
    line(900, y, 860, y)
    text(912, y + 5, pin, 14)
    if net == "pwr":
        power(860, y, "3V3")
    elif net == "gnd":
        ground(860, y)
    else:
        text(850, y + 5, net, 16, "700", "end")

# ---------------------------------------------------------------- ADS1115
rect(900, 440, 350, 250)
text(920, 472, "ADS1115", 20, "700")
text(920, 493, "16-bit &#916;&#931; ADC &#183; I&#178;C 0x48", 13, fill=GREY)
text(1230, 522, "GAIN &#177;4.096 V", 12, fill=GREY, anchor="end")
text(1230, 542, "RATE 860 SPS", 12, fill=GREY, anchor="end")
text(1230, 562, "A1&#8211;A3 : NC", 12, fill=GREY, anchor="end")

ads_pins = [(525, "VDD", "pwr"), (559, "GND", "gnd"),
            (593, "SDA", "SDA"), (627, "SCL", "SCL"),
            (661, "ADDR", "gnd")]
for y, pin, net in ads_pins:
    line(900, y, 860, y)
    text(912, y + 5, pin, 14)
    if net == "pwr":
        power(860, y, "3V3")
    elif net == "gnd":
        ground(860, y)
        if pin == "ADDR":
            text(840, y + 5, "0x48", 12, fill=GREY, anchor="end")
    else:
        text(850, y + 5, net, 16, "700", "end")

# A0 leaves the bottom edge and runs down to the divider node
line(1000, 690, 1000, 1040)
text(1012, 762, "A0", 16, "700")
text(1012, 780, "sense node", 12, fill=GREY)

# ---------------------------------------------------------------- DS18B20
rect(90, 880, 270, 240)
text(112, 912, "DS18B20", 20, "700")
text(112, 933, "1-Wire &#183; 12-bit &#183; 2 s interval", 13, fill=GREY)

for y, pin in [(960, "VDD"), (1030, "DQ"), (1100, "GND")]:
    line(360, y, 400, y)
    text(348, y + 5, pin, 15, anchor="end")

# mandatory 4.7k pull-up, DQ -> 3V3
junction(400, 960)
line(400, 960, 400, 934)
power(400, 934, "3V3")
line(400, 960, 460, 960)
res_v(460, 960, 70)
line(460, 1030, 400, 1030)
junction(400, 1030)
text(482, 992, "4.7 k&#937;", 13, "700")
text(482, 1010, "pull-up", 11, fill=GREY)
text(408, 1022, "DQ", 16, "700")
ground(400, 1100)

# ---------------------------------------------------------------- button
text(430, 1180, "BTN", 16, "700")
line(470, 1175, 500, 1175)
P.append(f'<circle cx="504" cy="1175" r="4" fill="{INK}" />')
P.append(f'<circle cx="576" cy="1175" r="4" fill="{INK}" />')
line(518, 1157, 518, 1193, 3)
line(562, 1157, 562, 1193, 3)
line(513, 1148, 567, 1148, 3)
line(540, 1130, 540, 1148, 3)
text(540, 1122, "SW1", 12, fill=GREY, anchor="middle")
line(580, 1175, 610, 1175)
wire([(610, 1175), (610, 1196)])
ground(610, 1196)
text(540, 1222, "instant reading", 11, fill=GREY, anchor="middle")

# ---------------------------------------------------------------- front end
rect(700, 900, 560, 312, sw=2.5, fill="#fcfcfc", dash="9 6", rx=3)
text(718, 928, "ELECTRODE CELL + EXCITATION DIVIDER", 14, "700", spacing="0.5")

text(718, 1045, "SIG (GPIO26)", 15, "700")
line(830, 1040, 860, 1040)
res_h(860, 1040, 100)
text(905, 1064, "10 k&#937;", 13, "700", anchor="middle")
line(960, 1040, 1000, 1040)
junction(1000, 1040)

text(718, 975, "V_node = 3.3 V &#215; R_cell / (10 k&#937; + R_cell)", 12, fill=GREY)
text(718, 993, "I_max = 330 &#956;A into the liquid", 12, fill=GREY)
text(718, 1011, "A0 is bounded 0&#8211;3.3 V &#8212; no clamps", 12, fill=GREY)

# node -> electrode 1
wire([(1000, 1040), (1000, 1065), (970, 1065), (970, 1165)])
# electrode 2 -> ground
line(1035, 1075, 1035, 1165, 3.5)
wire([(1035, 1075), (1105, 1075), (1105, 1102)])
ground(1105, 1102)

# beaker
P.append(f'<path d="M930 1090 L930 1150 Q930 1180 960 1180 L1040 1180 '
         f'Q1070 1180 1070 1150 L1070 1090" fill="none" stroke="{INK}" '
         f'stroke-width="2.5" />')
P.append(f'<path d="M933 1122 L1067 1122 L1067 1148 Q1067 1177 1038 1177 '
         f'L962 1177 Q933 1177 933 1148 Z" fill="#e6e6e6" />')
line(933, 1122, 1067, 1122, 2)
text(1000, 1204, "fuel under test", 11, fill=GREY, anchor="middle")
text(956, 1112, "E1", 12, "700", "end")
text(1052, 1112, "E2", 12, "700")

# ---------------------------------------------------------------- notes
rect(1300, 195, 360, 700, sw=2, fill="#fcfcfc", rx=3)
text(1322, 228, "DESIGN NOTES", 17, "700")
notes = [
    ("700", "Excitation: 20 Hz, 0&#8211;3.3 V,"),
    ("", "50 % duty, GPIO26 hardware timer."),
    ("", ""),
    ("700", "Node voltage"),
    ("", "V_node = 3.3 &#215; R_cell / (10 k&#937; + R_cell)"),
    ("", ""),
    ("700", "Max current into the liquid:"),
    ("", "3.3 V / 10 k&#937; = 330 &#956;A."),
    ("", ""),
    ("700", "A0 is bounded 0&#8211;3.3 V by"),
    ("", "construction &#8212; no clamps fitted."),
    ("", ""),
    ("700", "ADC: single-ended, &#177;4.096 V,"),
    ("", "860 SPS, 48-sample burst every 5 s."),
    ("", "Lookup key = max of the burst."),
    ("", ""),
    ("700", "All logic 3.3 V. Never apply 5 V"),
    ("", "to ADS1115 inputs (abs max 3.6 V)."),
    ("", ""),
    ("700", "Not fitted: HX711, load cell,"),
    ("", "ICL8038, LM358."),
    ("", ""),
    ("700", "Nets with the same name are joined."),
]
ny = 262
for weight, s in notes:
    if s:
        text(1322, ny, s, 13, weight if weight else "400",
             fill=INK if weight else GREY)
    ny += 26

# ---------------------------------------------------------------- legend
rect(1300, 920, 360, 320, sw=2, fill="#fcfcfc", rx=3)
text(1322, 952, "SYMBOL LEGEND", 17, "700")

junction(1338, 990)
text(1372, 995, "connection (nets joined)", 12, fill=GREY)

line(1338, 1018, 1338, 1032)
line(1320, 1032, 1356, 1032)
line(1326, 1039, 1350, 1039)
line(1332, 1046, 1344, 1046)
text(1372, 1040, "common ground (0 V)", 12, fill=GREY)

line(1338, 1082, 1338, 1068)
line(1325, 1068, 1351, 1068)
text(1372, 1082, "3V3 rail", 12, fill=GREY)

line(1326, 1108, 1350, 1108)
nc(1350, 1108)
text(1372, 1113, "no connect", 12, fill=GREY)

text(1322, 1142, "SDA", 13, "700")
line(1360, 1138, 1400, 1138)
text(1408, 1142, "net label", 12, fill=GREY)
text(1322, 1172, "Same net name = same electrical node.", 11, fill=LIGHT)

# ---------------------------------------------------------------- output
svg = (
    '<?xml version="1.0" encoding="UTF-8"?>\n'
    f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
    f'viewBox="0 0 {W} {H}" font-family="Arial, Helvetica, sans-serif">\n'
    f'<rect width="{W}" height="{H}" fill="#ffffff" />\n'
    + "\n".join(P)
    + "\n</svg>\n"
)

here = os.path.dirname(os.path.abspath(__file__))
svg_path = os.path.join(here, "ethanolblend-schematic.svg")
png_path = os.path.join(here, "ethanolblend-schematic.png")
pdf_path = os.path.join(here, "ethanolblend-schematic.pdf")

with open(svg_path, "w", encoding="utf-8") as f:
    f.write(svg)

cairosvg.svg2png(bytestring=svg.encode("utf-8"), write_to=png_path, scale=3,
                 background_color="white")
# A4 landscape page for direct inclusion in the report
cairosvg.svg2pdf(bytestring=svg.encode("utf-8"), write_to=pdf_path)
print("wrote", svg_path)
print("wrote", png_path)
print("wrote", pdf_path)
