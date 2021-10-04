
/*
 * this example reads standard from input and writes to the default
 * PCM device or one can change it to hw:0,0 or plughw, and the data
 * is written for 5 seconds.
 */

/* use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <iostream>
#include <alsa/asoundlib.h>

int main() {
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;
    unsigned int tmp;

    /* open PCM device for playback */
    rc = snd_pcm_open(&handle, "plughw:devOnPort10", SND_PCM_STREAM_PLAYBACK, 0);

    if (rc < 0) {
        std::cout << "unable to open pcm device " << snd_strerror(rc) << std::endl;
        exit(1);
    }

    /* allocate a hardware parameters object, this object has invalid values */
    snd_pcm_hw_params_alloca(&params);

    /* fill the object with default values i.e fill params with a full configuration space for a PCM,
     * the configuration space will be filled with all possible ranges for the PCM device */
    snd_pcm_hw_params_any(handle, params);

    /* set the desired hardware parameters */

    /* interleaved mode */
    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);

    int channels(2);
    /* two channel(stereo = 2)(monno = 1) */
    snd_pcm_hw_params_set_channels(handle, params, channels);

    /* 44100 samples/second sampling rate (CD quality) */
    val = 44100; // 16000 as steve said
    snd_pcm_hw_params_set_rate_near(handle, params, &val, &dir);
    //snd_pcm_hw_params_set_rate(handle, params, val, dir);

    frames = 32;
    snd_pcm_hw_params_set_period_size_near(handle, params, &frames, &dir);

    /* write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        std::cout << "unable to set the hw parameters " << snd_strerror(rc) << std::endl;
        exit(0);
    }

    /* resume information */
    std::cout << "PCM name: " << snd_pcm_name(handle) << std::endl;
    std::cout << "PCM state: " << snd_pcm_state_name(snd_pcm_state(handle)) << std::endl;

    snd_pcm_hw_params_get_channels(params, &tmp);

    std::cout << "channels: " << tmp << std::endl;

    channels = tmp;

    if (tmp == 1) {
        std::cout << "mono audio." << std::endl;
    } else {
        std::cout << "stereo audio." << std::endl;
    }

    snd_pcm_hw_params_get_rate(params, &tmp, 0);
    std::cout << "rate: " << tmp << " bps" << std::endl;
   
    int seconds = 5;

    /* use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(params, &frames, &dir);

    size = frames * channels * 2; /* 2 bytes/sample */
    buffer = (char *)malloc(size);

    /* we want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);

    /* seconds=5 secs in microseconds divided by period time */
    loops = seconds * 1000000 / val;

    while (loops > 0) {
        --loops;

        // read from the std::cin.. like: ./a.out < xy.wav
        rc = read(0, buffer, size);
        if (rc == 0) {
            std::cout << "end of file on input" << std::endl;
            break;
        } else if (rc != size) {
            std::cout << "short read: read " << rc << "bytes" << std::endl;
        }

        rc = snd_pcm_writei(handle, buffer, frames);

        if (rc == -EPIPE) {
            /* EPIPE means underrun */
            std::cout << "underrun occurred" << std::endl;
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            std::cout << "error from writei: " << snd_strerror(rc);
        } else if (rc != (int)frames) {
            std::cout << "short write, write " << rc << " frames";
        }
    }

    // For playback stopping after all remaining frames in the buffer 
    // have finished playing and then stop the PCM. For Capture stop
    // a PCM permitting to retrieve residual frames.
    std::cout << "here 1" << std::endl;
    snd_pcm_drain(handle);
    std::cout << "here 2" << std::endl;
    // closes the specified PCM handle and frees all associated resources
    snd_pcm_close(handle);


    std::cout << "here 3" << std::endl;
    free(buffer);

    return 0;
}
