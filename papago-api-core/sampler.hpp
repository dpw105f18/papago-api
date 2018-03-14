#pragma once

class Sampler2D
{
public:
	enum Filter {
		NEAREST,
		ANISOTROPIC,
		LINEAR
	};

	enum Wrap {
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER
	};

	void setMagFilter(Filter);
	void setMinFilter(Filter);
	void setTextureWrapS(Wrap);
	void setTextureWrapT(Wrap);
};