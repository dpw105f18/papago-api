#pragma once
#include <vector>
#include <memory>

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

//TODO: make IDynamicBuffer
class DynamicBuffer {
	std::unique_ptr<IBufferResource> m_buffer;
public:
	size_t m_alignment;
	size_t m_objectCount;

	DynamicBuffer(
		std::unique_ptr<IBufferResource>&&	buffer,
		size_t								alignment,
		size_t								objectCount);

	IBufferResource& innerBuffer() { return *m_buffer; }

	template<typename T>
	std::vector<T> download();
	template<typename T>
	void upload(std::vector<T>);
};

//TODO: move to buffer_resource.cpp?
inline DynamicBuffer::DynamicBuffer(
	std::unique_ptr<IBufferResource>&& buffer,
	const size_t                       alignment,
	const size_t						objectCount)
	: m_buffer(std::move(buffer))
	, m_alignment(alignment)
	, m_objectCount(objectCount)
{ }

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
std::vector<T> DynamicBuffer::download()
{
	auto data = m_buffer->download();
	std::vector<T> result(data.size()/m_alignment);
	for(auto index = 0, offset = 0; offset < data.size(); ++index, offset += m_alignment)
	{
		memcpy(&result[index], &data[offset], sizeof(T));
	}
	return result;
}

template<typename T>
void DynamicBuffer::upload(std::vector<T> data)
{
	std::vector<char> result(data.size() * m_alignment);
	auto size = sizeof(T);
	for(auto index = 0, offset = 0; index < data.size(); ++index, offset += m_alignment)
	{
		memcpy(&result[offset], &data[index], sizeof(T));
	}
	m_buffer->upload(result);
}
