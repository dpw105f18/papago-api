#pragma once
#include <vector>
#include <memory>

class IDynamicBuffer
{
public:
	virtual ~IDynamicBuffer() = default;
	template<typename T>
	void upload(const std::vector<T>& data);

	template<typename T>
	void upload(const std::vector<T>& data, size_t index);

	template<typename T = char>
	std::vector<T> download();

private:
	virtual std::vector<char> internalDownload() = 0;
	virtual void internalUpload(const std::vector<char>&, size_t offset) = 0;
	virtual size_t getAlignment() = 0;
};

class IBufferResource {
public:
	virtual ~IBufferResource() = default;

	template<typename T>
	void upload(const std::vector<T>& data);

	template<typename T = char>
	std::vector<T> download();

	virtual bool inUse() = 0;
protected:
	virtual void internalUpload(const std::vector<char>& data) = 0;
	virtual std::vector<char> internalDownload() = 0;
};

template<>
inline void IBufferResource::upload<char>(const std::vector<char>& data)
{
	internalUpload(data);
}

template<typename T>
void IBufferResource::upload(const std::vector<T>& data)
{
	std::vector<char> buffer(sizeof(T)*data.size());
	memcpy(buffer.data(), data.data(), buffer.size());
	internalUpload(buffer);
}


template<>
inline std::vector<char> IBufferResource::download<char>()
{
	return internalDownload();
}

template<typename T>
std::vector<T> IBufferResource::download()
{
	auto data = internalDownload();
	std::vector<T> buffer;
	buffer.resize(data.size() / sizeof(T));
	memcpy(buffer.data(), data.data(), sizeof(T));
	return buffer;
}

template<typename T>
inline void IDynamicBuffer::upload(const std::vector<T>& data)
{
	auto alignment = getAlignment();
	std::vector<char> result(data.size() * alignment);
	auto size = sizeof(T);
	for (auto index = 0, offset = 0; index < data.size(); ++index, offset += alignment)
	{
		memcpy(&result[offset], &data[index], sizeof(T));
	}
	internalUpload(result, 0);
}

template<typename T>
inline void IDynamicBuffer::upload(const std::vector<T>& data, size_t index)
{
	auto alignment = getAlignment();
	std::vector<char> result(data.size() * alignment);
	auto size = sizeof(T);

	auto offset = alignment * index;
	memcpy(&result[offset], &data[index], sizeof(T));
	internalUpload(result, offset);
}





template<typename T>
inline std::vector<T> IDynamicBuffer::download()
{
	auto data = internalDownload();
	std::vector<T> result(data.size() / sizeof(T));
	auto alignment = getAlignment();
	for (auto index = 0, offset = 0; offset < data.size(); ++index, offset += alignment)
	{
		memcpy(&result[index], &data[offset], sizeof(T));
	}
	return result;
}
