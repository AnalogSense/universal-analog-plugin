#include <soup/AnalogueKeyboard.hpp>
#include <soup/HidScancode.hpp>
#include <soup/os.hpp>
#include <soup/RecursiveMutex.hpp>
#include <soup/Thread.hpp>
#include <soup/SharedPtr.hpp>

#define ABI_VERSION_TARGET 0

#define LOGGING false

// Some Analog SDK apps rely on read_full_buffer sending a 0 value for released keys instead of stopping to report the key.
#define REPORT_RELEASED_KEYS true

#if LOGGING
#include <iostream>
#endif

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

// Rust interop stuff

#if ABI_VERSION_TARGET == 0
#pragma comment(lib, "wooting_analog_common.lib")
#pragma comment(lib, "Userenv.lib")
#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "Bcrypt.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Advapi32.lib")

extern "C"
{
	DeviceInfo* new_device_info(uint16_t vendor_id, uint16_t product_id, const char* manufacturer_name, const char* device_name, DeviceID device_id, DeviceType device_type);
	void drop_device_info(DeviceInfo* device);
}
#else
inline DeviceInfo* new_device_info(uint16_t vendor_id, uint16_t product_id, const char* manufacturer_name, const char* device_name, DeviceID device_id, DeviceType device_type)
{
	return new DeviceInfo(vendor_id, product_id, manufacturer_name, device_name, device_id, device_type);
}

inline void drop_device_info(DeviceInfo* device)
{
	delete device;
}
#endif

// Boilerplate

SOUP_CEXPORT const uint32_t ANALOG_SDK_PLUGIN_ABI_VERSION = ABI_VERSION_TARGET;

#if ABI_VERSION_TARGET >= 1
#define _name name
#define _device_info device_info
#define _initialise initialise
#define _read_full_buffer read_full_buffer
#endif

SOUP_CEXPORT const char* _name()
{
	return "Universal Analog Plugin";
}

SOUP_CEXPORT bool is_initialised()
{
	return true;
}

// Devices

[[nodiscard]] static DeviceID make_device_id(const soup::AnalogueKeyboard& kbd)
{
	return (((DeviceID)kbd.hid.vendor_id) << 16) | kbd.hid.product_id;
}

struct Device
{
	DeviceID id;
	DeviceInfo* info;
	soup::AnalogueKeyboard kbd;
	soup::Thread thrd;
	uint8_t actives = 0;
	uint16_t active_codes[16];
	float active_analogues[16];

	Device(soup::AnalogueKeyboard&& _kbd)
		: id(make_device_id(_kbd)), kbd(std::move(_kbd))
	{
		auto manufacturer_name = kbd.hid.getManufacturerName();
		info = new_device_info(kbd.hid.vendor_id, kbd.hid.product_id, manufacturer_name.c_str(), kbd.name.c_str(), this->id, DeviceType::Keyboard);
	}

	~Device()
	{
		drop_device_info(info);
	}
};

static soup::RecursiveMutex devices_mtx{};
static std::vector<soup::SharedPtr<Device>> devices{};

SOUP_CEXPORT int _device_info(DeviceInfo* buffer[], uint32_t len)
{
	devices_mtx.lock();
	if (len > devices.size())
	{
		len = devices.size();
	}
	for (uint32_t i = 0; i != len; ++i)
	{
		buffer[i] = devices[i]->info;
	}
	devices_mtx.unlock();
	return len;
}

// Actual important stuff

static bool running = true;
static soup::Thread discover_thread;

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

using event_handler_t = void(*)(void* data, DeviceEventType eventType, DeviceInfo* deviceInfo);

static void* event_handler_data;
static event_handler_t event_handler;

