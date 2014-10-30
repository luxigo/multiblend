#define I g_images[i]

void trim8(void* bitmap, uint32 w, uint32 h, int bpp, int* top, int* left, int* bottom, int* right) {
  size_t p;
  int x,y;
  uint32* b=(uint32*)bitmap;

// find first solid pixel
  x=0; y=0; p=0;
  while (p<(w*h)) {
    if (b[p++]>0x00ffffff) { // or maybe 0x00ffffff
      *top=y;
      *left=x;
      break;
    }
    x++; if (x==w) { x=0; y++; }
  }

// find last solid pixel
  x=w-1; y=h-1; p=w*h-1;
  while (p>=0) {
    if (b[p--]>0x00ffffff) {
      *bottom=y;
      *right=x;
      break;
    }
    x--; if (x<0) { x=w-1; y--; }
  }

  b+=*top*w;
  for (y=*top; y<=*bottom; y++) {
    for (x=0; x<*left; x++) {
      if (b[x]>0x00ffffff) {
        *left=x;
        break;
      }
    }
    for (x=w-1; x>*right; x--) {
      if (b[x]>0x00ffffff) {
        *right=x;
        break;
      }
    }
    b+=w;
  }
}

void trim16(void* bitmap, uint32 w, uint32 h, int bpp, int* top, int* left, int* bottom, int* right) {
  size_t p;
  int x,y;
  uint16* b=(uint16*)bitmap;

// find first solid pixel
  x=0; y=0; p=3;
  while (p<(w*h)) {
    if (b[p]!=0x0000) { // or maybe 0x00ffffff
      *top=y;
      *left=x;
      break;
    }
		p+=4;
    x++; if (x==w) { x=0; y++; }
  }

// find last solid pixel
  x=w-1; y=h-1; p=w*h*4-1;
  while (p>=0) {
    if (b[p]!=0x0000) {
      *bottom=y;
      *right=x;
      break;
    }
		p-=4;
    x--; if (x<0) { x=w-1; y--; }
  }

	b+=*top*w*4;
  for (y=*top; y<=*bottom; y++) {
    for (x=0; x<*left; x++) {
      if (b[x*4+3]!=0x0000) {
        *left=x;
        break;
      }
    }
    for (x=w-1; x>*right; x--) {
      if (b[x*4+3]!=0x0000) {
        *right=x;
        break;
      }
    }
    b+=w*4;
  }
}

void trim(void* bitmap, int w, int h, int bpp, int* top, int* left, int* bottom, int* right) {
  if (bpp==8) trim8(bitmap,w,h,bpp,top,left,bottom,right); else trim16(bitmap,w,h,bpp,top,left,bottom,right);
}

void extract8(struct_image* image, void* bitmap) {
  int x,y;
  size_t p,up;
  int mp=0;
  int masklast=-1,maskthis;
  int maskcount=0;
  size_t temp;
  uint32 pixel;

  image->binary_mask.rows=(uint32*)malloc((image->height+1)*sizeof(uint32));

  up=image->top*image->tiff_width+image->left;

  p=0;
  for (y=0; y<image->height; y++) {
    image->binary_mask.rows[y]=mp;

		pixel=((uint32*)bitmap)[up++];
    if (pixel>0xfeffffff) { // pixel is solid
      ((uint8*)image->channels[0].data)[p]=pixel&0xff;
      ((uint8*)image->channels[1].data)[p]=(pixel>>8)&0xff;
      ((uint8*)image->channels[2].data)[p]=(pixel>>16)&0xff;
      masklast=1;
    } else {
			masklast=0;
		}
    maskcount=1;
	  p++;

		for (x=1; x<image->width; x++) {
      pixel=((uint32*)bitmap)[up++];
      if (pixel>0xfeffffff) { // pixel is solid
				((uint8*)image->channels[0].data)[p]=pixel&0xff;
				((uint8*)image->channels[1].data)[p]=(pixel>>8)&0xff;
				((uint8*)image->channels[2].data)[p]=(pixel>>16)&0xff;
        maskthis=1;
      } else maskthis=0;
      if (maskthis!=masklast) {
        ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
        masklast=maskthis;
        maskcount=1;
      } else maskcount++;
      p++;
    }
    ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
    up+=image->tiff_width-image->width;
  }
	image->binary_mask.rows[y]=mp;

  image->binary_mask.data=(uint32*)malloc(mp<<2);
  temp=mp;

	memcpy(image->binary_mask.data,bitmap,mp<<2);
}

