//
//  main.c
//  helloTable
//
//  Created by Sansi Rajalingam on 22/11/2020.
//  Copyright © 2020 Sansi Rajalingam. All rights reserved.
//

#include <stdio.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define PI 3.14159265
#define TABLE_LEN 512
#define SINE 0
#define SQUARE 1
#define SAW 2
#define TRIANGLE 3

float table[TABLE_LEN];

void fill_sine() {
    int j;
    for (j = 0; j < TABLE_LEN; j++) {
        table[j] = (float) sin(2 * PI * j/SAMPLE_RATE);
    }
}

void fill_square() {
    int j;
    for (j = 0; j < TABLE_LEN/2; j++) {
        table[j] = 1;
    }
    for (j = TABLE_LEN/2; j < TABLE_LEN; j++) {
        table[j] = -1;
    }
}

void fill_saw() {
    int j;
    for (j = 0; j < TABLE_LEN; j++) {
        table[j] = 1 - (2 * (float) j / (float) TABLE_LEN);
    }
}

void fill_triangle() {
    int j;
    for (j = 0; j < TABLE_LEN/2; j++) {
        table[j] = 2 * (float) j / (float) (TABLE_LEN/2) - 1;
    }
    for (j = TABLE_LEN/2; j < TABLE_LEN; j++) {
        table[j] = 1- (2 * (float) (j - TABLE_LEN/2) / (float) (TABLE_LEN/2));
    }
}

#ifndef REALTIME
#include <tinyAudioLib.h>
#elif defined(BINARY_RAW_FILE)
#include <portsf.h>
PSF_PROPS props;
int ofd;
#endif

void outSample(float sample) {
#ifdef REALTIME
    outSampleMono(sample);
#elif defined(BINARY_RAW_FILE)
    short isample = (short) (sample * 32000);
    fwrite(&isample, sizeof(short), 1, file);
#elif defined(WAVE_FILE)
    psf_sndWriteFloatFrames(ofd, &sample, 1);
#else
    printf("%f\n", sample);
#endif
}

void init() {
#ifdef REALTIME
    tinyInit();
#elif defined(BINARY_RAW_FILE)
    file = fopen("hellotable.raw", "wb");
#elif defined(WAVE_FILE)
    props.srate = 44100;
    props.chans = 1;
    props.samptype = PSF_SAMP_16;
    props.format = PSF_STDWAVE;
    props.chformat = STDWAVE;
    psf_init();
    ofd = psf_sndCreate("hellotable.wav", &props, 0, 0, PSF_CREATE_RDWR);
#else
    printf("\nNothing to initialise.\n");
#endif
}

void cleanup() {
    printf("Cleaning up...\n");
#ifdef REALTIME
    tinyExit();
#elif defined(BINARY_RAW_FILE)
    fclose(file);
#elif defined(WAVE_FILE)
    {
        int err1, err2;
        err1 = psf_sndClose(ofd);
        err2 = psf_finish();
        if (err1 || err2) {
            printf("Warning! An error occured while writing WAV file.\n");
        }
    }
#else
    printf("Nothing to clean up...\n");
#endif
}

void main()
{
    int waveform;
    const float frequency, duration;
    
    printf("Type the frequency of the wave to output in Hz, and press ENTER: ");
    scanf("%f", &frequency);
    
    printf("\nType the duration of tone in seconds, and press ENTER: ");
    scanf("%f", &duration);
    
wrong_waveform:
    printf("\nType a number between 0 and 3 corresponding to the type of waveform\n");
    printf("(0 = sine, 1 = square, 2 = saw, 3 = triangle), and press ENTER: ");
    scanf("%d", &waveform);
    if (waveform < 0 || waveform > 3) {
        printf("Please type a valid number.\n");
        goto wrong_waveform;
    }
    
    switch (waveform) {
        case SINE:
            printf("You've chosen a sine wave\n");
            fill_sine();
            break;
        case SQUARE:
            printf("You've chosen a square wave\n");
            fill_square();
            break;
        case SAW:
            printf("You've chosen a saw wave\n");
            fill_saw();
            break;
        case TRIANGLE:
            printf("You've chosen a triangle wave\n");
            fill_triangle();
            break;
        default:
            printf("Inavlid type. Ending proram.\n");
            return;
    }
    
    init();
    
    {
        int j;
        double sample_increment = frequency * TABLE_LEN / SAMPLE_RATE;
        double phase = 0;
        float sample;
        
        for (j = 0; j < duration * SAMPLE_RATE; j++) {
            sample = table[(long) phase];
            outSample(sample);
            phase += sample_increment;
            if (phase > TABLE_LEN) phase -= TABLE_LEN;
        }
    }
    
    cleanup();
    printf("End of process.\n");
}
