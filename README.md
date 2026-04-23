# Glep-Doll

## Requirements

- CMake ≥ 3.13  
- ARM GCC Toolchain (`arm-none-eabi-gcc`)  
- Git  
- Raspberry Pi Pico SDK  
- Pico Extras (required for `pico_sleep`)  
- Ninja or Make  

### Environment Variables

Linux / macOS:

```bash
export PICO_SDK_PATH=/path/to/pico-sdk
export PICO_EXTRAS_PATH=/path/to/pico-extras
```

Windows (PowerShell):

```powershell
$env:PICO_SDK_PATH="C:\path\to\pico-sdk"
$env:PICO_EXTRAS_PATH="C:\path\to\pico-extras"
```

---

## Setup

Clone this repository:

```bash
git clone [<your-repo-url>](https://github.com/Bruno-Noobador/Glep-Doll.git)
cd Glep-Doll
```

Clone LittleFS into the project (required if not already present):

```bash
git clone https://github.com/littlefs-project/littlefs.git
```

---

## Compilation

Create and enter build directory:

```bash
mkdir build
cd build
```

Configure project:

```bash
cmake ..
```

(Optional: use Ninja)

```bash
cmake -G Ninja ..
```

Build:

```bash
ninja
```

or

```bash
make
```

---

## Output

The compiled firmware will be generated as:

```bash
glep_firmware.uf2
```