void extract16(struct_image* image, void* bitmap) {
  int x,y;
  size_t p,up;
  int mp=0;
  int masklast=-1,maskthis;
  int maskcount=0;
  size_t temp;
	int mask;

  image->binary_mask.rows=(uint32*)malloc((image->height+1)*sizeof(uint32));

  up=(image->top*image->tiff_width+image->left)*4;

  p=0;
  for (y=0; y<image->height; y++) {
    image->binary_mask.rows[y]=mp;

    mask=((uint16*)bitmap)[up+3];
    if (mask==0xffff) { // pixel is 100% opaque
			((uint16*)image->channels[0].data)[p]=((uint16*)bitmap)[up+2];
      ((uint16*)image->channels[1].data)[p]=((uint16*)bitmap)[up+1];
      ((uint16*)image->channels[2].data)[p]=((uint16*)bitmap)[up];
      masklast=1;
    } else {
			masklast=0;
		}
    maskcount=1;

		up+=4;
		p++;

		for (x=1; x<image->width; x++) {
      mask=((uint16*)bitmap)[up+3];
      if (mask==0xffff) { // pixel is 100% opaque
        ((uint16*)image->channels[0].data)[p]=((uint16*)bitmap)[up+2];
        ((uint16*)image->channels[1].data)[p]=((uint16*)bitmap)[up+1];
        ((uint16*)image->channels[2].data)[p]=((uint16*)bitmap)[up];
        maskthis=1;
      } else {
				maskthis=0;
			}
      if (maskthis!=masklast) {
        ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
        masklast=maskthis;
        maskcount=1;
      } else maskcount++;

			up+=4;
      p++;
    }
    ((uint32*)bitmap)[mp++]=masklast<<31|maskcount;
    up+=(image->tiff_width-image->width)*4;
  }
	image->binary_mask.rows[y]=mp;

  image->binary_mask.data=(uint32*)malloc(mp<<2);
  temp=mp;

	memcpy(image->binary_mask.data,bitmap,mp<<2);
}

void extract(struct_image* image, void* bitmap) {
  if (image->bpp==8) extract8(image, bitmap); else extract16(image, bitmap);
}

#define NEXTMASK { mask=*mask_pointer++; maskcount=mask&0x7fffffff; mask=mask>>31; }
#define PREVMASK { mask=*--mask_pointer; maskcount=mask&0x7fffffff; mask=mask>>31; }

void inpaint8(struct_image* image, uint32* edt) {
  int x,y;
	int c;
  uint32* edt_p=edt;
  uint32* mask_pointer=image->binary_mask.data;
  int maskcount,mask;
  uint32 dist,temp_dist;
  int copy,temp_copy;
	uint8** chan_pointers=(uint8**)malloc(g_numchannels*sizeof(uint8*));
	int* p=(int*)malloc(g_numchannels*sizeof(int));
  bool lastpixel;

	for (c=0; c<g_numchannels; c++) chan_pointers[c]=(uint8*)image->channels[c].data;

// top-left to bottom-right
// first line, first block
  x=0;

  NEXTMASK;
  dist=(1-mask)<<31;
  for (; maskcount>0; maskcount--) edt_p[x++]=dist;

// first line, remaining blocks in first row
  while (x<image->width) {
    NEXTMASK;
    if (mask) {
      for (; maskcount>0; maskcount--) edt_p[x++]=0;
    } else { // mask if off, so previous mask must have been on
      dist=0;
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x-1];
      for (; maskcount>0; maskcount--) {
        dist+=2;
				for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        edt_p[x++]=dist;
      }
    }
  }

  for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p+=image->width;
		for (c=0; c<g_numchannels; c++) chan_pointers[c]+=image->width;
    x=0;

    while (x<image->width) {
      NEXTMASK;
      if (mask) {
        for (; maskcount>0; maskcount--) edt_p[x++]=0;
      } else {
        if (x==0) {
          copy=x-image->width+1;
          dist=edt_p[copy]+3;

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          edt_p[x++]=dist;
          maskcount--;
        }
        if (x+maskcount==image->width) {
          lastpixel=true;
          maskcount--;
        }

        for (; maskcount>0; maskcount--) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          temp_copy=x-image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist; // dist
        }
        if (lastpixel) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist;
        }
      }
    }
  }

