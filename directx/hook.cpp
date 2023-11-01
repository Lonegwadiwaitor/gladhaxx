#include "../src/pch.hpp"

#include "hook.hpp"

#include "../libraries/kiero/kiero.h"

#include "../libraries/imgui/imgui.h"
#include "../libraries/imgui/imgui_internal.h"
#include "../libraries/imgui/imgui_impl_uwp.h"
#include "../libraries/imgui/imgui_impl_dx11.h"

#include "../src/src.hpp"
#include "../rbx/bypass/console.hpp"
#include "../libraries/os/file.hpp"
#include "../libraries/os/output.hpp"

#include "imgui/window.hpp"
#include "draw/element.hpp"

#include <cstdint>
#include "../ehook.hpp"

#include <Luau/Parser.h>

#include "ui/script_editor.hpp"

#include "../rbx/sync/rapi.hpp"
#include "../rbx/scheduler/task_scheduler.hpp"
#include <Luau/Compiler.h>

#include "lua.h"

using Present = HRESULT(__stdcall*) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
using PresentBuffer = HRESULT(__stdcall*) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags, UINT params);
using ResizeBuffers = HRESULT(__stdcall*) (IDXGISwapChain* pSwapChain, UINT uCount, UINT uWidth, UINT uHeight, DXGI_FORMAT dxNewFormat, UINT uFlags);
using WNDPROC = LRESULT(CALLBACK*)(HWND, UINT, WPARAM, LPARAM);
using PTR = uintptr_t;

static HWND hWindow;
static ID3D11Device* pDevice = nullptr;
static ID3D11DeviceContext* pContext = nullptr;
static ID3D11RenderTargetView* pMainRenderTargetView = nullptr;

static Present oPresent = nullptr;
static Present hPresent = nullptr;
static ResizeBuffers oResizeBuffers = nullptr;

static bool streamer_mode = false;
static std::once_flag initialized_present;

bool directx::IsGuiOpen;

HWND directx::get_hwnd() {
	return hWindow;
}

bool directx::get_streamer_mode() {
	return streamer_mode;
}

void directx::set_streamer_mode(bool mode) {
	streamer_mode = mode;
}

