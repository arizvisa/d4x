/*	WebDownloader for X-Window
 *	Copyright (C) 1999-2002 Koshelev Maxim
 *	This Program is free but not GPL!!! You can't modify it
 *	without agreement with author. You can't distribute modified
 *	program but you can distribute unmodified program.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include "sndserv.h"
#include "locstr.h"
#include "var.h"

#define	WAVE_FORMAT_UNKNOWN		(0x0000)
#define	WAVE_FORMAT_PCM			(0x0001)
#define	WAVE_FORMAT_ADPCM		(0x0002)
#define	WAVE_FORMAT_ALAW		(0x0006)
#define	WAVE_FORMAT_MULAW		(0x0007)
#define	WAVE_FORMAT_OKI_ADPCM		(0x0010)
#define	WAVE_FORMAT_DIGISTD		(0x0015)
#define	WAVE_FORMAT_DIGIFIX		(0x0016)
#define	IBM_FORMAT_MULAW         	(0x0101)
#define	IBM_FORMAT_ALAW			(0x0102)
#define	IBM_FORMAT_ADPCM         	(0x0103)

enum FMT_ENUM{
	FMT_U8,
	FMT_S8,
	FMT_U16_LE,
	FMT_U16_BE,
	FMT_U16_NE,
	FMT_S16_LE,
	FMT_S16_BE,
	FMT_S16_NE
};

class d4xAudio{
public:
	d4xAudio();
	virtual int write(void *data, int length);
	virtual int open(int fmt, int rate, int nch);
	virtual ~d4xAudio();
};

#ifdef D4X_WITH_OSS
#include <sys/soundcard.h>

class d4xOssAudio:public d4xAudio{
	int fragsize,oss_format,format,channels,frequency;
	int fd;
	int bps,ebps;
	int blk_size;
	int convert_stereo;
	void setup_format(int fmt,int rate, int nch);
	void set_audio_params();
public:
	d4xOssAudio();
	~d4xOssAudio();
	int write(void *data, int length);
	int open(int fmt, int rate, int nch);
};

#endif //D4X_WITH_OSS

class d4xSndFile{
public:
	d4xSndFile(){};
	d4xSndFile(char *filename);
	virtual void play()=0;
	virtual ~d4xSndFile(){};
};

class d4xWaveFile:public d4xSndFile{
	FILE *file;
	short format,channels,align,bits,eof;
	long samples_per_sec, avg_bytes_per_sec;
	int position, length;
	int data_offset;
	d4xAudio *audio;
	int read_long(long *len);
	int read_short(short *val);
	void play_begin();
public:
	d4xWaveFile(char *filename);
	void play();
	~d4xWaveFile();
};

#ifdef D4X_WITH_ESD
#include <esd.h>
class d4xEsdFile:public d4xSndFile{
	int fd;
	char *file;
public:
	d4xEsdFile(char *filename);
	void play();
	~d4xEsdFile();
};

/***********************************************************/

d4xEsdFile::d4xEsdFile(char *filename){
	file=copy_string(filename);
	fd=-1;
};

void d4xEsdFile::play(){
	if (fd<0)
		fd=esd_open_sound(NULL);//"localhost");
	if (fd>=0)
		esd_play_file(NULL,file,0); // ???
};

d4xEsdFile::~d4xEsdFile(){
	if (fd>=0) esd_close(fd);
};
#endif //D4X_WITH_ESD

/***********************************************************/

