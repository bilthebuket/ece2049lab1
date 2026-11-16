#include <stdlib.h>
#include <stdbool.h>
#include "display.h"
#include "dwt_module.h"
#include "stm32h5xx_hal_def.h"
#include "stm32h5xx_hal_rcc_ex.h"
#include "stm32h5xx_hal_rng.h"
#include "Adafruit_TFTShield18_Macros.h"
#include "Adafruit_TFTShield18_API.h"
#include "game.h"
#include "shared.h"
#include "display.h"

#define INITIAL_ROUND 1

int game_round = INITIAL_ROUND;
int game_state = STATE_WELCOME;
int* game_current_pattern = NULL;

#define GAME_TRANSITION_DELAY 500
#define GAME_ON_DELAY 1000
#define GAME_OFF_DELAY 50
#define GAME_OVER_DELAY 50
#define GAME_OVER_NUM_FLASHES 20

#define DELAY_DECREASE_CUTOFF 5

int get_game_on_delay(int round)
{
    int val = ((int) log2((double) round)) + 1;
    if (val > DELAY_DECREASE_CUTOFF)
    {
        return GAME_ON_DELAY / DELAY_DECREASE_CUTOFF;
    }
    else
    {
        return GAME_ON_DELAY / val;
    }
}

bool check_for_restart(void)
{
    if (!(TFTShield18_readButtons() & TFTSHIELD_BUTTON_IN))
    {
        return true;
    }
    return false;
}

void set_led(int led, bool on)
{
    switch (led)
    {
        case GREEN:
        {
            if (on)
            {
                GREEN_LED_PORT->ODR |= GREEN_LED_OUTPUT;
            }
            else
            {
                GREEN_LED_PORT->ODR &= ~GREEN_LED_OUTPUT;
            }
            break;
        }
        case BLUE:
        {
            if (on)
            {
                BLUE_LED_PORT->ODR |= BLUE_LED_OUTPUT;
            }
            else
            {
                BLUE_LED_PORT->ODR &= ~BLUE_LED_OUTPUT;
            }
            break;
        }
        case RED:
        {
            if (on)
            {
                RED_LED_PORT->ODR |= RED_LED_OUTPUT;
            }
            else
            {
                RED_LED_PORT->ODR &= ~RED_LED_OUTPUT;
            }
            break;
        }
        case YELLOW:
        {
            if (on)
            {
                YELLOW_LED_PORT->ODR |= YELLOW_LED_OUTPUT;
            }
            else
            {
                YELLOW_LED_PORT->ODR &= ~YELLOW_LED_OUTPUT;
            }
            break;
        }
    }
}

int poll_for_input(void)
{
    while (1)
    {
        if (GREEN_BUTTON_PORT->IDR & GREEN_BUTTON_INPUT)
        {
            return GREEN;
        }
        if (BLUE_BUTTON_PORT->IDR & BLUE_BUTTON_INPUT)
        {
            return BLUE;
        }
        if (RED_BUTTON_PORT->IDR & RED_BUTTON_INPUT)
        {
            return RED;
        }
        if (YELLOW_BUTTON_PORT->IDR & YELLOW_BUTTON_INPUT)
        {
            return YELLOW;
        }
        if (check_for_restart())
        {
            return QUIT;
        }
    }
}

void game_init(void)
{
    BLUE_LED_PORT->MODER &= ~BLUE_LED_MODE;
    BLUE_LED_PORT->MODER |= BLUE_LED_MODE_0;
    GREEN_LED_PORT->MODER &= ~GREEN_LED_MODE;
    GREEN_LED_PORT->MODER |= GREEN_LED_MODE_0;
    RED_LED_PORT->MODER &= ~RED_LED_MODE;
    RED_LED_PORT->MODER |= RED_LED_MODE_0;
    YELLOW_LED_PORT->MODER &= ~YELLOW_LED_MODE;
    YELLOW_LED_PORT->MODER |= YELLOW_LED_MODE_0;

    BLUE_BUTTON_PORT->MODER &= ~BLUE_BUTTON_MODE;
    BLUE_BUTTON_PORT->PUPDR &= ~BLUE_BUTTON_PUPD;
    BLUE_BUTTON_PORT->PUPDR |= BLUE_BUTTON_PUPD_1;
    GREEN_BUTTON_PORT->MODER &= ~GREEN_BUTTON_MODE;
    GREEN_BUTTON_PORT->PUPDR &= ~GREEN_BUTTON_PUPD;
    GREEN_BUTTON_PORT->PUPDR |= GREEN_BUTTON_PUPD_1;
    RED_BUTTON_PORT->MODER &= ~RED_BUTTON_MODE;
    RED_BUTTON_PORT->PUPDR &= ~RED_BUTTON_PUPD;
    RED_BUTTON_PORT->PUPDR |= RED_BUTTON_PUPD_1;
    YELLOW_BUTTON_PORT->MODER &= ~YELLOW_BUTTON_MODE;
    YELLOW_BUTTON_PORT->PUPDR &= ~YELLOW_BUTTON_PUPD;
    YELLOW_BUTTON_PORT->PUPDR |= YELLOW_BUTTON_PUPD_1;
}

