# Universal Analog Plugin

A plugin for the [Wooting Analog SDK](https://github.com/WootingKb/wooting-analog-sdk) that makes it support a wider range of keyboards.

## Setup

1. Download the latest `universal-analog-plugin.dll` from [the releases page](https://github.com/calamity-inc/universal-analog-plugin/releases)
2. Navigate to `C:\Program Files\WootingAnalogPlugins` in your File Explorer
3. Create a new folder called `universal-analog-plugin`
4. Drop the `universal-analog-plugin.dll` into the `universal-analog-plugin` folder

## Supported Keyboards

- Razer Huntsman V2 Analog
- Razer Huntsman Mini Analog*‡
- Razer Huntsman V3 Pro*
- Razer Huntsman V3 Pro Mini*
- Razer Huntsman V3 Pro Tenkeyless*
- Keychron Q1 HE†

If your keyboard is not mentioned here, take a look at [The List](https://github.com/calamity-inc/universal-analog-plugin/issues/1) for everything that's on my radar. If your keyboard is not on my radar, please let me know!

Note that the actual logic for interacting with the devices is in [soup::AnalogueKeyboard](https://github.com/calamity-inc/Soup/blob/senpai/soup/AnalogueKeyboard.cpp).

---

\* Razer Synapse needs to be running for analogue inputs to be received from this keyboard.

† The support for this keyboard is polling based, so lag and missed inputs are to be expected.

‡ I don't own this keyboard, so I've not had a chance to test it, but it should work.
