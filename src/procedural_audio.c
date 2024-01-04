#include "raylib.h"
#include <string.h>
#include <math.h>
#include <pthread.h>

#define SAMPLE_RATE 44100

float sine_wave(float x)
{
    return sinf(2 * PI * x);
}

float triangle_wave(float x)
{
    float intermediate = fmod(x + .25f, 1.0f);
    return intermediate < .5f ? 4 * intermediate - 1 : -4 * intermediate + 3;
}

/* // Unused wave functions

float square_wave(float x) {
    return fmod(x, 1.0f) < .5f ? 1.0f : -1.0f;
}

float sawtooth_wave(float x) {
    return fmod(2 * x + 1, 2.0f) - 1.0f;
}

float no_wave(float x) {
    return 0.0f;
}
*/

/**
 * @brief Calculates sound frequency based on the value of the array element
 * @param value A float representing the value of the array element. Should be between 0 and 1
 * @return A float represting the frequency of the sound in hertz
 */
float frequency(float value)
{
    return 1320.0f * value;
}

/** A linked list representing a sound produced by the visualizer */
typedef struct SoundList
{
    /** The waveform of the sound
     * @param generic_parameter_name A float representing the number of waves plus the portion of a wave passed
     * @returns A float representing the height of the wave at the time inputted
     */
    float (*waveform)(float);
    /** The volume of the sound */
    float volume;
    /** The value of the array item represented by this sound to be converted into its frequency; should be between 0 and 1 */
    float value;
    /** The duration of the sound; how long it should sustain (in seconds) */
    float duration;
    /** An internal variable whose initial value should be set to 0 representing the amound of time (in seconds) elapsed since the sound begun playing */
    float elapsed;
    /** An internal variable whose initial value should be set to 1 representing the portion of the sound's amplitude that should remain at this point */
    float remaining_amplitude;
    /** The next item in the linked list */
    struct SoundList *next_item;
} *SoundList;

/** The linked list representing all currently active sounds */
SoundList sound_list = NULL;
/**
 * The mutex which should be locked when accessing or modifying `sound_list`
 * @see sound_list
 */
pthread_mutex_t sound_list_mutex = PTHREAD_ERRORCHECK_MUTEX_INITIALIZER;

/**
 * @brief Pushes a new sound to the start of the `sound_list` linked list
 * @param waveform The waveform of the new sound. This will become the new sound's `waveform` property
 * @param volume The volume of the new sound. This will become the new sound's `volume` property
 * @param value The value of the array item represented by the new sound to be converted into its frequency; should be between 0 and 1. This will become the new sound's `value` property
 * @param duration The duration (in seconds) of the new sound. This will become the new sound's `duration` property
 * @see sound_list
 * @see SoundList
 */
void push_sound(float (*waveform)(float), float volume, float value, float duration)
{
    pthread_mutex_lock(&sound_list_mutex);
    SoundList new_item = &((struct SoundList){waveform, volume, value, duration, 0.0f, 1.0f, sound_list});
    // new item data needs to be copied over to newly allocated memory because it will be overwritten next time this code is run, causing a circular reference
    sound_list = MemAlloc(sizeof(struct SoundList));
    memmove(sound_list, new_item, sizeof(struct SoundList));
    pthread_mutex_unlock(&sound_list_mutex);
}

/** Processes `sound_list` and generates the next audio sample */
short next_sample()
{
    float accumulated_amplitude = 0.0f;
    SoundList previous_item = NULL;
    pthread_mutex_lock(&sound_list_mutex);
    for (SoundList current_item = sound_list; current_item != NULL; current_item = current_item->next_item)
    {
        accumulated_amplitude += current_item->waveform(frequency(current_item->value) * current_item->elapsed) * current_item->volume * current_item->remaining_amplitude;

        current_item->remaining_amplitude -= 1.0f / current_item->duration / SAMPLE_RATE;

        if (current_item->remaining_amplitude >= 0)
        {
            current_item->elapsed += 1.0f / SAMPLE_RATE;
            previous_item = current_item;
            continue;
        }

        // delete the current item from sound_list

        SoundList sound_to_replace = current_item->next_item;
        MemFree(current_item);
        if (previous_item == NULL) // there is no previous item therefore i == sound_list
            sound_list = sound_to_replace;
        else
            previous_item->next_item = sound_to_replace;
        current_item = &(struct SoundList){.next_item = sound_to_replace}; // on the next iteration i becomes sound_to_replace
    }
    pthread_mutex_unlock(&sound_list_mutex);
    return accumulated_amplitude >= 1 ? 32767 : accumulated_amplitude < -1 ? -32768
                                                                           : accumulated_amplitude * 32768.0f;
}

/** Upon audio initialization, this function will be passed into the `SetAudioStreamCallback` function */
void audio_callback(void *buffer, unsigned int num_samples)
{
    for (unsigned int i = 0; i < num_samples; i++)
        ((short *)buffer)[i] = next_sample();
}

/** The audio stream to stream procedurally generated audio */
AudioStream audio_stream;

/** Initializes Raylib Sorting Visualizer's procedural audio (it is expected that InitAudioDevice is called first) */
void initialize_procedural_audio()
{
    audio_stream = LoadAudioStream(SAMPLE_RATE, 16, 1);
    SetAudioStreamCallback(audio_stream, audio_callback);
    PlayAudioStream(audio_stream);
}

/** Deinitializes Raylib Sorting Visualizer's procedural audio (it is expected that CloseAudioDevice is called after) */
void deinitialize_procedural_audio()
{
    StopAudioStream(audio_stream);
    UnloadAudioStream(audio_stream);
}