void buzzer_on(Buzzer* b, int color)
{
    switch (color)
    {
        case GREEN:
        {
            buzzerSetNote(b, NOTE_A, 0);
            break;
        }
        case BLUE:
        {
            buzzerSetNote(b, NOTE_B, 0);
            break;
        }
        case RED:
        {
            buzzerSetNote(b, NOTE_C, 0);
            break;
        }
        case YELLOW:
        {
            buzzerSetNote(b, NOTE_D, 0);
            break;
        }
    }
    buzzerEnable(b);
}

void buzzer_off(Buzzer* b)
{
    buzzerDisable(b);
}

void update_high_score(int round)
{
    for (int i = 0; i < NUM_HIGH_SCORES; i++)
    {
        if (high_scores[i] < round)
        {
            for (int j = NUM_HIGH_SCORES - 1; j > i; j--)
            {
                high_scores[j] = high_scores[j - 1];
            }
            high_scores[i] = round;
            return;
        }
    }
}

void game_welcome(void)
{
    DWT_delayMs(GAME_TRANSITION_DELAY);
    while (!check_for_restart()) {}
    display_countdown();
    game_state = STATE_DISPLAY_PATTERN;
    game_round = INITIAL_ROUND;
}

void game_over(RNG_HandleTypeDef* hrng, Buzzer* b)
{
    for (int i = 0; i < GAME_OVER_NUM_FLASHES; i++)
    {
        uint32_t rand;
        HAL_StatusTypeDef status = HAL_RNG_GenerateRandomNumber(hrng, &rand);
        if (status != HAL_OK)
        {
            return;
        }
        int color = rand % NUM_LEDS;
        set_led(color, true);
        buzzer_on(b, color);
        DWT_delayMs(GAME_OVER_DELAY);
        set_led(color, false);
        buzzer_off(b);
    }
    update_high_score(game_round - 1);
    game_state = STATE_WELCOME;
}

void game_display_pattern(RNG_HandleTypeDef* hrng, Buzzer* b)
{
    game_current_pattern = malloc(sizeof(int) * game_round);
    if (game_current_pattern == NULL)
    {
        game_state = STATE_WELCOME;
        return;
    }

    for (int i = 0; i < game_round; i++)
    {
        uint32_t rand;
        HAL_StatusTypeDef status = HAL_RNG_GenerateRandomNumber(hrng, &rand);
        if (status != HAL_OK || check_for_restart())
        {
            free(game_current_pattern);
            game_state = STATE_WELCOME;
            return;
        }
        game_current_pattern[i] = rand % NUM_LEDS;
        set_led(game_current_pattern[i], true);
        buzzer_on(b, game_current_pattern[i]);
        DWT_delayMs(get_game_on_delay(game_round));
        set_led(game_current_pattern[i], false);
        buzzer_off(b);
        DWT_delayMs(GAME_OFF_DELAY);
    }
    game_state = STATE_GET_PATTERN;
    DWT_delayMs(GAME_TRANSITION_DELAY);
}

void game_get_pattern(Buzzer* b)
{
    display_pressed_button(NO_BUTTON);
    for (int i = 0; i < game_round; i++)
    {
        int color = poll_for_input();
        if (color == QUIT)
        {
            free(game_current_pattern);
            game_state = STATE_WELCOME;
            return;
        }
        int store_color = game_current_pattern[i];
        game_current_pattern[i] = color;
        set_led(color, true);
        buzzer_on(b, color);
        display_pressed_button(color);
        if (i < BUTTON_HISTORY_NUM)
        {
            display_button_history(&game_current_pattern[0], i + 1);
        }
        else
        {
            display_button_history(&game_current_pattern[i - BUTTON_HISTORY_NUM + 1], BUTTON_HISTORY_NUM);
        }
        DWT_delayMs(get_game_on_delay(game_round));
        set_led(color, false);
        buzzer_off(b);
        display_pressed_button(NO_BUTTON);
        game_current_pattern[i] = store_color;
        if (color != game_current_pattern[i])
        {
            free(game_current_pattern);
            game_state = STATE_GAME_OVER;
            return;
        }
    }
    display_success(game_round);
    game_round++;
    game_state = STATE_DISPLAY_PATTERN;
    free(game_current_pattern);
    DWT_delayMs(GAME_TRANSITION_DELAY);
}