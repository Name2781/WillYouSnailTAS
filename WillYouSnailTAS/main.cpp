#include "includes.h"
#include <stdio.h>
#include <thread>
#include <iostream>
#include <fstream>
#include "windows.h"
#include <vector>
using namespace std;
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
HANDLE Handle_Of_runTas = 0;

bool isOpen = false;

std::vector<int> tasInputs;
std::vector<int> tasInputs2;
std::vector<const char*> fakeTasInputs;
std::vector<int> tasInputsTime;

void AddNewInput(int input, int length) {
	switch (input) {
		case 1:
			tasInputs.push_back(0x57);
			tasInputs2.push_back(0);
			fakeTasInputs.push_back("JUMP");
			tasInputsTime.push_back(length);
			break;
		case 2:
			tasInputs.push_back(0x41);
			tasInputs2.push_back(0);
			fakeTasInputs.push_back("LEFT");
			tasInputsTime.push_back(length);
			break;
		case 3:
			tasInputs.push_back(0x44);
			tasInputs2.push_back(0);
			fakeTasInputs.push_back("RIGHT");
			tasInputsTime.push_back(length);
			break;
		case 4:
			tasInputs.pop_back();
			tasInputs2.pop_back();
			fakeTasInputs.pop_back();
			tasInputsTime.pop_back();
			break;
		case 5:
			tasInputs.push_back(0x41);
			tasInputs2.push_back(0x57);
			fakeTasInputs.push_back("LEFT JUMP");
			tasInputsTime.push_back(length);
			break;
		case 6:
			tasInputs.push_back(0x44);
			tasInputs2.push_back(0x57);
			fakeTasInputs.push_back("RIGHT JUMP");
			tasInputsTime.push_back(length);
			break;
	}
}

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	// io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (isOpen) {
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

		return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

DWORD WINAPI runTas(LPVOID lpParam)
{
	for (int i = 0; i < tasInputs.size(); i++) {
		INPUT ip2;

		// if (tasInputs2[i] > 0)
		ip2.type = INPUT_KEYBOARD;
		ip2.ki.wScan = 0;
		ip2.ki.time = 0;
		ip2.ki.dwExtraInfo = 0;

		ip2.ki.wVk = tasInputs2[i];
		ip2.ki.dwFlags = 0;

		INPUT ip;

		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;

		ip.ki.wVk = tasInputs[i];
		ip.ki.dwFlags = 0;
		SendInput(1, &ip, sizeof(INPUT));
		if (tasInputs2[i] > 0)
			SendInput(1, &ip2, sizeof(INPUT));
		
		Sleep(tasInputsTime[i]);

		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));

		if (tasInputs2[i] > 0) {
			ip2.ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, &ip2, sizeof(INPUT));
		}
	}
	
	CloseHandle(Handle_Of_runTas);

	return 0;
}

void saveInputs() {
	fstream tasFile;
	tasFile.open("inputs.txt", ios::out);
	if (!tasFile) {

	}
	else {
		for (int i = 0; i < tasInputs.size(); i++)
		{
			tasFile << tasInputs[i] + "\n";
			tasFile << tasInputs2[i] + "\n";
			tasFile << tasInputsTime[i] + "\n";
			tasFile << fakeTasInputs[i] + '\n';
		}

		tasFile.close();
	}
}

void loadInputs() {

}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (GetAsyncKeyState(VK_INSERT) & 0x1) {
		isOpen ? isOpen = false : isOpen = true;
	}

	if (GetAsyncKeyState(VK_DELETE) & 0x1) {
		Handle_Of_runTas = CreateThread(NULL, 0,
			runTas, &Handle_Of_runTas, 0, NULL);

		return 0;
	}

	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)& pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)& pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (isOpen) {
		ImGui::Begin("TAS Editor", &isOpen, ImGuiWindowFlags_MenuBar);

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open..", "Ctrl+O")) {
					loadInputs();
				}
				if (ImGui::MenuItem("Save", "Ctrl+S")) {
					saveInputs();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		// const int tasInputs[] = { 0x44, 0x57, 0x41 };
		// const const char* fakeTasInputs[] = { "RIGHT", "UP", "LEFT" };
		// const int tasInputsTime[] = { 5000, 5000, 5000 };
		
		static int length = 1000;
		ImGui::Text("Length of input in milliseconds: "); 
		ImGui::SameLine(); 
		ImGui::InputInt("", &length);

		if (ImGui::Button("Press Jump")) {
			AddNewInput(1, length);
		}
		if (ImGui::Button("Press Left")) {
			AddNewInput(2, length);
		}
		if (ImGui::Button("Press Right")) {
			AddNewInput(3, length);
		}
		if (ImGui::Button("Jump Right")) {
			AddNewInput(6, length);
		}
		if (ImGui::Button("Jump Left")) {
			AddNewInput(5, length);
		}
		if (ImGui::Button("Remove Last Input")) {
			AddNewInput(4, 0);
		}

		ImGui::TextColored(ImVec4(1, 1, 0, 1), "Inputs");
		ImGui::BeginChild("Scrolling");

		if (!tasInputs.empty()) {
			for (int i = 0; i < tasInputs.size(); i++)
			{
				ImGui::Text("%i: %s", tasInputsTime[i], fakeTasInputs[i]);
			}
		}

		ImGui::EndChild();

		// ImGui::ShowDemoWindow();

		ImGui::End();
	}

	ImGui::Render();

	pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI MainThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)& oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hMod, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hMod);
		CreateThread(nullptr, 0, MainThread, hMod, 0, nullptr);
		break;
	case DLL_PROCESS_DETACH:
		kiero::shutdown();
		break;
	}
	return TRUE;
}