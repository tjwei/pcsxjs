/* SDL Driver for P.E.Op.S Sound Plugin
 * Copyright (c) 2010, Wei Mingzhi <whistler_wmz@users.sf.net>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1307 USA
 */

#include "stdafx.h"

#include "externals.h"
#include <emscripten.h>

//#define BUFFER_SIZE		22050
#define BUFFER_SIZE		44100

void SetupSound(void) {

}

void RemoveSound(void) {

}
int SoundGetBytesBuffered_value=0;
unsigned int ccc=0;
double in_buffer=0;
double last_time=0;
unsigned long SoundGetBytesBuffered(void) {
	/*ccc++;
	if(ccc==1001){
		printf("lBytes %d\n", SoundGetBytesBuffered_value);
		ccc=0;
	}*/	
	if(SoundGetBytesBuffered_value>22050){
		return SOUNDSIZE;
	}
	return 0;
	/*if(ccc==1){		
		printf("sound buffer cleared\n");
	}
	if(ccc==0)	
		return 0; 	
	ccc--;
	return SOUNDSIZE;*/
	//printf("SoundGetBytesBuffered %lu\n", SoundGetBytesBuffered_value);
	/*double now = emscripten_get_now();
	if(SoundGetBytesBuffered_value==SOUNDSIZE){
		//last_time = now;
		return SOUNDSIZE;
	}
	in_buffer = in_buffer -(now - last_time);
	last_time = now;
	if(in_buffer<0) in_buffer=0;
	if(in_buffer<400)
		return 0;
	else
		return SOUNDSIZE;*/
	
}

void SoundFeedStreamData(unsigned char *pSound, long lBytes) {
	double now = emscripten_get_now();
	in_buffer = in_buffer -(now - last_time);
	last_time = now;
	if(in_buffer<0) in_buffer=0;
	in_buffer+= (lBytes/(4*44.1));

	//SoundGetBytesBuffered_value = SOUNDSIZE;
	//printf("soundfeedstream %ld\n", lBytes);
	//ccc+=100;
	if(lBytes>4800){
		printf("lBytes>4800 %ld\n", lBytes);
		lBytes=4800;
	}
	SoundGetBytesBuffered_value+=lBytes;
	EM_ASM_({
		 SendSound($0, $1);
		}, pSound, lBytes);
	
}
