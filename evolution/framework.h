#define _CRT_SECURE_NO_WARNINGS
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <memory>
#include <string_view>
#include <cstdint>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include <random>
#include <dwmapi.h>
#include <psapi.h>
#include <d3d9.h>
#include "d3dx9.h"
#include <filesystem>
#include <thread>
#include <codecvt>
#include <Shlwapi.h>
#include "skCrypter.h"
#include "Interception/interception.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Interception/interception_x64.lib")
#pragma comment(lib, "shlwapi.lib")
using namespace std;

#include "inc/ltbasedefs.h"
#include "inc/iltbaseclass.h"
#include "kdmapper/kdmapper.hpp"
#include "driver.hpp"
#include "rawdata.h"
#include "functions.h"


