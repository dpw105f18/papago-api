#include "standard_header.hpp"
#include "..\include\api_enums.hpp"

DepthStencilFlags operator&(DepthStencilFlags lhs, DepthStencilFlags rhs)
{
	return static_cast<DepthStencilFlags>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

DepthStencilFlags operator|(DepthStencilFlags lhs, DepthStencilFlags rhs)
{
	return static_cast<DepthStencilFlags>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}
