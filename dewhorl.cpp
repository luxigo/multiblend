void dewhorl_old() {
	int l;
	int x;
//	int zero;
	int total=0;

	output(1,"dewhorling...\n");

	for (l=0; l<g_levels; l++) {
		if (l<g_levels-1) {
			if (g_workbpp==8) {
				memset(g_output_pyramid[l].data,0,g_output_pyramid[l].pitch<<2);
			} else {
				memset(g_output_pyramid[l].data,0,g_output_pyramid[l].pitch<<4);
			}
		} else {
			for (x=0; x<g_output_pyramid[l].pitch<<1; x++) {
				if (g_workbpp==8) {
					total+=((short*)(g_output_pyramid[l].data))[x];
				} else {
					total+=((int*)(g_output_pyramid[l].data))[x];
				}
			}

			total=(int)(total*(1.0/(g_output_pyramid[l].pitch<<1))+0.5);

      for (x=0; x<g_output_pyramid[l].pitch<<1; x++) {
				if (g_workbpp==8) {
				  ((short*)(g_output_pyramid[l].data))[x]=total;
				} else {
					((int*)(g_output_pyramid[l].data))[x]=total;
				}
			}
		}
	}
}

void dewhorl() {
	int xx,x,y;
	int end_row=8;
	int w,window;
	long long total=0;
	void* p=g_output_pyramid[0].data;
	double d;

	output(1,"dewhorling...\n");

  for (y=0; y<end_row; y++) {
//if (y==1) printf("%d\n=\n",y);
		d=1-sin(y*(1.0/end_row)*3.1415926535897932384626433832795*0.5);
		window=(int)(g_workwidth*0.5*d);
//		window=100;
//		printf("%d,%d\n",y,window);
		for (x=0; x<g_workwidth; x++) {
//if (y==1) printf("=%d: ",x);
			xx=(x-window);
			if (xx<0) xx+=g_workwidth;
//((short*)g_line0)[x]=((short*)p)[xx]; continue;
			total=0;
			for (w=0; w<window*2+1; w++) {
//if (y==1) printf("%d ",xx);
				if (g_workbpp==8) total+=((short*)p)[xx]; else total+=((int*)p)[xx];
				xx++;
				if (xx==g_workwidth) xx=0;
			}
			total=(int)(total*(1.0/(window*2+1))+0.5);
			if (g_workbpp==8) ((short*)g_line0)[x]=(short)total; else ((int*)g_line0)[x]=(int)total;
//			((short*)p)[x]=x<<3;
//if (y==1) getchar();
		}
		memcpy(p,g_line0,g_output_pyramid[0].pitch<<1); // needs changing for g_workbpp==16
		p=(void*)((int)p+(g_output_pyramid[0].pitch<<1)); // needs changing for g_workbpp==16
	}
}
