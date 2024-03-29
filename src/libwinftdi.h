//
// libwinftdi.h
// Copyright (C) 2019 Marius Greuel
//

#pragma once

#define WIN32_LEAN_AND_MEAN
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cerrno>
#include <memory>
#include <string>
#include <vector>
#include <windows.h>

namespace LibWinFtdi
{
    enum
    {
        FT_OK,
        FT_INVALID_HANDLE,
        FT_DEVICE_NOT_FOUND,
        FT_DEVICE_NOT_OPENED,
        FT_IO_ERROR,
        FT_INSUFFICIENT_RESOURCES,
        FT_INVALID_PARAMETER,
        FT_INVALID_BAUD_RATE,
        FT_DEVICE_NOT_OPENED_FOR_ERASE,
        FT_DEVICE_NOT_OPENED_FOR_WRITE,
        FT_FAILED_TO_WRITE_DEVICE,
        FT_EEPROM_READ_FAILED,
        FT_EEPROM_WRITE_FAILED,
        FT_EEPROM_ERASE_FAILED,
        FT_EEPROM_NOT_PRESENT,
        FT_EEPROM_NOT_PROGRAMMED,
        FT_INVALID_ARGS,
        FT_NOT_SUPPORTED,
        FT_OTHER_ERROR,
        FT_DEVICE_LIST_NOT_READY,
    };

    enum
    {
        FT_DEVICE_BM,
        FT_DEVICE_AM,
        FT_DEVICE_100AX,
        FT_DEVICE_UNKNOWN,
        FT_DEVICE_2232C,
        FT_DEVICE_232R,
        FT_DEVICE_2232H,
        FT_DEVICE_4232H,
        FT_DEVICE_232H,
        FT_DEVICE_X_SERIES,
        FT_DEVICE_4222H_0,
        FT_DEVICE_4222H_1_2,
        FT_DEVICE_4222H_3,
        FT_DEVICE_4222_PROG,
        FT_DEVICE_900,
        FT_DEVICE_930,
        FT_DEVICE_UMFTPD3A,
        FT_DEVICE_2233HP,
        FT_DEVICE_4233HP,
        FT_DEVICE_2232HP,
        FT_DEVICE_4232HP,
        FT_DEVICE_233HP,
        FT_DEVICE_232HP,
        FT_DEVICE_2232HA,
        FT_DEVICE_4232HA,
        FT_DEVICE_232RN,
    };

    enum
    {
        FT_FLAGS_OPENED = 1,
        FT_FLAGS_HISPEED = 2,
    };

    enum
    {
        FT_OPEN_BY_SERIAL_NUMBER = 1,
        FT_OPEN_BY_DESCRIPTION = 2,
        FT_OPEN_BY_LOCATION = 4,
    };

    enum
    {
        FT_PURGE_RX = 1,
        FT_PURGE_TX = 2,
    };

    enum
    {
        FT_EVENT_RXCHAR = 1,
        FT_EVENT_MODEM_STATUS = 2,
        FT_EVENT_LINE_STATUS = 4,
    };

    typedef PVOID FT_HANDLE;
    typedef ULONG FT_STATUS;

    struct DeviceInfo
    {
        DWORD dwFlags = 0;
        DWORD dwType = 0;
        DWORD dwID = 0;
        DWORD dwLocId = 0;
        int nInterface = 0;
        char acSerialNumber[16] = { 0 };
        char acPortSerialNumber[16] = { 0 };
        char acDescription[64] = { 0 };
        FT_HANDLE ftHandle = nullptr;
    };

    class FtdiApi
    {
    public:
        FtdiApi() = default;
        FtdiApi(const FtdiApi&) = delete;
        FtdiApi& operator=(const FtdiApi&) = delete;
        FtdiApi(FtdiApi&&) = delete;
        FtdiApi& operator=(FtdiApi&&) = delete;

        ~FtdiApi()
        {
            Unload();
        }

