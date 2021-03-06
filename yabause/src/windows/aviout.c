/* aviout.cpp
 *
 * Copyright (C) 2006-2008 Zeromus
 *
 * This file is part of DeSmuME
 *
 * DeSmuME is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * DeSmuME is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with DeSmuME; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/

#include "windows.h"
#include "assert.h"
#include "vfw.h"
#include "stdio.h"
#include "settings/settings.h"
#include "aviout.h"

void EMU_PrintError(const char* msg) {
//	LOG(msg);
}

void EMU_PrintMessage(const char* msg) {
//	LOG(msg);
}



//extern PALETTEENTRY *color_palette;
//extern WAVEFORMATEX wf;
//extern int soundo;

#define VIDEO_STREAM	0
#define AUDIO_STREAM	1

#define VIDEO_WIDTH		256

static struct AVIFile
{
	int					valid;
	int					fps;
	int					fps_scale;

	int					video_added;
	BITMAPINFOHEADER	bitmap_format;

	int					sound_added;
	WAVEFORMATEX		wave_format;

	AVISTREAMINFO		avi_video_header;
	AVISTREAMINFO		avi_sound_header;
	PAVIFILE			avi_file;
	PAVISTREAM			streams[2];
	PAVISTREAM			compressed_streams[2];

	AVICOMPRESSOPTIONS	compress_options[2];
	AVICOMPRESSOPTIONS*	compress_options_ptr[2];

	int					video_frames;
	int					sound_samples;

	u8					convert_buffer[320*224*3];
	int					start_scanline;
	int					end_scanline;
	
	long				tBytes, ByteBuffer;
} *avi_file = NULL;

struct VideoSystemInfo
{
	int					start_scanline;
	int					end_scanline;
	int					fps;
};


static char saved_cur_avi_fnameandext[MAX_PATH];
static char saved_avi_fname[MAX_PATH];
static char saved_avi_ext[MAX_PATH];
static int avi_segnum=0;
//static FILE* avi_check_file=0;
static struct AVIFile saved_avi_info;
static int use_prev_options=0;
static int use_sound=0;



static int truncate_existing(const char* filename)
{
	// this is only here because AVIFileOpen doesn't seem to do it for us
	FILE* fd = fopen(filename, "wb");
	if(fd)
	{
		fclose(fd);
		return 1;
	}

	return 0;
}

static void avi_create(struct AVIFile** avi_out)
{
	*avi_out = (struct AVIFile*)malloc(sizeof(struct AVIFile));
	memset(*avi_out, 0, sizeof(struct AVIFile));
	AVIFileInit();
}

static void avi_destroy(struct AVIFile** avi_out)
{
	if(!(*avi_out))
		return;

	if((*avi_out)->sound_added)
	{
		if((*avi_out)->compressed_streams[AUDIO_STREAM])
		{
			LONG test = AVIStreamClose((*avi_out)->compressed_streams[AUDIO_STREAM]);
			(*avi_out)->compressed_streams[AUDIO_STREAM] = NULL;
			(*avi_out)->streams[AUDIO_STREAM] = NULL;				// compressed_streams[AUDIO_STREAM] is just a copy of streams[AUDIO_STREAM]
		}
	}

	if((*avi_out)->video_added)
	{
		if((*avi_out)->compressed_streams[VIDEO_STREAM])
		{
			AVIStreamClose((*avi_out)->compressed_streams[VIDEO_STREAM]);
			(*avi_out)->compressed_streams[VIDEO_STREAM] = NULL;
		}

		if((*avi_out)->streams[VIDEO_STREAM])
		{
			AVIStreamClose((*avi_out)->streams[VIDEO_STREAM]);
			(*avi_out)->streams[VIDEO_STREAM] = NULL;
		}
	}

	if((*avi_out)->avi_file)
	{
		AVIFileClose((*avi_out)->avi_file);
		(*avi_out)->avi_file = NULL;
	}

	free(*avi_out);
	*avi_out = NULL;
}

static void set_video_format(const BITMAPINFOHEADER* bitmap_format, struct AVIFile* avi_out)
{
	memcpy(&((*avi_out).bitmap_format), bitmap_format, sizeof(BITMAPINFOHEADER));
	(*avi_out).video_added = 1;
}

static void set_sound_format(const WAVEFORMATEX* wave_format, struct AVIFile* avi_out)
{
	memcpy(&((*avi_out).wave_format), wave_format, sizeof(WAVEFORMATEX));
	(*avi_out).sound_added = 1;
}

static int avi_open(const char* filename, const BITMAPINFOHEADER* pbmih, const WAVEFORMATEX* pwfex, HWND HWnd)
{
	int error = 1;
	int result = 0;

	do
	{
		// close existing first
		DRV_AviEnd();

		if(!truncate_existing(filename))
			break;

		if(!pbmih)
			break;

		// create the object
		avi_create(&avi_file);

		// set video size and framerate
		/*avi_file->start_scanline = vsi->start_scanline;
		avi_file->end_scanline = vsi->end_scanline;
		avi_file->fps = vsi->fps;
		avi_file->fps_scale = 16777216-1;
		avi_file->convert_buffer = new u8[256*384*3];*/

		// open the file
		if(FAILED(AVIFileOpenA(&avi_file->avi_file, filename, OF_CREATE | OF_WRITE, NULL)))
			break;

		// create the video stream
		set_video_format(pbmih, avi_file);

		memset(&avi_file->avi_video_header, 0, sizeof(AVISTREAMINFO));
		avi_file->avi_video_header.fccType = streamtypeVIDEO;
		avi_file->avi_video_header.dwScale = 65536*256;
		avi_file->avi_video_header.dwRate = (int)(59.8261*65536*256);
		avi_file->avi_video_header.dwSuggestedBufferSize = avi_file->bitmap_format.biSizeImage;
		if(FAILED(AVIFileCreateStream(avi_file->avi_file, &avi_file->streams[VIDEO_STREAM], &avi_file->avi_video_header)))
			break;

		if(use_prev_options)
		{
			avi_file->compress_options[VIDEO_STREAM] = saved_avi_info.compress_options[VIDEO_STREAM];
			avi_file->compress_options_ptr[VIDEO_STREAM] = &avi_file->compress_options[0];
		}
		else
		{
			// get compression options
			memset(&avi_file->compress_options[VIDEO_STREAM], 0, sizeof(AVICOMPRESSOPTIONS));
			avi_file->compress_options_ptr[VIDEO_STREAM] = &avi_file->compress_options[0];
//retryAviSaveOptions: //mbg merge 7/17/06 removed
			error = 0;
			if(!AVISaveOptions(NULL, 0, 1, &avi_file->streams[VIDEO_STREAM], &avi_file->compress_options_ptr[VIDEO_STREAM])) //TODO is this right?
				break;
			error = 1;
		}

		// create compressed stream
		if(FAILED(AVIMakeCompressedStream(&avi_file->compressed_streams[VIDEO_STREAM], avi_file->streams[VIDEO_STREAM], &avi_file->compress_options[VIDEO_STREAM], NULL)))
			break;

		// set the stream format
		if(FAILED(AVIStreamSetFormat(avi_file->compressed_streams[VIDEO_STREAM], 0, (void*)&avi_file->bitmap_format, avi_file->bitmap_format.biSize)))
			break;

		// add sound (if requested)
		if(pwfex)
		{
			// add audio format
			set_sound_format(pwfex, avi_file);

			// create the audio stream
			memset(&avi_file->avi_sound_header, 0, sizeof(AVISTREAMINFO));
			avi_file->avi_sound_header.fccType = streamtypeAUDIO;
			avi_file->avi_sound_header.dwQuality = (DWORD)-1;
			avi_file->avi_sound_header.dwScale = avi_file->wave_format.nBlockAlign;
			avi_file->avi_sound_header.dwRate = avi_file->wave_format.nAvgBytesPerSec;
			avi_file->avi_sound_header.dwSampleSize = avi_file->wave_format.nBlockAlign;
			avi_file->avi_sound_header.dwInitialFrames = 1;
			if(FAILED(AVIFileCreateStream(avi_file->avi_file, &avi_file->streams[AUDIO_STREAM], &avi_file->avi_sound_header)))
				break;

			// AVISaveOptions doesn't seem to work for audio streams
			// so here we just copy the pointer for the compressed stream
			avi_file->compressed_streams[AUDIO_STREAM] = avi_file->streams[AUDIO_STREAM];

			// set the stream format
			if(FAILED(AVIStreamSetFormat(avi_file->compressed_streams[AUDIO_STREAM], 0, (void*)&avi_file->wave_format, sizeof(WAVEFORMATEX))))
				break;
		}

		// initialize counters
		avi_file->video_frames = 0;
		avi_file->sound_samples = 0;
		avi_file->tBytes = 0;
		avi_file->ByteBuffer = 0;

		// success
		error = 0;
		result = 1;
		avi_file->valid = 1;

	} while(0);

	if(!result)
	{
		avi_destroy(&avi_file);
		if(error)
			EMU_PrintError("Error writing AVI file");
	}

	return result;
}

