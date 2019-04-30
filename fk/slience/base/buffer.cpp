#include "slience/base/buffer.hpp"
#include <stdlib.h>
#include <string.h>

M_BASE_NAMESPACE_BEGIN

#define M_BUFFER_DEFAILT_SIZE 512

Buffer::Buffer() {
	_data._pos = _data._offset = (0);
	_data._size = M_BUFFER_DEFAILT_SIZE;
	_data._data = (s_byte_t*)malloc(_data._size);
}

Buffer::~Buffer() {
	free(_data._data);
}

void Buffer::Clear() {
	_data._pos = _data._offset = (0);
	if (_data._size >= 4 * 1024) {
		free(_data._data);
		_data._size = M_BUFFER_DEFAILT_SIZE;
		_data._data = (s_byte_t*)malloc(_data._size);
	}
}

void Buffer::CutData(s_int32_t len) {
	if (_data._offset + len > _data._pos)
		_data._offset = _data._pos;
	else
		_data._offset += len;
}

s_byte_t* Buffer::Data() {
	return _data._data + _data._offset;
}

const s_byte_t* Buffer::Data()const {
	return _data._data + +_data._offset;
}

s_byte_t* Buffer::Raw() {
	return _data._data;
}

const s_byte_t* Buffer::Raw()const {
	return _data._data;
}

s_uint32_t Buffer::Capacity()const {
	return _data._size;
}

s_uint32_t Buffer::Size()const {
	return _data._pos;
}

s_uint32_t Buffer::Length()const {
	return (_data._pos - _data._offset);
}

void Buffer::Write(const void* data, s_uint32_t len) {
	if (_data._pos + len > _data._size) {
		_data._size = M_BUFFER_DEFAILT_SIZE * ((_data._pos + len) / M_BUFFER_DEFAILT_SIZE + 1);
		s_byte_t* ptmp = (s_byte_t*)malloc(_data._size);
		memcpy(ptmp, _data._data, _data._pos);
		free(_data._data);
		_data._data = ptmp;
	}
	memcpy(_data._data + _data._pos, data, len);
	_data._pos += len;
}

void Buffer::Read(void* data, s_uint32_t len) {
	if (_data._offset + len > _data._pos)
		return;

	memcpy(data, _data._data + _data._offset, len);
	_data._offset += len;
}

void Buffer::Swap(Buffer& buffer) {
	_data_ d = this->_data;
	this->_data = buffer._data;
	buffer._data = d;
}

M_BASE_NAMESPACE_END