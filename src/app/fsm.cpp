#include "fsm.h"
#include "led.h"
#include "button.h"
#include "uart.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

static FsmState appState = FsmState::INIT; 	// Stan początkowy INIT
static IdlePhase idlePhase = IdlePhase::RED; 		// Faza początkowa dla GŁÓWNYCH LEDÓW = czerwony
static RunPhase runPhase = RunPhase::SEC_RED; 		// Faza początkowa dla LEDÓW = czerwony
static TickType_t phaseStart = 0; 					// Czas rozpoczęcia bieżącej fazy
static TickType_t safeBlinkStart = 0;				// Czas rozpoczęcia migania w trybie SAFE
static bool safeYellowOn = false; 					// Stan żółtego LED w trybie SAFE

// --- NEW: tracking total times and state entry time
static TickType_t stateEntryTime = 0;               // czas wejścia do bieżącego stanu
static TickType_t totalIdle = 0;
static TickType_t totalRun  = 0;
static TickType_t totalSafe = 0;

// Czas trwania faz/przejść w ms (przekonwertowane na TickType_t)
static const TickType_t dur_red = pdMS_TO_TICKS(3000);
static const TickType_t dur_y1  = pdMS_TO_TICKS(1000);
static const TickType_t dur_green = pdMS_TO_TICKS(3000);
static const TickType_t dur_y2  = pdMS_TO_TICKS(2000);
static const TickType_t dur_safe_blink = pdMS_TO_TICKS(400);

// Uruchamia akcje wejściowe przy ustawieniu stanu
static void on_state_enter(FsmState state, TickType_t now)
{
	switch (state) {
		case FsmState::IDLE:
			idlePhase = IdlePhase::RED;
			phaseStart = now;
			safeYellowOn = false;
			set_main_led(true, false, false);	// główne światła pokażą CZERWONE przy wejściu do IDLE
			set_ped_led(true, false); 			// piesi zawsze CZERWONE w IDLE
			break;

		case FsmState::RUN:
			runPhase = RunPhase::SEC_RED;
			phaseStart = now;
			set_main_led(true, false, false); 	// włącz tylko główne czerwone światło podczas RUN 
			set_ped_led(true, false);			// piesi zaczynają od CZERWONEGO, potem zmienią się w AKCJI
			break;

		case FsmState::SAFE:
			safeBlinkStart = now;
			safeYellowOn = false;
			set_main_led(false, false, false);	// zacznij z wszystkimi głównymi wyłączonymi, "miganie" obsługiwane w AKCJI
			set_ped_led(false, false); 			// ledy pieszych wyłączone
			break;

		case FsmState::FAULT:
			// wyłącz wszystko w FAULT
			set_main_led(false, false, false);
			set_ped_led(false, false);
			break;

		default:
			break;
	}
}
// Dostęp stanu FSM.
FsmState fsm_get_state()
{
	return static_cast<FsmState>(appState);
}

// Aktualizacja stanu FSM.
void fsm_set_state(FsmState s)
{
	// oblicz upływający czas dla poprzedniego stanu (tylko dla IDLE/RUN/SAFE)
	FsmState prev = appState;
	TickType_t now = xTaskGetTickCount();
	if (stateEntryTime != 0) {
		TickType_t elapsed = now - stateEntryTime;
		switch (prev) {
			case FsmState::IDLE:
				totalIdle += elapsed;
				uart_log_line("SYS: Exited IDLE, total time: ");
				uart_log_int(pdTICKS_TO_MS(totalIdle));
				uart_log_line(" ms\n\r");
				break;
			case FsmState::RUN:
				totalRun += elapsed;
				uart_log_line("SYS: Exited RUN, total time: ");
				uart_log_int(pdTICKS_TO_MS(totalRun));
				uart_log_line(" ms\n\r");
				break;
			case FsmState::SAFE:
				totalSafe += elapsed;
				uart_log_line("SYS: Exited SAFE, total time: ");
				uart_log_int(pdTICKS_TO_MS(totalSafe));
				uart_log_line(" ms\n\r");
				break;
			default:
				//brak akcji dla INIT/FAULT
				break;
		}
	}

	// uataw nowy stan i zaktualizuj czas wejścia, następnie uruchom akcje wejściowe z 'now'
	appState = s;
	stateEntryTime = now;
	on_state_enter(s, now);
}

// Czyszczenie kolejki zdarzeń przycisku i zwrócenie ostatniego zdarzenia (jeśli istnieje)
static bool clean_event_queue(QueueHandle_t q, ButtonEvent &out)
{
	if (q == NULL) return false;
	ButtonEvent tmp;
	bool in = false;
	while (xQueueReceive(q, &tmp, 0) == pdTRUE) {
		out = tmp;
		in = true;
	}
	return in;
}

