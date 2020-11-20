#include "lib.h"
#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define FPS 60
#define WIDTH 640
#define HEIGHT 480

ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_EVENT_QUEUE *event_queue = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_FONT *retroFont = NULL;

int initAllegro() {
    // Initialize allegro
    if (!al_init()) {
        fprintf(stderr, "Failed to initialize allegro.\n");
        return 1;
    }

    // Initialize the timer
    timer = al_create_timer(1.0 / FPS);
    if (!timer) {
        fprintf(stderr, "Failed to create timer.\n");
        return 1;
    }

    // Initialize keyboard
    if (!al_install_keyboard()){
        fprintf(stderr, "Failed to initialize keyboard.\n");
        return 0;
    }

    // Initialize the font add-on
    al_init_font_addon();

    // Initialize the tff add-on
    if (!al_init_ttf_addon()){
        fprintf(stderr, "Failed to initialize allegro_ttf add-on.\n");
        return 1;
    }

    // Initialize the font
    retroFont = al_load_font("fonts/retroGaming.ttf", 60, 0);
    if (!retroFont) {
        fprintf(stderr, "Failed to load font.\n");
        return 1;
    }

    // Create the display
    display = al_create_display(640, 480);
    if (!display) {
        fprintf(stderr, "Failed to create display.\n");
        return 1;
    }

    al_set_window_title(display, "Jogo sem Título");

    // Initialize the mouse
    if (!al_install_mouse()){
        fprintf(stderr, "Failed to initialize mouse.\n");
        return 1;
    }
    if (!al_set_system_mouse_cursor(display, ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT)){
        fprintf(stderr, "Failed to attribuite mouse pointer.\n");
        return 1;
    }

    // Create the event queue
    event_queue = al_create_event_queue();
    if (!event_queue) {
        fprintf(stderr, "Failed to create event queue.");
        return 1;
    }

    // Register event sources
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_timer_event_source(timer));
    al_register_event_source(event_queue, al_get_mouse_event_source());
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    return 0;
}

int main(int argc, char *argv[]) {
    printf("O segredo do universo é %d\n", SegredoDoUniverso());

    int err = initAllegro();
    if(err) {
        return err;
    }

    bool running = true;
    bool redraw = true;

    // Display a black screen
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();

    // Start the timer
    al_start_timer(timer);

    // Game loop
    while (running) {
        ALLEGRO_EVENT event;
        ALLEGRO_TIMEOUT timeout;

        // Initialize timeout
        al_init_timeout(&timeout, 0.06);

        // Fetch the event (if one exists)
        bool get_event = al_wait_for_event_until(event_queue, &event, &timeout);

        // Handle the event
        if (get_event) {
            switch (event.type) {
                case ALLEGRO_EVENT_TIMER:
                    redraw = true;
                    break;
                case ALLEGRO_EVENT_DISPLAY_CLOSE:
                    running = false;
                    break;
                default:
                    fprintf(stderr, "Unsupported event received: %d\n", event.type);
                    break;
            }
        }

        // Check if we need to redraw
        if (redraw && al_is_event_queue_empty(event_queue)) {
            // Redraw
            al_clear_to_color(al_map_rgb(0xff, 0xff, 0xff));
            al_flip_display();
            redraw = false;
        }
    } 

    // Clean up
    al_destroy_display(display);
    al_destroy_event_queue(event_queue);
    al_destroy_font(retroFont);

    return 0;
}