int d4xWaveFile::read_long(long *len){
	unsigned char buf[4];

	if (fread(buf, 1, 4, file) != 4)
		return 0;

#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	*len =(buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | buf[0];
#elif
	*len =(buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
#endif

	return 1;
};

int d4xWaveFile::read_short(short *val){
	unsigned char buf[2];
	if (fread(buf, 1, 2, file) != 2)
		return 0;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
	*val = (buf[1] << 8) | buf[0];
#elif
	*val = (buf[0] << 8) | buf[1];
#endif
	return 1;
}

d4xWaveFile::d4xWaveFile(char *filename){
#ifdef D4X_WITH_OSS
	audio=new d4xOssAudio;
#else
	audio=new d4xAudio;
#endif
	char data[4];
	format=channels=align=bits=eof=0;
	samples_per_sec=avg_bytes_per_sec=0;
	if ((file = fopen(filename, "r"))){
		fread(data, 1, 4, file);
		if (strncmp(data, "RIFF", 4)){
			fclose(file);
			file = NULL;
			return;
		}
		long tmp;
		read_long(&tmp);
		fread(data, 1, 4, file);
		if (strncmp(data, "WAVE", 4)){
			fclose(file);
			file = NULL;
			return;
		}
	};
};

d4xWaveFile::~d4xWaveFile(){
	if (file) fclose(file);
	if (audio) delete(audio);
};

void d4xWaveFile::play(){
	char data[4];
	unsigned long len;

	int audio_error = 0;
	if (file){
		while(1){
			fread(data, 1, 4, file);
			if (!read_long((long *)&len))
				return;
			if (!strncmp("fmt ", data, 4))
				break;
			fseek(file,len,SEEK_CUR);
		}
		if (len < 16){
			return;
		}
		read_short(&format);
		switch (format)
		{
			case WAVE_FORMAT_UNKNOWN:
			case WAVE_FORMAT_ALAW:
			case WAVE_FORMAT_MULAW:
			case WAVE_FORMAT_ADPCM:
			case WAVE_FORMAT_OKI_ADPCM:
			case WAVE_FORMAT_DIGISTD:
			case WAVE_FORMAT_DIGIFIX:
			case IBM_FORMAT_MULAW:
			case IBM_FORMAT_ALAW:
			case IBM_FORMAT_ADPCM:
				return;
		}
		if (!read_short(&channels)) return;
		if (!read_long(&samples_per_sec)) return;
		if (!read_long(&avg_bytes_per_sec)) return;
		if (!read_short(&align)) return;
		if (!read_short(&bits)) return;
/*
		printf("channels = %i\n",int(channels));
		printf("sps = %li\n",samples_per_sec);
		printf("abps = %li\n",avg_bytes_per_sec);
		printf("bps = %i\n",int(bits));
*/
		if (bits != 8 && bits != 16)
			return;
		len -= 16;
		if (len)
			fseek(file, len, SEEK_CUR);

		while(1){
			fread(data, 4, 1, file);
			if (!read_long((long *)&len)) return;
			if (!strncmp("data", data, 4))
				break;
			fseek(file, len, SEEK_CUR);
		}
		length = len;
		position = 0;

		if (audio->open((bits == 16) ? FMT_S16_LE : FMT_U8, samples_per_sec, channels) == 0){
			audio_error = 1;
			return;
		}
		eof=0;
		play_begin();
	}
}

void d4xWaveFile::play_begin(){
	char data[2048 * 2];

	int blk_size = 512 * (bits / 8) * channels;
//	printf("playing (blk_size=%i)\n",blk_size);
	while (!eof){
		int bytes = blk_size;
		if (length - position < bytes)
			bytes = length - position;
		if (bytes > 0){
			int actual_read = fread(data, 1, bytes, file);
			if (actual_read == -1){
				eof = 1;
			}else{
			        audio->write(data, bytes);
				position += actual_read;
			};
		}else{
			eof = 1;
		}
	}
}

/***********************************************************/
d4xAudio::d4xAudio(){
	/* abstract */
};

int d4xAudio::write(void *data, int length){
	return(length);
};

int d4xAudio::open(int fmt, int rate, int nch){
	return(1);
};

d4xAudio::~d4xAudio(){
	/* absctract */
};
/***********************************************************/

#ifdef D4X_WITH_OSS
void d4xOssAudio::setup_format(int fmt,int rate, int nch){
	format = fmt;
	frequency = rate;
	channels = nch;
	switch (fmt){
	case FMT_U8:
		oss_format = AFMT_U8;
		break;
	case FMT_S16_LE:
		oss_format = AFMT_S16_LE;
		break;
	}

	bps = rate * nch;
	if (oss_format == AFMT_S16_LE)
		bps *= 2;
	fragsize = 0;
	while ((1L << fragsize) < bps / 25)
		fragsize++;
	fragsize--;

}

void d4xOssAudio::set_audio_params(){
	ioctl(fd, SNDCTL_DSP_RESET, 0);
	int frag = (32 << 16) | fragsize;
	ioctl(fd, SNDCTL_DSP_SETFRAGMENT, &frag);
	ioctl(fd, SNDCTL_DSP_SETFMT, &oss_format);
	int stereo = channels - 1;
	ioctl(fd, SNDCTL_DSP_STEREO, &stereo);
	ioctl(fd, SNDCTL_DSP_SPEED, &frequency);

	if(ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &blk_size) == -1)
		blk_size = 1L << fragsize;

	ebps = frequency * channels;
	if (oss_format == AFMT_S16_LE)
		ebps *= 2;
	
}

int d4xOssAudio::open(int fmt, int rate, int nch){
	fd = ::open("/dev/dsp", O_WRONLY);
	if (fd == -1){
//		printf("open_audio(): Failed to open audio device (%s): %s",
//		       "/dev/dsp", strerror(errno));
		return 0;
	}
	setup_format(fmt,rate,nch);
	set_audio_params();
	return 1;
}

int d4xOssAudio::write(void * data, int length){
	return (::write(fd, data, length));
}

d4xOssAudio::d4xOssAudio(){
	fd=-1;
};

d4xOssAudio::~d4xOssAudio(){
	if (fd>=0){
		ioctl(fd, SNDCTL_DSP_POST, 0);
		close(fd);
	};
};
#endif

/***********************************************************/

d4xSndServer *SOUND_SERVER;

