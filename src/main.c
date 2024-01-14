#include "raylib.h"
#include <pthread.h>
#include "Array.c"
#include "procedural_audio.c"
#include "font_data.h"
#include "algorithms/shuffle/StandardShuffle.c"
#include "algorithms/sort/SelectionSort.c"

/** How long the sound lasts when an array access is made */
#define SOUND_SUSTAIN 0.05f
/** What portion of the original color will remain 1 second after an array access */
#define COLOR_SUSTAIN 1e-1

/**
 * @brief The delay to wait every time the sorting algorithm makes an array access (in milliseconds)
 */
float array_access_delay = 2.f;

//number of arrays to display and sort!
int array_nmb = 128;


//If the macro _WIN32 is not defined (meaning we're not on a Windows platform), then include the code that follows up to the matching #endif directive.  on peut le supprimer mais what if you are running a macOS professor? idk could be helpful im just paranoid like that..
#ifndef _WIN32
void strcpy_s(char *restrict dest, size_t destsz, const char *restrict src)
{
    while (true)
    {
        *dest = *src;
        if (*dest == '\0')
            return;
        destsz--;
        if (destsz == 0ULL)
            return;
        src++;
        dest++;
    }
}
#endif

/**
 * @brief Used in the pause_for macro, which waits until clock() exceeds this value
 */
float pause;
/**
 * @brief Intended to be used in a single thread and no other. Waits until `ms` milliseconds since the last pause_for call.
 */
#define pause_for(ms)                    \
    pause += ms * CLOCKS_PER_SEC / 1000; \
    while (clock() < pause)              \
        sched_yield();

/**
 * @brief The `Array` that the sorting algorithms act on
 */
Array sort_array;

pthread_mutex_t sort_array_read_lock = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;
size_t sort_array_read_len = 0;
/** Keeps track of the array items that were recently read to for the purpose of generating the colors of the bars */
float *sort_array_reads = NULL;

pthread_mutex_t sort_array_write_lock = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;
size_t sort_array_write_len = 0;
/** Keeps track of the array items that were recently written to for the purpose of generating the colors of the bars */
float *sort_array_writes = NULL;

char status_text[256] = "";

size_t array_read_count = 0;
size_t array_write_count = 0;

/** @note It is assumed that corresponding mutices are locked when this macro is called */
#define correct_array_length(accesses, access_len, target_len)       \
    if (access_len != array->len)                                    \
    {                                                                \
        accesses = MemRealloc(accesses, target_len * sizeof(float)); \
        for (size_t i = access_len; i < target_len; i++)             \
            accesses[i] = 0.0f;                                      \
        access_len = target_len;                                     \
    }

#define push_array_access(mutex, accesses, access_len, waveform) \
    pthread_mutex_lock(mutex);                                   \
    correct_array_length(accesses, access_len, array->len);      \
    accesses[index] = (float)clock() / CLOCKS_PER_SEC;           \
    pthread_mutex_unlock(mutex);                                 \
    push_sound(waveform, array_access_delay / 500 / SOUND_SUSTAIN, (float)array->_arr[index] / array->len, SOUND_SUSTAIN);

void my_array_read_callback(Array array, size_t index)
{
    // matensach TODO: make things work with external arrays

    if (array == sort_array)
    {
        push_array_access(&sort_array_read_lock, sort_array_reads, sort_array_read_len, sine_wave);
        array_read_count++;
        pause_for(array_access_delay);
    }
}

void my_array_write_callback(Array array, size_t index)
{
    // matensach TODO: make things work with external arrays

    if (array == sort_array)
    {
        push_array_access(&sort_array_write_lock, sort_array_writes, sort_array_write_len, triangle_wave);
        array_write_count++;
        pause_for(array_access_delay);
    }
}

