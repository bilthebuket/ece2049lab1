#ifndef DISPLAY_H
#define DISPLAY_H

void display_welcome(void);
void display_countdown(void);
void display_display_pattern(int round);
void display_game_over(void);
void display_get_pattern(int round);
void display_success(int round);
void display_pressed_button(int button);
void display_button_history(int* buttons, int num);

#endif