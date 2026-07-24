# 🚪 ESP32 跨網域智慧門控系統
## Smart Door Control System

> 一套整合伺服器部署、MQTT 通訊、嵌入式控制與硬體整合的 IoT 門控系統。  
> 透過 ESP32、MQTT 與 Node-RED 建立跨網域遠端控制架構，並於實際環境中連續穩定運行超過 200 天。

---

## 📌 Project Overview

本專案將既有實體門控設備整合至 IoT 架構中，使使用者除了保留原本實體遙控操作外，也能透過 Web Dashboard 進行遠端控制。

系統採用 MQTT Publish / Subscribe 架構，串聯：

- Ubuntu Server IoT 主機
- MQTT Broker
- Node-RED Dashboard
- ESP32 控制端
- 原有門控遙控器硬體

實現跨網域遠端控制、設備狀態監控與自動恢復連線能力。

---

# 🏗 System Architecture

```text
                      Internet
                          │
                          │
          ┌───────────────▼───────────────┐
          │          Ubuntu Server          │
          │                                │
          │  PPPoE Client                  │
          │  DDNS                          │
          │  systemd Service Management    │
          │                                │
          └───────────────┬────────────────┘
                          │
                          ▼
          ┌───────────────────────────────┐
          │          MQTT Broker           │
          │                                │
          │  Message Routing              │
          │  Client Connection Management │
          └───────────────┬───────────────┘
                          │
              MQTT Publish / Subscribe
                          │
                          ▼
          ┌───────────────────────────────┐
          │       Node-RED Dashboard       │
          │                                │
          │  Web Control Interface        │
          │  MQTT Client                  │
          │  Status Monitor               │
          └───────────────┬───────────────┘
                          │
                          │ MQTT
                          ▼
          ┌───────────────────────────────┐
          │        ESP32 Controller        │
          │                                │
          │  MQTT Client                  │
          │  Heartbeat                    │
          │  Auto Reconnect               │
          │  GPIO Control                 │
          └───────────────┬───────────────┘
                          │
                          │ GPIO Signal
                          ▼
          ┌───────────────────────────────┐
          │ Modified Remote Control PCB    │
          │ Hardware Integration           │
          └───────────────┬───────────────┘
                          │
                          ▼
                Original Door System
```
---

# ✨ Features

## 🌐 Cross-Network Remote Control

- 支援跨網域遠端門控操作
- 使用 MQTT 作為設備通訊協定
- 支援多組門控設備獨立控制
- 透過 Node-RED Dashboard 提供 Web 操作介面

## 🔄 Two-Way Communication

系統支援控制命令與設備狀態雙向交換：

- 門控操作命令
- 設備在線狀態
- 操作執行回饋
- 電源狀態同步

## 🖥️ Server Infrastructure

### Hardware
使用工控電腦作為 IoT Server，提供長時間穩定運行環境。

### Operating System
- Ubuntu Server

### Network Deployment
- **PPPoE Client**: 由 Ubuntu 主機直接建立 PPPoE 連線（`ISP` ➔ `Ubuntu Server` ➔ `Internet`），使伺服器自行管理外部網路連線。
- **DDNS**: 針對 ISP 動態 IP 配置，使用 DDNS 提供固定網域入口，維護外部設備穩定連線。

---

# ⚙️ MQTT Communication Platform

## MQTT Broker

MQTT Broker 作為系統訊息交換中心，負責：
- 接收 Publish 訊息
- 轉發 Subscribe 訊息
- 管理設備連線

## MQTT Topic Design

使用 Topic 區分不同設備與資料類型：

| Topic | Payload Example | Description |
|---|---|---|
| `gate1/door` | `V0` (Up) / `V1` (Down) / `V4` (Stop) | 門控操作指令 |
| `gate1/power` | `ON` / `OFF` | 電源狀態同步 |
| `gate1/door/led` | `HIGH` / `LOW` | 操作狀態回饋 |
| `remote/controller/status/gate1` | `online` / `offline` | 設備心跳與在線狀態 |

---

# 🌐 Node-RED Dashboard & ESP32 Control

## Node-RED Dashboard
Node-RED 作為 MQTT Client，提供使用者 Web 遠端控制介面、多門設備管理與 MQTT 訊息監控。

`User` ➔ `Node-RED Dashboard` ➔ `MQTT Broker` ➔ `ESP32`

## ESP32 Controller Firmware
- **MQTT Client 通訊**: 訂閱對應 Topic 並進行 Command 解析。
- **GPIO 控制**: 依據命令觸發對應接線邏輯。
- **自動機制**: 負責 Heartbeat 定期回報與網路異常自動重連。

---

# 🔧 Hardware Integration

本專案將 IoT 控制能力整合至既有門控設備：

- **電路分析**: 分析既有遙控器 PCB 電路架構與控制訊號流程。
- **硬體改裝**: 進行電子元件焊接與硬體修改，將 ESP32 GPIO 控制訊號整合至原控制介面。
- **雙軌控制**: 完整保留原始實體遙控功能，同時新增 Web 遠端控制能力。

---

# 🔄 Reliability Mechanism

- **Heartbeat (心跳檢測)**: ESP32 定期透過 MQTT 發送狀態數據至 Node-RED Status Monitor，實現即時斷線偵測。
- **Auto Recovery (自動復原)**: 支援 ESP32 Wi-Fi/MQTT 自動重連、systemd 伺服器服務自動重啟。當系統遭遇斷電或網路異常時，復原後可全自動重連運作。

---

# 📂 Directory Structure

```text
.
├── docs/                 # 架構圖、PCB 接線圖與 Demo 照片
├── esp32-firmware/       # ESP32 C++ / Arduino 韌體原始碼
├── nodered-flows/        # Node-RED Dashboard 匯出的 JSON 流程檔
└── server-config/        # systemd 服務腳本與 MQTT/DDNS 設定範例
```
# 🛠 Technology Stack

| Category | Technology |
|---|---|
| MCU | ESP32 |
| Firmware | Arduino Framework / C++ |
| Server OS | Ubuntu Server |
| IoT Messaging | MQTT / Mosquitto |
| Dashboard | Node-RED |
| Network | PPPoE / DDNS / TCP/IP |
| Hardware | PCB Analysis / Soldering / GPIO Integration |
| Service Management | systemd |

---

# 📊 System Status

- 實際環境連續運行 **超過 200 天零當機**
- 支援多組門控設備跨網域控制
- 具備完整設備狀態監控與全自動斷線重連機制

---
<!--
# 📷 Demo & Showcase

- [ ] Node-RED Dashboard 介面截圖
- [ ] ESP32 硬體改裝與 PCB 焊接照片
- [ ] 門控系統運作示範

---

-->