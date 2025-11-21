# ESP32 TOTP Authenticator

Autenticador TOTP (Time-based One-Time Password) basado en ESP32 con interfaz web para gestionar y generar códigos de autenticación de dos factores.

## 🎯 Características

- ✅ Generación de códigos TOTP RFC 6238 compatibles con Google Authenticator, Authy, etc.
- ✅ Interfaz web moderna y responsiva
- ✅ Almacenamiento persistente en NVS Flash
- ✅ Sincronización automática de hora vía NTP
- ✅ Soporte para múltiples servicios (hasta 20)
- ✅ Códigos que se actualizan cada 30 segundos
- ✅ Parser de URIs `otpauth://totp/...`
- ⏳ Escaneo de códigos QR con cámara (próximamente)

## 🔧 Hardware

- **Placa**: Freenove ESP32-WROVER-DEV
- **Cámara**: OV2640 (para futuras funcionalidades)
- **Conexión**: WiFi 2.4GHz

## 📦 Dependencias

```yaml
dependencies:
  espressif/esp32-camera: "^2.0.0"
  dlbeer/quirc: "~1.1.0"          # Para escaneo QR (futuro)
  espressif/mbedtls: "*"          # HMAC-SHA1
  espressif/json: "*"             # Parsing JSON
```

## 🚀 Instalación y Configuración

### 1. Configurar WiFi

Configura las credenciales WiFi usando el menú de configuración:

```bash
idf.py menuconfig
```

Navega a: **GMaker Configuration → WiFi Configuration**

- **WiFi SSID**: Nombre de tu red WiFi
- **WiFi Password**: Contraseña de tu red WiFi

Alternativamente, edita `sdkconfig` directamente:

```
CONFIG_GMAKER_WIFI_SSID="tu_red_wifi"
CONFIG_GMAKER_WIFI_PASSWORD="tu_contraseña"
```

### 2. Compilar y Flashear

```bash
# Configurar el proyecto (primera vez)
idf.py set-target esp32

# Compilar
idf.py build

# Flashear y monitorear
idf.py flash monitor
```

### 3. Conectarse a la Interfaz Web

Una vez iniciado, el ESP32 mostrará su IP en el monitor serial:

```
I (xxxx) wifi_helper: Got IP: 192.168.1.100
```

Abre tu navegador en: `http://192.168.1.100`

## 📱 Uso

### Agregar un Servicio

1. Abre la aplicación del servicio (GitHub, Google, etc.)
2. Activa la autenticación de dos factores (2FA)
3. Selecciona "Configurar manualmente" o "Ver clave"
4. Copia la URI completa que comienza con `otpauth://totp/...`
5. Pégala en el campo de entrada de la interfaz web
6. Haz clic en "Add"

**Ejemplo de URI:**
```
otpauth://totp/GitHub:usuario@email.com?secret=JBSWY3DPEHPK3PXP&issuer=GitHub
```

### Ver Códigos TOTP

1. Haz clic en cualquier servicio de la lista
2. Se mostrará el código de 6 dígitos
3. El código se actualiza automáticamente cada 30 segundos
4. Una barra de progreso indica el tiempo restante

### Eliminar un Servicio

1. Entra a la vista del código del servicio
2. Haz clic en "Delete"
3. Confirma la eliminación

## 🏗️ Arquitectura del Proyecto

```
main/
├── main.c                      # Punto de entrada
├── hardware/
│   ├── hardware.c/h           # Inicialización de hardware
│   ├── i2c_helper.c/h         # Helper I2C
│   └── wifi_helper.c/h        # Gestión WiFi
├── network/
│   ├── server.c/h             # Servidor HTTP
│   └── www/
│       └── index.html         # Interfaz web (SPA)
├── storage/
│   └── nvs_helper.c/h         # Abstracción NVS
├── totp/
│   ├── totp_engine.c/h        # Generación TOTP
│   ├── totp_storage.c/h       # Persistencia de servicios
│   └── totp_parser.c/h        # Parser de URIs otpauth://
└── utils/
    ├── base32.c/h             # Decodificador Base32
    └── ntp.c/h                # Sincronización NTP
```

## 🌐 API REST

### Listar Servicios
```http
GET /api/services
Response: [{"service_name":"GitHub","account":"user@email.com",...}]
```

### Agregar Servicio
```http
POST /api/services
Body: {"uri":"otpauth://totp/..."}
Response: {"success":true}
```

### Obtener Código TOTP
```http
GET /api/code/{index}
Response: {"code":"123456","remaining":25,"service":"GitHub"}
```

### Eliminar Servicio
```http
DELETE /api/services/{index}
Response: {"success":true}
```

## 🔒 Seguridad

- ⚠️ **Este proyecto es educativo/experimental**
- Los secrets TOTP se almacenan en NVS Flash sin cifrado adicional
- Asegúrate de que tu red WiFi sea segura
- No expongas el dispositivo a internet público sin autenticación adicional

## 🎯 Roadmap

- [ ] Escaneo de códigos QR con cámara OV2640
- [ ] Autenticación web (login/password)
- [ ] Backup/restore de servicios
- [ ] Soporte para HOTP (counter-based)
- [ ] Display físico para mostrar códigos sin WiFi
- [ ] Cifrado de secrets en NVS

## 📚 Referencias

- [RFC 6238 - TOTP](https://tools.ietf.org/html/rfc6238)
- [RFC 4226 - HOTP](https://tools.ietf.org/html/rfc4226)
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/)



