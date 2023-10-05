#include <thread>

#include <soup/AnalogueKeyboard.hpp>
#include <soup/Thread.hpp>

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

SOUP_CEXPORT int _device_info(DeviceInfo** buffer, uint32_t len)
{
	buffer[0] = &dev_info;
	return 1;
}

// Actual important stuff

static bool running = true;
static soup::Thread poll_thread;

static uint8_t actives = 0;
static uint16_t active_codes[16];
static float active_analogues[16];

SOUP_CEXPORT int _initialise(void* data, void(*callback)(void* data, DeviceEventType eventType, DeviceInfo* deviceInfo))
{
	poll_thread.start([]
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
						active_codes[i] = (key.getSoupKey() == soup::KEY_FN ? 0x409 : key.getHidScancode());
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

SOUP_CEXPORT int _read_full_buffer(uint16_t* code_buffer, float* analog_buffer, uint32_t len, DeviceID device)
{
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
