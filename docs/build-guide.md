# RasCube 1U — build and assembly guide

Companion document to `rascube-flight-stack-wiring.svg`. Read the wiring drawing alongside this guide — the zone letters (A–F) referenced below match the zones on that sheet.

---

## 1. Checklist — sensors and payloads

### 1.1 Already in hand

- [x] ESP32-S3 N16R8 (Edgehax S3 Pro) — flight computer
- [x] ESP32-CAM with default OV2640 camera — imaging payload
- [x] SX1278 Ra-02 433 MHz LoRa module ×2 — one flight, one ground
- [x] 433 MHz SMA antenna ×2
- [x] IPEX1-to-SMA pigtail, 10 cm ×2

### 1.2 Sensors and payload — still to buy

- [ ] BME280 (get the 6-pin breakout with an SDO pin exposed, not the 4-pin version — you need SDO to set the I2C address away from the BMP280)
- [ ] BMP280
- [ ] MPU6050 (GY-521 breakout is fine)
- [ ] microSD card, ≤8 GB, class 10, formatted FAT32

### 1.3 Power

- [ ] 18650 Li-ion cell, protected, 2600–3400 mAh
- [ ] 18650 holder with wire leads (not a bare clip — you want strain relief)
- [ ] TP4056 charge module — the variant with a separate DW01 + FS8205 protection IC, not the bare 4-pin TP4056
- [ ] MT3608 boost converter module
- [ ] Panel-mount or pigtail USB-C for charging access without opening the chassis

### 1.4 Hardware, consumables, and small parts