// bottom-right to top-left
  // last line

  while (x>0) {
    PREVMASK;
    if (mask) {
      x-=maskcount;
    } else {
      if (x==image->width) {
        x--;
        maskcount--;
      }
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
      for (; maskcount>0; maskcount--) {
        dist=edt_p[x]+2;
        x--;
        if (dist<edt_p[x]) {
          edt_p[x]=dist;
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        } else {
    			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
        }
      }
    }
  }

// remaining lines
  for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p-=image->width;
		for (c=0; c<g_numchannels; c++) chan_pointers[c]-=image->width;
    x=image->width-1;

    while (x>=0) {
      PREVMASK;
      if (mask) {
        x-=maskcount;
      } else {
        if (x==image->width-1) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
          maskcount--;
        }
        if (x-maskcount==-1) {
          lastpixel=true;
          maskcount--;
        }
        for (; maskcount>0; maskcount--) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
        if (lastpixel) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
      }
    }
  }
}

void inpaint16(struct_image* image, uint32* edt) {
  int x,y;
	int c;
  uint32* edt_p=edt;
  uint32* mask_pointer=image->binary_mask.data;
  int maskcount,mask;
  uint32 dist,temp_dist;
  int copy,temp_copy;
	uint16** chan_pointers=(uint16**)malloc(g_numchannels*sizeof(uint16*));
	int* p=(int*)malloc(g_numchannels*sizeof(int));
  bool lastpixel;

	for (c=0; c<g_numchannels; c++) chan_pointers[c]=(uint16*)image->channels[c].data;

// top-left to bottom-right
// first line, first block
  x=0;
  
  NEXTMASK;
  dist=(1-mask)<<31;
  for (; maskcount>0; maskcount--) edt_p[x++]=dist;

// first line, remaining blocks in first row
  while (x<image->width) {
    NEXTMASK;
    if (mask) {
      for (; maskcount>0; maskcount--) edt_p[x++]=0;
    } else { // mask if off, so previous mask must have been on
      dist=0;
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x-1];
      for (; maskcount>0; maskcount--) {
        dist+=2;
				for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        edt_p[x++]=dist;
      }
    }
  }

	for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p+=image->width;
		for (c=0; c<g_numchannels; c++) chan_pointers[c]+=image->width; //p[c]+=image->width;
    x=0;

    while (x<image->width) {
      NEXTMASK;
      if (mask) {
        for (; maskcount>0; maskcount--) edt_p[x++]=0;
      } else {
        if (x==0) {
          copy=x-image->width+1;
          dist=edt_p[copy]+3;

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          edt_p[x++]=dist;
          maskcount--;
        }
        if (x+maskcount==image->width) {
          lastpixel=true;
          maskcount--;
        }

        for (; maskcount>0; maskcount--) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          temp_copy=x-image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist; // dist
        }
        if (lastpixel) {
          dist=edt_p[x-1]+2;
          copy=x-1;

          temp_copy=x-image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x-image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }
          
          if (dist<0x10000000) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
          }
          edt_p[x++]=dist;
        }
      }
    }
  }

