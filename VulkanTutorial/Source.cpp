#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>

#include <algorithm>
// std::optional
#include <optional>
#include <set>

struct PhysicalDeviceSupportIndices {
	std::optional<uint32_t> queueFamily;
	std::optional<uint32_t> surfaceSupport;

	bool isComplete() {
		return queueFamily.has_value() && surfaceSupport.has_value();
	}
};

struct SurfaceAndPresentProperties {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	std::vector<VkSurfaceFormatKHR> surfaceFormats;
	std::vector<VkPresentModeKHR> presentModes;
};


class App {
private:
	GLFWwindow* window;
	VkInstance instance;
	VkPhysicalDevice physicalDevice;
	VkDevice logicalDevice;
	VkQueue deviceQueue;
	VkQueue surfaceQueue;
	VkSurfaceKHR windowSurface;

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	const std::vector<const char*> requiredDeviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
public:
	App() : window(nullptr), physicalDevice(VK_NULL_HANDLE) {}

private:
	std::optional<std::vector<const char*>> getRequiredExtensions(const std::vector<VkExtensionProperties> &availableExtensions, const std::vector<const char*> &requiredExtensionNames, std::string type="*") {
		std::cout << "Available " << type << " Extensions:" << std::endl;
		for (auto& extension : availableExtensions) {
			std::cout << extension.extensionName << std::endl;
		}
		std::cout << std::endl;

		std::cout << "Required " << type << " Extensions:" << std::endl;
		for (auto& extensionName : requiredExtensionNames) {
			std::cout << extensionName << std::endl;
		}
		std::cout << std::endl;

		std::vector<const char*> extensionsFound;
		std::vector<const char*> extensionsMissing;

		for (const auto& requiredExtensionName : requiredExtensionNames) {
			uint32_t i = 0;
			while (i < availableExtensions.size()) {
				if (std::strcmp(requiredExtensionName, availableExtensions[i].extensionName) == 0) {
					extensionsFound.push_back(requiredExtensionName);
					break;
				}
				++i;
			}
			if (i == availableExtensions.size()) {
				extensionsMissing.push_back(requiredExtensionName);
			}
		}

		std::cout << "Found " << type << " Extensions:" << std::endl;
		for (const auto& extension : extensionsFound) {
			std::cout << extension << std::endl;
		}
		std::cout << std::endl;

		std::cout << "Missing " << type << " Extensions:" << std::endl;
		for (const auto& extension : extensionsMissing) {
			std::cout << extension << std::endl;
		}
		std::cout << std::endl;

		if (extensionsFound.size() == requiredExtensionNames.size()) {
			return { extensionsFound };
		}
		return {};
	}

	std::optional<std::vector<const char*>> getRequiredInstanceExtensions() {
		/*
		 * Get complete instance extension list and place all available instances into a vector.
		 */
		uint32_t availableExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());

		/*
		* Place all required extensions names from glfw into a vector.
		*/

		uint32_t requiredExtensionNameCount = 0;
		const char** requiredExtensionNames_pp;
		requiredExtensionNames_pp = glfwGetRequiredInstanceExtensions(&requiredExtensionNameCount);

		std::vector<const char *> requiredExtensionNames(requiredExtensionNames_pp, requiredExtensionNames_pp+ requiredExtensionNameCount);

