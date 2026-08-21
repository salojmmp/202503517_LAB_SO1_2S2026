# Manual Técnico y Guía de Instalación

## Tabla de Contenidos
1. [Requisitos](#requisitos)
2. [Instalación Host](#instalación-del-host)
3. [Creación de VMs](#creación-de-máquinas-virtuales)
4. [Instalación de Runtimes](#instalación-de-runtimes)
5. [Construcción de APIs](#construcción-de-las-apis)
6. [Ejecución de Contenedores](#ejecución-de-contenedores)
7. [Pruebas](#pruebas-de-comunicación)
8. [Troubleshooting](#troubleshooting)

---

## Requisitos

- Linux Ubuntu 22.04 (Host)
- 8GB RAM mínimo
- 50GB espacio disco
- VirtualBox o KVM/QEMU
- Go 1.23+
- Docker instalado

---

## Instalación del Host

### 1. Instalar KVM y herramientas de virtualización

```bash
sudo apt update
sudo apt install -y qemu-system-x86 libvirt-daemon-system libvirt-clients bridge-utils virt-manager

sudo usermod -aG libvirt $USER
sudo usermod -aG kvm $USER
```

### 2. Instalar Go

```bash
wget https://go.dev/dl/go1.23.0.linux-amd64.tar.gz
sudo rm -rf /usr/local/go
sudo tar -C /usr/local -xzf go1.23.0.linux-amd64.tar.gz
echo 'export PATH=$PATH:/usr/local/go/bin' >> ~/.bashrc
source ~/.bashrc

# Verificar
go version
```

### 3. Descargar ISO Ubuntu

```bash
cd ~
wget https://releases.ubuntu.com/jammy/ubuntu-22.04.5-live-server-amd64.iso
mkdir -p iso-folder
mv ubuntu-22.04.5-live-server-amd64.iso iso-folder/
```

---

## Creación de Máquinas Virtuales

### Crear VM1 (Containerd)

```bash
sudo virt-install \
  --name vm1-containerd \
  --memory 1536 \
  --vcpus 1 \
  --disk size=5,format=qcow2 \
  --cdrom ~/iso-folder/ubuntu-22.04.5-live-server-amd64.iso \
  --os-variant ubuntu22.04 \
  --network default \
  --graphics vnc,listen=127.0.0.1 \
  --check disk_size=off \
  --noautoconsole
```

### Clonar para VM2 y VM3

```bash
sudo virt-clone --original vm1-containerd --name vm2-podman --auto-clone
sudo virt-clone --original vm1-containerd --name vm3-docker-zot --auto-clone
```

### Obtener IPs de las VMs

```bash
sudo virsh domifaddr vm1-containerd
sudo virsh domifaddr vm2-podman
sudo virsh domifaddr vm3-docker-zot
```

Anotarlas (ej: 192.168.122.10, 192.168.122.11, 192.168.122.12)

---

## Instalación de Runtimes

### VM1: Instalar Containerd

```bash
sudo virsh start vm1-containerd
ssh ubuntu@192.168.122.10

sudo apt update
sudo apt install -y containerd.io
sudo systemctl start containerd
sudo systemctl enable containerd

# Verificar
containerd --version
```

### VM2: Instalar Podman

```bash
sudo virsh start vm2-podman
ssh ubuntu@192.168.122.11

sudo apt update
sudo apt install -y podman
sudo systemctl start podman
sudo systemctl enable podman

# Verificar
podman --version
```

### VM3: Instalar Docker

```bash
sudo virsh start vm3-docker-zot
ssh ubuntu@192.168.122.12

sudo apt update
sudo apt install -y docker.io
sudo systemctl start docker
sudo systemctl enable docker

# Verificar
docker --version
```

### VM3: Instalar Zot Registry

```bash
# En VM3
cd /tmp
wget https://github.com/project-zot/zot/releases/download/v2.0.0/zot-linux-amd64
chmod +x zot-linux-amd64

sudo mkdir -p /var/lib/zot
sudo chown ubuntu:ubuntu /var/lib/zot

cat > ~/zot-config.json << 'EOL'
{
  "distSpecVersion": "1.1.0",
  "storage": {
    "rootDirectory": "/var/lib/zot"
  },
  "http": {
    "address": "0.0.0.0",
    "port": "5000"
  },
  "log": {
    "level": "debug"
  }
}
EOL

sudo /tmp/zot-linux-amd64 serve ~/zot-config.json &

# Verificar
curl http://localhost:5000/v2/
```

---

## Construcción de las APIs

### Desde el Host, en ~/TU_CARNET_LAB_SO1_2S2026/

```bash
cd api1 && docker build -t api1-202503517:latest .
cd ../api2 && docker build -t api2-202503517:latest .
cd ../api3 && docker build -t api3-202503517:latest .

# Verificar
docker images | grep 202503517
```

---

## Exportar e Importar Imágenes a VMs

### Exportar desde Host

```bash
docker save api1-202503517:latest > /tmp/api1-202503517.tar
docker save api2-202503517:latest > /tmp/api2-202503517.tar
docker save api3-202503517:latest > /tmp/api3-202503517.tar
```

### Importar en VM1 (Containerd)

```bash
scp /tmp/api1-202503517.tar ubuntu@192.168.122.10:/tmp/
scp /tmp/api2-202503517.tar ubuntu@192.168.122.10:/tmp/

# En VM1
ssh ubuntu@192.168.122.10
sudo ctr -n k8s.io images import /tmp/api1-202503517.tar
sudo ctr -n k8s.io images import /tmp/api2-202503517.tar
sudo ctr -n k8s.io images ls
```

### Importar en VM2 (Podman)

```bash
scp /tmp/api3-202503517.tar ubuntu@192.168.122.11:/tmp/

# En VM2
ssh ubuntu@192.168.122.11
sudo podman load -i /tmp/api3-202503517.tar
sudo podman images
```

### Importar en VM3 (Docker)

```bash
scp /tmp/api1-202503517.tar ubuntu@192.168.122.12:/tmp/
scp /tmp/api2-202503517.tar ubuntu@192.168.122.12:/tmp/
scp /tmp/api3-202503517.tar ubuntu@192.168.122.12:/tmp/

# En VM3
ssh ubuntu@192.168.122.12
sudo docker load -i /tmp/api1-202503517.tar
sudo docker load -i /tmp/api2-202503517.tar
sudo docker load -i /tmp/api3-202503517.tar
sudo docker images
```

---

## Ejecución de Contenedores

### VM1 (Containerd)

```bash
ssh ubuntu@192.168.122.10

# Ejecutar API1 y API2
sudo ctr run -d --net-host api1-202503517:latest api1 /app/api1
sudo ctr run -d --net-host api2-202503517:latest api2 /app/api2

# Verificar
sudo ctr tasks ls
```

### VM2 (Podman)

```bash
ssh ubuntu@192.168.122.11

# Ejecutar API3
sudo podman run -d --net host api3-202503517:latest /app/api3

# Verificar
sudo podman ps
```

### VM3 (Docker)

```bash
ssh ubuntu@192.168.122.12

# Ejecutar APIs
sudo docker run -d --network host api1-202503517:latest /app/api1
sudo docker run -d --network host api2-202503517:latest /app/api2

# Verificar
sudo docker ps
```

---

## Pruebas de Comunicación

### Desde VM1

```bash
ssh ubuntu@192.168.122.10

# Probar endpoints locales
curl http://localhost:8001/health
curl http://localhost:8002/health

# Llamadas a APIs remotas
curl http://localhost:8001/api1/202503517/call-api3
curl http://localhost:8002/api2/202503517/call-api3
```

### Desde VM2

```bash
ssh ubuntu@192.168.122.11

# Probar endpoint local
curl http://localhost:8003/health

# Llamadas a APIs remotas
curl http://localhost:8003/api3/202503517/call-api1
curl http://localhost:8003/api3/202503517/call-api2
```

**Resultado esperado:** Todas las respuestas deben tener `"connection": true` y `"status": "UP"`.

---

## Troubleshooting

### Las APIs no se conectan entre VMs

**Solución:**
- Verificar que las IPs en el código coincidan con las reales
- Probar ping entre VMs: `ping 192.168.122.X`
- Verificar que los puertos 8001, 8002, 8003 estén abiertos

### Contenedores no inician

```bash
# VM1 (Containerd)
sudo ctr tasks list
sudo ctr logs <task-id>

# VM2 (Podman)
sudo podman logs <container-id>

# VM3 (Docker)
sudo docker logs <container-id>
```

### Zot no responde

```bash
# En VM3
curl http://localhost:5000/v2/
sudo netstat -tlnp | grep 5000
```

---

## Conclusión

El proyecto integra exitosamente:
- 3 máquinas virtuales independientes
- 3 runtimes de contenedores diferentes
- APIs comunicándose entre VMs via HTTP/REST
- Registro privado de imágenes (Zot)

