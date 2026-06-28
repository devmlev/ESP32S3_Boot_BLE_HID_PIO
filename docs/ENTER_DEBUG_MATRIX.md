# Enter Debug Matrix

Objetivo: descobrir qual método de Enter é confiável com a biblioteca `HijelHID_BLEKeyboard`.

Todos os testes ficam em:

```text
include/project_config.h
```

## Teste A — Enter como tecla real com releaseAll

```cpp
#define ENTER_SEND_MODE ENTER_MODE_KEY_RETURN_RELEASE_ALL
#define ENTER_PRE_GUARD_MS 100
#define ENTER_KEY_HOLD_MS 20
#define ENTER_POST_GUARD_MS 50
```

Resultado esperado:

```text
1234567890 Marcel Levinspuhl
1234567890 Marcel Levinspuhl
1234567890 Marcel Levinspuhl
```

Se não compilar, a biblioteca não implementa `releaseAll()`.

## Teste B — Enter como tecla real com release(KEY_RETURN)

```cpp
#define ENTER_SEND_MODE ENTER_MODE_KEY_RETURN_RELEASE
#define ENTER_PRE_GUARD_MS 100
#define ENTER_KEY_HOLD_MS 20
#define ENTER_POST_GUARD_MS 50
```

Se o primeiro Enter funciona e os próximos não, suspeita: `release(KEY_RETURN)` não limpa corretamente o estado da biblioteca.

## Teste C — Enter como newline textual

```cpp
#define ENTER_SEND_MODE ENTER_MODE_PRINT_NEWLINE
#define ENTER_PRE_GUARD_MS 100
#define ENTER_POST_GUARD_MS 50
```

Se falhar, aumente:

```cpp
#define ENTER_PRE_GUARD_MS 150
```

ou:

```cpp
#define ENTER_PRE_GUARD_MS 200
```

## Regra de calibração

1. Não mexer no texto enquanto testa o Enter.
2. Testar 20 acionamentos seguidos.
3. Registrar se perdeu Enter, duplicou texto ou falhou após o primeiro uso.
4. Só reduzir `ENTER_PRE_GUARD_MS` depois de 20/20 testes corretos.