// bottom-right to top-left
  // last line

  while (x>0) {
    PREVMASK;
    if (mask) {
      x-=maskcount;
    } else {
      if (x==image->width) {
        x--;
        maskcount--;
      }
			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
      for (; maskcount>0; maskcount--) {
        dist=edt_p[x]+2;
        x--;
        if (dist<edt_p[x]) {
          edt_p[x]=dist;
					for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=p[c];
        } else {
    			for (c=0; c<g_numchannels; c++) p[c]=chan_pointers[c][x];
        }
      }
    }
  }

// remaining lines
  for (y=image->height; y>1; y--) {
    lastpixel=false;
    edt_p-=image->width;
		for (c=0; c<g_numchannels; c++) chan_pointers[c]-=image->width;
    x=image->width-1;

    while (x>=0) {
      PREVMASK;
      if (mask) {
        x-=maskcount;
      } else {
        if (x==image->width-1) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
          maskcount--;
        }
        if (x-maskcount==-1) {
          lastpixel=true;
          maskcount--;
        }
        for (; maskcount>0; maskcount--) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width-1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
        if (lastpixel) {
          dist=edt_p[x];
          copy=0;

          temp_copy=x+1;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width+1;
          temp_dist=edt_p[temp_copy]+3;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          temp_copy=x+image->width;
          temp_dist=edt_p[temp_copy]+2;
          if (temp_dist<dist) {
            dist=temp_dist;
            copy=temp_copy;
          }

          if (copy!=0) {
						for (c=0; c<g_numchannels; c++) chan_pointers[c][x]=chan_pointers[c][copy];
            edt_p[x--]=dist;
          } else x--;
        }
      }
    }
  }
}

void inpaint(struct_image* image, uint32* edt) {
	if (image->bpp==8) inpaint8(image,edt); else inpaint16(image,edt);
}

void tighten() {
  int i;
	int max_right=0,max_bottom=0;

  g_min_left=0x7fffffff;
  g_min_top=0x7fffffff;

  for (i=0; i<g_numimages; i++) {
    g_min_left=min(g_min_left,g_images[i].xpos);
    g_min_top=min(g_min_top,g_images[i].ypos);
  }

  for (i=0; i<g_numimages; i++) {
    g_images[i].xpos-=g_min_left;
    g_images[i].ypos-=g_min_top;
  }

	for (i=0; i<g_numimages; i++) {
		max_right=max(max_right,g_images[i].xpos+g_images[i].width);
		max_bottom=max(max_bottom,g_images[i].ypos+g_images[i].height);
	}

	g_workwidth=max_right;
	g_workheight=max_bottom;
}

