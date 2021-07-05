#pragma once

#include <cassert>
#include <cstddef>
#include <iosfwd>
#include <memory>

namespace Grafkit
{

	// TODO: Create mixins for all of this this 
	class IStream;
	typedef std::vector<uint8_t> StreamData; 

	/**
	 * Interface and Wrapper for streams
	 */
	class IStream
	{
	public:
		virtual ~IStream() = default;

		virtual void Read(char * const & buffer, size_t length) = 0;
		virtual void Write(const char * buffer, size_t length) = 0;
		[[nodiscard]] virtual bool IsSuccess() const = 0;

		[[nodiscard]] virtual bool ReadAll(StreamData & outBuffer) = 0;

		// Dirty trick to be backward compatible toward STD
		// This one used for testing purposes I guess
		explicit virtual operator std::istream &() const = 0;
		explicit virtual operator std::ostream &() const = 0;

		operator bool() const { return IsSuccess(); }
	};

	/**
	 *
	 * @tparam StreamType
	 */
	template <class StreamType> class Stream final : public IStream
	{
	public:
		explicit Stream(StreamType & stream) : mStream(stream) {}

		~Stream() noexcept override { mStream.flush(); }
		void Read(char * const & buffer, size_t length) override { mStream.read(buffer, length); }
		void Write(const char * const buffer, size_t length) override { mStream.write(buffer, length); }
		[[nodiscard]] bool IsSuccess() const override { return bool(mStream); };

		[[nodiscard]] bool ReadAll(StreamData & outBuffer) override
		{
			if (!mStream.good()) { return false; }
			// Something is not quite right about this one 
			std::copy(std::istream_iterator<uint8_t>(mStream), std::istream_iterator<uint8_t>{}, std::back_inserter(outBuffer));
			mStream.seekg(0, mStream.beg);
			//return IsSuccess();
			return true;
		}

		explicit operator std::istream &() const override { return static_cast<std::istream &>(mStream); }
		explicit operator std::ostream &() const override { return static_cast<std::ostream &>(mStream); }

	protected:
		StreamType & mStream;
	};

	/**
	 *
	 * @tparam StreamType
	 */
	template <class StreamType> class InputStream final : public IStream
	{
	public:
		explicit InputStream(StreamType & stream) : mStream(stream) {}

		~InputStream() override = default;

		void Read(char * const & buffer, size_t length) override { mStream.read(buffer, length); }

		void Write(const char * const buffer, size_t length) override { throw std::runtime_error("Can't write to an InputStream"); }
		[[nodiscard]] bool IsSuccess() const override { return bool(mStream); };

		[[nodiscard]] bool ReadAll(StreamData & outBuffer) override
		{
			if (!mStream.good()) { return false; }
			// Something is not quite right about this one 
			std::copy(std::istream_iterator<uint8_t>(mStream), std::istream_iterator<uint8_t>{}, std::back_inserter(outBuffer));
			mStream.seekg(0, mStream.beg);
			//return IsSuccess();
			return true;
		}

		explicit operator std::istream &() const override { return static_cast<std::istream &>(mStream); }
		explicit operator std::ostream &() const override { throw std::runtime_error("Can't write to an InputStream"); }

	protected:
		StreamType & mStream;
	};

	/**
	 *
	 * @tparam StreamType
	 */
	template <class StreamType> class OutputStream final : public IStream
	{
	public:
		explicit OutputStream(StreamType & stream) : mStream(stream) {}

		~OutputStream() override { mStream.flush(); }
		void Read(char * const & buffer, size_t length) override { throw std::runtime_error("Can't read from an OutputStream"); }
		void Write(const char * const buffer, size_t length) override { mStream.write(buffer, length); }

		[[nodiscard]] bool IsSuccess() const override { return bool(mStream); };

		[[nodiscard]] bool ReadAll(StreamData & outBuffer) override { throw std::runtime_error("Can't read from an OutputStream"); }

		explicit operator std::istream &() const override { throw std::runtime_error("Can't read from an OutputStream"); }
		explicit operator std::ostream &() const override { return static_cast<std::ostream &>(mStream); }

	protected:
		StreamType & mStream;
	};

	// TODO: use mixins maybe for I/O functions? 

} // namespace Grafkit
