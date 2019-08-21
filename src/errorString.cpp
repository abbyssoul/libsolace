#include "solace/details/string_utils.hpp"

#include <cstdlib>
#include <cstring>
#include <cstdio>

#include <algorithm>  // std::min


namespace Solace {
namespace details {

ErrorString::~ErrorString() noexcept {
	if (_message) {
		free(const_cast<char*>(_message));
		_message = nullptr;
	}
}



ErrorString::ErrorString(StringView view)
	: _size{view.size()}
	, _message{static_cast<value_type*>(calloc(view.size() + 1, 1))}
{
	memcpy(_message, view.data(), _size);
}



StringWriter::size_type
StringWriter::measure(size_t value) noexcept {
	return narrow_cast<size_type>(snprintf(nullptr, 0, "%lu", value));
}

StringWriter::StringWriter(size_type memSize)
	: _size{memSize}
	, _offset{0}
	, _buffer{static_cast<details::ErrorString::value_type*>(calloc(memSize + 1, 1))}
{
	if (!_buffer) {
		_size = 0;
	}
}

StringWriter&
StringWriter::append(StringView data) {
	size_type const dataCopied = std::min(data.size(), remaining());
	memcpy(currentBuffer(), data.data(), dataCopied);
	_offset += dataCopied;

	return *this;
}

StringWriter&
StringWriter::appendFormated(size_t value) {
	_offset += snprintf(currentBuffer(), remaining(), "%lu", value); return *this;
}

StringWriter&
StringWriter::appendFormated(int32 value) {
	_offset += snprintf(currentBuffer(), remaining(), "%d", value); return *this;
}

StringWriter&
StringWriter::append(const char* value) {
	_offset += snprintf(currentBuffer(), remaining(), "%s", value); return *this;
}

}  // namespace details
}  // namespace Solace