static void *_snd_serv_run_(void *serv){
	d4xSndServer *server=(d4xSndServer *)serv;

	sigset_t oldmask,newmask;
	sigemptyset(&newmask);
	sigaddset(&newmask,SIGTERM);
	sigaddset(&newmask,SIGINT);
	sigaddset(&newmask,SIGUSR1);
	sigaddset(&newmask,SIGUSR2);
	pthread_sigmask(SIG_BLOCK,&newmask,&oldmask);

	server->run();
	pthread_exit(NULL);
	return(NULL);
};

d4xSndServer::d4xSndServer(){
	stop_now=0;
	for (int i=0;i<SND_LAST;i++)
		snd_table[i]=NULL;
	queue=new tQueue;
	thread_id=0;
	pthread_cond_init(&cond,NULL);
};

d4xSndServer::~d4xSndServer(){
	delete(queue);
	pthread_cond_destroy(&cond);
	for (int i=0;i<SND_LAST;i++)
		if (snd_table[i]) delete[] snd_table[i];
};

void d4xSndServer::stop_thread(){
	void *val;
	exit_lock.lock();
	my_mutex.lock();
	stop_now=1;
	pthread_cond_signal(&cond);
	my_mutex.unlock();
	exit_lock.lock();
	exit_lock.unlock();
	pthread_join(thread_id,&val);
	thread_id=0;
};

void d4xSndServer::run_thread(){
	pthread_attr_t attr_p;
	pthread_attr_init(&attr_p);
	pthread_attr_setdetachstate(&attr_p,PTHREAD_CREATE_JOINABLE);
	pthread_attr_setscope(&attr_p,PTHREAD_SCOPE_SYSTEM);
	pthread_create(&thread_id,&attr_p,_snd_serv_run_,(void *)this);
};

void d4xSndServer::add_event(int event){
	if (thread_id==0 || CFG.ENABLE_SOUNDS==0 || CFG.WITHOUT_FACE)
		return;
	d4xSndEvent *snd=new d4xSndEvent;
	snd->event=event;
	snd->birth=time(NULL);
	my_mutex.lock();
	queue->insert(snd);
	pthread_cond_signal(&cond);
	my_mutex.unlock();
};

void d4xSndServer::play_sound(int event){
	if (event>=0 && event<SND_LAST &&
	    snd_table[event]!=NULL){
		d4xSndFile *wav=NULL;
#ifdef D4X_WITH_ESD		
		if (CFG.ESD_SOUND)
			wav=new d4xEsdFile(snd_table[event]);
		else
#endif// D4X_WITH_ESD
			wav=new d4xWaveFile(snd_table[event]);
		wav->play();
		delete(wav);
	};
};

void d4xSndServer::run(){
	while(1){
		my_mutex.lock();
		pthread_cond_wait(&cond,&(my_mutex.m));
		while(queue->first()){
			d4xSndEvent *snd=(d4xSndEvent *)(queue->first());
			queue->del(snd);
			my_mutex.unlock();
			time_t now=time(NULL);
			if (snd->birth-now<4 && snd->birth-now>-4){
				/* playing */
				play_sound(snd->event);
			};
			delete(snd);
			my_mutex.lock();
		};
		my_mutex.unlock();
		if (stop_now) break;
	};
	exit_lock.unlock();
};

void d4xSndServer::set_sound_file(int event,char *path){
	my_mutex.lock();
	if (event>=0 && event<SND_LAST){
		if (snd_table[event]) delete[] snd_table[event];
		if (path && *path)
			snd_table[event]=copy_string(path);
		else
			snd_table[event]=NULL;
	};
	my_mutex.unlock();
};

char *d4xSndServer::get_sound_file(int event){
	if (event>=0 && event<SND_LAST)
		return(snd_table[event]);
	return(NULL);
};

#define SND_REINIT(a,b){		\
	if (!equal(snd_table[a],b))	\
		set_sound_file(a,b);	\
};

void d4xSndServer::reinit_sounds(){
	SND_REINIT(SND_STARTUP,CFG.SOUND_STARTUP);
	SND_REINIT(SND_FAIL,CFG.SOUND_FAIL);
	SND_REINIT(SND_COMPLETE,CFG.SOUND_COMPLETE);
	SND_REINIT(SND_ADD,CFG.SOUND_ADD);
	SND_REINIT(SND_DND_DROP,CFG.SOUND_DND_DROP);
	SND_REINIT(SND_QUEUE_FINISH,CFG.SOUND_QUEUE_FINISH);
};

void d4xSndServer::init_sounds(){
	set_sound_file(SND_STARTUP,CFG.SOUND_STARTUP);
	set_sound_file(SND_ADD,CFG.SOUND_ADD);
	set_sound_file(SND_DND_DROP,CFG.SOUND_DND_DROP);
	set_sound_file(SND_COMPLETE,CFG.SOUND_COMPLETE);
	set_sound_file(SND_FAIL,CFG.SOUND_FAIL);
	set_sound_file(SND_QUEUE_FINISH,CFG.SOUND_QUEUE_FINISH);
//	set_sound_file(SND_,CFG.SOUND_);
};
