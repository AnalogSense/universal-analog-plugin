# Universal Analog Plugin

A plugin for the [Wooting Analog SDK](https://github.com/WootingKb/wooting-analog-sdk) that makes it support a wider range of keyboards.

Note that universal-analog-plugin acts as a replacement for wooting-analog-plugin.

To install it, follow these simple steps:

1. Navigate to `C:\Program Files\WootingAnalogPlugins` in your File Explorer
2. Create a new folder called `_disabled`
3. Move the `wooting-analog-plugin` folder into the `_disabled` folder
4. Create a new folder called `universal-analog-plugin`
5. Drop the `universal-analog-plugin.dll` into the `universal-analog-plugin` folder

## Supported Keyboards

- Wooting Two HE
- Wooting Two HE ARM
- Wooting 60 HE ARM
- Wooting Lekker
- Wooting Two
- Wooting One
- Wooting UwU
- Wooting UwU RGB
- Razer Huntsman V2 Analog
- Razer Huntsman Mini Analog (Untested)

Note that most of the heavy lifting of supporting the keyboards is done in [soup::AnalogueKeyboard](https://github.com/calamity-inc/Soup/blob/senpai/soup/AnalogueKeyboard.cpp).

If your keyboard is not mentioned here, please refer to [The List](https://github.com/calamity-inc/universal-analog-plugin/issues/1).