//32 to 24, discard alpha
static void do_video_conversion(const u8* buffer)
{
	u8* outbuf = avi_file->convert_buffer; //+ 320*(224-1)*3;
	int y;
	int x;

	for(y=0;y<224;y++)
	{
		for(x=0;x<320;x++)
		{
			u16 a,b,c;

			a = *buffer++;
			b = *buffer++;
			c = *buffer++;
			*buffer++;

			*outbuf++ = c;
			*outbuf++ = b;
			*outbuf++ = a;
		}
	}
}


static int AviNextSegment(HWND HWnd)
{
	int ret;
	char avi_fname[MAX_PATH];
	char avi_fname_temp[MAX_PATH];
	strcpy(avi_fname,saved_avi_fname);
	sprintf(avi_fname_temp, "%s_part%d%s", avi_fname, avi_segnum+2, saved_avi_ext);
	saved_avi_info=*avi_file;
	use_prev_options=1;
	avi_segnum++;
	ret = DRV_AviBegin(avi_fname_temp, HWnd);
	use_prev_options=0;
	strcpy(saved_avi_fname,avi_fname);
	return ret;
}


int DRV_AviBegin(const char* fname, HWND HWnd)
{
	WAVEFORMATEX wf;
	BITMAPINFOHEADER bi;
	WAVEFORMATEX* pwf;
	char* dot;

	DRV_AviEnd();

	memset(&bi, 0, sizeof(bi));
	bi.biSize = 0x28;    
	bi.biPlanes = 1;
	bi.biBitCount = 24;
	bi.biWidth = 320;
	bi.biHeight = 224;
	bi.biSizeImage = 3 * 320 * 224;

	
	wf.cbSize = sizeof(WAVEFORMATEX);
	wf.nAvgBytesPerSec = 44100 * 4;
	wf.nBlockAlign = 4;
	wf.nChannels = 2;
	wf.nSamplesPerSec = 44100;
	wf.wBitsPerSample = 16;
	wf.wFormatTag = WAVE_FORMAT_PCM;
	

	saved_avi_ext[0]='\0';

	//mbg 8/10/08 - decide whether there will be sound in this movie
	//if this is a new movie..
	/*if(!avi_file) {
		if(FSettings.SndRate)
			use_sound = 1;
		else use_sound = 0;
	}*/

	//mbg 8/10/08 - if there is no sound in this movie, then dont open the audio stream
	pwf = &wf;
	//if(!use_sound)
	//	pwf = 0;


	if(!avi_open(fname, &bi, pwf, HWnd))
	{
		saved_avi_fname[0]='\0';
		return 0;
	}

	// Don't display at file splits
	if(!avi_segnum) {
		EMU_PrintMessage("AVI recording started.");
//		SetMessageToDisplay("AVI recording started.");
	}

	strncpy(saved_cur_avi_fnameandext,fname,MAX_PATH);
	strncpy(saved_avi_fname,fname,MAX_PATH);
	dot = strrchr(saved_avi_fname, '.');
	if(dot && dot > strrchr(saved_avi_fname, '/') && dot > strrchr(saved_avi_fname, '\\'))
	{
		strcpy(saved_avi_ext,dot);
		dot[0]='\0';
	}
	return 1;
}

