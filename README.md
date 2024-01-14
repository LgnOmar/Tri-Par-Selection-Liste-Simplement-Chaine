This was a homework for our Algorithms professor, this is a sort visualizer using the selection sort algorithm using C and a simple GUI with Raylib with simple features such as procedurally generated audio..


I wanted it to be something special so the special touches would land on the precedural_audio.c file! to be clear: this software does NOT
use any pirth party files for audio, as that would be impossible to implement in the way we implemented the audio, the audio is smooth and it is generated many time throughout the excution of the software, it deals directly with the system audio, the system audio directly gives the function the needed paramaters, whenerver the audio system needs data the 'SetAudioStreamCallback' function fills the buffer witih audio samples!   CONSLUSION: the SetAudioStreamCallback functions is invoked directly by the audio system.

We used the Sine function to generate audio frequencies and eventually producde a list or a structure called SoundList, contains:
    - WaveForm: The waveform of the sound
    - Volume: The volume of the sound
    - Value:The value of the array item represented by this sound to be converted into its frequency; should be between 0 and 1 */
    - Duration: The duration of the sound; how long it should sustain (in seconds)
    - Elapsed: An internal variable whose initial value should be set to 0 representing the amound of time (in seconds) elapsed since the sound begun playing.
    - Amplitude: An internal variable whose initial value should be set to 1 representing the portion of the sound's amplitude that should remain at this point.
    
In the process of making this software we reffered to many resources such as GTP4, Bard, Various Github Repos:
    https://github.com/psycotica0/tone-generator            //This is a tone generator by the user "psycotica0" using SDL 
    https://gist.github.com/gcatlin/0dd61f19d40804173d015c01a80461b8   //This is another example with sample rate of 48000Hz [We used 41000]
    https://github.com/bingmann/sound-of-sorting //github of Sound Of Sort
    https://panthema.net/2013/sound-of-sorting/  //better idea of the sound generator and how it is done
    
    
    small detail: in the previous repos i found different documentation comments methods such as the one used in Doxygen documentation, please note that i did not delete the commands such as @brief, @param or @return .. etc for transparecy and just because it  facilitates easy generation of documentation if needed in the future, and it visually distinguishes commands from plain text, making comments easier to scan and understand.

    
    
    
    LAGGOUNE Ahmed Amar | ISIL-B | G2 | 212135084638 |
    DJEGHLAF Soheib| ISIL-B | G2 | 212131099839 |
    HECHAM Idriss | ISIL-B | G2 | 212131086629 |