static void discover_devices(bool initial)
{
	for (auto& kbd : soup::AnalogueKeyboard::getAll())
	{
#ifndef WOOTING_SUPPORT
		if (kbd.hid.usage_page == 0xFF54)
		{
			continue;
		}
#endif

		const auto device_id = make_device_id(kbd);
		bool known = false;
		devices_mtx.lock();
		for (auto it = devices.begin(); it != devices.end(); )
		{
			if (!(*it)->thrd.isRunning())
			{
#if LOGGING
				std::cout << "Removing " << kbd.name << " from devices array" << std::endl;
#endif
				it = devices.erase(it);
				continue;
			}
			if ((*it)->id == device_id)
			{
				known = true;
				break;
			}
			++it;
		}
		devices_mtx.unlock();
		if (!known)
		{
#if LOGGING
			std::cout << "New device: " << kbd.name << std::endl;
#endif
			auto spDev = soup::make_shared<Device>(std::move(kbd));
			if (!initial)
			{
				event_handler(event_handler_data, DeviceEventType::Connected, spDev->info);
			}

			spDev->thrd.start([](soup::Capture&& cap)
			{
				auto& spDev = cap.get<soup::SharedPtr<Device>>();
				Device& dev = *spDev;
				soup::AnalogueKeyboard& kbd = spDev->kbd;
				while (running && !kbd.disconnected)
				{
					auto keys = kbd.getActiveKeys();
					uint8_t i = 0;
					for (const auto& key : keys)
					{
						dev.active_codes[i] = mapToWootingKey(key.getSoupKey());
						dev.active_analogues[i] = key.getFValue();
						++i;
					}
					dev.actives = i;
				}
#if LOGGING
				std::cout << "Thread for " << kbd.name << " is stopping" << std::endl;
#endif
			}, spDev);

			devices_mtx.lock();
			devices.emplace_back(std::move(spDev));
			devices_mtx.unlock();
		}
	}
}

SOUP_CEXPORT int _initialise(void* data, event_handler_t callback)
{
#if LOGGING
	AllocConsole();
	{
		FILE* f;
		freopen_s(&f, "CONIN$", "r", stdin);
		freopen_s(&f, "CONOUT$", "w", stderr);
		freopen_s(&f, "CONOUT$", "w", stdout);
	}
#endif

	event_handler_data = data;
	event_handler = callback;
	discover_devices(true);
#if LOGGING
	std::cout << "Discovered " << devices.size() << " initial devices" << std::endl;
	for (const auto& dev : devices)
	{
		std::cout << "- " << dev->kbd.name << std::endl;
	}
#endif
	const auto num_initial_devices = static_cast<int>(devices.size());
	discover_thread.start([](soup::Capture&&)
	{
		uint8_t i = 0;
		while (running)
		{
			if (++i == 100)
			{
				i = 0;
				discover_devices(false);
			}
			soup::os::sleep(10);
		}
	});
	return num_initial_devices;
}

#if REPORT_RELEASED_KEYS
static std::unordered_set<uint16_t> pending_release;
#endif

SOUP_CEXPORT float read_analog(uint16_t code, DeviceID device_id)
{
	float ret = 0.0f;

	devices_mtx.lock();
	for (auto& dev : devices)
	{
		if (device_id == 0 || dev->id == device_id)
		{
			for (uint8_t i = 0; i != dev->actives; ++i)
			{
				if (dev->active_codes[i] == code)
				{
					ret = dev->active_analogues[i];
					goto _break_2;
				}
			}
		}
	}
_break_2:
	devices_mtx.unlock();

	return ret;
}

SOUP_CEXPORT int _read_full_buffer(uint16_t* code_buffer, float* analog_buffer, uint32_t len, DeviceID device_id)
{
	SOUP_IF_UNLIKELY (len == 0)
	{
		return 0;
	}

	uint32_t actives = 0;
	devices_mtx.lock();
	for (auto& dev : devices)
	{
		if (device_id == 0 || dev->id == device_id)
		{
			for (uint8_t i = 0; i != dev->actives; ++i)
			{
				code_buffer[actives] = dev->active_codes[i];
				analog_buffer[actives] = dev->active_analogues[i];
				if (++actives == len)
				{
					goto _break_2;
				}
			}
		}
	}
_break_2:
	devices_mtx.unlock();

#if REPORT_RELEASED_KEYS
	if (!pending_release.empty())
	{
		// Keys that are still being pressed don't need a release this update
		for (uint8_t i = 0; i != actives; ++i)
		{
			pending_release.erase(code_buffer[i]);
		}

		// Discharge released keys
		for (const auto& code : pending_release)
		{
			if (actives != len)
			{
				code_buffer[actives] = code;
				analog_buffer[actives] = 0.0f;
				++actives;
			}
		}
		pending_release.clear();
	}

	// Remember active keys for next update
	for (uint8_t i = 0; i != actives; ++i)
	{
		if (analog_buffer[i] != 0.0f)
		{
			pending_release.emplace(code_buffer[i]);
		}
	}
#endif

	return actives;
}

SOUP_CEXPORT void unload()
{
	running = false;
	discover_thread.awaitCompletion();

	devices_mtx.lock();
	for (auto& dev : devices)
	{
		dev->kbd.hid.cancelReceiveReport();
		dev->thrd.awaitCompletion();
	}
	devices_mtx.unlock();

#if LOGGING
	FreeConsole();
#endif
}
