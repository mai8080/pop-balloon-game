/*
 * Balloon pop game: move the red triangle with left/right arrows to pop
 * balloons before they reach the top. If one gets to the top, game over.
 */

#include "raylib.h"
#include <stdio.h>

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600
#define NUM_BALLOONS  10
#define BALLOON_RADIUS 24.0f /* pixles in float format */
#define BALLOON_SPEED  2.2f /* pixels per frame */
#define BALLOON_SPACING 85.0f

/* Colours for the sky background (gradient from top to bottom) */
#define SKY_TOP     CLITERAL(Color){ 135, 206, 235, 255 } /* Create litral used by raylib o make code clear and portable */
#define SKY_BOTTOM  CLITERAL(Color){ 176, 224, 230, 255 } /* red, gree, blue, alpha (trancperancy) */

/*
 * Returns true if the circle (balloon) touches the triangle (player).
 * We check: is the circle's centre inside the triangle, or does the circle touches any of the three edges?
 */
static bool check_balloon_touches_player(Vector2 balloon_centre, float radius,
                                         Vector2 tri_left, Vector2 tri_bottom, Vector2 tri_right)
{
    if (CheckCollisionPointTriangle(balloon_centre, tri_left, tri_bottom, tri_right))
        return true;
    if (CheckCollisionCircleLine(balloon_centre, radius, tri_left, tri_bottom))
        return true;
    if (CheckCollisionCircleLine(balloon_centre, radius, tri_bottom, tri_right))
        return true;
    if (CheckCollisionCircleLine(balloon_centre, radius, tri_right, tri_left))
        return true;
    return false;
}

