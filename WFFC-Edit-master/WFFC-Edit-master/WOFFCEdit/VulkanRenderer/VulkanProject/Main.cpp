#pragma once

#include "Renderer.h"
#include "Window.h"
#include "VulkanDeferredApplication.h"
#include "VisibilityBuffer.h"

int main()
{
	Renderer r;
	VulkanDeferredApplication va(&r,1000,800);
	return 0;
}