void load_images(char** argv, int argc) {
  int i;
	int c;
  int rowsperstrip;
  int s;
  size_t minstripsize;
  int minstripcount;
	int rowsmissing;
  size_t temp;
  size_t temp_t;
	size_t untrimmed_pixels_max=0;
	size_t untrimmed_pixels;
	size_t channel_bytes_max=0;
  float tiff_xpos,tiff_ypos;
  float tiff_xres,tiff_yres;
	int bottom,right;
  uint16 spp;
  uint16 compression;
	void* untrimmed;
	void** channels;
	char* temp_path;

	g_numimages=argc;

  g_images=(struct_image*)malloc(g_numimages*sizeof(struct_image));

	for (i=0; i<g_numimages; i++) {
		I.channels=(struct_channel*)malloc(g_numchannels*sizeof(struct_channel));
		for (c=0; c<g_numchannels; c++) I.channels[c].f=0;
	}

/*************************************************************************
 * open images, get information
 *************************************************************************/
  output(1,"opening images");
	if (g_caching) output(1," (caching enabled)");
	output(1,"...\n");

  for (i=0; i<g_numimages; i++) {
#ifdef WIN32
    strcpy_s(g_images[i].filename,256,argv[i]);
#else
    strncpy(g_images[i].filename,argv[i],256);
#endif

// open image
		I.tiff=TIFFOpen(I.filename, "r");
		if (!I.tiff) die("couldn't open file!");

		if (!TIFFGetField(I.tiff, TIFFTAG_XPOSITION, &tiff_xpos)) tiff_xpos=-1;
		if (!TIFFGetField(I.tiff, TIFFTAG_YPOSITION, &tiff_ypos)) tiff_ypos=-1;
		if (!TIFFGetField(I.tiff, TIFFTAG_XRESOLUTION, &tiff_xres)) tiff_xres=-1;
		if (!TIFFGetField(I.tiff, TIFFTAG_YRESOLUTION, &tiff_yres)) tiff_yres=-1;
		TIFFGetField(I.tiff, TIFFTAG_IMAGEWIDTH, &I.tiff_width);
		TIFFGetField(I.tiff, TIFFTAG_IMAGELENGTH, &I.tiff_height);
		TIFFGetField(I.tiff, TIFFTAG_BITSPERSAMPLE, &I.bpp);
		TIFFGetField(I.tiff, TIFFTAG_SAMPLESPERPIXEL, &spp);
		TIFFGetField(I.tiff, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		TIFFGetField(I.tiff, TIFFTAG_COMPRESSION, &compression);

		if (I.bpp!=8 && I.bpp!=16) die("%dbpp not valid!",I.bpp);
		if (spp!=4) die("can't handle <>4spp!");

		if (i>0 && I.bpp!=g_images[i-1].bpp) die ("can't handle a mix of 8bpp and 16bpp images!");

	// read geotiff offsets
		g_images[i].geotiff.set=false;
		if (tiff_xpos==-1 && tiff_ypos==-1) {
			// try to read geotiff tags
			if (geotiff_read(I.tiff, &(g_images[i].geotiff))) {
				I.xpos=(int)(g_images[i].geotiff.XGeoRef/g_images[0].geotiff.XCellRes);
				I.ypos=(int)(g_images[i].geotiff.YGeoRef/g_images[0].geotiff.YCellRes);
			}
		} else {
			if (tiff_xpos!=-1 && tiff_xres>0) I.xpos=(int)(tiff_xpos*tiff_xres+0.5);
			if (tiff_ypos!=-1 && tiff_yres>0) I.ypos=(int)(tiff_ypos*tiff_yres+0.5);
			if (g_xres==-1 || g_yres==-1) {
				g_xres=tiff_xres;
				g_yres=tiff_yres;
			} else if (g_xres!=tiff_xres || g_yres!=tiff_yres) {
				output(0,"warning: image resolution mismatch (%f %f/%f %f)\n", g_xres,g_yres, tiff_xres,tiff_yres);
			}
		}

		I.first_strip=0;
		I.last_strip=TIFFNumberOfStrips(I.tiff)-1;

		if (tiff_xpos<=0 && tiff_ypos<=0 && compression!=1) {
			minstripsize=0x7fffffff;
			minstripcount=0;
			for (s=0; s<(int)TIFFNumberOfStrips(I.tiff)-1; s++) {
				temp=TIFFRawStripSize(I.tiff,s);
//				printf("%d: %d\n",s,temp);
//				if ((s&63)==0) getchar();
				if (temp<minstripsize) { minstripsize=temp; minstripcount=1; }
				else if (temp==minstripsize) minstripcount++;
			}

			if (minstripcount>2) {
				I.first_strip=-1;
				for (s=0; s<(int)TIFFNumberOfStrips(I.tiff)-1; s++) {
					temp=TIFFRawStripSize(I.tiff,s);
					if (temp!=minstripsize) {
						if (I.first_strip==-1) I.first_strip=s;
						I.last_strip=s;
					}
				}
				if (I.first_strip==-1) I.first_strip=0;
				if (I.last_strip==s-1) I.last_strip=s; // after loop, s==TIFFNumberOfStrips(I.tiff)-1 (i.e. last strip)
			}
		} // else output(1,"uncompressed image; smartcropping disabled\n");

    I.ypos+=I.first_strip*rowsperstrip;
		rowsmissing=TIFFNumberOfStrips(I.tiff)*rowsperstrip-I.tiff_height;

		I.tiff_u_height=(I.last_strip+1-I.first_strip)*rowsperstrip;
		if (I.last_strip==TIFFNumberOfStrips(I.tiff)-1) I.tiff_u_height-=rowsmissing;

		untrimmed_pixels=I.tiff_u_height*I.tiff_width;
		if (untrimmed_pixels>untrimmed_pixels_max) untrimmed_pixels_max=untrimmed_pixels;
	}

	temp_t=untrimmed_pixels_max<<(g_images[0].bpp>>2); // bad assumption unless 8-bit+16-bit is forbidden
  untrimmed=_TIFFmalloc(temp_t);
	if (!untrimmed) die("not enough memory to process images");

  channels=(void**)malloc(g_numchannels*sizeof(void*));

	if (g_caching) {
		for (c=0; c<g_numchannels; c++) {
			channels[c]=malloc(untrimmed_pixels_max<<(g_images[0].bpp>>4));
			if (!channels[c]) die("not enough memory for cache channel %d",c);
		}

#ifdef WIN32
		temp_path=(char*)malloc(MAX_PATH);
		GetTempPath(MAX_PATH,temp_path);
#endif
	}

/*************************************************************************
 * image processing loop
 *************************************************************************/
	for (i=0; i<g_numimages; i++) {
    output(1,"processing %s...\n",I.filename);

		char* pointer=(char*)untrimmed;
		for (s=I.first_strip; s<=I.last_strip; s++) pointer+=TIFFReadEncodedStrip(I.tiff, s, pointer, -1);

		g_workwidth=max(g_workwidth,(int)(I.xpos+I.tiff_width));
    g_workheight=max(g_workheight,(int)(I.ypos+I.tiff_height));

		trim(untrimmed,I.tiff_width,I.tiff_u_height,I.bpp,&I.top,&I.left,&bottom,&right);

    I.xpos+=I.left; I.ypos+=I.top;
    I.width=right-I.left+1; I.height=bottom-I.top+1;

		temp_t=(I.width*I.height)<<(I.bpp>>4);
		channel_bytes_max=max(channel_bytes_max, temp_t);

		if (g_caching) {
			for (c=0; c<g_numchannels; c++) {
				I.channels[c].data=channels[c];
#ifdef WIN32
				I.channels[c].filename=(char*)malloc(MAX_PATH);
				GetTempFileName(temp_path,"mb",0,I.channels[c].filename);
				if (fopen_s(&I.channels[c].f,I.channels[c].filename,"wb")) die("couldn't open channel file");
#else
				I.channels[c].f=tmpfile();
				if (!I.channels[c].f) die("couldn't open channel file");
#endif
			}
		} else {
			for (c=0; c<g_numchannels; c++) if (!(I.channels[c].data=(void*)malloc(temp_t))) die("not enough memory for image channel");
		}

		extract(&I, untrimmed);
		inpaint(&I, (uint32*)untrimmed);

		if (g_caching) for (c=0; c<g_numchannels; c++) {
			fwrite(I.channels[c].data, temp_t, 1, I.channels[c].f);
#ifdef WIN32
			fclose(I.channels[c].f);
			fopen_s(&I.channels[c].f,I.channels[c].filename,"rb"); // reopen required under Windows
#endif
		}
	}

	if (g_caching) {
		for (c=0; c<g_numchannels; c++) free(channels[c]);
		g_cache_bytes=channel_bytes_max;
	}

	if (g_crop) tighten();

	if (g_workbpp_cmd!=0) {
		if (g_workbpp_cmd==16 && g_jpegquality!=-1) {
			output(0,"warning: JPEG output; overriding to 8bpp\n");
			g_workbpp_cmd=8;
		}
		g_workbpp=g_workbpp_cmd;
	} else {
		g_workbpp=g_images[0].bpp;
	}
}
