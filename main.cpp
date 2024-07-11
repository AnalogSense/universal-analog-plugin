#include <thread>

#include <soup/AnalogueKeyboard.hpp>
#include <soup/HidScancode.hpp>
#include <soup/Thread.hpp>

// Some Analog SDK apps rely on read_full_buffer sending a 0 value for released keys instead of stopping to report the key.
#define REPORT_RELEASED_KEYS true

#if REPORT_RELEASED_KEYS
#include <unordered_set>
#endif

// Types

using DeviceID = uint64_t;

enum class DeviceEventType : int
{
	Connected = 1,
	Disconnected = 2,
};

enum class DeviceType : int
{
	Keyboard = 1,
	Keypad = 2,
	Other = 3,
};

struct DeviceInfo
{
	uint16_t vendor_id;
	uint16_t product_id;
	const char* manufacturer_name;
	const char* device_name;
	DeviceID device_id;
	DeviceType device_type;
};

// Boilerplate

SOUP_CEXPORT const uint32_t ANALOG_SDK_PLUGIN_ABI_VERSION = 0;

SOUP_CEXPORT const char* _name()
{
	return "Universal Analog Plugin";
}

SOUP_CEXPORT bool is_initialised()
{
	return true;
}

// Dummy device

static DeviceInfo dev_info{ 0xFFFF, 0xFFFF, "Unknown", "Soup-compatible Analog Keyboard", 0, DeviceType::Keyboard };

SOUP_CEXPORT int _device_info(DeviceInfo* buffer[], uint32_t len)
{
	buffer[0] = &dev_info;
	return 1;
}

// Actual important stuff

static bool running = true;
static soup::Thread poll_thread;

static uint8_t actives = 0;
static uint16_t active_codes[REPORT_RELEASED_KEYS ? 32 : 16];
static float active_analogues[REPORT_RELEASED_KEYS ? 32 : 16];

[[nodiscard]] static uint16_t mapToWootingKey(soup::Key key)
{
	switch (key)
	{
	case soup::KEY_NEXT_TRACK: return 0x3B5;
	case soup::KEY_PREV_TRACK: return 0x3B6;
	case soup::KEY_STOP_MEDIA: return 0x3B7;
	case soup::KEY_PLAY_PAUSE: return 0x3CD;
	case soup::KEY_OEM_1: return 0x403;
	case soup::KEY_OEM_2: return 0x404;
	case soup::KEY_OEM_3: return 0x405;
	case soup::KEY_OEM_4: return 0x408;
	case soup::KEY_FN: return 0x409;
	default:;
	}
	return soup::soup_key_to_hid_scancode(key);
}

SOUP_CEXPORT int _initialise(void* data, void(*callback)(void* data, DeviceEventType eventType, DeviceInfo* deviceInfo))
{
	poll_thread.start([](soup::Capture&&)
	{
		while (running)
		{
			for (auto& kbd : soup::AnalogueKeyboard::getAll())
			{
				while (running && !kbd.disconnected)
				{
					auto keys = kbd.getActiveKeys();
					uint8_t i = 0;
					for (const auto& key : keys)
					{
						active_codes[i] = mapToWootingKey(key.getSoupKey());
						active_analogues[i] = key.getFValue();
						++i;
					}
					actives = i;
				}
			}
			if (!running)
			{
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	});
	return 1;
}

#if REPORT_RELEASED_KEYS
static std::unordered_set<uint16_t> pending_release;
#endif

SOUP_CEXPORT float read_analog(uint16_t code)
{
	for (uint8_t i = 0; i != actives; ++i)
	{
		if (active_codes[i] == code)
		{
			return active_analogues[i];
		}
	}
	return 0.0f;
}

SOUP_CEXPORT int _read_full_buffer(uint16_t* code_buffer, float* analog_buffer, uint32_t len, DeviceID device)
{
#if REPORT_RELEASED_KEYS
	if (!pending_release.empty())
	{
		// Keys that are still being pressed don't need a release this update
		for (uint8_t i = 0; i != actives; ++i)
		{
			pending_release.erase(active_codes[i]);
		}

		// Discharge released keys
		for (const auto& code : pending_release)
		{
			active_codes[actives] = code;
			active_analogues[actives] = 0.0f;
			actives++;
		}
		pending_release.clear();
	}

	// Remember active keys for next update
	for (uint8_t i = 0; i != actives; ++i)
	{
		if (active_analogues[i] != 0.0f)
		{
			pending_release.emplace(active_codes[i]);
		}
	}
#endif
	if (len > actives)
	{
		len = actives;
	}
	memcpy(code_buffer, active_codes, len * sizeof(uint16_t));
	memcpy(analog_buffer, active_analogues, len * sizeof(float));
	return len;
}

SOUP_CEXPORT void unload()
{
	running = false;
	CancelSynchronousIo(poll_thread.handle);
	poll_thread.awaitCompletion();
}
