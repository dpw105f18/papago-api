#pragma once
#include <vector>
#include <memory>

class IDynamicBufferResource
{
public:
	virtual ~IDynamicBufferResource() = default;

	template<typename T>
	void upload(const std::vector<T>& data);

	template<typename T>
	void upload(const std::vector<T>& data, size_t index);

	template<typename T>
	std::vector<T> download();

	template<typename T>
	T download(size_t index);

	virtual void uploadPadded(const std::vector<char>&) = 0;

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
inline void IDynamicBufferResource::upload(const std::vector<T>& data)
{
	auto alignment = getAlignment();
	std::vector<char> result(data.size() * alignment);
	auto size = data.size();
	auto tSize = sizeof(T);
	for (auto index = 0, offset = 0; index < size; ++index, offset += alignment)
	{
		memcpy(&result[offset], &data[index], tSize);
	}
	internalUpload(result, 0);
}

template<typename T>
inline void IDynamicBufferResource::upload(const std::vector<T>& data, size_t index)
{
	auto alignment = getAlignment();
	std::vector<char> result(data.size() * alignment);
	auto size = sizeof(T);

	for (auto i = 0, o = 0; i < data.size(); ++i, o += alignment) {
		memcpy(&result[o], &data[i], sizeof(T));
	}

	auto offset = alignment * index;
	
	internalUpload(result, offset);
}

template<typename T>
inline std::vector<T> IDynamicBufferResource::download()
{
	auto data = internalDownload();
	auto alignment = getAlignment();
	std::vector<T> result(data.size() / alignment);
	for (auto index = 0, offset = 0; offset < data.size(); ++index, offset += alignment)
	{
		memcpy(&result[index], &data[offset], sizeof(T));
	}
	return result;
}

template<typename T>
inline T IDynamicBufferResource::download(size_t index)
{
	auto data = internalDownload();
	auto alignment = getAlignment();
	T result;

	memcpy(&result, &data[index * alignment], sizeof(T));
	
	return result;
}