        HRESULT Load()
        {
            if (m_module != nullptr)
                return S_FALSE;

            HMODULE module = LoadLibraryExW(L"ftd2xx.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
            if (module == nullptr)
            {
                return HRESULT_FROM_WIN32(GetLastError());
            }

            HRESULT hr = S_OK;
            if (FAILED(hr = LoadImport(module, "FT_CreateDeviceInfoList", &FT_CreateDeviceInfoList)) ||
                FAILED(hr = LoadImport(module, "FT_GetDeviceInfoDetail", &FT_GetDeviceInfoDetail)) ||
                FAILED(hr = LoadImport(module, "FT_OpenEx", &FT_OpenEx)) ||
                FAILED(hr = LoadImport(module, "FT_Close", &FT_Close)) ||
                FAILED(hr = LoadImport(module, "FT_ResetDevice", &FT_ResetDevice)) ||
                FAILED(hr = LoadImport(module, "FT_ResetPort", &FT_ResetPort)) ||
                FAILED(hr = LoadImport(module, "FT_CyclePort", &FT_ResetPort)) ||
                FAILED(hr = LoadImport(module, "FT_Purge", &FT_Purge)) ||
                FAILED(hr = LoadImport(module, "FT_SetTimeouts", &FT_SetTimeouts)) ||
                FAILED(hr = LoadImport(module, "FT_SetBaudRate", &FT_SetBaudRate)) ||
                FAILED(hr = LoadImport(module, "FT_SetBitMode", &FT_SetBitMode)) ||
                FAILED(hr = LoadImport(module, "FT_GetBitMode", &FT_GetBitMode)) ||
                FAILED(hr = LoadImport(module, "FT_SetLatencyTimer", &FT_SetLatencyTimer)) ||
                FAILED(hr = LoadImport(module, "FT_GetQueueStatus", &FT_GetQueueStatus)) ||
                FAILED(hr = LoadImport(module, "FT_SetEventNotification", &FT_SetEventNotification)) ||
                FAILED(hr = LoadImport(module, "FT_SetUSBParameters", &FT_SetUSBParameters)) ||
                FAILED(hr = LoadImport(module, "FT_Read", &FT_Read)) ||
                FAILED(hr = LoadImport(module, "FT_Write", &FT_Write)))
            {
                FreeLibrary(module);
                return hr;
            }

            m_module = module;
            return S_OK;
        }

        HRESULT Unload()
        {
            if (m_module == nullptr)
                return S_FALSE;

            FreeLibrary(m_module);
            m_module = nullptr;

            return S_OK;
        }

    private:
        template<typename T>
        static HRESULT LoadImport(HMODULE module, LPCSTR lpProcName, T** ptr)
        {
            auto proc = GetProcAddress(module, lpProcName);
            if (proc == nullptr)
            {
                *ptr = nullptr;
                return HRESULT_FROM_WIN32(GetLastError());
            }

            *ptr = reinterpret_cast<T*>(proc);
            return S_OK;
        }

    private:
        HMODULE m_module = nullptr;

    protected:
        FT_STATUS(WINAPI* FT_CreateDeviceInfoList)(LPDWORD lpdwNumDevs) = nullptr;
        FT_STATUS(WINAPI* FT_GetDeviceInfoDetail)(
            DWORD dwIndex,
            LPDWORD lpdwFlags,
            LPDWORD lpdwType,
            LPDWORD lpdwID,
            LPDWORD lpdwLocId,
            LPVOID lpSerialNumber,
            LPVOID lpDescription,
            FT_HANDLE* pftHandle) = nullptr;
        FT_STATUS(WINAPI* FT_OpenEx)(PVOID pArg1, DWORD Flags, FT_HANDLE* pHandle) = nullptr;
        FT_STATUS(WINAPI* FT_Close)(FT_HANDLE ftHandle) = nullptr;
        FT_STATUS(WINAPI* FT_ResetDevice)(FT_HANDLE ftHandle) = nullptr;
        FT_STATUS(WINAPI* FT_ResetPort)(FT_HANDLE ftHandle) = nullptr;
        FT_STATUS(WINAPI* FT_CyclePort)(FT_HANDLE ftHandle) = nullptr;
        FT_STATUS(WINAPI* FT_Purge)(FT_HANDLE ftHandle, DWORD dwMask) = nullptr;
        FT_STATUS(WINAPI* FT_SetTimeouts)(FT_HANDLE ftHandle, DWORD dwReadTimeout, DWORD dwWriteTimeout) = nullptr;
        FT_STATUS(WINAPI* FT_SetBaudRate)(FT_HANDLE ftHandle, DWORD dwBaudRate) = nullptr;
        FT_STATUS(WINAPI* FT_SetBitMode)(FT_HANDLE ftHandle, UCHAR ucMask, UCHAR ucEnable) = nullptr;
        FT_STATUS(WINAPI* FT_GetBitMode)(FT_HANDLE ftHandle, PUCHAR pucMode) = nullptr;
        FT_STATUS(WINAPI* FT_SetLatencyTimer)(FT_HANDLE ftHandle, UCHAR ucTimer) = nullptr;
        FT_STATUS(WINAPI* FT_GetQueueStatus)(FT_HANDLE ftHandle, DWORD* dwRxBytes) = nullptr;
        FT_STATUS(WINAPI* FT_SetEventNotification)(FT_HANDLE ftHandle, DWORD Mask, PVOID Param) = nullptr;
        FT_STATUS(WINAPI* FT_SetUSBParameters)(FT_HANDLE ftHandle, DWORD dwInTransferSize, DWORD dwOutTransferSize) = nullptr;
        FT_STATUS(WINAPI* FT_Read)(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesReturned) = nullptr;
        FT_STATUS(WINAPI* FT_Write)(FT_HANDLE ftHandle, LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten) = nullptr;
    };

