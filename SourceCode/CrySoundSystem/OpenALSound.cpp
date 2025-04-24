#ifndef LINUX64
#include "CrySound.h"
#else
#include "CrySound64.h"
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <cstdio>
#include <vector>
#include "stb_vorbis.c"

#define MAX_SOUND_FILENAME 128

typedef struct
{
	ALuint buf;
	int flags;
	char filename[MAX_SOUND_FILENAME];
} ALSample_t;

ALCdevice* aldevice;
ALCcontext* alcontext;
#define MAX_SOURCES 10
ALuint sources[MAX_SOURCES];
std::vector<ALSample_t*> buffers;
CS_OPENCALLBACK my_fopen;
CS_CLOSECALLBACK my_fclose;
CS_READCALLBACK my_fread;
CS_SEEKCALLBACK my_fseek;
CS_TELLCALLBACK my_ftell;

#define SOURCE_OUT_OF_BOUNDS 0

ALSample_t* GetSampleFromName(const char* filename)
{
	for (size_t i = 0; i < buffers.size(); i++)
	{
		if (!strcmp(buffers[i]->filename, filename))
		{
			return buffers[i];
		}
	}

	return NULL;
}

int audio_next_available_source(void)
{
	int status;
	unsigned int i;

	for (i = 0; i < MAX_SOURCES; i++)
	{
		if (sources[i] == 0)
		{
			__builtin_trap();
			return -1;
		}
		alGetSourcei(sources[i], AL_SOURCE_STATE, &status);
		if (status != AL_PLAYING)
		{
			return (ALint)i;
		}
	}

	return -1;
}

DLL_API signed char     F_API CS_Init(int mixrate, int maxsoftwarechannels, unsigned int flags)
{
	aldevice = alcOpenDevice(0);
	alcontext = alcCreateContext(aldevice, 0);
	alcMakeContextCurrent(alcontext);
	alGenSources(MAX_SOURCES, sources);
	return 1;
}

DLL_API void            F_API CS_Close()
{
	printf("OpenAL: Deleting %lu buffers.\n", buffers.size());
	for (size_t i = 0; i < buffers.size(); i++)
	{
		alDeleteBuffers(1, &buffers[i]->buf);
		delete buffers[i];
	}
	buffers.clear();
	alDeleteSources(MAX_SOURCES, sources);
	alcMakeContextCurrent(0);
	if (alcontext) alcDestroyContext(alcontext);
	if (aldevice) alcCloseDevice(aldevice);
}

DLL_API signed char     F_API CS_SetOutput(int outputtype)
{
	return 0;
}
DLL_API signed char     F_API CS_SetDriver(int driver)
{
	return 0;
}
DLL_API signed char     F_API CS_SetMixer(int mixer)
{
	return 0;
}
DLL_API signed char     F_API CS_SetBufferSize(int len_ms)
{
	return 0;
}
DLL_API signed char     F_API CS_SetHWND(void *hwnd)
{
	return 0;
}
DLL_API signed char     F_API CS_SetMinHardwareChannels(int min)
{
	return 0;
}
DLL_API signed char     F_API CS_SetMaxHardwareChannels(int max)
{
	return 0;
}
DLL_API signed char     F_API CS_SetMemorySystem(void *pool, 
                                                     int poollen, 
                                                     CS_ALLOCCALLBACK   useralloc,
                                                     CS_REALLOCCALLBACK userrealloc,
                                                     CS_FREECALLBACK    userfree)
{
	return 0;
}

DLL_API signed char     F_API CS_SetFrequency(int channel, int freq)
{
	return 0;
}
DLL_API signed char     F_API CS_SetVolume(int channel, int vol)
{
	if (channel < 0 || channel >= MAX_SOURCES)
	{
		__builtin_trap();
		return SOURCE_OUT_OF_BOUNDS;
	}

	alSourcef(sources[channel], AL_GAIN, (float)vol / 255.0f);
	return 1;
}

DLL_API signed char     F_API CS_SetPan(int channel, int pan)
{
	return 0;
}

DLL_API signed char     F_API CS_SetMute(int channel, signed char mute)
{
	return 0;
}
DLL_API signed char     F_API CS_SetPriority(int channel, int priority)
{
	return 0;
}