/** Helper function to interpolate between colors with a gamma of 2 */
Color interpolate_colors(Color from, Color to, float t)
{
    return (Color){
        sqrtf((to.r * to.r - from.r * from.r) * t + from.r * from.r),
        sqrtf((to.g * to.g - from.g * from.g) * t + from.g * from.g),
        sqrtf((to.b * to.b - from.b * from.b) * t + from.b * from.b),
        sqrtf((to.a * to.a - from.a * from.a) * t + from.a * from.a)};
}

/**
 * @brief Draws an `Array` onto the screen using Raylib
 */
void draw_array(Array array, int width, int height, int x, int y)
{
    const Color RECTANGLE_COLORS[4] = {WHITE, RED, BLUE, GREEN};

    float *reads = NULL;
    float *writes = NULL;

    if (array == sort_array)
    {
        pthread_mutex_lock(&sort_array_read_lock);
        pthread_mutex_lock(&sort_array_write_lock);

        correct_array_length(sort_array_reads, sort_array_read_len, array->len);
        correct_array_length(sort_array_writes, sort_array_write_len, array->len);
        float time = (float)clock() / CLOCKS_PER_SEC;
        reads = MemAlloc(array->len * sizeof(float));
        writes = MemAlloc(array->len * sizeof(float));
        for (size_t i = 0; i < array->len; i++)
        {
            reads[i] = powf(COLOR_SUSTAIN, time - sort_array_reads[i]);
            writes[i] = powf(COLOR_SUSTAIN, time - sort_array_writes[i]);
        }

        pthread_mutex_unlock(&sort_array_read_lock);
        pthread_mutex_unlock(&sort_array_write_lock);
    }

    for (size_t i = 0; i < array->len; i++)
    {
        int rect_height = (array->_arr[i] + 1) * height / array->len;
        int rect_left = i * width / array->len;
        int rect_right = (i + 1) * width / array->len - 1;
        if (rect_right - rect_left < 1)
            rect_right = rect_left + 1;
        Color rect_color = reads == NULL || writes == NULL || array != sort_array ? RECTANGLE_COLORS[0] : reads[i] > writes[i]
            ? interpolate_colors(RECTANGLE_COLORS[0], interpolate_colors(RECTANGLE_COLORS[1], RECTANGLE_COLORS[3], writes[i] / reads[i]), reads[i])
            : interpolate_colors(RECTANGLE_COLORS[0], interpolate_colors(RECTANGLE_COLORS[2], RECTANGLE_COLORS[3], reads[i] / writes[i]), writes[i]);
        DrawRectangle(x + rect_left, y + height - rect_height, rect_right - rect_left, rect_height, rect_color);
    }

    if (reads != NULL)
        MemFree(reads);
    if (writes != NULL)
        MemFree(writes);
}

/**
 * @brief Demonstrates a sorting algorithm
 *
 * @param sort The algorithm to demonstrate
 * @param array_size The size of the `Array` to demonstrate the algorithm on
 * @param delay The delay between array accesses
 * @param shuffle The algorithm used to shuffle the `Array` before sorting
 * @return Whether the demonstration was successful
 */
bool show_sort(Algorithm sort, size_t array_size, float delay, Algorithm shuffle)
{
    status_text[255] = '\0';

    pause_for(750.f);
    array_read_count = 0;
    array_write_count = 0;
    strcpy_s(status_text, 255, TextFormat("Initializing %llu-element array", array_size));
    float old_d = array_access_delay;
    array_access_delay = 0.f; // for instant array initialization
    Array_free(sort_array);
    sort_array = Array_new_init(array_size);
    array_access_delay = old_d;
    strcpy_s(status_text, 255, "");

    pause_for(750.f);
    array_read_count = 0;
    array_write_count = 0;
    SetRandomSeed(0);
    strcpy_s(status_text, 255, TextFormat("Shuffling: %s (%llu elements)", shuffle.name, array_size));
    old_d = array_access_delay;
    array_access_delay = 500.f / 4 / array_size; // 4 array accesses required per element when shuffling
    if (!shuffle.fun(sort_array))
        return false;
    array_access_delay = old_d;
    strcpy_s(status_text, 255, "");

    pause_for(750.f);
    array_read_count = 0;
    array_write_count = 0;
    SetRandomSeed(0);
    strcpy_s(status_text, 255, TextFormat("Sorting: %s (%llu elements)", sort.name, array_size));
    old_d = array_access_delay;
    array_access_delay = delay;
    if (!sort.fun(sort_array))
        return false;
    array_access_delay = old_d;
    strcpy_s(status_text, 255, "");

    return true;
}