    class FtdiEnumerator : public FtdiApi
    {
    public:
        FtdiEnumerator() = default;
        FtdiEnumerator(const FtdiEnumerator&) = delete;
        FtdiEnumerator& operator=(const FtdiEnumerator&) = delete;
        FtdiEnumerator(FtdiEnumerator&&) = delete;
        FtdiEnumerator& operator=(FtdiEnumerator&&) = delete;

    public:
        FT_STATUS EnumerateDevices()
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            DWORD dwNumDevs = 0;
            FT_STATUS status = FT_CreateDeviceInfoList(&dwNumDevs);
            if (status != FT_OK)
                return status;

            for (DWORD i = 0; i < dwNumDevs; i++)
            {
                DeviceInfo device;
                status = FT_GetDeviceInfoDetail(
                    i,
                    &device.dwFlags,
                    &device.dwType,
                    &device.dwID,
                    &device.dwLocId,
                    device.acSerialNumber,
                    device.acDescription,
                    &device.ftHandle);
                if (status == FT_OK)
                {
                    std::memcpy(device.acPortSerialNumber, device.acSerialNumber, sizeof(device.acSerialNumber));

                    if (HasMultipleInterfaces(device.dwType))
                    {
                        size_t nSerialNumberLength = strlen(device.acSerialNumber);
                        if (nSerialNumberLength > 0)
                        {
                            nSerialNumberLength--;

                            char chLast = device.acSerialNumber[nSerialNumberLength];
                            if (chLast >= 'A' && chLast <= 'D')
                            {
                                device.nInterface = chLast - 'A';
                                device.acSerialNumber[nSerialNumberLength] = 0;
                            }
                        }

                        size_t nDescriptionLength = strlen(device.acDescription);
                        if (nDescriptionLength > 0)
                        {
                            nDescriptionLength--;

                            char chLast = device.acDescription[nDescriptionLength];
                            if (chLast >= 'A' && chLast <= 'D')
                            {
                                if (nDescriptionLength > 0 && device.acDescription[nDescriptionLength - 1] == ' ')
                                {
                                    nDescriptionLength--;
                                }

                                device.acDescription[nDescriptionLength] = 0;
                            }
                        }
                    }

                    m_devices.push_back(device);
                }
            }

