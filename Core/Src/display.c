#include <math.h>
#include <string.h>
#include "Adafruit_ST7735_API.h"
#include "Adafruit_ST7735_Macros.h"
#include "dwt_module.h"
#include "shared.h"
#include "display.h"

#define MAX_NUM_DIGITS 10
#define CIRCLE_RADIUS 10
#define TEXT_HEIGHT_PIXELS 10

void put_string(int y, int x, const char* str)
{
    Display_setCursor(x, y);
    Display_print(str);
}

int chars_to_pixels(int chars)
{
    return chars * 5;
}

void put_string_middle(const char* str)
{
    put_string
    (
        Display_height() / 2,
        0,
        str
    );
}

void int_to_str(int num, char* buf)
{
    if (num == 0)
    {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    
    for (int i = 0; true; i++)
    {
        int divider = (int) pow(10, ((int) log10((double) num)));
        int digit = (num / divider) % 10;
        buf[i] = '0' + digit;
        num = num % divider;
        if (divider <= 1)
        {
            buf[i + 1] = '\0';
            return;
        }
    }
}

void put_round(int round)
{
    char buf[7 + MAX_NUM_DIGITS + 1];
    buf[0] = 'R';
    buf[1] = 'o';
    buf[2] = 'u';
    buf[3] = 'n';
    buf[4] = 'd';
    buf[5] = ':';
    buf[6] = ' ';
    int_to_str(round, &buf[7]);
    put_string(0,0,buf);
}

void display_welcome(void)
{
    Display_fillScreen(ST7735_BLACK);
    put_string(0,0,"Welcome to Simon Says");
    put_string(TEXT_HEIGHT_PIXELS,0,"High Scores:");
    for (int i = 0; i < NUM_HIGH_SCORES; i++)
    {
        char buf[3 + MAX_NUM_DIGITS + 1];
        buf[0] = '0' + i + 1;
        buf[1] = ':';
        buf[2] = ' ';
        int_to_str(high_scores[i], &buf[3]);
        put_string((i + 2) * TEXT_HEIGHT_PIXELS, 0, &buf[0]);
    }
}

void display_countdown(void)
{
    Display_fillScreen(ST7735_BLACK);
    put_string_middle("Starting in.. 3");
    DWT_delayMs(1000);
    Display_fillScreen(ST7735_BLACK);
    put_string_middle("Starting in.. 2");
    DWT_delayMs(1000);
    Display_fillScreen(ST7735_BLACK);
    put_string_middle("Starting in.. 1");
    DWT_delayMs(1000);
}

void display_display_pattern(int round)
{
    Display_fillScreen(ST7735_BLACK);
    put_round(round);
    put_string_middle("Remember the Pattern");
}

void display_game_over(void)
{
    Display_fillScreen(ST7735_BLACK);
    put_string_middle("Game Over");
}

void display_get_pattern(int round)
{
    Display_fillScreen(ST7735_BLACK);
    put_round(round);
    const char* str = "History:";
    // the 10 is an arbitrary amount of padding
    put_string(Display_height() - TEXT_HEIGHT_PIXELS, NUM_BUTTONS * 2 * CIRCLE_RADIUS + 10, str);
    put_string_middle("Enter the Pattern");
}

void display_success(int round)
{
    Display_fillScreen(ST7735_BLACK);
    put_round(round);
    put_string_middle("Pattern Correct");
}

int button_to_color(int button)
{
    switch (button)
    {
        default:
        {
            return 0;
            break;
        }
        case GREEN:
        {
            return ST7735_GREEN;
            break;
        }
        case BLUE:
        {
            return ST7735_BLUE;
            break;
        }
        case RED:
        {
            return ST7735_RED;
            break;
        }
        case YELLOW:
        {
            return ST7735_YELLOW;
            break;
        }
    }
}

void display_pressed_button(int button)
{
    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        Display_fillCircle
        (
            CIRCLE_RADIUS + i * CIRCLE_RADIUS * 2,
            Display_height() - CIRCLE_RADIUS,
            CIRCLE_RADIUS,
            ST7735_WHITE
        );
    }
    int color = button_to_color(button);
    if (!color)
    {
        return;
    }
    Display_fillCircle
    (
        CIRCLE_RADIUS + button * CIRCLE_RADIUS * 2,
        Display_height() - CIRCLE_RADIUS,
        CIRCLE_RADIUS,
        color
    );
}

void display_button_history(int* buttons, int num)
{
    for (int i = 0; i < num; i++)
    {
        Display_fillCircle(Display_width() - CIRCLE_RADIUS, Display_height() - CIRCLE_RADIUS * (2 * (num - i - 1) + 1), CIRCLE_RADIUS, button_to_color(buttons[i]));
    }
}