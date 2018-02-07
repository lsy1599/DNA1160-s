#include "DLLVersion.h"
#include "stdafx.h"

/***************************************************************************

   Function:   GetDLLVersion

   Purpose:    Retrieves DLL major version, minor version and build numbers

   Input:      DLL file name
               Reference to Major number
               Reference to Minor number
               Reference to Build number

   Output:     TRUE only if successful

   Remarks:    This function first tries to get the DLL version the nice way,
               that is, call the DllGetVersion function in the DLL.

               If this fails, it tries to located the DLL file in the file system,
               read the file information block and retrieve the file version.

****************************************************************************/
BOOL CDLLVersion::GetDLLVersion (LPSTR szDLLFileName,
                                 DWORD &dwMajor, DWORD &dwMinor, DWORD &dwBuildNumber)
{
HINSTANCE   hDllInst;           // Instance of loaded DLL
char szFileName [_MAX_PATH];    // Temp file name
BOOL bRes = TRUE;               // Result

    lstrcpy (szFileName, szDLLFileName);    // Save a file name copy for the loading
    hDllInst = LoadLibrary(TEXT(szFileName));   //load the DLL
    if(hDllInst) {  // Could successfully load the DLL
        DLLGETVERSIONPROC pDllGetVersion;
        /*
        You must get this function explicitly because earlier versions of the DLL
        don't implement this function. That makes the lack of implementation of the
        function a version marker in itself.
        */
        pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hDllInst,
                          TEXT("DllGetVersion"));

        if(pDllGetVersion) {    // DLL supports version retrieval function
            DLLVERSIONINFO    dvi;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            HRESULT hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr)) { // Finally, the version is at our hands
                dwMajor = dvi.dwMajorVersion;
                dwMinor = dvi.dwMinorVersion;
                dwBuildNumber = dvi.dwBuildNumber;
            } else
                bRes = FALSE;   // Failure
        } else  // GetProcAddress failed, the DLL cannot tell its version
            bRes = FALSE;       // Failure

        FreeLibrary(hDllInst);  // Release DLL
    } else
        bRes = FALSE;   // DLL could not be loaded

    if (!bRes) // Cannot read DLL version the nice way 
        return CheckFileVersion (szFileName, SYS_DIR,
                                 dwMajor, dwMinor, dwBuildNumber); // Try the ugly way
    else
        return TRUE;
}

/***************************************************************************

   Function:   CheckFileVersion

   Purpose:    Check the version information of a given file

   Input:      File name
               File location (Windows dir, System dir, Current dir or none)
               Reference to Major number
               Reference to Minor number
               Reference to Build number

   Output:     TRUE only if successful

   Remarks:    Trashes original file name

****************************************************************************/
BOOL CDLLVersion::CheckFileVersion (LPSTR szFileName, FileLocationType FileLoc,
                                    DWORD &dwMajor, DWORD &dwMinor,
                                    DWORD &dwBuildNumber)
{
LPSTR   lpVersion;			            // String pointer to 'version' text
UINT    uVersionLen;
DWORD   dwVerHnd=0;			            // An 'ignored' parameter, always '0'

    FixFilePath (szFileName, FileLoc);  // Add necessary path prefix to file name

    DWORD dwVerInfoSize = GetFileVersionInfoSize (szFileName, &dwVerHnd);
    if (!dwVerInfoSize)     // Cannot reach the DLL file
        return FALSE;

    LPSTR lpstrVffInfo =
           (LPSTR) malloc (dwVerInfoSize);  // Alloc memory for file info
    if (lpstrVffInfo == NULL)
        return FALSE;   // Allocation failed

        // Try to get the info
    if (!GetFileVersionInfo(szFileName, dwVerHnd, dwVerInfoSize, lpstrVffInfo)) {
        free (lpstrVffInfo);
        return FALSE;   // Cannot read the file information - 
                        // wierd, since we could read the information size
    }
		/* The below 'hex' value looks a little confusing, but
		   essentially what it is, is the hexidecimal representation
		   of a couple different values that represent the language
		   and character set that we are wanting string values for.
		   040904E4 is a very common one, because it means:
			 US English, Windows MultiLingual characterset
		   Or to pull it all apart:
		   04------        = SUBLANG_ENGLISH_USA
		   --09----        = LANG_ENGLISH
		   ----04E4 = 1252 = Codepage for Windows:Multilingual
        */

    if (!VerQueryValue (    lpstrVffInfo,
                (LPSTR) (TEXT("\\StringFileInfo\\040904E4\\FileVersion")),
                (LPVOID *)&lpVersion, (UINT *)&uVersionLen)) {
        free (lpstrVffInfo);
        return FALSE;     // Query was unsuccessful
    }

    // Now we have a string that looks like this :
    // "MajorVersion.MinorVersion.BuildNumber", so let's parse it

    BOOL bRes = ParseVersionString (lpVersion, dwMajor, dwMinor, dwBuildNumber);
    free (lpstrVffInfo);
    return bRes;
}

/***************************************************************************

   Function:   ParseVersionString

   Purpose:    Parse version information string into 3 different numbers

   Input:      The version string formatted as "MajorVersion.MinorVersion.BuildNumber"
               Reference to Major number
               Reference to Minor number
               Reference to Build number

   Output:     TRUE only if successful

   Remarks:

****************************************************************************/
BOOL CDLLVersion::ParseVersionString (LPSTR lpVersion,
                                      DWORD &dwMajor, DWORD &dwMinor,
                                      DWORD &dwBuildNumber)
{
        // Get first token (Major version number)
    LPSTR token = strtok( lpVersion, TEXT (".") );
    if (token == NULL)  // End of string
        return FALSE;       // String ended prematurely
    dwMajor = atoi (token);

    token = strtok (NULL, TEXT ("."));  // Get second token (Minor version number)
    if (token == NULL)  // End of string
        return FALSE;       // String ended prematurely
    dwMinor = atoi (token);

    token = strtok (NULL, TEXT ("."));  // Get third token (Build number)
    if (token == NULL)  // End of string
        return FALSE;       // String ended prematurely
    dwBuildNumber = atoi (token);

    return TRUE;
}


/***************************************************************************

   Function:   FixFilePath

   Purpose:    Adds the correct path string to a file name according
               to given file location

   Input:      Original file name
               File location (Windows dir, System dir, Current dir or none)

   Output:     TRUE only if successful

   Remarks:    Trashes original file name

****************************************************************************/
BOOL CDLLVersion::FixFilePath (char * szFileName, FileLocationType FileLoc)
{
    char    szPathStr [_MAX_PATH];      // Holds path prefix

    switch (FileLoc) {
        case WIN_DIR:
                // Get the name of the windows directory
            if (GetWindowsDirectory (szPathStr, _MAX_PATH) ==  0)
                return FALSE;   // Cannot get windows directory
            break;

        case SYS_DIR:
                // Get the name of the windows SYSTEM directory
            if (GetSystemDirectory (szPathStr, _MAX_PATH) ==  0)
                return FALSE;   // Cannot get system directory
            break;

        case CUR_DIR:
                // Get the name of the current directory
            if (GetCurrentDirectory (_MAX_PATH, szPathStr) ==  0)
                return FALSE;   // Cannot get current directory
            break;

        case NO_DIR:
            lstrcpy (szPathStr,"");
            break;

        default:
            return FALSE;
    }
    lstrcat (szPathStr, _T("\\"));
    lstrcat (szPathStr, szFileName);
    lstrcpy (szFileName, szPathStr);
    return TRUE;
}