- [ ] Silicone hookup wire, 26–28 AWG, at least three colors
- [ ] 2.54 mm Dupont jumpers (M-M, M-F, F-F) — bench prototyping only, not for the final build
- [ ] 2.0 mm-to-2.54 mm adapter board for the Ra-02, or be prepared to solder wires straight onto its pads
- [ ] 4.7 kΩ resistors ×2 (I2C pull-ups — the S3's internal ones are too weak once wires get longer than a few cm)
- [ ] 100 kΩ resistors ×2 (battery voltage divider)
- [ ] Electrolytic capacitor, 100 µF, and ceramic, 0.1 µF (LoRa VCC decoupling)
- [ ] Electrolytic capacitor, 470 µF (5 V rail, near the camera)
- [ ] M2 and M2.5 standoffs and screws, nylon or brass, matched to your RasCube standoff pattern
- [ ] Kapton tape (insulate any bare pad or trace facing the chassis wall)
- [ ] Thin double-sided foam tape or servo tape (mounting sensors and modules without screws)
- [ ] Heat-shrink tubing, assorted diameters
- [ ] A short length of foam or rubber padding for the battery bay
- [ ] Threadlocker (small dot on any screw that will see vibration — drone tests count)

### 1.5 Tools

- [ ] Soldering iron with a fine tip, plus 0.5–0.8 mm solder
- [ ] Flux pen
- [ ] Multimeter with continuity beep
- [ ] Small flathead screwdriver (for the MT3608 trim pot)
- [ ] Wire strippers rated for 26–30 AWG
- [ ] Second USB-C cable and a spare 5 V supply (bench power, separate from the flight battery)
- [ ] Tweezers, ideally anti-static
- [ ] A second breadboard (see the wiring guide — the S3 header spacing eats a full breadboard on its own)

### 1.6 Recommended additions, not strictly required

- [ ] Polyfuse (500 mA–1 A) in series with the battery positive lead, as a cheap safety net against a wiring mistake
- [ ] A second, independent kill switch on the battery lead — separate from the S3's own power path — for the drone test phase specifically (see §8)
- [ ] A small silica gel packet inside the chassis, if you're not fully sealing it

---

## 2. Bench bring-up, before anything touches the chassis

Do all of this on an open bench, powered from USB, before a single part goes inside the RasCube shell. Every failure mode here is far easier to fix on a breadboard than after the stack is closed up.

1. **Flash a blank sketch to the S3** and confirm it enumerates over USB-C. Confirm you can reach both the native USB port and the UART port.
2. **Run a GPIO/SD scan.** This is the one open item from the wiring drawing — probe continuity from the microSD slot's CLK, CMD, and DAT0 contacts to header pins 38, 39, 47, 48, and 21 with the board unpowered. Then separately confirm 10–13 are *not* shared with the card. Update the wiring drawing once this is resolved; treat the dashed amber boxes in zone A as unconfirmed until then.
3. **Wire the I2C bus alone** (zone B) with only the BME280 and MPU6050 connected, pull-ups in place. Run an I2C scanner sketch. You should see `0x76` and `0x68`. Add the BMP280 with its SDO strapped to 3V3, rescan, confirm `0x77` appears alongside the other two.
4. **Wire the LoRa module alone** (zone C) on the second breadboard. Power it, and before connecting an antenna, run a receive-only test — confirm the module initializes and reports its version register over SPI. Only connect the antenna once you're ready to transmit.
5. **Bench-pair the two LoRa modules.** Flight module on the S3, ground module on the Raspberry Pi (per the earlier pinout). Send a counted packet every second from one side, log RSSI and packet loss on the other. Do this across the room, not side by side — at arm's length you'll get false confidence from near-field coupling.
6. **Wire the camera link alone** (zone D). Flash the ESP32-CAM standalone first with the jumpers on GPIO0/GND removed, confirm it boots and streams to a browser over its own Wi-Fi test sketch. Then wire it to the S3's UART1 and confirm the S3 can request and receive a full JPEG.
7. **Set the boost converter voltage before it touches anything else.** Power the MT3608 alone from a bench supply, output disconnected from every other board, and trim the pot until you read 5.00–5.05 V on a multimeter. Only then wire its output to the 5 V rail.
8. **Load-test the power chain.** Battery → TP4056 → MT3608 → 5 V rail, with the S3 and camera both drawing normally and the LoRa transmitting on a duty cycle. Watch the rail on a multimeter for at least ten minutes; it should not sag below 4.8 V during a transmit burst. If it does, the S3's own regulator is undersized for the combined load and the sensor 3.3 V rail needs its own small regulator instead of running off the S3's 3V3 pin.

Do not proceed to chassis integration until every one of these seven checks has passed cleanly.

---

## 3. Wiring assembly

Build the harness bus by bus, testing after each one, rather than wiring the whole stack and debugging it cold.

1. **Ground first.** Star all grounds back to a single point near the S3's GND pin rather than daisy-chaining them module to module. This matters more than it looks like it should — a shared ground return path is a common source of noisy ADC readings on the battery sense line.
2. **3.3 V rail** to the three sensors and the LoRa module (zone B and C). Confirm each module's supply pin with a multimeter before connecting signal wires — a reversed VCC/GND on a cheap breakout is the single most common way to lose a sensor.
3. **I2C bus** (zone B): SDA to IO8, SCL to IO9, pull-ups to 3.3 V. Address-strap the BME280 and BMP280 as noted in the checklist.
4. **SPI bus** (zone C): the four LoRa signal lines to IO10–IO13 in order, RESET to IO5, DIO0 to IO4. Keep this harness as short as your chassis allows — SPI at 8–10 MHz is not forgiving of long, loose wires.
5. **UART link** (zone D): camera TX to IO17, camera RX to IO18. Keep the jumpers to GPIO0 and GND accessible even after final assembly — you will want to reflash the camera at least once more.
6. **Power chain** (zone E): battery → TP4056 → MT3608 → 5 V rail → S3 and camera. Fit the polyfuse and kill switch here if you're using them. Battery voltage divider onto IO1.
7. **Dress the harness.** Route wires along the chassis walls, not across the middle where they'll shadow the camera or press against the antenna feed-through. Kapton-tape any exposed solder joint that could touch the chassis.

---

## 4. Firmware bring-up order

Bring subsystems online one at a time on the assembled-but-still-open stack, in this order:

1. S3 boots, prints to console over USB.
2. I2C scan from inside the real firmware confirms all three sensor addresses.
3. Sensor readings look physically sane — pressure near your local station pressure, temperature near ambient, accelerometer reads ~1 g on the vertical axis at rest.
4. LoRa initializes and can send a test packet the ground Pi receives.
5. S3 requests and stores one JPEG from the camera successfully.
6. Full telemetry loop: read sensors, write a log line to flash or SD, transmit a compressed telemetry packet, repeat on a fixed interval. Let this run for at least one hour on the bench and check nothing drifts, resets, or leaks memory.
7. Only after step 6 is stable, close up the chassis.

---

## 5. Mechanical integration into the RasCube shell

- Stack order, bottom to top: battery against one wall (heaviest single mass, keep it low and centered for a predictable center of gravity), S3 board next, sensor breakouts on standoffs above it, camera facing the aperture, LoRa module and antenna feed nearest the wall the antenna exits through.
- Leave the SMA antenna connector accessible from outside the shell, or route the antenna itself through a feed-through hole — do not coil the antenna inside a closed metal or carbon shell, it will kill your range.
- Mount the camera so its lens sits flush with the aperture, with no wire or standoff in its field of view.
- Leave the USB-C charging port and the boot/reset buttons reachable, either through cutouts or via a removable panel — you will want both during the test campaign in §8.
- Foam-pad the battery bay. It's the heaviest single part on board and the one you most want isolated from vibration.
- Once closed, re-run the I2C scan and a LoRa loopback test through the sealed shell before calling assembly done — closing the case can pinch a wire or press a connector loose in ways that aren't visible from outside.

---

## 6. Bench integration test, fully assembled

- Run the full telemetry loop (§4 step 6) for at least 4 hours continuously with the shell closed, on battery power, not USB.
- Confirm the LoRa link holds across the distance you actually intend to test at.
- Log battery voltage over the full run and compare against the discharge curve you'd expect from the cell's rated capacity — a curve that drops far faster than expected usually means a wiring or load problem, not a bad cell.
- Cycle power (hard reset) five times and confirm the firmware comes back up cleanly each time — brownouts during boot are a common failure mode with boost converters that haven't fully settled.

---

## 7. Controlled test campaign using a drone

A drone gives you repeatable, controlled conditions — altitude, vibration, and airflow you can vary on purpose — well before any uncontrolled drop or launch test. Treat it as an instrumented flight, not a stress test.

### 7.1 Mounting

- Use a rigid bracket rather than a tether for any test where you need clean IMU or barometer data — a tether swings and adds its own vibration signature that will show up in your accelerometer log and confuse the readings you're actually trying to collect.
- Keep a secondary tether or lanyard in addition to the rigid mount, purely as a drop-prevention safety measure — it should carry no load in normal operation.
- Mount as close to the drone's own center of gravity as the frame allows, and rebalance the drone (most flight controllers have an autotune or accelerometer calibration step) after adding the payload mass.
- Fit a quick-release or a small number of accessible screws — you'll be swapping the payload on and off between flights more often than you expect.

### 7.2 RF coexistence

- Check what band your drone's control link and video downlink use. Many consumer drones run control and FPV video on 2.4 GHz or 5.8 GHz. Your LoRa link at 433 MHz sits clear of both, so it shouldn't interfere — but the ESP32's Wi-Fi and Bluetooth radios are 2.4 GHz, and if you have Wi-Fi enabled on the S3 or camera for debugging, disable it for flight. Leave only LoRa active.
- Do a static RF check before the first flight: power the payload up next to the drone with motors armed but not spinning, and confirm the drone's RC link and telemetry are unaffected.
- Keep LoRa transmit power modest for these tests — you don't need long range for a controlled flight near the ground station, and lower power reduces any risk of desensitizing nearby receivers.

### 7.3 Suggested flight profiles

- **Static hover baseline.** Hover at a fixed altitude for several minutes to get a clean reference log with no altitude or attitude change, useful for spotting sensor noise floor and drift.
- **Stepped altitude climb.** Climb in fixed increments (for example every 20–30 m, subject to your local flight ceiling and permissions) and hold at each step. This calibrates your barometric altitude against the drone's own GPS or barometer altitude reading.
- **Controlled yaw and roll.** Gentle, deliberate rotations to validate the MPU6050 orientation output against the drone's known attitude.
- **Vibration characterization.** Log accelerometer data through a normal flight and compare against a hover — this tells you how much of your "noise" in later data is airframe vibration rather than sensor error, and whether your payload mounting is damping it adequately.
- **Prop-wash exposure check.** Downwash can transiently skew a barometer or temperature reading if the sensor sits in the airflow. Fly a pass with the payload in and out of the wash and compare — if it's significant, plan to shield or reposition the pressure sensor's intake in the final build.
- **RF range walk.** With the drone holding position, walk the ground station away and log RSSI and packet loss versus distance, giving you a real link budget rather than a datasheet estimate.

### 7.4 Data and review

- Log everything to the flight computer's own storage (flash or SD) in addition to whatever comes down over LoRa — the LoRa link is the one thing you're also testing, so don't rely on it as your only data path.
- Timestamp-sync your payload log against the drone's own flight log after landing (most flight controllers export one) so you can overlay altitude, attitude, and your sensor readings on the same time axis.
- Inspect the airframe and every connector after each flight before the next one — vibration loosens things gradually, and a connector that's half-seated will often still work right up until it doesn't.

### 7.5 Regulatory note

Drone flights are governed separately from anything CubeSat-specific — airspace rules, registration, and altitude limits are set by your national civil aviation authority and can change. Confirm current requirements before you fly rather than relying on this guide for that part.

---

## 8. Final pre-flight checklist (for each test session)

- [ ] Battery charged, TP4056 LED confirms full charge
- [ ] Boost converter output re-verified at 5.0–5.05 V
- [ ] All fasteners threadlocked and torqued, none finger-tight
- [ ] Antenna connector torqued, not cross-threaded
- [ ] Camera lens clean and unobstructed
- [ ] I2C scan clean, all three sensors present
- [ ] LoRa link confirmed with ground station before mounting on the drone
- [ ] Wi-Fi and Bluetooth radios disabled in firmware for the flight
- [ ] Payload mount torque-checked, quick-release seated
- [ ] Secondary tether attached
- [ ] Ground station logging started before the drone arms
- [ ] Spare battery and a spare S3 board on hand at the field, in case of a hard failure mid-session