void fsm_init()
{
	while (true) {
		TickType_t now = xTaskGetTickCount();
		ButtonEvent lastEvent;
		bool hasEvent = false;
		QueueHandle_t q = get_button_queue();

		// Jeśli kolejka zniknęła, przejdź do FAULT
		if (q == NULL) {
			if (fsm_get_state() != FsmState::FAULT) {
				fsm_set_state(FsmState::FAULT);
				uart_log_line("SYS: FAULT MODE\n\r");
			}
		} else {
			// opróżnij kolejkę aby uzyskać ostatnie zdarzenie
			hasEvent = clean_event_queue(q, lastEvent);
		}

		// Odczytaj aktualny zrzut i podejmij decyzje na podstawie najnowszych informacji
		FsmState cur = fsm_get_state();

		if (cur != FsmState::FAULT && hasEvent && lastEvent.type == ButtonEventType::LongPress) {
			// przełącz SAFE <-> IDLE
			if (cur == FsmState::SAFE) {
				fsm_set_state(FsmState::IDLE);
				uart_log_line("SYS: IDLE MODE\n\r");
			} else {
				fsm_set_state(FsmState::SAFE);
				uart_log_line("SYS: SAFE MODE\n\r");
			}
		}

		// ponowne sprawdzenie stanu po możliwej zmianie
		cur = fsm_get_state();

		if (cur == FsmState::IDLE && hasEvent && lastEvent.type == ButtonEventType::ShortPress) {
			// przejście IDLE -> RUN po krótkim naciśnięciu
			fsm_set_state(FsmState::RUN);
			uart_log_line("SYS: RUN MODE\n\r");
		}

		// --- LOGIKA ZMIANY LED (Akcje) ---
		//ponowne sprawdzenie stanu po możliwej zmianie
		cur = fsm_get_state();
		switch (cur) {
			case FsmState::INIT:
			// przejście INIT -> IDLE przy pierwszym uruchomieniu
				fsm_set_state(FsmState::IDLE);
				break;

			case FsmState::IDLE: {
				TickType_t elapsed = now - phaseStart;  		// czas w bieżącej fazie
				switch (idlePhase) {
					case IdlePhase::RED: 						// po czerwonym przejdź do YELLOW1
						if (elapsed >= dur_red) { 
							idlePhase = IdlePhase::YELLOW1; 	// przejdź do YELLOW1
							phaseStart = now; 					// zresetuj czas fazy
							set_main_led(false, true, false); 	// ustaw GŁÓWNY na żółty
						}
						break;
					case IdlePhase::YELLOW1:
						if (elapsed >= dur_y1) {
							idlePhase = IdlePhase::GREEN; 		// przejdź do GREEN
							phaseStart = now; 					// zresetuj czas fazy
							set_main_led(false, false, true); 	// ustaw GŁÓWNY na zielony
						}
						break;
					case IdlePhase::GREEN:
						if (elapsed >= dur_green) {
							idlePhase = IdlePhase::YELLOW2; 	// przejdź do YELLOW2
							phaseStart = now; 					// zresetuj czas fazy
							set_main_led(false, true, false); 	// ustaw GŁÓWNY na żółty
						}
						break;
					case IdlePhase::YELLOW2: 
						if (elapsed >= dur_y2) {
							idlePhase = IdlePhase::RED; 		// przejdź do RED
							phaseStart = now;   				// zresetuj czas fazy
							set_main_led(true, false, false);	// ustaw GŁÓWNY na czerwony
						}
						break;
				}
				break;
			}

			case FsmState::RUN: {
				TickType_t elapsed = now - phaseStart;
				if (runPhase == RunPhase::SEC_RED) {
					if (elapsed >= dur_red) {
						runPhase = RunPhase::SEC_GREEN; // przejdź do SEC_GREEN
						phaseStart = now;				// zresetuj czas fazy
						set_ped_led(false, true); 		// zmień światło pieszych na ZIELONE
					}
				} else {
					if (elapsed >= dur_green) {
						set_ped_led(true, false); 		// zmień światło pieszych na CZERWONE
						fsm_set_state(FsmState::IDLE); 	// powrót do IDLE po zakończeniu fazy zielonej	
						uart_log_line("SYS: IDLE MODE\n\r");
					}
				}

				break;
			}

			case FsmState::SAFE: {
				// miganie żółtym co dur_safe_blink
				if ((now - safeBlinkStart) >= dur_safe_blink) {
					safeBlinkStart = now;						// czas na przełączenie stanu migania
					safeYellowOn = !safeYellowOn; 				// przełącz stan żółtego LED
					set_main_led(false, safeYellowOn, false); 	// migaj żółtym
				}
				break;
			}

			case FsmState::FAULT:
				// krótkie opóźnienie w FAULT
				vTaskDelay(pdMS_TO_TICKS(200));
				break;
		}

		// aby uniknąć zapętlenia gdy nie jesteśmy w FAULT
		if (fsm_get_state() != FsmState::FAULT) {
			vTaskDelay(pdMS_TO_TICKS(10));
		}
	}
}