            return FT_OK;
        }

        const std::vector<DeviceInfo>& GetDevices() const
        {
            return m_devices;
        }

    private:
        static bool HasMultipleInterfaces(DWORD dwType)
        {
            switch (dwType)
            {
            case FT_DEVICE_2232C:
            case FT_DEVICE_2232H:
            case FT_DEVICE_2232HA:
            case FT_DEVICE_2232HP:
            case FT_DEVICE_2233HP:
            case FT_DEVICE_4232H:
            case FT_DEVICE_4232HA:
            case FT_DEVICE_4232HP:
            case FT_DEVICE_4233HP:
                return true;
            default:
                return false;
            }
        }

    private:
        std::vector<DeviceInfo> m_devices;
    };

    class FtdiDevice : public FtdiApi
    {
    public:
        FtdiDevice() = default;
        FtdiDevice(const FtdiDevice&) = delete;
        FtdiDevice& operator=(const FtdiDevice&) = delete;
        FtdiDevice(FtdiDevice&&) = delete;
        FtdiDevice& operator=(FtdiDevice&&) = delete;

        ~FtdiDevice()
        {
            if (m_event != nullptr)
            {
                CloseHandle(m_event);
            }
        }

        HANDLE GetNotificationEvent() const
        {
            return m_event;
        }

    public:
        FT_STATUS OpenBySerialNumber(const char* serialNumber)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_OpenEx(const_cast<char*>(serialNumber), FT_OPEN_BY_SERIAL_NUMBER, &m_handle);
        }

        FT_STATUS OpenByLocation(uint32_t location)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_OpenEx(&location, FT_OPEN_BY_LOCATION, &m_handle);
        }

        FT_STATUS Close()
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_Close(m_handle);
        }

        FT_STATUS ResetDevice()
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_ResetDevice(m_handle);
        }

        FT_STATUS ResetPort()
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_ResetPort(m_handle);
        }

        FT_STATUS CyclePort()
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_CyclePort(m_handle);
        }

        FT_STATUS Purge(DWORD dwMask = FT_PURGE_RX | FT_PURGE_TX)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_Purge(m_handle, dwMask);
        }

        FT_STATUS SetTimeouts(DWORD dwReadTimeout, DWORD dwWriteTimeout)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_SetTimeouts(m_handle, dwReadTimeout, dwWriteTimeout);
        }

        FT_STATUS SetBaudRate(DWORD dwBaudRate)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_SetBaudRate(m_handle, dwBaudRate);
        }

        FT_STATUS SetBitMode(UCHAR ucMask, UCHAR ucEnable)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_SetBitMode(m_handle, ucMask, ucEnable);
        }

        FT_STATUS GetBitMode(PUCHAR pucMode)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_GetBitMode(m_handle, pucMode);
        }

        FT_STATUS SetLatencyTimer(UCHAR ucTimer)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_SetLatencyTimer(m_handle, ucTimer);
        }

        FT_STATUS GetQueueStatus(DWORD* dwRxBytes)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_GetQueueStatus(m_handle, dwRxBytes);
        }

        FT_STATUS SetEventNotification(DWORD Mask)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            if (m_event == nullptr)
            {
                m_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
            }

            return FT_SetEventNotification(m_handle, Mask, m_event);
        }

        FT_STATUS SetUSBParameters(DWORD dwInTransferSize, DWORD dwOutTransferSize)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_SetUSBParameters(m_handle, dwInTransferSize, dwOutTransferSize);
        }

        FT_STATUS Read(LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesReturned)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_Read(m_handle, lpBuffer, dwBytesToRead, lpBytesReturned);
        }

        FT_STATUS Write(LPVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten)
        {
            if (FAILED(Load()))
            {
                return FT_INVALID_HANDLE;
            }

            return FT_Write(m_handle, lpBuffer, dwBytesToWrite, lpBytesWritten);
        }

    private:
        FT_HANDLE m_handle = nullptr;
        HANDLE m_event = nullptr;
    };
}
