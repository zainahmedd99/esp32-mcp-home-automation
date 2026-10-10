# ESP32 Model Context Protocol (MCP) Home Automation

An embedded systems and AI-integration project designed for a home workshop environment. This repository connects microcontroller sensor nodes (ESP32) over local Wi-Fi to a Python backend server implementing **Model Context Protocol (MCP)** standards, allowing AI agents to monitor and control home appliances (fans, lights, power circuits) in real time.

---

## 🛠️ System Architecture
1. **Edge Node (ESP32):** Reads environmental data (DHT22 temperature/humidity, SCT-013 current sensors) and controls relays (ceiling fans, lighting grids) via GPIO pins.
2. **Communication Layer:** Periodic HTTP POST telemetry sync over Wi-Fi sockets (`/api/mcp/telemetry`).
3. **MCP Backend Server:** FastAPI server exposing structured tool definitions (`/api/mcp/tools/get_workshop_climate`) for LLM agents and automated rule engines.

---

## 📂 Repository Contents
- **`firmware/main.cpp`**: C++ Arduino/ESP-IDF firmware handling Wi-Fi reconnections, sensor polling, JSON payloads, and server command parsing.
- **`server/mcp_server.py`**: Python FastAPI Model Context Protocol server managing live state storage and intelligent automation triggers.
