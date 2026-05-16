# ETLE Speed Detector

**Group 3**
 
| Name                                  | Student ID  |
| ------------------------------------- | ----------- |
| Yohana Indah Nathania Br Sihotang     | 2406368946  |
| Nayla Pramesti Adhina                 | 2406368901  |
| Muhammad Naufal Gilardino             | 2406450434  |
| Muhammad Agib Anugrah Pratama         | 2406450415  |

## 1. Introduction

### Problem
Road safety is a critical global concern, and excessive vehicle speed remains one of the leading causes of traffic accidents. Although speed-limit regulations are widely enforced in urban and semi-urban areas, their enforcement still depends largely on manual observation by traffic officers, an approach that is inconsistent, labor-intensive, and prone to human error. This creates a clear need for automated, real-time speed detection systems that can operate continuously without human supervision, and provide immediate visual and audible feedback when violations occur.

### Solution
ETLE Speed Detector is a microcontroller-based system that automatically measures the speed of a moving vehicle and classifies it against a user-adjustable threshold. The system simulates a real-world ETLE deployment at a smaller scale, using a Hot Wheels traveling between two infrared sensors placed 20 cm apart on a test track.
 
**Working principle:**
 
1. When the car crosses IR Sensor 1, an external interrupt on INT0 starts Timer1.
2. When the car crosses IR Sensor 2, an external interrupt on INT1 stops Timer1.
3. The elapsed time is converted into seconds, and the speed is computed using the following formula:
   ```
   Speed (km/h) = (Distance (m) ÷ Time (s)) × 3.6
   ```
   where Distance = 0.2 m.
4. The computed speed is compared against a user-set threshold read from a potentiometer through the ADC.
5. The result is displayed on an I²C LCD, indicated by a traffic-light LED, optionally accompanied by a buzzer alarm, and transmitted to a host PC via USART.

**Speed classification:**
 
| Condition                                      | LED      | LCD Message   | Buzzer |
| ---------------------------------------------- | -------- | --------------| ------ |
| Speed < 10 km/h                                | Yellow   | TOO SLOW      | Off    |
| 10 km/h ≤ Speed ≤ Threshold                    | Green    | SAFE SPEED    | Off    |
| Speed > Threshold                              | Red      | OVER SPEED!   | On     |


## 2. Hardware Design and Implementation

### Materials
| No. | Component / Tool                          | Qty       | Purpose                                    |
| --- | ----------------------------------------- | --------- | ------------------------------------------ |
| 1   | Arduino Uno (+cables)                     | 1         | Main microcontroller board                 |
| 2   | IR Sensor Module                          | 2         | Vehicle detection (Sensor 1 & Sensor 2)    |
| 3   | I²C LCD 16×2                              | 1         | Real-time speed and status display         |
| 4   | Traffic-Light LED Module                  | 1         | Visual speed-status indicator              |
| 5   | Active Buzzer Module                      | 1         | Overspeed alarm                            |
| 6   | Potentiometer                             | 1         | Adjustable speed-limit threshold (via ADC) |
| 7   | Breadboard                                | 1         | Circuit prototyping                        |
| 8   | Jumper Wires                              | Many      | Component interconnections                 |
| 9   | Hot Wheels                                | 1         | Test vehicle                               |
| 10  | Hot Wheels Launcher                       | 1         | Vehicle propulsion for testing             |

### Circuit Diagram
Proteus:
OTW (screenshot)

Physic:
OTW (screenshot)

### Pin Assignment/Hardware Connection
OTW


## 3. Software Implementation (wait for final code)
### System Initialization
- Configure `DDRD`: IR sensor pins (PD2, PD3) as inputs; LED and buzzer pins as outputs.
- Initialize USART0: set baud-rate divisor in `UBRR0H:UBRR0L`, enable transmitter via `UCSR0B`, set 8N1 frame format in `UCSR0C`.
- Initialize ADC: configure `ADMUX` for AVcc reference on channel 0 (PC0); enable ADC with prescaler in `ADCSRA`.
- Initialize TWI and LCD: write bit-rate divider to `TWBR`, enable peripheral via `TWCR`, perform LCD power-on sequence using the HD44780 nibble protocol over PCF8574.
- Configure external interrupts: set `EICRA` for falling-edge detection on INT0 and INT1; enable both lines in `EIMSK`.
- Read the last overspeed value from EEPROM and display it on the LCD.

### Main Loop
- Poll ADC channel 0, map the 10-bit result (0–1023) to a speed threshold in the range 20–100 km/h, and update the LCD threshold display.
- Check the flag set by the INT1 ISR; if set, read the captured Timer1 count, compute vehicle speed, classify it, and drive the outputs accordingly.
- If an overspeed event occurred, write the speed value to EEPROM using the `EEAR` / `EEDR` / `EECR` register sequence.
- Return to monitoring state.

## 4. Test and Performance Evaluation
### Test Cases
| # | Scenario             | Threshold (km/h) | Expected LED | Expected LCD     | Expected Buzzer |
| - | -------------------- | ---------------- | ------------ | ---------------- | --------------- |
| 1 | Below minimum range  | 50               | Yellow       | `TOO SLOW`       | Off             |
| 2 | Within safe range    | 50               | Green        | `SAFE SPEED`     | Off             |
| 3 | Above threshold      | 50               | Red          | `OVER SPEED!`    | On              |

### Video Result
YouTube link: OTW

### Evaluation
OTW after test

## 5. Conclusion and Future Work
### Conclusion
ETLE Speed Detector successfully integrates many lab modules. The system measures the speed of a moving toy car between two IR sensors, classifies it against a user-defined threshold, and provides visual, audible, and serial feedback while continuously logging the most recent overspeed event in EEPROM.

### Future Work
Potential improvements include: 
- **License-plate capture** by adding a camera module triggered on the overspeed event.
- **Calibration mode** to compensate for sensor placement error and IR response delay.