DLL_API signed char     F_API CS_SetPaused(int channel, signed char paused)
{
	if (channel < 0 || channel >= MAX_SOURCES)
	{
		__builtin_trap();
		return SOURCE_OUT_OF_BOUNDS;
	}

	if (paused)
	{
		alSourcePause(sources[channel]);
	}
	else
	{
		alSourcePlay(sources[channel]);
	}

	return 1;
}
DLL_API signed char     F_API CS_SetLoopMode(int channel, unsigned int loopmode)
{
	return 0;
}
DLL_API signed char     F_API CS_SetCurrentPosition(int channel, unsigned int offset)
{
	return 0;
}

static void stream_read(void* dest, void** source, size_t size)
{
	memcpy(dest, *source, size);
	*source = (char*)*source + size;
}

static int audio_wav_from_data_MEM(void* p, int bufsize, ALuint* buf)
{
	ALenum ALformat;
	char header[4], wave_header[4], subchunk1[4], subchunk2[4];
	char* temp_buffer;
	unsigned int size, frequency, subchunk2size;
	unsigned short num_channels, bits_per_sample, format;
	*buf = 0;

	stream_read(&header, &p, sizeof(header));

	if (strncmp(header, "RIFF", 4))
	{
		return 1;
	}

	stream_read(&size, &p, sizeof(size));
	stream_read(&wave_header, &p, sizeof(wave_header));

	if (strncmp(wave_header, "WAVE", 4))
	{
		return 1;
	}

	stream_read(&subchunk1, &p, sizeof(subchunk1));

	if (strncmp(subchunk1, "fmt", 3))
	{
		return 1;
	}

	p = (char*)p + sizeof(unsigned int); /* Subchunk 1 size */
	stream_read(&format, &p, sizeof(format));

	if (format != 1)
	{
		return 1;
	}

	stream_read(&num_channels, &p, sizeof(num_channels));
	stream_read(&frequency, &p, sizeof(frequency));

	p = (char*)p + sizeof(unsigned int); /* ByteRate */
	p = (char*)p + sizeof(unsigned short); /* BlockAlign */

	stream_read(&bits_per_sample, &p, sizeof(bits_per_sample));

	switch (bits_per_sample)
	{
		case 8:
			ALformat = num_channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		break;
		case 16:
			ALformat = num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		break;
		default:
			return 1;
		break;
	}

	stream_read(&subchunk2, &p, sizeof(subchunk2));

	if (strncmp(subchunk2, "data", 4))
	{
		return 1;
	}

	stream_read(&subchunk2size, &p, sizeof(subchunk2size));

	temp_buffer = new char[subchunk2size];

	stream_read(temp_buffer, &p, subchunk2size);

	alGenBuffers(1, buf);
	alBufferData(*buf, ALformat, temp_buffer, subchunk2size, frequency);
	delete [] temp_buffer;
	return 0;
}

#ifndef LINUX64
DLL_API CS_SAMPLE* F_API CS_Sample_Load(int index, const char* name_or_data, unsigned int mode, int memlength)
#else
DLL_API CS_SAMPLE * F_API CS_Sample_Load(int index, const char *name_or_data, unsigned int mode, int offset, int length)
#endif
{
	ALuint thebuf;
	int ret;
	ALSample_t* samp = nullptr;
	if (mode & CS_LOADMEMORY)
	{
		ret = audio_wav_from_data_MEM(name_or_data, 0, &thebuf);
		if (ret == 0)
		{
			samp = new ALSample_t;
			samp->buf = thebuf;
			samp->flags = mode;
			strcpy(samp->filename, "<MEMORY>");

			buffers.push_back(samp);
			printf("OpenAL: There are now %i buffers.\n", buffers.size());
		}
		else
		{
			printf("OpenAL: Failed to load wav\n");
		}
	}
	else
	{
		__builtin_trap();
	}
	return (CS_SAMPLE*)samp;
}

DLL_API CS_SAMPLE* F_API CS_Sample_Alloc(int index, int length, unsigned int mode, int deffreq, int defvol, int defpan, int defpri)
{
	return 0;
}

DLL_API void            F_API CS_Sample_Free(CS_SAMPLE* sptr)
{
	std::vector<ALSample_t*>::iterator it;
	ALSample_t* samp = (ALSample_t*)sptr;

	for (it = buffers.begin(); it != buffers.end(); it++)
	{
		if (*it == samp)
		{
			alDeleteBuffers(1, &(*it)->buf);
			buffers.erase(it);
			return;
		}
	}
}
#if 0
DLL_API signed char     F_API CS_Sample_Upload(CS_SAMPLE* sptr, void* srcdata, unsigned int mode)
{
	return 0;
}

