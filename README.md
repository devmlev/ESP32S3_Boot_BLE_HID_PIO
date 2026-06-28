# ESP32S3_Boot_BLE_HID_PIO

Workspace PlatformIO para VS Code.

## Nome do workspace

Crie/abra a pasta com este nome:

```text
ESP32S3_Boot_BLE_HID_PIO
```

No VS Code:

```text
File > Open Folder... > ESP32S3_Boot_BLE_HID_PIO
```

## Estrutura

```text
ESP32S3_Boot_BLE_HID_PIO/
├── platformio.ini
├── include/
│   └── project_config.h
├── src/
│   └── main.cpp
├── lib/
│   └── HijelHID_BLEKeyboard/
│       └── PUT_LIBRARY_HERE.txt
├── docs/
│   └── ENTER_DEBUG_MATRIX.md
└── .vscode/
    ├── extensions.json
    └── settings.json
```

## Passo crítico: biblioteca HijelHID_BLEKeyboard

A biblioteca `HijelHID_BLEKeyboard` precisa ser copiada do seu ambiente Arduino para:

```text
lib/HijelHID_BLEKeyboard/
```

A pasta final precisa conter o arquivo `.h`, por exemplo:

```text
lib/HijelHID_BLEKeyboard/src/HijelHID_BLEKeyboard.h
```

ou:

```text
lib/HijelHID_BLEKeyboard/HijelHID_BLEKeyboard.h
```

dependendo da estrutura original da biblioteca.

## Comandos no PlatformIO

No painel do PlatformIO:

```text
Build  = compilar
Upload = compilar + gravar na placa
Clean  = limpar build
```

Atalhos comuns:

```text
Ctrl + Alt + B = Build
Ctrl + Alt + U = Upload
```

## Arquivo principal de configuração

Edite este arquivo para mudar texto, Enter e tempos:

```text
include/project_config.h
```

Principais constantes:

```cpp
#define HID_TEXT_TO_TYPE "1234567890 Marcel Levinspuhl"
#define ENTER_SEND_MODE ENTER_MODE_KEY_RETURN_RELEASE_ALL
#define ENTER_PRE_GUARD_MS 100
#define ENTER_KEY_HOLD_MS 20
#define ENTER_POST_GUARD_MS 50
```

## Observação sobre Enter

O texto principal é enviado em bloco:

```cpp
keyboard.print(HID_TEXT_TO_TYPE);
```

O Enter é enviado separado, conforme `ENTER_SEND_MODE`.

Modo padrão:

```cpp
#define ENTER_SEND_MODE ENTER_MODE_KEY_RETURN_RELEASE_ALL
```

Se não compilar porque a biblioteca não tem `releaseAll()`, mude para:

```cpp
#define ENTER_SEND_MODE ENTER_MODE_KEY_RETURN_RELEASE
```

Se o Enter por tecla real não funcionar, teste:

```cpp
#define ENTER_SEND_MODE ENTER_MODE_PRINT_NEWLINE
```

## Erase flash

Não use erase total da flash para ajustes normais.

No desenvolvimento normal:

```text
Erase All Flash = Disabled
```