uintptr_t present_ptr;
uint8_t hook_present_bytes[] = { 0x5D, 0xC2, 0x0C, 0x00 };
uint8_t *old_hook_present_bytes;
ehook::ehook *hook_present, *hook_hook;
TextEditor editor;
std::string editorText;
static HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT uSyncInterval, UINT uFlags)
{
	std::call_once(initialized_present, [=]() {
		sys::cout(_STR("call_once\n"));

		DXGI_SWAP_CHAIN_DESC desc;
		ID3D11Texture2D* pBackBuffer = nullptr;

		// set up directx bullshit
		pSwapChain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&pDevice));
		pDevice->GetImmediateContext(&pContext);
		pSwapChain->GetDesc(&desc);
		pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBackBuffer));
		MH_Initialize();

		if (pBackBuffer != nullptr) {
			pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pMainRenderTargetView);
			pBackBuffer->Release();

			auto RobloxWindow = FindWindowA(NULL, "Roblox");
			
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			RECT rect;
			if (GetWindowRect(RobloxWindow, &rect))
			{
				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;
				io.DisplaySize = ImVec2(width, height);
			}

			
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			ImGui_ImplUwp_InitForCurrentView();
			ImGui_ImplDX11_Init(pDevice, pContext);
			
			draw::background_draw_list = ImGui::GetBackgroundDrawList();
			hWindow = RobloxWindow;

			//ig::initialize_font();

			std::thread(main).detach();
		}
	});

	if (streamer_mode && hook_hook != nullptr) {
		hook_hook->undo();
		ehook::memory::write_memory_raw((void*)(present_ptr + 5), hook_present_bytes, 4);
		hPresent(pSwapChain, uSyncInterval, uFlags);
		ehook::memory::write_memory_raw((void*)(present_ptr + 5), old_hook_present_bytes, 4);
		hook_hook->apply();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplUwp_NewFrame();

	ImGui::NewFrame();

	if (GetForegroundWindow() == hWindow) {
		for (ig::window& window : ig::windows) {
			window.render();
		}

		for (draw::element* element : draw::elements) {
			element->render();
		}

		if (directx::IsGuiOpen) {
			// TODO: redo this mess LOL
			// TODO: also build a proper UI


			if (editor.GetText() != editorText)
			{
				Luau::Allocator allocator;
				Luau::AstNameTable names(allocator);
				Luau::ParseResult result = Luau::Parser::parse(editor.GetText().c_str(), editor.GetText().size(), names, allocator, {});

				TextEditor::ErrorMarkers markers;

				for (const auto& value : result.errors)
				{
					markers.insert(std::make_pair<int, std::string>(value.getLocation().begin.line + 1, value.what()));
				}

				editor.SetErrorMarkers(markers);

				editorText = editor.GetText();
			}

			auto cpos = editor.GetCursorPosition();
			ImGui::Begin("SirMeme Hub", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

			editor.Render("TextEditor", ImVec2(800, 600 - 40));

			if (ImGui::Button("Execute", ImVec2{ 60, 20 })) {
				if (!editor.GetText().empty()) {
					auto& Sched = *rbx::scheduler;

					Sched.push_smh([]() {
						auto thread = rbx::execution_state;
						//lua_ref(rbx::base_state, 1); // no GCing! we might yield...
						bool error = false;

						const auto bytecode = Luau::compile(editor.GetText(), { 0, 2, 0 });

						if (error)
						{
							lua_getglobal(thread, "warn");
							lua_pushstring(thread, bytecode.data());
							lua_pcall(thread, 1, 0, 0);
							return;
						}

						lua_getglobal(thread, "spawn");
						if (luau_load(thread, "bytecode", bytecode, 0))
						{
							size_t len;
							const char* msg = lua_tolstring(thread, -1, &len);

							std::string error(msg, len);
							lua_pop(thread, 2);

							lua_getglobal(thread, "warn");
							lua_pushstring(thread, error.data());
						}

						lua_pcall(thread, 1, 0, 0);

					});
				}
			}

			ImGui::End();
		}

		if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		{
			ImVec2 mouse_pos = ImGui::GetMousePos();

			ImGui::RenderMouseCursor(mouse_pos, 1.00f, ImGuiMouseCursor_Arrow, ImGui::GetColorU32({ 1, 1, 1, 1 }), ImGui::GetColorU32({ 0, 0, 0, 1 }), 0);

			// Optionally, you can set the mouse cursor colors.
			// ImGui::PushStyleColor(ImGuiCol_MouseCursor, { 1.0f, 1.0f, 1.0f, 1.0f });
			// ImGui::PushStyleColor(ImGuiCol_MouseCursorHovered, { 0.0f, 0.0f, 0.0f, 1.0f });
			// ImGui::PushStyleColor(ImGuiCol_MouseCursorActive, { 1.0f, 0.0f, 0.0f, 1.0f });
			// ImGui::RenderMouseCursor(draw_list, mouse_pos, 0.75f, ImGuiMouseCursor_Arrow);
			// ImGui::PopStyleColor(3);
		}
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &pMainRenderTargetView, NULL);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	hook_present->undo();
	auto result = oPresent(pSwapChain, uSyncInterval, uFlags);
	hook_present->apply();

	return result;
}

static HRESULT WINAPI hkResizeBuffers(IDXGISwapChain *pSwapChain, UINT uCount, UINT uWidth, UINT uHeight, DXGI_FORMAT dxNewFormat, UINT uFlags) {
	std::printf("resize buf\n");
	pContext->OMSetRenderTargets(0, nullptr, nullptr);
	pMainRenderTargetView->Release();

	auto result = oResizeBuffers(pSwapChain, uCount, uWidth, uHeight, dxNewFormat, uFlags);

	ID3D11Texture2D *pBuffer = nullptr;
	pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&pBuffer));

	if (pBuffer) {
		pDevice->CreateRenderTargetView(pBuffer, nullptr, &pMainRenderTargetView);
		pBuffer->Release();
	}

	pContext->OMSetRenderTargets(1, &pMainRenderTargetView, NULL);

	D3D11_VIEWPORT vp{
		.TopLeftX = 0,
		.TopLeftY = 0,
		.Width = 0,
		.Height = 0,
		.MinDepth = 0.0f,
		.MaxDepth = 1.0f,
	};

	pContext->RSSetViewports(1, &vp);

	return result;
}


uint8_t present_bytes[8] = { 0x8b, 0xff, 0x55, 0x8b, 0xec, 0x83, 0xE4, 0xF8 };

static DWORD WINAPI MainThread(LPVOID _) {
	//rbx::bypass::console(L"helo");
	directx::IsGuiOpen = false;

	while (kiero::init(kiero::RenderType::D3D11) != kiero::Status::AlreadyInitializedError) {
		present_ptr = kiero::getMethodsTable()[8];

		if (*reinterpret_cast<uint8_t*>(present_ptr) == 0xE9)
		{
			auto absolute = present_ptr + 5 + *reinterpret_cast<uintptr_t*>(present_ptr + 1);
			hPresent = reinterpret_cast<Present>(absolute);

			hook_hook = new ehook::ehook(reinterpret_cast<uintptr_t>(hPresent), reinterpret_cast<uintptr_t>(hkPresent), nullptr, true);
			hook_hook->apply();

			hook_present = new ehook::ehook(present_ptr, reinterpret_cast<uintptr_t>(hPresent), present_bytes, true);
			old_hook_present_bytes = ehook::memory::read_memory_raw(reinterpret_cast<void*>(present_ptr + 5), 4);
		}
		else
		{
			hook_present = new ehook::ehook(present_ptr, reinterpret_cast<uintptr_t>(hkPresent), nullptr, true);
			hook_present->apply();
		}

		oPresent = reinterpret_cast<Present>(present_ptr);
		kiero::bind(13, reinterpret_cast<void**>(&oResizeBuffers), hkResizeBuffers);
	}

	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		char path[MAX_PATH];
		GetModuleFileNameA(hModule, path, MAX_PATH);

		DisableThreadLibraryCalls(hModule);
		
		sys::set_root_dir(std::string(path, strlen(path) - strlen(_STR("loader.dll"))));

		CreateThread(NULL, 0, MainThread, NULL, 0, NULL);

		return TRUE; /* prevent NULL injection */
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}

	return TRUE;
}