DLL_API signed char     F_API CS_Sample_Lock(CS_SAMPLE* sptr, int offset, int length, void** ptr1, void** ptr2, unsigned int* len1, unsigned int* len2)
{
	return 0;
}

DLL_API signed char     F_API CS_Sample_Unlock(CS_SAMPLE* sptr, void* ptr1, void* ptr2, unsigned int len1, unsigned int len2)
{
	return 0;
}

DLL_API int             F_API CS_GetError()
{
	return 0;
}
#endif
DLL_API float           F_API CS_GetVersion()
{
	return 3.74f;
}

DLL_API int             F_API CS_GetOutput()
{
	return 0;
}
#if 0
DLL_API void* F_API CS_GetOutputHandle()
{
	return 0;
}
#endif
DLL_API int             F_API CS_GetDriver()
{
	return 0;
}

DLL_API int             F_API CS_GetMixer()
{
	return 0;
}

DLL_API int             F_API CS_GetNumDrivers()
{
	return 1;
}

#ifndef LINUX64
DLL_API signed char* F_API CS_GetDriverName(int id)
#else
DLL_API const char *    F_API CS_GetDriverName(int id)
#endif
{
	if (id == 0)
	{
		return "OpenAL";
	}
	return nullptr;
}

DLL_API signed char     F_API CS_GetDriverCaps(int id, unsigned int* caps)
{
	return 0;
}

DLL_API int             F_API CS_GetOutputRate()
{
	return 0;
}

DLL_API int             F_API CS_GetMaxChannels()
{
	return 0;
}

DLL_API int             F_API CS_GetMaxSamples()
{
	return 0;
}

DLL_API int             F_API CS_GetSFXMasterVolume()
{
	return 0;
}

DLL_API int             F_API CS_GetNumHardwareChannels()
{
	return 0;
}

DLL_API int             F_API CS_GetChannelsPlaying()
{
	return 0;
}

DLL_API float           F_API CS_GetCPUUsage()
{
	return 0;
}

DLL_API void            F_API CS_GetMemoryStats(unsigned int* currentalloced, unsigned int* maxalloced)
{
	
}

DLL_API signed char     F_API CS_Stream_SetBufferSize(int ms)
{
	return 0;
}

DLL_API CS_STREAM* F_API CS_Stream_OpenFile(const char* filename, unsigned int mode, int memlength)
{
	return 0;
}

static int audio_wav_from_data_FILE(FILE* fp, int bufsize, ALuint* buf)
{
	ALenum ALformat;
	char header[4], wave_header[4], subchunk1[4], subchunk2[4];
	char* temp_buffer;
	unsigned int size, frequency, subchunk2size, unused_int;
	unsigned short num_channels, bits_per_sample, format, unused_short;
	*buf = 0;

	my_fread(&header, sizeof(header), fp);

	if (strncmp(header, "RIFF", 4))
	{
		return 1;
	}

	my_fread(&size, sizeof(size), fp);
	my_fread(&wave_header, sizeof(wave_header), fp);

	if (strncmp(wave_header, "WAVE", 4))
	{
		return 1;
	}

	my_fread(&subchunk1, sizeof(subchunk1), fp);

	if (strncmp(subchunk1, "fmt", 3))
	{
		return 1;
	}

	my_fread(&unused_int, sizeof(unused_int), fp); //subchunk 1 size
	my_fread(&format, sizeof(format), fp);

	if (format != 1)
	{
		return 2;
	}

	my_fread(&num_channels, sizeof(num_channels), fp);
	my_fread(&frequency, sizeof(frequency), fp);

	my_fread(&unused_int, sizeof(unused_int), fp); //ByteRate
	my_fread(&unused_short, sizeof(unused_short), fp); //BlockAlign

	my_fread(&bits_per_sample, sizeof(bits_per_sample), fp);

	switch (bits_per_sample)
	{
		case 8:
			ALformat = num_channels == 2 ? AL_FORMAT_STEREO8 : AL_FORMAT_MONO8;
		break;
		case 16:
			ALformat = num_channels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
		break;
		default:
			return 1;
		break;
	}

	my_fread(&subchunk2, sizeof(subchunk2), fp);

	if (strncmp(subchunk2, "data", 4))
	{
		return 1;
	}

	my_fread(&subchunk2size, sizeof(subchunk2size), fp);

	temp_buffer = new char[subchunk2size];

	my_fread(temp_buffer, subchunk2size, fp);

	alGenBuffers(1, buf);
	alBufferData(*buf, ALformat, temp_buffer, subchunk2size, frequency);
	delete [] temp_buffer;
	return 0;
}

