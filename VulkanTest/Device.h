#pragma once

#ifndef NDEBUG
	const bool enableValidationLayers = true;
#else
	const bool enableValidationLayers = false;
#endif // !NDEBUG

class Device
{
public:
	void createInstance();
	void checkValidationLayerSupport();



};

