#include "common.h"

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateInstanceExtensionProperties
 * When pLayerName parameter is NULL, only extensions provided by the Vulkan implementation or by implicitly enabled layers are returned. When pLayerName is the name of a layer,
 * the instance extensions provided by that layer are returned.
 * If pProperties is NULL, then the number of extensions properties available is returned in pPropertyCount. Otherwise, pPropertyCount must point to a variable set by the user
 * to the number of elements in the pProperties array, and on return the variable is overwritten with the number of structures actually written to pProperties.
 * If pPropertyCount is less than the number of extension properties available, at most pPropertyCount structures will be written. If pPropertyCount is smaller than the number of extensions available,
 * VK_INCOMPLETE will be returned instead of VK_SUCCESS, to indicate that not all the available properties were returned.
 * Because the list of available layers may change externally between calls to vkEnumerateInstanceExtensionProperties,
 * two calls may retrieve different results if a pLayerName is available in one call but not in another. The extensions supported by a layer may also change between two calls,
 * e.g. if the layer implementation is replaced by a different version between those calls.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(
		const char*                                 pLayerName,
		uint32_t*                                   pPropertyCount,
		VkExtensionProperties*                      pProperties)
{
	assert(!pLayerName); //TODO layers ignored for now
	assert(pPropertyCount);

	if(!pProperties)
	{
		*pPropertyCount = numInstanceExtensions;
		return VK_SUCCESS;
	}

	int arraySize = *pPropertyCount;
	int elementsWritten = min(numInstanceExtensions, arraySize);

	for(int c = 0; c < elementsWritten; ++c)
	{
		pProperties[c] = instanceExtensions[c];
	}

	*pPropertyCount = elementsWritten;

	if(arraySize < numInstanceExtensions)
	{
		return VK_INCOMPLETE;
	}
	else
	{
		return VK_SUCCESS;
	}
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateInstance
 * There is no global state in Vulkan and all per-application state is stored in a VkInstance object. Creating a VkInstance object initializes the Vulkan library
 * vkCreateInstance verifies that the requested layers exist. If not, vkCreateInstance will return VK_ERROR_LAYER_NOT_PRESENT. Next vkCreateInstance verifies that
 * the requested extensions are supported (e.g. in the implementation or in any enabled instance layer) and if any requested extension is not supported,
 * vkCreateInstance must return VK_ERROR_EXTENSION_NOT_PRESENT. After verifying and enabling the instance layers and extensions the VkInstance object is
 * created and returned to the application.
 */
VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
		const VkInstanceCreateInfo*                 pCreateInfo,
		const VkAllocationCallbacks*                pAllocator,
		VkInstance*                                 pInstance)
{
	assert(pInstance);
	assert(pCreateInfo);

	*pInstance = ALLOCATE(sizeof(_instance), 1, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE);

	if(!*pInstance)
	{
		return VK_ERROR_OUT_OF_HOST_MEMORY;
	}

	(*pInstance)->numEnabledExtensions = 0;

	//TODO: possibly we need to load layers here
	//and store them in pInstance
	assert(pCreateInfo->enabledLayerCount == 0);

	if(pCreateInfo->enabledExtensionCount)
	{
		assert(pCreateInfo->ppEnabledExtensionNames);
	}

	for(int c = 0; c < pCreateInfo->enabledExtensionCount; ++c)
	{
		int findres = findInstanceExtension(pCreateInfo->ppEnabledExtensionNames[c]);
		if(findres > -1)
		{
			(*pInstance)->enabledExtensions[(*pInstance)->numEnabledExtensions] = findres;
			(*pInstance)->numEnabledExtensions++;
		}
		else
		{
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//TODO ignored for now
	//pCreateInfo->pApplicationInfo

	//TODO is there a way to check if there's a gpu (and it's the rPi)?
	int gpuExists = access( "/dev/dri/card0", F_OK ) != -1; assert(gpuExists);

	(*pInstance)->dev.path = "/dev/dri/card0";
	(*pInstance)->dev.instance = *pInstance;

	int ret = openIoctl(); assert(ret != -1);

	(*pInstance)->chipVersion = vc4_get_chip_info(controlFd);
	(*pInstance)->hasTiling = vc4_test_tiling(controlFd);

	(*pInstance)->hasControlFlow = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_BRANCHES);
	(*pInstance)->hasEtc1 = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_ETC1);
	(*pInstance)->hasThreadedFs = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_THREADED_FS);
	(*pInstance)->hasMadvise = vc4_has_feature(controlFd, DRM_VC4_PARAM_SUPPORTS_MADVISE);

	return VK_SUCCESS;
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkDestroyInstance
 *
 */
VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(
		VkInstance                                  instance,
		const VkAllocationCallbacks*                pAllocator)
{
	assert(instance);

	closeIoctl();

	FREE(instance);
}

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkEnumerateInstanceVersion
 */
VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceVersion(
	uint32_t*                                   pApiVersion)
{
	assert(pApiVersion);
	*pApiVersion = VK_DRIVER_VERSION; //
	return VK_SUCCESS;
}

