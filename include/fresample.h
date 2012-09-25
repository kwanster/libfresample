/* Copyright 2012 Dietrich Epp <depp@zdome.net> */
#ifndef FRESAMPLE_H
#define FRESAMPLE_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#define LFR_RESTRICT
#define LFR_PUBLIC
#if defined(LFR_IMPLEMENTATION)
# define LFR_PRIVATE
#endif

#if defined(_MSC_VER)
# undef LFR_RESTRICT
# define LFR_RESTRICT __restrict
#endif

#if defined(__GNUC__)
# undef LFR_RESTRICT
# define LFR_RESTRICT __restrict
# undef LFR_PRIVATE
# undef LFR_PUBLIC
# if defined(LFR_IMPLEMENTATION)
#  if defined(__ELF__)
#   define LFR_PRIVATE __attribute__((visibility("internal")))
#   define LFR_PUBLIC __attribute__((visibility("protected")))
#  else
#   define LFR_PRIVATE __attribute__((visibility("hidden")))
#   define LFR_PUBLIC __attribute__((visibility("default")))
#  endif
# else
#  define LFR_PUBLIC
# endif
#endif

#if defined(__STDC_VERSION__)
# if __STDC_VERSION__ >= 199901L
#  undef LFR_RESTRICT
#  define LFR_RESTRICT restrict
# endif
#endif

/*
  CPU features to use or disable.
*/
enum {
    LFR_CPU_MMX    = (1u << 0),
    LFR_CPU_SSE    = (1u << 1),
    LFR_CPU_SSE2   = (1u << 2),
    LFR_CPU_SSE3   = (1u << 3),
    LFR_CPU_SSSE3  = (1u << 4),
    LFR_CPU_SSE4_1 = (1u << 5),
    LFR_CPU_SSE4_2 = (1u << 6),

    LFR_CPU_NONE = 0,
    LFR_CPU_ALL = 0xffffffff
};

/*
  Set which CPU features are allowed or disallowed.  This is primarily
  used for comparing the performance and correctness of vector
  implementations and scalar implementations.  It can also be used to
  prohibit features that your CPU supports but which your OS does not.

  Returns the CPU flags actually enabled, which will be the
  intersection of the set of allowed flags (the argument) with the set
  of features that the current CPU actually supports.
*/
LFR_PUBLIC unsigned
lfr_setcpufeatures(unsigned flags);

/*
  Names for filter quality presets.
*/
enum {
    LFR_QUALITY_LOW,
    LFR_QUALITY_MEDIUM,
    LFR_QUALITY_HIGH,
    LFR_QUALITY_ULTRA
};

/*
  A low-pass filter for resampling 16-bit integer audio.
*/
struct lfr_s16;

/*
  Free a low-pass filter.
*/
LFR_PUBLIC void
lfr_s16_free(struct lfr_s16 *fp);

/*
  Create a new windowed sinc filter with the given parameters.
  Normally this function is not called directly.

  nsamp: filter size, in samples

  log2nfilt: base 2 logarithm of the number of filters

  cutoff: cutoff frequency, in cycles per sample

  beta: Kaiser window beta parameter

  Returns NULL when out of memory.
*/
LFR_PUBLIC struct lfr_s16 *
lfr_s16_new_sinc(
    int nsamp, int log2nfilt, double cutoff, double beta);

/*
  Create a new low-pass filter for 16-bit data with the given
  parameters: sample rate, pass band frequency, stop band frequency,
  and signal to noise ratio.  Frequencies are measured in Hz, the SNR
  is measured in dB.  The SNR is clipped at 96 due to limitations when
  working with 16-bit data.  Normally, this function is not called
  directly.

  Returns NULL when out of memory.

  This function works by calling lfr_s16_new_sinc().
*/
LFR_PUBLIC struct lfr_s16 *
lfr_s16_new_lowpass(
    double f_rate, double f_pass,
    double f_stop, double snr);

/*
  Create a new low-pass filter for 16-bit data for resampling with the
  given parameters: input sample rate, output sample rate, signal to
  noise ratio, transition width, and "loose" flag.

  The input and output frequencies are measured in Hz.

  The signal to noise ratio is measured in dB, and clipped at 96 dB
  due to limitations when working with 16-bit data.

  The transition width is specified as a fraction of the input sample
  rate.  This allows controlling the trade-off between bandwidth and
  filter size.  Note that the transition band will be narrowed
  (increasing filter size) to ensure that the pass band is at least
  50% of the input or output bandwidth, whichever is smaller.  The
  transition band will also be widened (reducing filter size) to limit
  the pass band to 20 kHz, as there's no sense in wasting CPU cycles
  to preserve ultrasonic frequencies.

  The "loose" flag, when set, allows the stop band to start at higher
  frequencies.  This will create ultrasonic artifacts above the noise
  floor.  Most people probably won't hear them, but some people have
  better hearing, and poor audio systems can modulate ultrasonics to
  audible frequencies.

  Returns NULL when out of memory.

  This function works by calling lfr_s16_new_lowpass().
*/
LFR_PUBLIC struct lfr_s16 *
lfr_s16_new_resample(
    int f_inrate, int f_outrate,
    double snr, double transition, int loose);

/*
  Create a new low-passs filter for 16-bit data for resampling between
  the given sample rates.  The filter quality is an integer in the
  range 0..3 which specifies the filter quality.  Quality levels 2 and
  3 are considered high quality.  The predefined LFR_QUALITY constants
  can be used here.

  Returns NULL when out of memory.

  This function works by calling lfr_s16_new_resample().
*/
LFR_PUBLIC struct lfr_s16 *
lfr_s16_new_preset(
    int f_inrate, int f_outrate, int quality);

/*
  Resample 16-bit integer audio.  Lengths are measured in frames, not
  samples.
*/
LFR_PUBLIC void
lfr_s16_resample_mono(
    short *LFR_RESTRICT out, size_t outlen, int outrate,
    const short *LFR_RESTRICT in, size_t inlen, int inrate,
    const struct lfr_s16 *LFR_RESTRICT filter);

LFR_PUBLIC void
lfr_s16_resample_stereo(
    short *LFR_RESTRICT out, size_t outlen, int outrate,
    const short *LFR_RESTRICT in, size_t inlen, int inrate,
    const struct lfr_s16 *LFR_RESTRICT filter);

#ifdef __cplusplus
}
#endif
#endif