		return getRequiredExtensions(availableExtensions, requiredExtensionNames, "Instance");
	}

	std::optional<std::vector<const char *>> getRequiredDeviceExtensions(VkPhysicalDevice physicalDevice) {
		uint32_t availableExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, nullptr);
		std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &availableExtensionCount, availableExtensions.data());

		return getRequiredExtensions(availableExtensions, requiredDeviceExtensions, "Device");
	}

	void initInstance() {
		/*
		 * VkApplicationInfo Setup
		 */
		VkApplicationInfo applicationInfo{};
		applicationInfo.apiVersion = VK_API_VERSION_1_0;

		applicationInfo.pApplicationName = "Vulkan Application";
		applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);

		applicationInfo.pEngineName = "No Engine";
		applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

		//applicationInfo.pNext = ;

		applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

		/*
		 * vkInstanceCreateInfo Setup
		 */
		VkInstanceCreateInfo instanceCreateInfo{};

		/*
		 * If the required extensions are unsupported then exit.
		 */
		std::optional<std::vector<const char *>> optional = getRequiredInstanceExtensions();
		if (!optional.has_value()) {
			throw std::runtime_error("Required instance extensions are unsupported!");
		}


		/**
		 * Use the required extensions.
		 */
		instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(optional.value().size());
		instanceCreateInfo.ppEnabledExtensionNames = optional.value().data();


		//instanceCreateInfo.enabledLayerCount;
		//instanceCreateInfo.ppEnabledLayerNames;

		//instanceCreateInfo.flags;
		instanceCreateInfo.pApplicationInfo = &applicationInfo;

		//instanceCreateInfo.pNext;

		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

		if (vkCreateInstance(&instanceCreateInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create VkInstance.");
		}
	}



	void finiInstance() {
		vkDestroyInstance(instance, nullptr);
	}

	/*
	bool isPhysicalDeviceExtensionsSuitable(VkPhysicalDevice physicalDevice) {
		uint32_t extensionPropertiesCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertiesCount, nullptr);
		std::vector<VkExtensionProperties> extensionProperties(extensionPropertiesCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertiesCount, extensionProperties.data());

		std::set<std::string> requiredExtensionsSet(requiredDeviceExtensions.begin(), requiredDeviceExtensions.end());
		for (const auto& extension : extensionProperties) {
			requiredExtensionsSet.erase(extension.extensionName);
		}

		*
		 * If all requiredExtensions are found then this set should be empty.
		 *
		return requiredExtensionsSet.empty();
	}*/

	
	bool isPhysicalDeviceSuitable(VkPhysicalDevice physicalDevice) {
		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		VkPhysicalDeviceFeatures physicalDeviceFeatures;
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
		/*
		 * https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VkPhysicalDeviceType.html#_description
		 */
		auto index = getSupportedGraphicsQueueIndex(physicalDevice);
		bool physicalDeviceSuitable = !(getRequiredDeviceExtensions(physicalDevice).has_value());
		//bool physicalDeviceSuitable = isPhysicalDeviceExtensionsSuitable(physicalDevice);
		if (physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			physicalDeviceFeatures.geometryShader &&
			index.isComplete() && physicalDeviceSuitable) {
			return true;
		}
		return false;
	}


	void selectPhysicalDevices() {
		/*
		 * Get all physical devices present on system.
		 */
		uint32_t physicalDeviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
		if (physicalDeviceCount == 0) {
			throw std::runtime_error("No devices with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
		vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

		/*
		 * Iterate through devices and select the most suitable device(s) for use.
		 */
		for (const auto& device : physicalDevices) {
			if (isPhysicalDeviceSuitable(device)) {
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE) {
			throw std::runtime_error("unable to find suitable device");
		}
	}

	PhysicalDeviceSupportIndices getSupportedGraphicsQueueIndex(VkPhysicalDevice physicalDevice) {
		uint32_t queueFamilyPropertyCount = 0;
		PhysicalDeviceSupportIndices physicalDeviceSupportIndices;

		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

		uint32_t index = 0;
		for (const auto& queueFamilyProperty : queueFamilyProperties) {
			if (queueFamilyProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT && queueFamilyProperty.queueFlags & VK_QUEUE_COMPUTE_BIT) {				
				physicalDeviceSupportIndices.queueFamily = index;
			}
			VkBool32 surfaceSupported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, index, windowSurface, &surfaceSupported);
			if (surfaceSupported) {
				physicalDeviceSupportIndices.surfaceSupport = index;
			}
			++index;
		}

		return physicalDeviceSupportIndices;
	}

	void createLogicalDevice() {
		auto physicalDeviceSupportIndices = getSupportedGraphicsQueueIndex(physicalDevice);


		std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
		// Use a set.  If the values are identical, the set will only contain a single value.  (We only want to submit one CreateInfo in this case).
		std::set<uint32_t> physicalDeviceSupportIndicesSet = { physicalDeviceSupportIndices.queueFamily.value() , physicalDeviceSupportIndices.surfaceSupport.value() };

		for (auto& physicalDeviceSupportIndex : physicalDeviceSupportIndicesSet) {
			VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
			deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			deviceQueueCreateInfo.queueFamilyIndex = physicalDeviceSupportIndex;
			float queuePriorities[1] = { 1.0f };
			deviceQueueCreateInfo.pQueuePriorities = queuePriorities;
			deviceQueueCreateInfo.queueCount = 1;
			deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = deviceQueueCreateInfos.data();
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size());
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;
		
		/*
		 * If the required extensions are unsupported then exit.
		 */
		std::optional<std::vector<const char*>> optional = getRequiredDeviceExtensions(physicalDevice);
		if (!optional.has_value()) {
			throw std::runtime_error("Required device extensions are unsupported!");
		}

		/**
		 * Use the required extensions.
		 */
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(optional.value().size());
		deviceCreateInfo.ppEnabledExtensionNames = optional.value().data();


		deviceCreateInfo.enabledLayerCount = 0;

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		vkGetDeviceQueue(logicalDevice, physicalDeviceSupportIndices.queueFamily.value(), 0, &deviceQueue);
		vkGetDeviceQueue(logicalDevice, physicalDeviceSupportIndices.surfaceSupport.value(), 0, &surfaceQueue);
	}

	void init() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

		initInstance();
		createWindowSurface();
		selectPhysicalDevices();
		createLogicalDevice();
	}

	void createWindowSurface() {
		if (glfwCreateWindowSurface(instance, window, nullptr, &windowSurface) != VK_SUCCESS) {
			throw std::runtime_error("Unable to create window surface!");
		}
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void fini() {
		vkDestroyDevice(logicalDevice, nullptr);
		vkDestroySurfaceKHR(instance, windowSurface, nullptr);
		finiInstance();

		glfwDestroyWindow(window);
		window = nullptr;
		glfwTerminate();
	}

public:
	void run() {
		init();

		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		std::cout << extensionCount << " extensions supported\n";

		glm::mat4 matrix;
		glm::vec4 vec;
		auto test = matrix * vec;

		mainLoop();
		fini();
	}
};

int main() {
	App app;

	try {
		app.run();
	}
	catch (const std::exception& exception) {
		std::cerr << exception.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}