
### Samsung Galaxy Tab 10.1 USB switch ###

There are 3 USB modes controlled by two GPIOs (usb-sel1 / usb-sel2)

| mode | usb-sel1 | usb-sel2 |
| --- | :---: | :---: |
| USB_SEL_AP_USB  | 1 | 1 |
| USB_SEL_CP_USB  | 0 | 0 |
| USB_SEL_ADC  | 0 | 1 |

#### Modes ####
AP - Application Processor USB
 * Standard USB operating mode

CP - Cellular Processor USB
 * Cellular mode

ADC - Analog Digital Controller
 * A/D measurement mode. Switch to this mode to take measurements from the STMPE811 A/D Controller. A voltage level of > 800 and < 1000 means an AC power cable is connected.

#### GPIOS ####
    usb-sel1 = 10

    usb-sel2 = 113
