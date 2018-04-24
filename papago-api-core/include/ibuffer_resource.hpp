#pragma once

class IBufferResource {
public:
	virtual ~IBufferResource() = default;

	template<typename T>
	void upload(const std::vector<T>& data);

	template<typename T = char>
	std::vector<T> download();
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
inline void IBufferResource::upload(const std::vector<T>& data)
{
	std::vector<char> buffer(sizeof(T)*data.size());
	memcpy(buffer.data(), &data, buffer.size());
	internalUpload(buffer);
}


template<>
inline std::vector<char> IBufferResource::download<char>()
{
	return internalDownload();
}

template<typename T>
inline std::vector<T> IBufferResource::download()
{
	auto data = internalDownload();
	T buffer;
	memcpy(&buffer, data.data(), sizeof(T));
	return buffer;
}