/**
 * @brief The procedure used to demonstrate all the sorting algorithms
 * NOTE: The arguments and return value are not used; they are only there because this function is called in a new thread
 */
void *sort_proc(void *args)
{
    if (
        !show_sort(SelectionSort, array_nmb, 2.003f, StandardShuffle))
    {
        TraceLog(LOG_ERROR, "Sorting Visualizer: algorithm returned false; stopped prematurely");
        return NULL;
    }
    return NULL;
}

void draw_text_with_line_spacing(Font font, const char *text, Vector2 position, float font_size, float char_spacing, float line_spacing, Color tint) {
    int line_count;
    const char **lines = TextSplit(text, '\n', &line_count);
    for (int i = 0; i < line_count; i++)
        DrawTextEx(font, lines[i], (Vector2){position.x, position.y + i * line_spacing}, font_size, char_spacing, tint);
}

/** The window width before enabling fullscreen */
int previous_window_width = 640;
/** The window height before enabling fullscreen */
int previous_window_height = 480;

int main()
{
    SetTraceLogLevel(LOG_ALL);

    sort_array_reads = MemAlloc(0);
    sort_array_writes = MemAlloc(0);

    Array_set_at_callback(my_array_read_callback);
    Array_set_set_callback(my_array_write_callback);
    sort_array = Array_new_init(array_nmb);

    InitAudioDevice();
    initialize_procedural_audio();

    InitWindow(640, 480, "Sorting Visualizer");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(10, 10);
    SetWindowState(FLAG_VSYNC_HINT);

    int font_data_size;
    const unsigned char *font_data = DecompressData(COMPRESSED_FONT_DATA, sizeof(COMPRESSED_FONT_DATA), &font_data_size);
    Font font = LoadFontFromMemory(".ttf", font_data, font_data_size, 30, NULL, 0);
    MemFree((void *)font_data);

    pthread_t sort_thread;
    pause = clock();
    pthread_create(&sort_thread, NULL, sort_proc, NULL);

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_F11)) {
            if (IsWindowFullscreen())
            {
                ToggleFullscreen();
                SetWindowSize(previous_window_width, previous_window_height);
            }
            else
            {
                previous_window_width = GetScreenWidth();
                previous_window_height = GetScreenHeight();
                int monitor = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
                ToggleFullscreen();
            }
        }
        BeginDrawing();
        ClearBackground(BLACK);
        size_t array_runs = 1;
        for (size_t i = 1; i < sort_array->len; i++)
            if (sort_array->_arr[i] < sort_array->_arr[i - 1])
                array_runs++;
        draw_array(sort_array, GetScreenWidth() - 10, GetScreenHeight() - 10, 5, 5);
        draw_text_with_line_spacing(
            font,
            TextFormat("%s\nArray Accesses: %llu\n\t(%llu reads, %llu writes)\n%llu elements in array (%llu run%s)\nDelay: %.3fms",
                       status_text,
                       array_read_count + array_write_count,
                       array_read_count, array_write_count,
                       sort_array->len, array_runs, array_runs == 1 ? "" : "s",
                       array_access_delay),
            (Vector2){10, 10}, font.baseSize, 0, font.baseSize, WHITE);

        EndDrawing();
    }

    CloseWindow();

    deinitialize_procedural_audio();
    CloseAudioDevice();

    #ifndef SIGTERM
        #define SIGTERM 15
    #endif

    pthread_kill(sort_thread, SIGTERM);
    Array_free(sort_array);

    MemFree(sort_array_reads);
    MemFree(sort_array_writes);

    return 0;
}
