#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>

#include <Arduino.h>

#if __has_include(<SPI.h>)
#include <SPI.h>
#define NPB_HAS_SPI_TRANSPORT 1
#endif

#include "ITransport.h"

namespace npb
{

#if defined(NPB_HAS_SPI_TRANSPORT)

#ifndef NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ
#define NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ 10000000UL
#endif
	static constexpr uint32_t SpiClockDefaultHz = NEOPIXELBUS_SPI_CLOCK_DEFAULT_HZ;

	struct SpiTransportSettings
	{
		bool invert = false;
		uint32_t clockRateHz = SpiClockDefaultHz;
		BitOrder bitOrder = MSBFIRST;
		uint8_t dataMode = SPI_MODE0;
		int clockPin = -1;
		int dataPin = -1;
		SPIClass *spi = nullptr;
	};

	class SpiTransport : public ITransport
	{
	public:
		using TransportSettingsType = SpiTransportSettings;
		using TransportCategory = TransportTag;

		explicit SpiTransport(SpiTransportSettings config)
			: _config{config}
		{
		}


		void begin() override
		{
			if (_config.spi == nullptr)
			{
				return;
			}

			_config.spi->begin();
			if (_config.clockPin >= 0 && _config.dataPin >= 0)
			{
				pinMode(_config.clockPin, OUTPUT);
				pinMode(_config.dataPin, OUTPUT);
			}
		}

		void beginTransaction() override
		{
			if (_config.spi == nullptr)
			{
				return;
			}

			_config.spi->beginTransaction(SPISettings(_config.clockRateHz, _config.bitOrder, _config.dataMode));
		}

		void transmitBytes(span<const uint8_t> data) override
		{
			if (_config.spi == nullptr || data.empty())
			{
				return;
			}

			if (!_config.invert)
			{
				for (size_t index = 0; index < data.size(); ++index)
				{
					_config.spi->transfer(data[index]);
				}
				return;
			}

			for (size_t index = 0; index < data.size(); ++index)
			{
				_config.spi->transfer(static_cast<uint8_t>(~data[index]));
			}
		}

		void endTransaction() override
		{
			if (_config.spi == nullptr)
			{
				return;
			}

			_config.spi->endTransaction();
		}

	private:
		SpiTransportSettings _config;
	};

#endif

} // namespace npb