void DRV_AviVideoUpdate(const u16* buffer, HWND HWnd)
{
	if(!avi_file || !avi_file->valid)
		return;

	do_video_conversion((const u8*)buffer);

    if(FAILED(AVIStreamWrite(avi_file->compressed_streams[VIDEO_STREAM],
                                 avi_file->video_frames, 1, avi_file->convert_buffer,
                                 avi_file->bitmap_format.biSizeImage, AVIIF_KEYFRAME,
                                 NULL, &avi_file->ByteBuffer)))
	{
		avi_file->valid = 0;
		return;
	}

	avi_file->video_frames++;
	avi_file->tBytes += avi_file->ByteBuffer;

	// segment / split AVI when it's almost 2 GB (2000MB, to be precise)
	if(!(avi_file->video_frames % 60) && avi_file->tBytes > 2097152000)
		AviNextSegment(HWnd);
}

void DRV_AviSoundUpdate(void* soundData, int soundLen)
{
	int nBytes;

	if(!avi_file || !avi_file->valid || !avi_file->sound_added)
		return;

	nBytes = soundLen * avi_file->wave_format.nBlockAlign;
    if(FAILED(AVIStreamWrite(avi_file->compressed_streams[AUDIO_STREAM],
                             avi_file->sound_samples, soundLen,
                             soundData, nBytes, 0, NULL, &avi_file->ByteBuffer)))
	{
		avi_file->valid = 0;
		return;
	}

	avi_file->sound_samples += soundLen;
	avi_file->tBytes += avi_file->ByteBuffer;
}

void DRV_AviEnd()
{
	if(!avi_file)
		return;

	// Don't display if we're just starting another segment
	if(avi_file->tBytes <= 2097152000) {
		EMU_PrintMessage("AVI recording ended.");
	}

	avi_destroy(&avi_file);
}

int DRV_AviIsRecording()
{
	if(avi_file)
		return 1;

	return 0;
}
