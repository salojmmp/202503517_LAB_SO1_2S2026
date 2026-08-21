# Proyecto SO1: Desarrollo, Conexión y Gestión de Contenedores

**Estudiante:** 202503517  
**Auxiliares:** @JoseLorenzana272, @KINGROX

---

## Resumen

Este proyecto implementa un entorno virtualizado con:
- **3 Máquinas Virtuales** usando KVM
- **3 APIs REST** desarrolladas en Go
- **3 Runtimes de contenedores**: Containerd, Podman, Docker
- **Registro privado** con Zot
- **Comunicación HTTP** entre APIs en diferentes VMs

---

## Arquitectura

| VM | Runtime | APIs | IP |
|----|---------|------|-----|
| VM1 | Containerd | API1, API2 | 192.168.122.80 |
| VM2 | Podman | API3 | 192.168.122.80 |
| VM3 | Docker | Zot (5000) | 192.168.122.80 |

---

## Endpoints

### API1 (Puerto 8001)
- `GET /health` - Estado de la API
- `GET /api1/202503517/call-api2` - Contacta API2
- `GET /api1/202503517/call-api3` - Contacta API3

### API2 (Puerto 8002)
- `GET /health` - Estado de la API
- `GET /api2/202503517/call-api1` - Contacta API1
- `GET /api2/202503517/call-api3` - Contacta API3

### API3 (Puerto 8003)
- `GET /health` - Estado de la API
- `GET /api3/202503517/call-api1` - Contacta API1
- `GET /api3/202503517/call-api2` - Contacta API2

---

## Estructura del Proyecto

├── README.md├── api1/│   ├── main.go│   └── Dockerfile├── api2/│   ├── main.go│   └── Dockerfile├── api3/│   ├── main.go│   └── Dockerfile├── docs/│   └── MANUAL_TECNICO.md└── evidencia/└── (capturas de pruebas)

---

## Documentación

Ver [MANUAL_TECNICO.md](./docs/MANUAL_TECNICO.md) para guía completa de instalación y configuración.
