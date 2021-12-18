#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <functional>
#include <exception>
#include <stdexcept>
#include <optional>
#include <vector>

namespace ToolHelp32Ex {

	using Error = std::runtime_error;

	template <typename _EntryType>
	using IterProc = std::function<bool(const _EntryType&)>;

	template <typename _EntryType>
	using FirstProc = std::function<BOOL(HANDLE, _EntryType*)>;

	template <typename _EntryType>
	using NextProc = std::function<BOOL(HANDLE, _EntryType*)>;

	template <typename _EntryType>
	static bool Iter(DWORD _Type, DWORD _ID, const FirstProc<_EntryType>& _First, const NextProc<_EntryType>& _Next, const IterProc<_EntryType>& _Proc) {
		auto snapshot = CreateToolhelp32Snapshot(_Type, _ID);
		if (snapshot == INVALID_HANDLE_VALUE) {
			throw Error("ToolHelp32Ex: CreateToolhelp32Snapshot Failed.");
		}
		_EntryType entry = { 0 };
		entry.dwSize = sizeof(entry);
		for (auto status = _First(snapshot, &entry); status == TRUE; status = _Next(snapshot, &entry)) {
			if (!_Proc(entry)) {
				CloseHandle(snapshot);
				return false;
			}
		}
		CloseHandle(snapshot);
		return true;
	}

	template <typename _EntryType>
	using FindProc = std::function<bool(const _EntryType&)>;

	template <typename _EntryType>
	static std::optional<_EntryType> Find(DWORD _Type, DWORD _ID, const FirstProc<_EntryType>& _First, const NextProc<_EntryType>& _Next, const FindProc<_EntryType>& _Proc) {
		std::optional<_EntryType> entry;
		Iter<_EntryType>(_Type, _ID, _First, _Next, [&](const _EntryType& e) -> bool {
			if (_Proc(e)) {
				entry = e;
				return false;
			}
			else {
				return true;
			}
		});
		return entry;
	}

	template <typename _EntryType>
	static std::vector<_EntryType> List(DWORD _Type, DWORD _ID, const FirstProc<_EntryType>& _First, const NextProc<_EntryType>& _Next) {
		std::vector<_EntryType> vector;
		Iter<_EntryType>(_Type, _ID, _First, _Next, [&](const _EntryType& e) -> bool {
			vector.push_back(e);
			return true;
		});
		return vector;
	}

	template <typename _EntryType>
	using ListProc = std::function<bool(const _EntryType&)>;

	template <typename _EntryType>
	static std::vector<_EntryType> List(DWORD _Type, DWORD _ID, const FirstProc<_EntryType>& _First, const NextProc<_EntryType>& _Next, const ListProc<_EntryType>& _Proc) {
		std::vector<_EntryType> vector;
		Iter<_EntryType>(_Type, _ID, _First, _Next, [&](const _EntryType& e) -> bool {
			if (_Proc(e)) {
				vector.push_back(e);
			}
			return true;
			});
		return vector;
	}

	static bool IterProcess(const IterProc<PROCESSENTRY32>& _Proc) {
		return Iter<PROCESSENTRY32>(TH32CS_SNAPPROCESS, NULL, Process32First, Process32Next, _Proc);
	}

	static bool IterModule(DWORD _ID, const IterProc<MODULEENTRY32>& _Proc) {
		return Iter<MODULEENTRY32>(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, _ID, Module32First, Module32Next, _Proc);
	}

	static bool IterThread(const IterProc<THREADENTRY32>& _Proc) {
		return Iter<THREADENTRY32>(TH32CS_SNAPTHREAD, NULL, Thread32First, Thread32Next, _Proc);
	}

	static std::optional<PROCESSENTRY32> FindProcess(const FindProc<PROCESSENTRY32>& _Proc) {
		return Find<PROCESSENTRY32>(TH32CS_SNAPPROCESS, NULL, Process32First, Process32Next, _Proc);
	}

	static std::optional<MODULEENTRY32> FindModule(DWORD _ID, const FindProc<MODULEENTRY32>& _Proc) {
		return Find<MODULEENTRY32>(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, _ID, Module32First, Module32Next, _Proc);
	}

	static std::optional<THREADENTRY32> FindThread(const FindProc<THREADENTRY32>& _Proc) {
		return Find<THREADENTRY32>(TH32CS_SNAPTHREAD, NULL, Thread32First, Thread32Next, _Proc);
	}

	static std::vector<PROCESSENTRY32> ListProcess() {
		return List<PROCESSENTRY32>(TH32CS_SNAPPROCESS, NULL, Process32First, Process32Next);
	}

	static std::vector<MODULEENTRY32> ListModule(DWORD _ID) {
		return List<MODULEENTRY32>(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, _ID, Module32First, Module32Next);
	}

	static std::vector<THREADENTRY32> ListThread() {
		return List<THREADENTRY32>(TH32CS_SNAPTHREAD, NULL, Thread32First, Thread32Next);
	}

	static std::vector<PROCESSENTRY32> ListProcess(const ListProc<PROCESSENTRY32>& _Proc) {
		return List<PROCESSENTRY32>(TH32CS_SNAPPROCESS, NULL, Process32First, Process32Next, _Proc);
	}

	static std::vector<MODULEENTRY32> ListModule(DWORD _ID, const ListProc<MODULEENTRY32>& _Proc) {
		return List<MODULEENTRY32>(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, _ID, Module32First, Module32Next, _Proc);
	}

	static std::vector<THREADENTRY32> ListThread(const ListProc<THREADENTRY32>& _Proc) {
		return List<THREADENTRY32>(TH32CS_SNAPTHREAD, NULL, Thread32First, Thread32Next, _Proc);
	}

}