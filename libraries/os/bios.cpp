#include "../../src/pch.hpp"

#include "bios.hpp"

// TODO: you may see various references to "security" functions, ignore these! :^)

static IWbemClassObject* object = nullptr;
static IWbemServices* services = nullptr;
static IEnumWbemClassObject* enumerator = nullptr;

static void query(std::wstring query, std::wstring query_language = _STRW(L"WQL")) {
	VM_DOLPHIN_WHITE_START

	DWORD result{};

	services->ExecQuery(query_language.data(), query.data(), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &enumerator);
	enumerator->Next(WBEM_INFINITE, 1, &object, &result);

	VM_DOLPHIN_WHITE_END
}

void sys::bios::init() {
	VM_DOLPHIN_WHITE_START

	IWbemLocator* locator = nullptr;
	std::wstring network = _STRW(L"root\\CIMV2");

	DWORD ret;
	HRESULT res;

	auto CoInitEx = reinterpret_cast<HRESULT(*)(LPVOID pvReserved, DWORD dwCoInit)>(security::get_secure_function(_STR("Ole32.dll"), _STR("CoInitializeEx")));
	auto CoInitSecurity = reinterpret_cast<HRESULT(*)(PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE * asAuthSvc, void* pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void* pAuthList, DWORD dwCapabilities, void* pReserved3)>(security::get_secure_function(_STR("Ole32.dll"), _STR("CoInitializeSecurity")));
	auto CoCreateInstance = reinterpret_cast<HRESULT(*)(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID riid, LPVOID * ppv)>(security::get_secure_function(_STR("Ole32.dll"), _STR("CoCreateInstance")));

	if (!CoInitEx || !CoInitSecurity || !CoCreateInstance) {
		MessageBoxA(NULL, _STR("Initialization Error (2)"), _STR("SirMeme Hub"), MB_ICONERROR);

		while (true) {
			exit(1);
			ExitProcess(1);
			quick_exit(1);
		}
	}

	res = CoInitEx(0, COINIT_MULTITHREADED);
	res = CoInitSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
	res = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (void**)&locator);

	locator->ConnectServer(network.data(), NULL, NULL, NULL, NULL, NULL, NULL, &services);

	VM_DOLPHIN_WHITE_END
}

std::string sys::bios::get_serial_id() {
	VM_FISH_WHITE_START

	VARIANT Val{};

	query(_STRW(L"SELECT * from Win32_BIOS"));
	object->Get(_STRW(L"SerialNumber"), 0, &Val, 0, 0);

	std::wstring wide_serial_id(V_BSTR(&Val));
	std::string serial_id{ wide_serial_id.begin(), wide_serial_id.end() };
	std::string result;

	CryptoPP::StringSource(serial_id, true,
		new CryptoPP::HexEncoder(
			new CryptoPP::StringSink(result)
		)
	);

	VM_FISH_WHITE_END

	return result;
}

std::string sys::bios::get_uuid() {
	VM_FISH_WHITE_START
	
	VARIANT Val{};

	query(_STRW(L"SELECT * from Win32_ComputerSystemProduct"));
	object->Get(_STRW(L"UUID"), 0, &Val, 0, 0);

	std::wstring wide_serial_id(V_BSTR(&Val));

	VM_FISH_WHITE_END

	return { wide_serial_id.begin(), wide_serial_id.end() };
}

std::string sys::bios::get_disk_id() {
	VM_FISH_WHITE_START

	VARIANT Val{};

	query(_STRW(L"SELECT * from Win32_DiskDrive"));
	object->Get(_STRW(L"SerialNumber"), 0, &Val, 0, 0);

	std::wstring wide_serial_id(V_BSTR(&Val));

	VM_FISH_WHITE_END

	return { wide_serial_id.begin(), wide_serial_id.end() };
}
