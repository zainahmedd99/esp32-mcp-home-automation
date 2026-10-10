from fastapi import FastAPI, Request
from pydantic import BaseModel
import uvicorn
import datetime

app = FastAPI(
    title="Home Workshop MCP Server",
    description="Model Context Protocol bridge connecting ESP32 sensor nodes to LLM AI agents.",
    version="1.0.0"
)

telemetry_db = {
    "node_id": "ESP32-NODE-01",
    "temperature_c": 0.0,
    "humidity_pct": 0.0,
    "current_amps": 0.0,
    "relay_status": False,
    "last_updated": None
}

class TelemetryPayload(BaseModel):
    node_id: str
    temperature_c: float
    humidity_pct: float
    current_amps: float
    relay_status: bool

@app.post("/api/mcp/telemetry")
async def receive_esp32_telemetry(payload: TelemetryPayload):
    """
    Primary endpoint where ESP32 pushes live environmental telemetry.
    """
    global telemetry_db
    telemetry_db["node_id"] = payload.node_id
    telemetry_db["temperature_c"] = payload.temperature_c
    telemetry_db["humidity_pct"] = payload.humidity_pct
    telemetry_db["current_amps"] = payload.current_amps
    telemetry_db["relay_status"] = payload.relay_status
    telemetry_db["last_updated"] = datetime.datetime.now().isoformat()
    
    print(f"[Telemetry Received] Node: {payload.node_id} | Temp: {payload.temperature_c}°C | Hum: {payload.humidity_pct}% | Current: {payload.current_amps}A")
    
    
    response_command = "NONE"
    if payload.temperature_c > 31.5 and not payload.relay_status:
        response_command = "TURNON"
        print("[MCP Engine] Threshold exceeded: Triggering cooling fan ON.")
    elif payload.temperature_c < 26.0 and payload.relay_status:
        response_command = "TURNOFF"
        print("[MCP Engine] Temperature normal: Shutting down cooling fan.")

    return {
        "status": "success",
        "synced_with_mcp": True,
        "command": response_command,
        "timestamp": telemetry_db["last_updated"]
    }

@app.get("/api/mcp/tools/get_workshop_climate")
async def get_workshop_climate():
    """
    MCP Tool Specification: Allows an LLM AI Agent to query real-time climate data on demand.
    """
    return {
        "tool": "get_workshop_climate",
        "protocol": "Model Context Protocol v1.0",
        "status": "active",
        "data": telemetry_db,
        "ai_recommendation": "Environment is stable." if telemetry_db["temperature_c"] < 30 else "High temperature detected; automated relay activation recommended."
    }

if __name__ == "__main__":
    uvicorn.run("mcp_server:app", host="0.0.0.0", port=8000, reload=True)
