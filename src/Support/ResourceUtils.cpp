/* ===================================================================== *
 * ResourceUtils.cpp (MeV/Application Framework)
 * ===================================================================== */

#include "ResourceUtils.h"

// Application Kit
#include <Application.h>
#include <Cursor.h>
#include <Message.h>
#include <Roster.h>
// Interface Kit
#include <Bitmap.h>
// Storage Kit
#include <Directory.h>
#include <Entry.h>
#include <File.h>
#include <Resources.h>
// Support Kit
#include <Debug.h>
#include <TypeConstants.h>
// Translation Kit
#include <TranslationUtils.h>

// Debugging Macros
#define D_OPERATION(x) //PRINT (x)

// ---------------------------------------------------------------------------
// Operations

void *
ResourceUtils::LoadResource(
	type_code type,
	int32 resourceID,
	size_t *size)
{
	D_OPERATION(("ResourceUtils::LoadResource(%ld)\n", resourceID));

	app_info appInfo;
	status_t error = be_app->GetAppInfo(&appInfo);
	if (error) {
		D_OPERATION((" -> couldn't get app_info (%s) !\n",
					 strerror(error)));
		return NULL;
	}

	// Get the parent directory of the application
	BEntry appEntry(&appInfo.ref);
	BDirectory appDir;
	appEntry.GetParent(&appDir);

	// Get the resource file of the application
	BFile appFile(&appEntry, B_READ_ONLY);
	BResources resources;
	error = resources.SetTo(&appFile);
	if (error) {
		D_OPERATION((" -> resource file not valid (%s) !\n",
					strerror(error)));
		return NULL;
	}
	
	return resources.FindResource(type, resourceID, size);
}

void *
ResourceUtils::LoadResource(
	type_code type,
	const char *resourceName,
	size_t *size)
{
	D_OPERATION(("ResourceUtils::LoadResource('%s')\n", resourceName));

	app_info appInfo;
	status_t error = be_app->GetAppInfo(&appInfo);
	if (error) {
		D_OPERATION((" -> couldn't get app_info (%s) !\n",
					 strerror(error)));
		return NULL;
	}

	// Get the parent directory of the application
	BEntry appEntry(&appInfo.ref);
	BDirectory appDir;
	appEntry.GetParent(&appDir);

	// Get the resource file of the application
	BFile appFile(&appEntry, B_READ_ONLY);
	BResources resources;
	error = resources.SetTo(&appFile);
	if (error) {
		D_OPERATION((" -> resource file not valid (%s) !\n",
					strerror(error)));
		return NULL;
	}
	
	return resources.FindResource(type, resourceName, size);
}

BBitmap *
ResourceUtils::LoadImage(
	int32 resourceID)
{
	D_OPERATION(("ResourceUtils::LoadImage(%ld)\n", resourceID));

	// first try to acquire the image via translators
	BBitmap *bitmap = BTranslationUtils::GetBitmap(B_RAW_TYPE, resourceID);
	if (bitmap == NULL)
	{
		// if that failed, assume the image is an archived BBitmap
		size_t size;
		void *data = LoadResource(B_RAW_TYPE, resourceID, &size);
		BMessage message;
		message.Unflatten(static_cast<char *>(data));
		bitmap = new BBitmap(&message);
	}
	return bitmap;
}

BBitmap *
ResourceUtils::LoadImage(
	const char *resourceName)
{
	D_OPERATION(("ResourceUtils::LoadImage('%s')\n", resourceName));

	// first try to acquire the image via translators
	BBitmap *bitmap = BTranslationUtils::GetBitmap(B_RAW_TYPE, resourceName);
	if (bitmap == NULL)
	{
		// if that failed, assume the image is an archived BBitmap
		size_t size;
		void *data = LoadResource(B_RAW_TYPE, resourceName, &size);
		BMessage message;
		message.Unflatten(static_cast<char *>(data));
		bitmap = new BBitmap(&message);
	}
	return bitmap;
}

uint8 *
ResourceUtils::LoadCursor(
	int32 resourceID)
{
	D_OPERATION(("ResourceUtils::LoadCursor(%ld)\n", resourceID));

	size_t size;
	return (uint8 *)LoadResource('CURS', resourceID, &size);
}

uint8 *
ResourceUtils::LoadCursor(
	const char *resourceName)
{
	D_OPERATION(("ResourceUtils::LoadCursor('%s')\n", resourceName));

	size_t size;
	return (uint8 *)LoadResource('CURS', resourceName, &size);
}

// END - ResourceUtils.cpp
