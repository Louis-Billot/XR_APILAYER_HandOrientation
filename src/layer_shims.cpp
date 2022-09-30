// SPDX-FileCopyrightText: 2021 Arthur Brainville (Ybalrid) <ybalrid@ybalrid.info>
//
// SPDX-License-Identifier: MIT
//
// Initial Author: Arthur Brainville <ybalrid@ybalrid.info>

#include "layer_shims.hpp"
#include <iostream>

#include <fstream>

XrPath rightHandPath = NULL;
XrSpace rightHandSpace = nullptr;
XrInstance this_instance = NULL;

// std::unordered_map<XrPath, const char*> testmap;
// std::unordered_map<XrSpace, XrPath> mapSpace;

std::fstream myfile, errors, spaces;

//Define the functions implemented in this layer like this:
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrEndFrame(XrSession session,
	const XrFrameEndInfo* frameEndInfo)
{
	//First time this runs, it will fetch the pointer from the loaded OpenXR dispatch table
	static PFN_xrEndFrame nextLayer_xrEndFrame = GetNextLayerFunction(xrEndFrame);

	//Do some additional things;
	// std::cout << "Display frame time is " << frameEndInfo->displayTime << "\n";

	//Call down the chain
	const auto result = nextLayer_xrEndFrame(session, frameEndInfo);

	//Maybe do something with the original return value?
	if(result == XR_ERROR_TIME_INVALID)
		std::cout << "frame time is invalid?\n";

	//Return what should be returned as if you were the actual function
	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrStringToPath(XrInstance instance, const char* pathString, XrPath* path)
{
	//First time this runs, it will fetch the pointer from the loaded OpenXR dispatch table
	static PFN_xrStringToPath nextLayer_xrStringToPath = GetNextLayerFunction(xrStringToPath);

	//Do some additional things;

	//Call down the chain
	const auto result = nextLayer_xrStringToPath(instance, pathString, path);

	//Maybe do something with the original return value?
	if( XR_SUCCEEDED(result) )
	{
		this_instance = instance;

		myfile.open("test.txt", std::fstream::app | std::fstream::out);
		myfile << pathString << "\n";

		if( strcmp(pathString, "/user/hand/right") == 0 )
		{
			std::cout << pathString << " " << *path << "\n";
			myfile << pathString << " " << *path << "\n";
			rightHandPath = *path;
		}

		// testmap[*path] = pathString;

		myfile.close();
	}

	//Return what should be returned as if you were the actual function
	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrCreateActionSpace(XrSession session, const XrActionSpaceCreateInfo* createInfo, XrSpace* space)
{
	//First time this runs, it will fetch the pointer from the loaded OpenXR dispatch table
	static PFN_xrCreateActionSpace nextLayer_xrCreateActionSpace = GetNextLayerFunction(xrCreateActionSpace);

	//Do some additional things;

	//Call down the chain
	const auto result = nextLayer_xrCreateActionSpace(session, createInfo, space);

	// mapSpace[*space] = createInfo->subactionPath;

	//Maybe do something with the original return value?
	if( XR_SUCCEEDED(result) && rightHandPath == createInfo->subactionPath )
	{
		std::cout << rightHandPath  << " " << createInfo->subactionPath << " " << *space << "\n";
		rightHandSpace = *space;
	}

	//Return what should be returned as if you were the actual function
	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrLocateSpace(XrSpace space, XrSpace baseSpace, XrTime time, XrSpaceLocation* location)
{
	//First time this runs, it will fetch the pointer from the loaded OpenXR dispatch table
	static PFN_xrLocateSpace nextLayer_xrLocateSpace = GetNextLayerFunction(xrLocateSpace);

	//Do some additional things;

	//Call down the chain
	const auto result = nextLayer_xrLocateSpace(space, baseSpace, time, location);

	// spaces.open("spaces.txt", std::fstream::app | std::fstream::out);
	// spaces << space << " " << testmap[mapSpace[space]] << "\n";
	// spaces.close();

	//Maybe do something with the original return value?
	if( XR_SUCCEEDED(result) && space == rightHandSpace )
	{
		// std::cout << /*rightHandPath <<*/ location->pose.position.x << "\n";
	}

	//Return what should be returned as if you were the actual function
	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrLocateViews(
    XrSession                                   session,
    const XrViewLocateInfo*                     viewLocateInfo,
    XrViewState*                                viewState,
    uint32_t                                    viewCapacityInput,
    uint32_t*                                   viewCountOutput,
    XrView*                                     views)
{
	//First time this runs, it will fetch the pointer from the loaded OpenXR dispatch table
	static PFN_xrLocateViews nextLayer_xrLocateViews = GetNextLayerFunction(xrLocateViews);
	static PFN_xrResultToString nextLayer_xrResultToString = GetNextLayerFunction(xrResultToString);

	//Call down the chain
	const auto result = nextLayer_xrLocateViews(session, viewLocateInfo, viewState, viewCapacityInput, viewCountOutput, views);

	//Maybe do something with the original return value?
	if( XR_SUCCEEDED(result) )
	{
		XrSpaceLocation location{XR_TYPE_SPACE_LOCATION};
		auto this_result = thisLayer_xrLocateSpace(rightHandSpace, viewLocateInfo->space, viewLocateInfo->displayTime, &location);
		if( XR_SUCCEEDED(this_result))
		{

			// views[0].pose.position = {0, 0, 0};
			// views[1].pose.position = {0, 0, 0};
			// std::cout << views[0].pose.orientation.x - views[1].pose.orientation.x << ' ' << views[0].pose.orientation.y - views[1].pose.orientation.y << ' ' << views[0].pose.orientation.z - views[1].pose.orientation.z << ' ' << views[0].pose.orientation.w - views[1].pose.orientation.w << ' ' << std::endl;

			for (uint32_t i = 0; i < *viewCountOutput; i ++)
			{
				// views[i].pose.orientation = {0, 0, 0, 1};
				views[i].pose.orientation = location.pose.orientation;
				// views[i].pose.orientation = poses[1].orientation;
				// views[i].pose.position = poses[1].position;
			}
			// std::cout << sqrt(pow((views[0].pose.position.x - views[1].pose.position.x), 2.0) + pow((views[0].pose.position.y - views[1].pose.position.y), 2.0) + pow((views[0].pose.position.z - views[1].pose.position.z), 2.0)) << '\n';
		}
		else
		{
			errors.open("errors.txt", std::fstream::app | std::fstream::out);
			char buffer[XR_MAX_RESULT_STRING_SIZE];
			nextLayer_xrResultToString(this_instance, this_result, buffer);
			errors << buffer << "\n";
			errors.close();
		}
	}

	//Return what should be returned as if you were the actual function
	return result;
}

XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrResultToString(
    XrInstance                                  instance,
    XrResult                                    value,
    char                                        buffer[XR_MAX_RESULT_STRING_SIZE])
{
	static PFN_xrResultToString nextLayer_xrResultToString = GetNextLayerFunction(xrResultToString);
	
	const auto result = nextLayer_xrResultToString(instance, value, buffer);

	return result;
}

#if XR_THISLAYER_HAS_EXTENSIONS
//The following function doesn't exist in the spec, this is just a test for the extension mecanism
XRAPI_ATTR XrResult XRAPI_CALL thisLayer_xrTestMeTEST(XrSession session)
{
	(void)session;
	std::cout << "xrTestMe()\n";
	return XR_SUCCESS;
}
#endif

//This functions returns the list of function pointers and name we implement, and is called during the initialization of the layer:
std::vector<OpenXRLayer::ShimFunction> ListShims()
{
	std::vector<OpenXRLayer::ShimFunction> functions;

	//List every functions that is callable on this API layer
	functions.emplace_back("xrEndFrame", PFN_xrVoidFunction(thisLayer_xrEndFrame));
	functions.emplace_back("xrStringToPath", PFN_xrVoidFunction(thisLayer_xrStringToPath));
	functions.emplace_back("xrCreateActionSpace", PFN_xrVoidFunction(thisLayer_xrCreateActionSpace));
	functions.emplace_back("xrLocateSpace", PFN_xrVoidFunction(thisLayer_xrLocateSpace));
	functions.emplace_back("xrLocateViews", PFN_xrVoidFunction(thisLayer_xrLocateViews));
	functions.emplace_back("xrResultToString", PFN_xrVoidFunction(thisLayer_xrResultToString));

#if XR_THISLAYER_HAS_EXTENSIONS
	if (OpenXRLayer::IsExtensionEnabled("XR_TEST_test_me"))
		functions.emplace_back("xrTestMeTEST", PFN_xrVoidFunction(thisLayer_xrTestMeTEST));
#endif

	return functions;
}