#define RETFUNC(f) if(!strcmp(pName, #f)) return f

/*
 * https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetInstanceProcAddr
 */
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(
	VkInstance                                  instance,
	const char*                                 pName)
{
	//TODO take instance into consideration
	//eg only return extension functions that are enabled?

	if(!instance && !(
		   !strcmp(pName, "vkEnumerateInstanceVersion") ||
		   !strcmp(pName, "vkEnumerateInstanceExtensionProperties") ||
		   !strcmp(pName, "vkEnumerateInstanceLayerProperties") ||
		   !strcmp(pName, "vkCreateInstance")
		   ))
	{
		return 0;
	}

	RETFUNC(vkCreateInstance);
	RETFUNC(vkEnumerateInstanceVersion);
	RETFUNC(vkDestroyInstance);
	RETFUNC(vkEnumeratePhysicalDevices);
	RETFUNC(vkGetPhysicalDeviceFeatures);
	RETFUNC(vkGetPhysicalDeviceFormatProperties);
	RETFUNC(vkGetPhysicalDeviceImageFormatProperties);
	RETFUNC(vkGetPhysicalDeviceProperties);
	RETFUNC(vkGetPhysicalDeviceQueueFamilyProperties);
	RETFUNC(vkGetPhysicalDeviceMemoryProperties);
	RETFUNC(vkGetInstanceProcAddr);
	RETFUNC(vkGetDeviceProcAddr);
	RETFUNC(vkCreateDevice);
	RETFUNC(vkDestroyDevice);
	RETFUNC(vkEnumerateInstanceExtensionProperties);
	RETFUNC(vkEnumerateDeviceExtensionProperties);
	RETFUNC(vkEnumerateInstanceLayerProperties);
	RETFUNC(vkEnumerateDeviceLayerProperties);
	RETFUNC(vkGetDeviceQueue);
	RETFUNC(vkQueueSubmit);
	RETFUNC(vkQueueWaitIdle);
	RETFUNC(vkDeviceWaitIdle);
	RETFUNC(vkAllocateMemory);
	RETFUNC(vkFreeMemory);
	RETFUNC(vkMapMemory);
	RETFUNC(vkUnmapMemory);
	RETFUNC(vkFlushMappedMemoryRanges);
	RETFUNC(vkInvalidateMappedMemoryRanges);
	RETFUNC(vkGetDeviceMemoryCommitment);
	RETFUNC(vkBindBufferMemory);
	RETFUNC(vkBindImageMemory);
	RETFUNC(vkGetBufferMemoryRequirements);
	RETFUNC(vkGetImageMemoryRequirements);
	RETFUNC(vkGetImageSparseMemoryRequirements);
	RETFUNC(vkGetPhysicalDeviceSparseImageFormatProperties);
	RETFUNC(vkQueueBindSparse);
	RETFUNC(vkCreateFence);
	RETFUNC(vkDestroyFence);
	RETFUNC(vkResetFences);
	RETFUNC(vkGetFenceStatus);
	RETFUNC(vkWaitForFences);
	RETFUNC(vkCreateSemaphore);
	RETFUNC(vkDestroySemaphore);
	RETFUNC(vkCreateEvent);
	RETFUNC(vkDestroyEvent);
	RETFUNC(vkGetEventStatus);
	RETFUNC(vkSetEvent);
	RETFUNC(vkResetEvent);
	RETFUNC(vkCreateQueryPool);
	RETFUNC(vkDestroyQueryPool);
	RETFUNC(vkGetQueryPoolResults);
	RETFUNC(vkCreateBuffer);
	RETFUNC(vkDestroyBuffer);
	RETFUNC(vkCreateBufferView);
	RETFUNC(vkDestroyBufferView);
	RETFUNC(vkCreateImage);
	RETFUNC(vkDestroyImage);
	RETFUNC(vkGetImageSubresourceLayout);
	RETFUNC(vkCreateImageView);
	RETFUNC(vkDestroyImageView);
	RETFUNC(vkCreateShaderModule);
	RETFUNC(vkDestroyShaderModule);
	RETFUNC(vkCreatePipelineCache);
	RETFUNC(vkDestroyPipelineCache);
	RETFUNC(vkGetPipelineCacheData);
	RETFUNC(vkMergePipelineCaches);
	RETFUNC(vkCreateGraphicsPipelines);
	RETFUNC(vkCreateComputePipelines);
	RETFUNC(vkDestroyPipeline);
	RETFUNC(vkCreatePipelineLayout);
	RETFUNC(vkDestroyPipelineLayout);
	RETFUNC(vkCreateSampler);
	RETFUNC(vkDestroySampler);
	RETFUNC(vkCreateDescriptorSetLayout);
	RETFUNC(vkDestroyDescriptorSetLayout);
	RETFUNC(vkCreateDescriptorPool);
	RETFUNC(vkDestroyDescriptorPool);
	RETFUNC(vkResetDescriptorPool);
	RETFUNC(vkAllocateDescriptorSets);
	RETFUNC(vkFreeDescriptorSets);
	RETFUNC(vkUpdateDescriptorSets);
	RETFUNC(vkCreateFramebuffer);
	RETFUNC(vkDestroyFramebuffer);
	RETFUNC(vkCreateRenderPass);
	RETFUNC(vkDestroyRenderPass);
	RETFUNC(vkGetRenderAreaGranularity);
	RETFUNC(vkCreateCommandPool);
	RETFUNC(vkDestroyCommandPool);
	RETFUNC(vkResetCommandPool);
	RETFUNC(vkAllocateCommandBuffers);
	RETFUNC(vkFreeCommandBuffers);
	RETFUNC(vkBeginCommandBuffer);
	RETFUNC(vkEndCommandBuffer);
	RETFUNC(vkResetCommandBuffer);
	RETFUNC(vkCmdBindPipeline);
	RETFUNC(vkCmdSetViewport);
	RETFUNC(vkCmdSetScissor);
	RETFUNC(vkCmdSetLineWidth);
	RETFUNC(vkCmdSetDepthBias);
	RETFUNC(vkCmdSetBlendConstants);
	RETFUNC(vkCmdSetDepthBounds);
	RETFUNC(vkCmdSetStencilCompareMask);
	RETFUNC(vkCmdSetStencilWriteMask);
	RETFUNC(vkCmdSetStencilReference);
	RETFUNC(vkCmdBindDescriptorSets);
	RETFUNC(vkCmdBindIndexBuffer);
	RETFUNC(vkCmdBindVertexBuffers);
	RETFUNC(vkCmdDraw);
	RETFUNC(vkCmdDrawIndexed);
	RETFUNC(vkCmdDrawIndirect);
	RETFUNC(vkCmdDrawIndexedIndirect);
	RETFUNC(vkCmdDispatch);
	RETFUNC(vkCmdDispatchIndirect);
	RETFUNC(vkCmdCopyBuffer);
	RETFUNC(vkCmdCopyImage);
	RETFUNC(vkCmdBlitImage);
	RETFUNC(vkCmdCopyBufferToImage);
	RETFUNC(vkCmdCopyImageToBuffer);
	RETFUNC(vkCmdUpdateBuffer);
	RETFUNC(vkCmdFillBuffer);
	RETFUNC(vkCmdClearColorImage);
	RETFUNC(vkCmdClearDepthStencilImage);
	RETFUNC(vkCmdClearAttachments);
	RETFUNC(vkCmdResolveImage);
	RETFUNC(vkCmdSetEvent);
	RETFUNC(vkCmdResetEvent);
	RETFUNC(vkCmdWaitEvents);
	RETFUNC(vkCmdPipelineBarrier);
	RETFUNC(vkCmdBeginQuery);
	RETFUNC(vkCmdEndQuery);
	RETFUNC(vkCmdResetQueryPool);
	RETFUNC(vkCmdWriteTimestamp);
	RETFUNC(vkCmdCopyQueryPoolResults);
	RETFUNC(vkCmdPushConstants);
	RETFUNC(vkCmdBeginRenderPass);
	RETFUNC(vkCmdNextSubpass);
	RETFUNC(vkCmdEndRenderPass);
	RETFUNC(vkCmdExecuteCommands);
	RETFUNC(vkEnumeratePhysicalDeviceGroups);
	RETFUNC(vkGetPhysicalDeviceFeatures2);
	RETFUNC(vkGetPhysicalDeviceProperties2);
	RETFUNC(vkGetPhysicalDeviceFormatProperties2);
	RETFUNC(vkGetPhysicalDeviceImageFormatProperties2);
	RETFUNC(vkGetPhysicalDeviceQueueFamilyProperties2);
	RETFUNC(vkGetPhysicalDeviceMemoryProperties2);
	RETFUNC(vkGetPhysicalDeviceSparseImageFormatProperties2);
	RETFUNC(vkGetPhysicalDeviceExternalBufferProperties);
	RETFUNC(vkGetPhysicalDeviceExternalFenceProperties);
	RETFUNC(vkGetPhysicalDeviceExternalSemaphoreProperties);
	RETFUNC(vkBindImageMemory2);
	RETFUNC(vkGetDeviceGroupPeerMemoryFeatures);
	RETFUNC(vkCmdSetDeviceMask);
	RETFUNC(vkCmdDispatchBase);
	RETFUNC(vkGetImageMemoryRequirements2);
	RETFUNC(vkGetBufferMemoryRequirements2);
	RETFUNC(vkGetImageSparseMemoryRequirements2);
	RETFUNC(vkTrimCommandPool);
	RETFUNC(vkGetDeviceQueue2);
	RETFUNC(vkCreateSamplerYcbcrConversion);
	RETFUNC(vkDestroySamplerYcbcrConversion);
	RETFUNC(vkCreateDescriptorUpdateTemplate);
	RETFUNC(vkDestroyDescriptorUpdateTemplate);
	RETFUNC(vkUpdateDescriptorSetWithTemplate);
	RETFUNC(vkGetDescriptorSetLayoutSupport);
	RETFUNC(vkBindBufferMemory2);


	return 0;
}
