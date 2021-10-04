
#define ALSA_PCM_NEW_HW_PARAMS_API

#include <iostream>
#include <alsa/asoundlib.h>

int main() {
    long loops;
    int rc;
    int size;
    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    unsigned int val, bytesPerSample;
    int dir;
    snd_pcm_uframes_t frames;
    char *buffer;

    rc = snd_pcm_open(&handle, "plughw:devOnPort10", SND_PCM_STREAM_CAPTURE, 0);

    if (rc < 0) {
        std::cout << "uable to open pcm device : " << snd_strerror(rc) << 
        std::endl;
        exit(1);
    }

    snd_pcm_hw_params_alloca(&params);

    /* fill it in with default values */
    snd_pcm_hw_params_any(handle, params);

    snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
    bytesPerSample = 2;

    int channels(2);
    snd_pcm_hw_params_set_channels(handle, params, channels);

    val = 8000;
    snd_pcm_hw_params_set_rate(handle, params, val, dir);

    frames = 160;
    snd_pcm_hw_params_set_period_size(handle, params, frames, dir);

    /* write the parameters to the driver */
    rc = snd_pcm_hw_params(handle, params);
    if (rc < 0) {
        std::cout << "unable to set the hw parameters : " << snd_strerror(rc) << std::endl;

        exit(1);
    }

    /* use a buffer large enough to hold one period */
    //snd_pcm_hw_params_get_period_size(params, &frames, &dir);

    size = frames * channels * bytesPerSample; /* 2 bytes/sample, 2 channels */

    buffer = (char *) malloc(size);

    /* we want to loop for 5 seconds */
    snd_pcm_hw_params_get_period_time(params, &val, &dir);
    
    loops = 5 * 1000000 / val;

    while (loops > 0) {
        loops--;
        rc = snd_pcm_readi(handle, buffer, frames);
        if (rc == -EPIPE) {
            /* EPIPE means overrun */
            std::cout << "overrun occured\n";
            snd_pcm_prepare(handle);
        } else if (rc < 0) {
            std::cout << "error from read: " << snd_strerror(rc) << std::endl;
        } else if (rc != (int)frames) {
            std::cout << "short read, read " << rc << " frames" << std::endl;
        }

        rc = write(1, buffer, size);
        if (rc != size)
            std::cout << "short write: wrote " << rc << " bytes" << std::endl;
    }

    snd_pcm_drain(handle);
    snd_pcm_close(handle);

    free(buffer);

    return 0;

}
