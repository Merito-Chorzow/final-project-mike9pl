#include "uart.h"

static SemaphoreHandle_t s_uartMutex = NULL; // Mutex do synchronizacji dostępu do UART

// Wykonuje funkcję z blokadą mutexa UART
static void withLock(void (*fn)(void *), void *arg)
{
    // Jeśli mutex nie jest zainicjowany, wykonaj funkcję bez blokady
    if (s_uartMutex == NULL) {
        fn(arg); // Wykonaj przekazaną funkcję bez blokady
        return;
    }
    // Uzyskaj blokadę mutexa przed wykonaniem funkcji
    if (xSemaphoreTake(s_uartMutex, portMAX_DELAY) == pdTRUE) {
        fn(arg); // Wykonaj przekazaną funkcję
        xSemaphoreGive(s_uartMutex); // Zwolnij mutex po wykonaniu funkcji
    }
}

void uart_init(unsigned long baud)
{
    if (s_uartMutex == NULL) {
        s_uartMutex = xSemaphoreCreateMutex(); // Utwórz mutex, jeśli jeszcze nie istnieje
    }

    Serial.begin(baud); // Inicjalizuj UART z podaną prędkością transmisji
}

bool uart_read_byte(uint8_t &out)
{
    int v = Serial.read(); // Odczytaj bajt z UART
    if (v < 0) {
        return false; // Brak dostępnych danych do odczytu
    }
    out = static_cast<uint8_t>(v); // Przypisz odczytany bajt do wyjściowej referencji
    return true;
}
// Struktura argumentów dla funkcji zapisu bajtów
struct WriteBytesArg {
    const uint8_t *data;
    size_t len;
};
// Funkcja zapisu bajtów do UART
static void writeBytesFn(void *p)
{
    auto *a = static_cast<WriteBytesArg *>(p);  // Rzutuj argument na strukturę WriteBytesArg
    Serial.write(a->data, a->len);              // Zapisz bajty do UART
}
// Zapis bajtów do UART z blokadą mutexa
void uart_write_bytes(const uint8_t *data, size_t len)
{
    WriteBytesArg arg{data, len};               // Utwórz strukturę argumentów
    withLock(writeBytesFn, &arg);               // Wykonaj zapis z blokadą mutexa
}
// Struktura argumentów dla funkcji zapisu łańcucha znaków
struct WriteStrArg {
    const char *s;
};
// Funkcja zapisu łańcucha znaków do UART
static void writeStrFn(void *p)
{
    auto *a = static_cast<WriteStrArg *>(p);    // Rzutuj argument na strukturę WriteStrArg
    Serial.print(a->s);                         // Zapisz łańcuch znaków do UART
}
// Zapis łańcucha znaków do UART z blokadą mutexa
void uart_write_str(const char *s)
{
    WriteStrArg arg{s};                         // Utwórz strukturę argumentów
    withLock(writeStrFn, &arg);                 // Wykonaj zapis z blokadą mutexa
}
// Struktura argumentów dla funkcji logowania linii
struct LogLineArg {
    const char *s;
};
// Funkcja logowania linii do UART
static void logLineFn(void *p)
{
    auto *a = static_cast<LogLineArg *>(p); // Rzutuj argument na strukturę LogLineArg
    Serial.printf(a->s);                   // Zaloguj linię do UART
}
// Logowanie linii do UART z blokadą mutexa
void uart_log_line(const char *s)
{
    LogLineArg arg{s};                         // Utwórz strukturę argumentów
    withLock(logLineFn, &arg);                 // Wykonaj logowanie z blokadą mutexa
}

// Przeciążona funkcja logowania linii unsigned long 
void uart_log_int(unsigned long v)
{
    char buf[12];                           // Bufor na reprezentację łańcucha znaków liczby
    snprintf(buf, sizeof(buf), "%lu", v);   // Konwertuj liczbę na łańcuch znaków
    uart_log_line(buf);                     // Zaloguj linię do UART
}