int main(void)
{
    /* --- Setup: window and frame rate --- */
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pop baloon");

    /* Initialize the audio device right after the window */
    InitAudioDevice(); 
    
    /* Load sounds and music */
    Music bg_music = LoadMusicStream("music.mp3"); /* this is the background music */
    Sound pop_sound = LoadSound("pop.wav"); /* this is the sound when a balloon is popped */
    Sound gameover_sound = LoadSound("gameover.wav"); /* this is the sound when the game is over */
    
    /* Start the background music */
    PlayMusicStream(bg_music);

    /* Setup window and frame rate */
    SetWindowState(FLAG_WINDOW_RESIZABLE);  /* allow user to resize window by dragging edges */
    SetWindowMinSize(400, 300);             /* prevent window from getting too small */
    SetTargetFPS(60);
    SetRandomSeed(0);

    /* List of colours we use for balloons (no sky blue, no red so they stay visible) */
    const Color colours_for_balloons[10] = {
        { 0, 228, 48, 255 },   /* green */
        { 0, 255, 0, 255 },   /* lime */
        { 255, 255, 0, 255 }, /* yellow */
        { 255, 203, 0, 255 }, /* gold */
        { 255, 165, 0, 255 }, /* orange */
        { 255, 192, 203, 255 },/* pink */
        { 255, 0, 255, 255 }, /* magenta */
        { 160, 32, 240, 255 }, /* purple */
        { 238, 130, 238, 255 },/* violet */
        { 245, 245, 220, 255 } /* beige */
    };
    const int numBalloonColors = 10;

    /* Player triangle: position and size. Triangle points down, base at top of screen. */
    float triangle_top_y = 0.0f;
    float triangle_tip_y = 70.0f;
    float triangle_half_width = 50.0f;
    float player_speed = 9.0f;
    float player_x = SCREEN_WIDTH / 2.0f;

    /* Balloon positions and colours. We store x, y and colour for each balloon. */
    float balloon_pos_x[NUM_BALLOONS];
    float balloon_pos_y[NUM_BALLOONS];
    Color balloon_colour[NUM_BALLOONS];
    for (int i = 0; i < NUM_BALLOONS; i++)
    {
        balloon_pos_x[i] = (float)(GetRandomValue((int)BALLOON_RADIUS, (int)(SCREEN_WIDTH - BALLOON_RADIUS)));
        balloon_pos_y[i] = SCREEN_HEIGHT + (float)(i * (int)BALLOON_SPACING);
        balloon_colour[i] = colours_for_balloons[GetRandomValue(0, numBalloonColors - 1)];
    }

    /* Game state */
    bool is_game_over = false;
    int popped_count = 0;
    double time_at_start = GetTime();
    double time_when_finished = 0.0;

    /* --- Main loop: runs every frame until the user closes the window --- */
    while (!WindowShouldClose())
    {
        /* start the background music stream while the game is running */      
        UpdateMusicStream(bg_music);

        if (!is_game_over)
        {
            /* Read keyboard and move the player left/right */
            if (IsKeyDown(KEY_LEFT))
                player_x -= player_speed;
            if (IsKeyDown(KEY_RIGHT))
                player_x += player_speed;
            /* Keep player inside the screen (use current window size when resized) */
            if (player_x < triangle_half_width)
                player_x = triangle_half_width;
            if (player_x > GetScreenWidth() - triangle_half_width)
                player_x = (float)GetScreenWidth() - triangle_half_width;

            /* Triangle corners for this frame (used for collision and drawing) */
            Vector2 tri_left   = { player_x - triangle_half_width, triangle_top_y };
            Vector2 tri_right  = { player_x + triangle_half_width, triangle_top_y };
            Vector2 tri_bottom = { player_x, triangle_tip_y };

            /* Update each balloon: move up, check if it reached the top or hit the player */
            for (int i = 0; i < NUM_BALLOONS; i++)
            {
                balloon_pos_y[i] -= BALLOON_SPEED; /* move balloon up by BALLOON_SPEED pixels */

                /* Balloon reached the top of the screen => game over */
                if (balloon_pos_y[i] + BALLOON_RADIUS < 0)
                {
                    is_game_over = true;
                    time_when_finished = GetTime() - time_at_start;
                    StopMusicStream(bg_music); /* Stop background music on game over */
                    PlaySound(gameover_sound);  /* Start game over sound exactly once when the game ends */
                    break; /* stop checking other balloons */
                }

                /* If balloon touches the player, we "pop" it: resend at bottom with new position and colour */
                Vector2 balloon_centre = { balloon_pos_x[i], balloon_pos_y[i] };
                if (check_balloon_touches_player(balloon_centre, BALLOON_RADIUS, tri_left, tri_bottom, tri_right))
                {
                    popped_count++;
                    PlaySound(pop_sound); /* Start the pop sound */
                    balloon_pos_x[i] = (float)(GetRandomValue((int)BALLOON_RADIUS, (int)(GetScreenWidth() - BALLOON_RADIUS)));
                    balloon_pos_y[i] = (float)GetScreenHeight() + BALLOON_RADIUS + (float)GetRandomValue(0, (int)(BALLOON_SPACING * 4));
                    balloon_colour[i] = colours_for_balloons[GetRandomValue(0, numBalloonColors - 1)];
                }
            }
        }

        /* --- Drawing: clear with sky, then draw either game-over screen or game --- */
        BeginDrawing();
        DrawRectangleGradientV(0, 0, GetScreenWidth(), GetScreenHeight(), SKY_TOP, SKY_BOTTOM);

        if (is_game_over)
        {
            DrawText("GAME OVER", GetScreenWidth()/2 - 120, GetScreenHeight()/2 - 60, 40, RED);
            DrawText("A balloon reached the top!", GetScreenWidth()/2 - 160, GetScreenHeight()/2 - 10, 20, WHITE);

            /* Try Again button: draw it and check if the mouse is on it */
            Rectangle try_again_button = { GetScreenWidth()/2.0f - 100, GetScreenHeight()/2.0f + 30, 200, 50 };
            Vector2 mouse_pos = GetMousePosition();
            bool is_mouse_over_button = CheckCollisionPointRec(mouse_pos, try_again_button);

            DrawRectangleRec(try_again_button, is_mouse_over_button ? LIME : DARKGREEN); /* draw the button with the colour of the mouse */
            DrawRectangleLinesEx(try_again_button, 2, BLACK); /* draw the border of the button */
            DrawText("Try Again", (int)try_again_button.x + 45, (int)try_again_button.y + 14, 22, WHITE); /* draw the text of the button */

            if (is_mouse_over_button && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                is_game_over = false;
                player_x = GetScreenWidth() / 2.0f;
                popped_count = 0; /* reset the popped count */
                time_at_start = GetTime();

                /* Restart background music from the beginning. */
                SeekMusicStream(bg_music, 0.0f);
                PlayMusicStream(bg_music);

                for (int i = 0; i < NUM_BALLOONS; i++) /* reset the balloons */
                {
                    balloon_pos_x[i] = (float)(GetRandomValue((int)BALLOON_RADIUS, (int)(GetScreenWidth() - BALLOON_RADIUS))); /* reset the x position of the balloon */
                    balloon_pos_y[i] = (float)GetScreenHeight() + (float)(i * (int)BALLOON_SPACING); /* reset the y position of the balloon */
                    balloon_colour[i] = colours_for_balloons[GetRandomValue(0, numBalloonColors - 1)]; /* reset the colour of the balloon */
                }
            }

            /* Show final score and time (timer stopped when game ended) */
            char stats_text[64];
            snprintf(stats_text, sizeof(stats_text), "Popped: %d  |  Time: %.0f s", popped_count, time_when_finished);
            DrawText(stats_text, GetScreenWidth()/2 - 140, GetScreenHeight()/2 + 100, 18, DARKGRAY);
        }
        else
        {
            /* Draw the player triangle (red, pointing down) */
            Vector2 tri_left   = { player_x - triangle_half_width, triangle_top_y };
            Vector2 tri_right  = { player_x + triangle_half_width, triangle_top_y };
            Vector2 tri_bottom = { player_x, triangle_tip_y };
            DrawTriangle(tri_left, tri_bottom, tri_right, RED);
            DrawTriangleLines(tri_left, tri_bottom, tri_right, MAROON);

            /* Draw all balloons that are still on screen */
            for (int i = 0; i < NUM_BALLOONS; i++)
            {
                if (balloon_pos_y[i] + BALLOON_RADIUS >= 0)
                    DrawCircle((int)balloon_pos_x[i], (int)balloon_pos_y[i], BALLOON_RADIUS, balloon_colour[i]);
            }

            /* On-screen stats: how many popped and time so far */
            char hud_text[80];
            double time_elapsed = GetTime() - time_at_start;
            snprintf(hud_text, sizeof(hud_text), "Popped: %d", popped_count);
            DrawText(hud_text, 12, 12, 22, DARKBLUE);
            snprintf(hud_text, sizeof(hud_text), "Time: %.1f s", time_elapsed);
            DrawText(hud_text, GetScreenWidth() - 120, 12, 22, DARKBLUE);
        }

        EndDrawing();
    }

    /* Clean audios before closing the window */
    UnloadMusicStream(bg_music);
    UnloadSound(pop_sound);
    UnloadSound(gameover_sound);
    CloseAudioDevice();

    CloseWindow();
    return 0;
}