static int audio_ogg_from_data(unsigned char* p, int bufsize, ALuint* buf)
{
	ALshort* ogg_buffer;
	int size, length_samples, vorbis_error;
	stb_vorbis_info info;
	ALenum format;
	stb_vorbis* ogg = stb_vorbis_open_memory(p, bufsize, &vorbis_error, NULL);

	if (!ogg)
	{
		return vorbis_error;
	}

	info = stb_vorbis_get_info(ogg);
	format = (info.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	length_samples = stb_vorbis_stream_length_in_samples(ogg) * info.channels;
	size = length_samples * sizeof(ALshort);

	ogg_buffer = (ALshort*)malloc(size);

	stb_vorbis_get_samples_short_interleaved(ogg,
		info.channels, ogg_buffer, length_samples);

	stb_vorbis_close(ogg);

	alGenBuffers(1, buf);
	alBufferData(*buf, format, ogg_buffer, size, info.sample_rate);

	free(ogg_buffer);

	return 0;
}

DLL_API CS_STREAM*    F_API CS_Stream_Open(const char *name_or_data, unsigned int mode, int offset, int length)
{
	FILE *file = (FILE*)my_fopen(name_or_data);
	int len;
	unsigned char* buf;
	int ret;
	ALuint thebuf = 0;
	ALSample_t* samp = nullptr;
	const char* ext = strrchr(name_or_data, '.');

	if (strlen(name_or_data) >= MAX_SOUND_FILENAME)
	{
		__builtin_trap();
	}

	samp = GetSampleFromName(name_or_data);

	if (samp)
	{
		return (CS_STREAM*)samp;
	}

	if (!strcmp(ext, ".ogg"))
	{
		if (file)
		{
			my_fseek(file, 0, SEEK_END);
			len = my_ftell(file);
			
			buf = new unsigned char [len];
			my_fseek(file, 0, SEEK_SET);
			my_fread(buf, len, file);
			ret = audio_ogg_from_data(buf, len, &thebuf);
			my_fclose(file);
			delete [] buf;
			if (ret == 0)
			{
				samp = new ALSample_t;
				if (thebuf == 0)
				{
					__builtin_trap();
				}
				samp->buf = thebuf;
				samp->flags = mode;
				strcpy(samp->filename, name_or_data);
				
				buffers.push_back(samp);
				printf("OpenAL: There are now %lu buffers.\n", buffers.size());
			}
			else
			{
				__builtin_trap();
			}
			
		}
		else
		{
			__builtin_trap();
		}

	}
	else if (!strcmp(ext, ".wav"))
	{
		if (file)
		{
			ret = audio_wav_from_data_FILE(file, 0, &thebuf);
			my_fclose(file);
			if (ret == 0)
			{
				samp = new ALSample_t;
				samp->buf = thebuf;
				samp->flags = mode;
				strcpy(samp->filename, name_or_data);
				
				buffers.push_back(samp);
				printf("OpenAL: There are now %lu buffers.\n", buffers.size());
			}
			else
			{
				printf("OpenAL: %s has a bad format!\n", name_or_data);
			}
		}
		else
		{
			__builtin_trap();
		}
	}
	else
	{
		__builtin_trap();
	}

	
	return (CS_STREAM*)samp;
}

#ifndef LINUX64
DLL_API CS_STREAM* F_API CS_Stream_Create(CS_STREAMCALLBACK callback, int length, unsigned int mode, int samplerate, int userdata)
#else
DLL_API CS_STREAM* F_API CS_Stream_Create(CS_STREAMCALLBACK callback, int length, unsigned int mode, int samplerate, void *userdata)
#endif
{
	return 0;
}

DLL_API signed char     F_API CS_Stream_Close(CS_STREAM* stream)
{
	__builtin_trap();
	return 0;
}

DLL_API int             F_API CS_Stream_Play(int channel, CS_STREAM* stream)
{
	return -1;
}

DLL_API int             F_API CS_Stream_PlayEx(int channel, CS_STREAM* stream, CS_DSPUNIT* dsp, signed char startpaused)
{
	ALSample_t* samp = (ALSample_t*)stream;
	int i;
	ALuint src;
	if (channel == CS_FREE)
	{
		i = audio_next_available_source();
	}
	else
	{
		__builtin_trap();
		return 0;
	}

	if (i >= 0)
	{
		src = sources[i];
	}
	else
	{
		return -1;
	}
	
	alSourcei(src, AL_BUFFER, samp->buf);
	//alSourcei(src, AL_LOOPING, samp->flags & CS_LOOP_NORMAL ? AL_TRUE : AL_FALSE);
	alSourcePlay(src);
	if (startpaused)
	{
		alSourcePause(src);
	}
	
	return i;
}

DLL_API signed char     F_API CS_Stream_Stop(CS_STREAM* stream)
{
	size_t i;
	ALint buffer;
	ALSample_t* samp = (ALSample_t*)stream;
	for (i = 0; i < MAX_SOURCES; i++)
	{
		alGetSourcei(sources[i], AL_BUFFER, &buffer);
		if (buffer == samp->buf)
		{
			alSourceStop(sources[i]);
			alSourcei(sources[i], AL_BUFFER, 0);
			return 1;
		}
	}
	//__builtin_trap();
	return 0;
}
#if 0
DLL_API int             F_API CS_Stream_GetOpenState(CS_STREAM* stream)
{
	return 0;
}
#endif
DLL_API signed char     F_API CS_Stream_SetPosition(CS_STREAM* stream, unsigned int position)
{
	return 0;
}

DLL_API unsigned int    F_API CS_Stream_GetPosition(CS_STREAM* stream)
{
	return 0;
}

DLL_API signed char     F_API CS_Stream_SetTime(CS_STREAM* stream, int ms)
{
	return 0;
}

DLL_API int             F_API CS_Stream_GetTime(CS_STREAM* stream)
{
	return 0;
}

DLL_API int             F_API CS_Stream_GetLength(CS_STREAM* stream)
{
	ALSample_t* samp = (ALSample_t*)stream;
	ALint sizeInBytes;
	ALint channels;
	ALint bits;
	int lengthInSamples;
	if (samp->buf == 0)
	{
		__builtin_trap();
	}

	alGetBufferi(samp->buf, AL_SIZE, &sizeInBytes);
	alGetBufferi(samp->buf, AL_CHANNELS, &channels);
	alGetBufferi(samp->buf, AL_BITS, &bits);
	lengthInSamples = sizeInBytes * 8 / (channels * bits);

	return lengthInSamples;
}

DLL_API int             F_API CS_Stream_GetLengthMs(CS_STREAM* stream)
{
	unsigned int lengthInSamples = CS_Stream_GetLength(stream);
	ALSample_t* samp = (ALSample_t*)stream;
	ALint frequency;
	float durationInMilliseconds;
	alGetBufferi(samp->buf, AL_FREQUENCY, &frequency);
	durationInMilliseconds = ((float)lengthInSamples / (float)frequency) * 1000.0f;
	return (int)durationInMilliseconds;
}

DLL_API int             F_API CS_FX_Enable(int channel, unsigned int fx)
{
	return 0;
}

DLL_API signed char     F_API CS_FX_SetI3DL2Reverb(int fxid, int Room, int RoomHF,
	float RoomRolloffFactor, float DecayTime, float DecayHFRatio, int Reflections,
	float ReflectionsDelay, int Reverb, float ReverbDelay, float Diffusion,
	float Density, float HFReference)
{
	return 0;
}

DLL_API signed char     F_API CS_FX_SetParamEQ(int fxid, float Center, float Bandwidth,
	float Gain)
{
	return 0;
}
DLL_API signed char     F_API CS_FX_SetWavesReverb(int fxid, float InGain, float ReverbMix,
	float ReverbTime, float HighFreqRTRatio)
{
	return 0;
}

DLL_API void            F_API CS_Update()
{

}

DLL_API void            F_API CS_SetSpeakerMode(unsigned int speakermode)
{

}
DLL_API void            F_API CS_SetSFXMasterVolume(int volume)
{
	size_t i;
	for (i = 0; i < MAX_SOURCES; i++)
	{
		alSourcef(sources[i], AL_GAIN, (float)volume / 255.0f);
	}
}
DLL_API void            F_API CS_SetPanSeperation(float pansep)
{

}
DLL_API void            F_API CS_File_SetCallbacks(CS_OPENCALLBACK  useropen,
                                                       CS_CLOSECALLBACK userclose,
                                                       CS_READCALLBACK  userread,
                                                       CS_SEEKCALLBACK  userseek,
                                                       CS_TELLCALLBACK  usertell)
{
	my_fopen = useropen;
	my_fclose = userclose;
	my_fread = userread;
	my_fseek = userseek;
	my_ftell = usertell;
}
#if 0
DLL_API void            F_API CS_3D_SetDopplerFactor(float scale)
{

}

DLL_API void            F_API CS_3D_SetDistanceFactor(float scale)
{

}
#endif
DLL_API void            F_API CS_3D_SetRolloffFactor(float scale)
{

}

#ifndef LINUX64
DLL_API signed char     F_API CS_3D_SetAttributes(int channel, float *pos, float *vel)
#else
DLL_API signed char     F_API CS_3D_SetAttributes(int channel, const float *pos, const float *vel)
#endif
{
	return 0;
}
#if 0
DLL_API signed char     F_API CS_3D_GetAttributes(int channel, float *pos, float *vel)
{
	return 0;
}

DLL_API void            F_API CS_3D_Listener_SetCurrent(int current, int numlisteners)
{

}
#endif
#ifndef LINUX64
DLL_API void            F_API CS_3D_Listener_SetAttributes(float *pos, float *vel, float fx,
	float fy, float fz, float tx, float ty, float tz)
#else
DLL_API void            F_API CS_3D_Listener_SetAttributes(const float *pos, const float *vel,
	float fx, float fy, float fz, float tx, float ty, float tz)
#endif
{

}
#if 0
DLL_API void            F_API CS_3D_Listener_GetAttributes(float *pos, float *vel, float *fx,
	float *fy, float *fz, float *tx, float *ty, float *tz)
{

}
#endif
DLL_API signed char     F_API CS_IsPlaying(int channel)
{
	int status;
	if (channel < 0 || channel >= MAX_SOURCES)
	{
		__builtin_trap();
		return SOURCE_OUT_OF_BOUNDS;
	}
	alGetSourcei(sources[channel], AL_SOURCE_STATE, &status);
	if (status == AL_PLAYING)
	{
		return 1;
	}
	return 0;
}
#if 0
DLL_API signed char     F_API CS_GetReserved(int channel)
{
	return 0;
}
#endif
DLL_API unsigned int    F_API CS_GetLoopMode(int channel)
{
	return 0;
}
DLL_API unsigned int    F_API CS_GetCurrentPosition(int channel)
{
	int currbytes, size;
	if (channel < 0 || channel >= MAX_SOURCES)
	{
		//__builtin_trap();
		return SOURCE_OUT_OF_BOUNDS;
	}

	alGetSourcei(sources[channel], AL_BYTE_OFFSET, &currbytes);

	return currbytes;
}
DLL_API CS_SAMPLE * F_API CS_GetCurrentSample(int channel)
{
	return nullptr;
}
DLL_API signed char     F_API CS_GetCurrentLevels(int channel, float *l, float *r)
{
	return 0;
}

DLL_API signed char     F_API CS_DSP_MixBuffers(void *destbuffer, void *srcbuffer, int len, int freq, int vol, int pan, unsigned int mode)
{
	return 0;
}

DLL_API void            F_API CS_DSP_ClearMixBuffer()
{

}

DLL_API int             F_API CS_DSP_GetBufferLength()
{
	return 0;
}

DLL_API int             F_API CS_DSP_GetBufferLengthTotal()
{
	return 0;
}

DLL_API float *         F_API CS_DSP_GetSpectrum()
{
	return nullptr;
}

DLL_API CS_DSPUNIT *F_API CS_DSP_Create(CS_DSPCALLBACK callback, int priority, void *userdata)
{
	return nullptr;
}

DLL_API void            F_API CS_DSP_Free(CS_DSPUNIT *unit)
{
}

DLL_API void            F_API CS_DSP_SetActive(CS_DSPUNIT *unit, signed char active)
{
}

DLL_API unsigned int    F_API CS_Sample_GetLength(CS_SAMPLE *sptr)
{
	ALSample_t* samp = (ALSample_t*)sptr;
	ALint sizeInBytes;
	ALint channels;
	ALint bits;
	ALint frequency;
	int lengthInSamples;

	alGetBufferi(samp->buf, AL_SIZE, &sizeInBytes);
	alGetBufferi(samp->buf, AL_CHANNELS, &channels);
	alGetBufferi(samp->buf, AL_BITS, &bits);
	alGetBufferi(samp->buf, AL_FREQUENCY, &frequency);
	lengthInSamples = sizeInBytes * 8 / (channels * bits);

	return lengthInSamples;
}

DLL_API signed char     F_API CS_Sample_GetLoopPoints(CS_SAMPLE *sptr, int *loopstart, int *loopend)
{
	return 0;
}

DLL_API signed char     F_API CS_Sample_GetDefaults(CS_SAMPLE *sptr, int *deffreq, int *defvol, int *defpan, int *defpri)
{
	ALSample_t* samp = (ALSample_t*)sptr;
	if (deffreq != nullptr)
	{
		alGetBufferi(samp->buf, AL_FREQUENCY, deffreq);
	}

	return 0;
}

DLL_API signed char     F_API CS_Sample_GetDefaultsEx(CS_SAMPLE *sptr, int *deffreq, int *defvol, int *defpan, int *defpri, int *varfreq, int *varvol, int *varpan)
{
	return 0;
}

DLL_API int             F_API CS_PlaySound(int channel, CS_SAMPLE *sptr)
{
	return 0;
}

DLL_API int             F_API CS_PlaySoundEx(int channel, CS_SAMPLE *sptr, CS_DSPUNIT *dsp, signed char startpaused)
{
	ALSample_t* samp = (ALSample_t*)sptr;
	int i;
	ALuint src;

	if (channel == CS_FREE)
	{
		i = audio_next_available_source();
	}
	else
	{
		__builtin_trap();
		return -1;
	}

	if (i >= 0)
	{
		src = sources[i];
	}
	else
	{
		return -1;
	}

	if (i >= MAX_SOURCES)
	{
		__builtin_trap();
	}

	alSourcei(src, AL_BUFFER, samp->buf);
	//alSourcei(src, AL_LOOPING, samp->flags & CS_LOOP_NORMAL ? AL_TRUE : AL_FALSE);
	alSourcePlay(src);
	if (startpaused)
	{
		alSourcePause(src);
	}
	
	return i;
}

DLL_API signed char     F_API CS_StopSound(int channel)
{
	size_t i;
	if (channel >= MAX_SOURCES)
	{
		__builtin_trap();
		return SOURCE_OUT_OF_BOUNDS;
	}

	if (channel == CS_FREE)
	{
		for (i = 0; i < MAX_SOURCES; i++)
		{
			alSourceStop(sources[i]);
			alSourcei(sources[i], AL_BUFFER, 0);
		}
		return 1;
	}

	alSourceStop(sources[channel]);
	alSourcei(sources[channel], AL_BUFFER, 0);
	return 1;
}

DLL_API signed char     F_API CS_Sample_SetMode(CS_SAMPLE *sptr, unsigned int mode)
{
	ALSample_t* samp = (ALSample_t*)sptr;
	samp->flags = mode;
	return 1;
}

DLL_API signed char     F_API CS_Sample_SetLoopPoints(CS_SAMPLE *sptr, int loopstart, int loopend)
{
	return 0;
}

DLL_API signed char     F_API CS_Sample_SetDefaults(CS_SAMPLE *sptr, int deffreq, int defvol, int defpan, int defpri)
{
	return 0;
}

DLL_API signed char     F_API CS_Sample_SetMinMaxDistance(CS_SAMPLE *sptr, float min, float max)
{
	return 0;
}

DLL_API signed char     F_API CS_Sample_SetMaxPlaybacks(CS_SAMPLE *sptr, int max)
{
	return 0;
}

DLL_API signed char      CS_Reverb_SetProperties(const CS_REVERB_PROPERTIES *prop)
{
	return 0;
}

DLL_API signed char     F_API CS_Reverb_GetProperties(CS_REVERB_PROPERTIES *prop)
{
	return 0;
}

DLL_API signed char     F_API CS_Reverb_SetChannelProperties(int channel, const CS_REVERB_CHANNELPROPERTIES *prop)
{
	return 0;
}

DLL_API signed char     F_API CS_Reverb_GetChannelProperties(int channel, CS_REVERB_CHANNELPROPERTIES *prop)
{
	return 0;
}