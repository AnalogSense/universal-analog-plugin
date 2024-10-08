# Universal Analog Plugin

A plugin for the [Wooting Analog SDK](https://github.com/WootingKb/wooting-analog-sdk) that makes it support a wider range of keyboards.

## Setup

1. Download the latest `universal-analog-plugin.dll` from [the releases page](https://github.com/calamity-inc/universal-analog-plugin/releases)
2. Navigate to `C:\Program Files\WootingAnalogPlugins` in your File Explorer
3. Create a new folder called `universal-analog-plugin`
4. Drop the `universal-analog-plugin.dll` into the `universal-analog-plugin` folder

## Supported Keyboards

- Razer Huntsman V2 Analog<sup>R</sup>
- Razer Huntsman Mini Analog<sup>R</sup>
- Razer Huntsman V3 Pro<sup>R</sup>
- Razer Huntsman V3 Pro Mini<sup>R</sup>
- Razer Huntsman V3 Pro Tenkeyless<sup>R</sup>
- DrunkDeer A75
- DrunkDeer A75 Pro<sup>U</sup>
- DrunkDeer G75<sup>U</sup>
- DrunkDeer G65<sup>U</sup>
- DrunkDeer G60<sup>U</sup>
- Keychron Q1 HE<sup>P</sup>
- Keychron Q3 HE<sup>P, U</sup>
- Keychron Q5 HE<sup>P, U</sup>
- Keychron K2 HE<sup>P, U</sup>

If your keyboard is not mentioned here, take a look at [The List](https://github.com/calamity-inc/universal-analog-plugin/issues/1) for everything that's on my radar. If your keyboard is not on my radar, please let me know!

Wooting devices are also supported, but only with the `universal-analog-plugin.with-wooting-device-support.dll`, in which case it acts as a replacement for the wooting-analog-plugin.

Note that the actual logic for interacting with the devices is in [soup::AnalogueKeyboard](https://github.com/calamity-inc/Soup/blob/senpai/soup/AnalogueKeyboard.cpp).

---

<sup>R</sup> Razer Synapse needs to be installed and running for analogue inputs to be received from this keyboard.

<sup>P</sup> The official firmware only supports polling individual keys, which can lead to lag and missed inputs given the sheer amount of keys to scan, but you can use [custom firmware with full analog report functionality](https://analogsense.org/firmware/).

<sup>U</sup> I don't own this keyboard, so I've not had a chance to test it, but it should work.
