#include "common.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#devsandqueues-physical-device-enumeration
 * If pPhysicalDevices is NULL, then the number of physical devices available is returned in pPhysicalDeviceCount. Otherwise, pPhysicalDeviceCount must point to a
 * variable set by the user to the number of elements in the pPhysicalDevices array, and on return the variable is overwritten with the number of handles actually
 * written to pPhysicalDevices. If pPhysicalDeviceCount is less than the number of physical devices available, at most pPhysicalDeviceCount structures will be written.
 * If pPhysicalDeviceCount is smaller than the number of physical devices available, VK_INCOMPLETE will be returned instead of VK_SUCCESS, to indicate that not all the
 * available physical devices were returned.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDevices(
		VkInstance                                  instance,
		uint32_t*                                   pPhysicalDeviceCount,
		VkPhysicalDevice*                           pPhysicalDevices)
{
	assert(instance);

	int numGPUs = 1;

	assert(pPhysicalDeviceCount);

	if(!pPhysicalDevices)
	{
		*pPhysicalDeviceCount = numGPUs;
		return VK_SUCCESS;
	}

	int arraySize = *pPhysicalDeviceCount;
	int elementsWritten = min(numGPUs, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pPhysicalDevices[c] = &instance->dev;
	}

	*pPhysicalDeviceCount = elementsWritten;

	if(arraySize < numGPUs)
	{
		return VK_INCOMPLETE;
	}
	else
	{
		return VK_SUCCESS;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceProperties
 */
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(
		VkPhysicalDevice                            physicalDevice,
		VkPhysicalDeviceProperties*                 pProperties)
{
	assert(physicalDevice);
	assert(pProperties);

	VkPhysicalDeviceSparseProperties sparseProps =
	{
		.residencyStandard2DBlockShape = 1,
		.residencyStandard2DMultisampleBlockShape = 1,
		.residencyStandard3DBlockShape = 1,
		.residencyAlignedMipSize = 1,
		.residencyNonResidentStrict = 1
	};

	pProperties->apiVersion = VK_DRIVER_VERSION;
	pProperties->driverVersion = 1; //we'll simply call this v1
	pProperties->vendorID = 0x14E4; //Broadcom
	pProperties->deviceID = 0; //TODO dunno?
	pProperties->deviceType = VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
	strcpy(pProperties->deviceName, "VideoCore IV HW");
	//pProperties->pipelineCacheUUID
	pProperties->limits = _limits;
	pProperties->sparseProperties = sparseProps;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceFeatures
 */
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures(
		VkPhysicalDevice                            physicalDevice,
		VkPhysicalDeviceFeatures*                   pFeatures)
{
	assert(physicalDevice);
	assert(pFeatures);

	*pFeatures = _features;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateDeviceExtensionProperties
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(
		VkPhysicalDevice                            physicalDevice,
		const char*                                 pLayerName,
		uint32_t*                                   pPropertyCount,
		VkExtensionProperties*                      pProperties)
{
	assert(physicalDevice);
	assert(!pLayerName); //layers ignored for now
	assert(pPropertyCount);

	if(!pProperties)
	{
		*pPropertyCount = numDeviceExtensions;
		return VK_SUCCESS;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numDeviceExtensions, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c] = deviceExtensions[c];
	}

	*pPropertyCount = elementsWritten;

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceQueueFamilyProperties
 * If pQueueFamilyProperties is NULL, then the number of queue families available is returned in pQueueFamilyPropertyCount.
 * Otherwise, pQueueFamilyPropertyCount must point to a variable set by the user to the number of elements in the pQueueFamilyProperties array,
 * and on return the variable is overwritten with the number of structures actually written to pQueueFamilyProperties. If pQueueFamilyPropertyCount
 * is less than the number of queue families available, at most pQueueFamilyPropertyCount structures will be written.
 */
VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties(
		VkPhysicalDevice                            physicalDevice,
		uint32_t*                                   pQueueFamilyPropertyCount,
		VkQueueFamilyProperties*                    pQueueFamilyProperties)
{
	assert(physicalDevice);
	assert(pQueueFamilyPropertyCount);

	if(!pQueueFamilyProperties)
	{
		*pQueueFamilyPropertyCount = 1;
		return;
	}

	int arraySize = *pQueueFamilyPropertyCount;
	int elementsWritten = min(numQueueFamilies, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pQueueFamilyProperties[c] = _queueFamilyProperties[c];
	}

	*pQueueFamilyPropertyCount = elementsWritten;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetPhysicalDeviceSurfaceSupportKHR
 * does this queue family support presentation to this surface?
 */
VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(
		VkPhysicalDevice                            physicalDevice,
		uint32_t                                    queueFamilyIndex,
		VkSurfaceKHR                                surface,
		VkBool32*                                   pSupported)
{
	assert(pSupported);
	assert(surface);
	assert(physicalDevice);

	assert(queueFamilyIndex < numQueueFamilies);

	//TODO if we plan to support headless rendering, there should be 2 families
	//one using /dev/dri/card0 which has modesetting
	//other using /dev/dri/renderD128 which does not support modesetting, this would say false here
	*pSupported = VK_TRUE;
	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateDevice
 * vkCreateDevice verifies that extensions and features requested in the ppEnabledExtensionNames and pEnabledFeatures
 * members of pCreateInfo, respectively, are supported by the implementation. If any requested extension is not supported,
 * vkCreateDevice must return VK_ERROR_EXTENSION_NOT_PRESENT. If any requested feature is not supported, vkCreateDevice must return
 * VK_ERROR_FEATURE_NOT_PRESENT. Support for extensions can be checked before creating a device by querying vkEnumerateDeviceExtensionProperties
 * After verifying and enabling the extensions the VkDevice object is created and returned to the application.
 * If a requested extension is only supported by a layer, both the layer and the extension need to be specified at vkCreateInstance
 * time for the creation to succeed. Multiple logical devices can be created from the same physical device. Logical device creation may
 * fail due to lack of device-specific resources (in addition to the other errors). If that occurs, vkCreateDevice will return VK_ERROR_TOO_MANY_OBJECTS.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(
		VkPhysicalDevice                            physicalDevice,
		const VkDeviceCreateInfo*                   pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkDevice*                                   pDevice)
{
	assert(physicalDevice);
	assert(pDevice);
	assert(pCreateInfo);

	//check for enabled extensions
	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findDeviceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres == -1)
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//check for enabled features
	VkBool32* requestedFeatures = pCreateInfo->pEnabledFeatures;
	VkBool32* supportedFeatures = &_features;

	if(requestedFeatures)
	{
		for(int c = 0; c < numFeatures; ++c)
		{
			if(requestedFeatures[c] && !supportedFeatures[c])
			{
				return VK_ERROR_FEATURE_NOT_PRESENT;
			}
		}
	}

	*pDevice = ALLOCATE(sizeof(_device), 1, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE);
	if(!*pDevice)
	{
		return VK_ERROR_TOO_MANY_OBJECTS;
	}

	(*pDevice)->dev = physicalDevice;

	(*pDevice)->numEnabledExtensions = 0;

	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findDeviceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres > -1)
		{
			(*pDevice)->enabledExtensions[(*pDevice)->numEnabledExtensions] = findres;
			(*pDevice)->numEnabledExtensions++;
		}
	}

	if(requestedFeatures)
	{
		for(int c = 0; c < numFeatures; ++c)
		{
			if(requestedFeatures[c] && !supportedFeatures[c])
			{
				return VK_ERROR_FEATURE_NOT_PRESENT;
			}
		}

		(*pDevice)->enabledFeatures = *pCreateInfo->pEnabledFeatures;
	}
	else
	{
		memset(&(*pDevice)->enabledFeatures, 0, sizeof((*pDevice)->enabledFeatures)); //just disable everything
	}

	//layers ignored per spec
	//pCreateInfo->enabledLayerCount

	for(int c = 0; c < numQueueFamilies; ++c)
	{
		(*pDevice)->queues[c] = 0;
	}

	if(pCreateInfo->queueCreateInfoCount > 0)
	{
		for(int c = 0; c < pCreateInfo->queueCreateInfoCount; ++c)
		{
			(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex] = ALLOCATE(sizeof(_queue)*pCreateInfo->pQueueCreateInfos[c].queueCount, 1, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

			if(!(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex])
			{
				return VK_ERROR_OUT_OF_HOST_MEMORY;
			}

			for(int d = 0; d < pCreateInfo->pQueueCreateInfos[c].queueCount; ++d)
			{
				(*pDevice)->queues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex][d].lastEmitSeqno = 0;
			}

			(*pDevice)->numQueues[pCreateInfo->pQueueCreateInfos[c].queueFamilyIndex] = pCreateInfo->pQueueCreateInfos[c].queueCount;
		}
	}

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetDeviceQueue
 * vkGetDeviceQueue must only be used to get queues that were created with the flags parameter of VkDeviceQueueCreateInfo set to zero.
 * To get queues that were created with a non-zero flags parameter use vkGetDeviceQueue2.
 */
VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(
		VkDevice                                    device,
		uint32_t                                    queueFamilyIndex,
		uint32_t                                    queueIndex,
		VkQueue*                                    pQueue)
{
	assert(device);
	assert(pQueue);

	assert(queueFamilyIndex < numQueueFamilies);
	assert(queueIndex < device->numQueues[queueFamilyIndex]);

	*pQueue = &device->queues[queueFamilyIndex][queueIndex];
	(*pQueue)->dev = device;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyDevice
 * To ensure that no work is active on the device, vkDeviceWaitIdle can be used to gate the destruction of the device.
 * Prior to destroying a device, an application is responsible for destroying/freeing any Vulkan objects that were created using that device as the
 * first parameter of the corresponding vkCreate* or vkAllocate* command
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(
		VkDevice                                    device,
		const VkAllocationCallbacks*                pAllocator)
{
	assert(device);

	_device* dev = device;
	for(int c = 0; c < numQueueFamilies; ++c)
	{
		for(int d = 0; d < dev->numQueues[c]; ++d)
		{
			FREE(dev->queues[d]);
		}
	}

	FREE(dev);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumeratePhysicalDeviceGroups
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDeviceGroups(
	VkInstance                                  instance,
	uint32_t*                                   pPhysicalDeviceGroupCount,
	VkPhysicalDeviceGroupProperties*            pPhysicalDeviceGroupProperties)
{
	assert(instance);
	assert(pPhysicalDeviceGroupCount);

	if(!pPhysicalDeviceGroupProperties)
	{
		*pPhysicalDeviceGroupCount = 1;
		return VK_SUCCESS;
	}

	//TODO
	uint32_t c = 0;
	for(; c < *pPhysicalDeviceGroupCount; ++c)
	{
		pPhysicalDeviceGroupProperties[c].physicalDeviceCount = 1;
		pPhysicalDeviceGroupProperties[c].physicalDevices[0] = &instance->dev;
		pPhysicalDeviceGroupProperties[c].subsetAllocation = 0;
	}

	if(c < 1)
	{
		return VK_INCOMPLETE;
	}

	return VK_SUCCESS;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(
	VkDevice                                    device,
	const char*                                 pName)
{
	if(
	!strcmp("vkDestroyInstance", pName) ||
	!strcmp("vkEnumeratePhysicalDevices", pName) ||
	!strcmp("vkGetPhysicalDeviceFeatures", pName) ||
	!strcmp("vkGetPhysicalDeviceFormatProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceImageFormatProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceQueueFamilyProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceMemoryProperties", pName) ||
	!strcmp("vkCreateDevice", pName) ||
	!strcmp("vkEnumerateDeviceExtensionProperties", pName) ||
	!strcmp("vkEnumerateDeviceLayerProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceSparseImageFormatProperties", pName) ||
	!strcmp("vkEnumeratePhysicalDeviceGroups", pName) ||
	!strcmp("vkGetPhysicalDeviceFeatures2", pName) ||
	!strcmp("vkGetPhysicalDeviceProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceFormatProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceImageFormatProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceQueueFamilyProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceMemoryProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceSparseImageFormatProperties2", pName) ||
	!strcmp("vkGetPhysicalDeviceExternalBufferProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceExternalFenceProperties", pName) ||
	!strcmp("vkGetPhysicalDeviceExternalSemaphoreProperties", pName)
	)
	{
		return 0;
	}


	//TODO
	_device* d = device;
	return vkGetInstanceProcAddr(d->dev->instance, pName);
}
