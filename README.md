# FreeRTOS Wi-Fi Driver Simulation (Dual-Task)

這是一個基於 FreeRTOS 的多執行緒 Wi-Fi 驅動程式模擬專案。
本專案模擬了 **Host Driver (驅動層)** 與 **Firmware (韌體層)** 之間的互動，並實作了生產者-消費者模型 (Producer-Consumer Model)。

## 🚀 專案亮點 (Key Features)
* **雙任務通訊架構**：模擬 `Driver Task` (負責 TX) 與 `Firmware Task` (負責 RX) 之間的封包傳遞。
* **並發控制 (Concurrency Control)**：使用 FreeRTOS 的 **Queue** 與 **Semaphore** 來處理資料傳輸，防止 Race Condition。
* **臨界區保護 (Critical Section)**：模擬在處理關鍵資料時進行 **中斷遮蔽 (IRQ Masking)**，確保資料的一致性。

## 🛠 使用技術 (Tech Stack)
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-Blue?style=for-the-badge&logo=freertos&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-%23008FBA.svg?style=for-the-badge&logo=cmake&logoColor=white)

## ⚠️ 建置與執行說明 (Build Instructions)
本專案是基於 FreeRTOS 官方的 `Posix_GCC` 範例進行修改，因此需要依賴 FreeRTOS 的 Kernel 原始碼。

**如何執行此專案：**

1. 下載官方 Repo：[FreeRTOS/FreeRTOS](https://github.com/FreeRTOS/FreeRTOS)
2. 進入目錄：`FreeRTOS/Demo/Posix_GCC`
3. **替換檔案**：將本 Repo 中的所有檔案（`main.c`, `CMakeLists.txt` 等）複製並覆蓋到上述目錄中。
4. 執行以下指令進行編譯與執行：
   ```bash
   cmake -B build -S . -G "Unix Makefiles"
   make -C build -j
   ./build/WiFi_Sim