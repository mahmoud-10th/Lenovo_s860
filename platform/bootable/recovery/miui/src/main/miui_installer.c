/*
 * Copyright (C) 2014 lenovo miui
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */ 
/*
 * Descriptions:
 * -------------
 * Installer Proccess
 *
 */
//#define DEBUG //wangxf14_debug

#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#include "../miui_inter.h"
#include "../miui.h"
#include "../../../miui_intent.h"
#include "../../../recovery.h"
#include "../../../lenovo_ota.h"

static byte      ai_run              = 0;
static byte      ai_progress_run  = 0;
static int       ai_progani_pos      = 0;
static float     ai_progress_pos     = 0;
static float     ai_progress_fract   = 0; 
static int       ai_progress_fract_n = 0;
static int       ai_progress_fract_c = 0;
static long      ai_progress_fract_l = 0;
static int       ai_progress_w     = 0;
static int       ai_prog_x         = 0;
static int       ai_prog_y         = 0;
static int       ai_prog_w         = 0;
static int       ai_prog_h         = 0;
static int       ai_prog_r         = 0;
static int       ai_prog_ox        = 0;
static int       ai_prog_oy        = 0;
static int       ai_prog_ow        = 0;
static int       ai_prog_oh        = 0;
static int       ai_prog_or        = 0;
static int       ai_indeterminate_prog_ox        = 0;
static int       ai_indeterminate_prog_oy        = 0;
static int       ai_indeterminate_prog_ow        = 0;
static int       ai_indeterminate_prog_oh        = 0;

static CANVAS *  ai_bg             = NULL;
static CANVAS *  ai_progress_bg = NULL;
static CANVAS *  ai_cv             = NULL;
static CANVAS *  ai_progress_cv = NULL;
static char      ai_progress_text[64];
static char      ai_progress_info[101];
static AWINDOWP  ai_win;
static AWINDOWP  ai_progress_win;
static ACONTROLP ai_buftxt;
static ACONTROLP ai_update; //lenovo-sw wangxf14 20130705 add, add for update controls
static ACONTROLP ai_indeterminate_progress;
static int ai_indeterminate_frames = 8;

static pthread_mutex_t ai_progress_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t ai_canvas_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
	const char *ctl1;
	const char *ctl2;
}WIPE_CONTROLD, * WIPE_CONTROLDP;

void ai_canvas_lock()
{
    pthread_mutex_lock(&ai_canvas_mutex);
}
void ai_canvas_unlock()
{
    pthread_mutex_unlock(&ai_canvas_mutex);
}

#define MIUI_INSTALL_LOG "/tmp/install.log"

static struct _miuiInstall miui_install_struct ={
    .pfun = NULL,
    .path = NULL,
    .install_file = NULL,
    .wipe_cache = 0
};
static struct _miuiInstall* pmiui_install = &miui_install_struct;

//echo text when install failed
void ai_rebuildtxt(int cx,int cy,int cw,int ch){
  char* buffer = NULL;
  struct stat st;
  if (stat(MIUI_INSTALL_LOG,&st) < 0) return;
  buffer = malloc(st.st_size+1);
  if (buffer == NULL) goto done;  
  FILE* f = fopen(MIUI_INSTALL_LOG, "rb");
  if (f == NULL) goto done;
  if (fread(buffer, 1, st.st_size, f) != st.st_size){
      fclose(f);
      goto done;
  }
  buffer[st.st_size] = '\0';
  fclose(f);
done:
  actext_rebuild(
    ai_buftxt,
    cx,cy,cw,ch,
    ((buffer!=NULL)?buffer:""),
    0,1);
  free(buffer);
  
}

void ai_actionsavelog(char * name){
  char* buffer = NULL;
  struct stat st;
  if (stat(MIUI_INSTALL_LOG,&st) < 0) return;
  buffer = malloc(st.st_size+1);
  if (buffer == NULL) goto done;
  
  FILE* f = fopen(MIUI_INSTALL_LOG, "rb");
  if (f == NULL) goto done;
  if (fread(buffer, 1, st.st_size, f) != st.st_size){
      fclose(f);
      goto done;
  }
  buffer[st.st_size] = '\0';
  fclose(f);
  
  f = fopen(name, "wb");
  if (f == NULL) 
  {
      miui_error("%s open failed!\n", name);
      goto done;
  }
  fprintf(f,"%s", buffer);
  fclose(f);
done:
  if (buffer!=NULL) free(buffer);
}

void ai_dump_logs(){
  char dumpname[256];
  char msgtext[256];
  snprintf(dumpname,255,"%s/install.log",RECOVERY_PATH);
  snprintf(msgtext,255,"Install Log will be saved into:\n\n<#060>%s</#>\n\nAre you sure you want to save it?",dumpname);
  
  byte res = aw_confirm(
    ai_win,
    "Save Install Log",
    msgtext,
    "@alert",
    NULL,
    NULL
  );
  
  if (res){
    ai_actionsavelog(dumpname);
    //rename(MIUI_INSTALL_LOG,dumpname);
    aw_alert(
      ai_win,
      "Save Install Log",
      "Install Logs has been saved...",
      "@info",
      NULL
    );
  }
  
}
static void *miui_install_package(void *cookie){
    int ret = 0;
    if (pmiui_install->pfun != NULL)
    {
        //run install process
        ret = pmiui_install->pfun(pmiui_install->path, &pmiui_install->wipe_cache, pmiui_install->install_file);
        if (pmiui_install->wipe_cache)
            miuiIntent_send(INTENT_WIPE, 1 , "/cache");
        miuiInstall_set_progress(1);
        if (ret == 0)
        {
            //sucecess installed
            aw_post(aw_msg(15, 0, 0, 0));
            miui_printf("miui_install_package install package sucess!\n");
        }
        //else installed failed
        else 
        {
            aw_post(aw_msg(16, 0, 0, 0));
            miui_error("miui_install_package install package failed!\n");
        }
        return NULL;
    }
    miui_error("pmiui_install->pfun is NULL, force return");
    return NULL;
}

/* Begin, lenovo-sw wangxf14 20130705 add, add for lenovo update progressthread */
static void *ac_lenovo_progressthread(void *cookie)
{
#if 0
  //-- COLORS
  dword hl1 = ag_calchighlight(acfg()->selectbg,acfg()->selectbg_g);
  byte sg_r = ag_r(acfg()->progressglow);
  byte sg_g = ag_g(acfg()->progressglow);
  byte sg_b = ag_b(acfg()->progressglow);
  sg_r = min(sg_r*1.4,255);
  sg_g = min(sg_g*1.4,255);
  sg_b = min(sg_b*1.4,255);
#endif
  while(ai_run){    
    miui_debug("ai_progress_fract_n = %d, ai_progress_pos = %f ai_prog_w = %d\n", ai_progress_fract_n, ai_progress_pos, ai_prog_w);
    //-- CALCULATE PROGRESS BY TIME
    pthread_mutex_lock(&ai_progress_mutex);
    if(ai_progress_fract_n<0){
      long curtick  = alib_tick();
      int  targetc  = abs(ai_progress_fract_n);
      long  tickdiff = curtick - ai_progress_fract_l;
      if (tickdiff>0){
        long diffms          = tickdiff*10;
        ai_progress_fract_l  = curtick;
        ai_progress_fract_n += diffms;
        if (ai_progress_fract_n>=0){
          diffms-=ai_progress_fract_n;
          ai_progress_fract_n = 0;
        }
        float curradd        = ai_progress_fract*diffms;
        ai_progress_pos     += curradd;
      }
    }
    
    //-- Safe Progress
    if (ai_progress_pos>1) ai_progress_pos=1.0;
    if (ai_progress_pos<0) ai_progress_pos=0.0;
#if 0	
    int prog_g = ai_prog_w; //-(ai_prog_r*2);
    int prog_w = round(ai_prog_w*ai_progress_pos);

    miui_debug("prog_g = %d prog_w = %d\n", prog_g, prog_w);
#endif
    pthread_mutex_unlock(&ai_progress_mutex);

    //-- Percent Text
    float prog_percent = 100 * ai_progress_pos;
    char prog_percent_str[10];
    int prog_percent_int = prog_percent;

    snprintf(prog_percent_str,9,"%2d%c",prog_percent_int,'%');
    miui_debug(" prog_percent = %d \n", prog_percent_int);
    miui_debug(" prog_percent_str = %s \n", prog_percent_str);

	
    int ptxt_w = ag_txtwidth(prog_percent_str,1);
    int ptxt_h = ag_txtheight(ptxt_w, prog_percent_str, 1);

    int ptxt_x = (get_ac_update_text_imgW(ai_update) - ptxt_w)/2;
    int ptxt_y = (get_ac_update_text_imgH(ai_update) - ptxt_h)/2;
	
    miui_debug("ai_prog_ox = %d, ai_prog_oy = %d, ai_prog_ow = %d, ai_prog_or = %d \n", ai_prog_ox, ai_prog_oy, ai_prog_ow, ai_prog_or);
    miui_debug("ptxt_y = %d, ptxt_w = %d, ptxt_x = %d\n", ptxt_y, ptxt_w, ptxt_x);
#if 0	
    miui_debug("ai_progress_w = %d, prog_w = %d\n", ai_progress_w, prog_w);
#endif
#if 0	
    if (ai_progress_w<prog_w){
      int diff       = ceil((prog_w-ai_progress_w)*0.1);
      ai_progress_w +=diff;
      if (ai_progress_w>prog_w) ai_progress_w=prog_w;
    }
    else if (ai_progress_w>prog_w){
      int diff       = ceil((ai_progress_w-prog_w)*0.1);
      ai_progress_w -=diff;
      if (ai_progress_w<prog_w) ai_progress_w=prog_w;
    }

    miui_debug("ai_progress_w = %d, (ai_prog_r*2) = %d\n", ai_progress_w, (ai_prog_r*2));	

    int issmall = -1;
    if (ai_progress_w<(ai_prog_r*2)){
      issmall = ai_progress_w;
      ai_progress_w = (ai_prog_r*2);
    }

    miui_debug("after compare : ai_progress_w = %d, (ai_prog_r*2) = %d\n", ai_progress_w, (ai_prog_r*2));	
#endif
    ai_canvas_lock();
    int tmp_oy = agh() - agdp() * 50 - agdp() * 4;
    ag_draw_ex(ai_cv,ai_bg,0,tmp_oy,0,tmp_oy,agw(),agh()-tmp_oy);//wangxf14_debug

    int curr_prog_w = round(ai_prog_ow*ai_progress_pos);

    miui_debug("curr_prog_w = %d\n", curr_prog_w);

    if (!atheme_draw("img.progress.primary",ai_cv,ai_prog_ox,ai_prog_oy,curr_prog_w,ai_prog_oh)){
      miui_debug("wangxf14 failure atheme draw img.progress.primary\n");
#if 0	  
      ag_roundgrad(ai_cv,ai_prog_x,ai_prog_y,ai_progress_w,ai_prog_h,acfg()->selectbg,acfg()->selectbg_g,ai_prog_r);
      ag_roundgrad_ex(ai_cv,ai_prog_x,ai_prog_y,ai_progress_w,ceil((ai_prog_h)/2.0),LOWORD(hl1),HIWORD(hl1),ai_prog_r,2,2,0,0);
      if (issmall>=0){
        ag_draw_ex(ai_cv,ai_bg,ai_prog_x+issmall,ai_prog_oy,ai_prog_x+issmall,ai_prog_oy,(ai_prog_r*2),ai_prog_oh);
      }
#endif	  
    }

   CANVAS tmpCanvas = get_ac_update_text_Client(ai_update);
	
    if(!atheme_draw("img.icon.install.normal", &tmpCanvas, 0, 0,get_ac_update_text_imgW(ai_update), get_ac_update_text_imgH(ai_update)))
    {
        miui_debug("miui_installer atheme_draw background failure!\n");
    }
    ag_texts (&tmpCanvas,ptxt_w,ptxt_x,ptxt_y,prog_percent_str,acfg()->textfg, 1);
    ag_draw_ex(ai_cv, &tmpCanvas, get_ac_update_text_imgX(ai_update), get_ac_update_text_imgY(ai_update), 0, 0, get_ac_update_text_imgW(ai_update), get_ac_update_text_imgH(ai_update));    

#if 0    
    prog_g = ai_prog_w-(ai_prog_r*2);

    miui_debug("prog_g = %d\n", prog_g);
#endif

#if 0	
    
    if (++ai_progani_pos>60) ai_progani_pos=0;
    int x    = ai_progani_pos;
    int hpos = prog_g/2;
    int vpos = ((prog_g+hpos)*x) / 60;
    int hhpos= prog_g/4;
    int hph  = ai_prog_h/2;
    int xx;    
    int  sgmp = agdp()*40;

    miui_debug("vpos = %d, hpos = %d\n", vpos, hpos);
    
    if ((vpos>0)&&(hpos>0)){
      for (xx=0;xx<prog_g;xx++){
        int alp     = 255;
        float alx   = 1.0;
        int vn = (vpos-xx)-hhpos;
        if ((vn>0)){
          if (vn<hhpos){
            alp = (((hhpos-vn) * 255) / hhpos);
          }
          else if (vn<hpos){
            alp = (((vn-hhpos) * 255) / hhpos);
          }
        }

        if (xx<sgmp){
          alx = 1.0-(((float) (sgmp -xx)) / sgmp);
        }
        else if (xx>prog_g-sgmp){
          alx = 1.0-(((float) (xx-(prog_g-sgmp))) / sgmp);
        }
        int alpha = min(max(alx * (255-alp),0),255);

	 miui_debug("alpha = %d\n", alpha);

        int anix = ai_prog_x+ai_prog_r+xx;
        int yy;
        byte er = 0;
        byte eg = 0;
        byte eb = 0;
        for (yy=0;yy<ai_prog_oh;yy++){
          color * ic = agxy(ai_cv,anix,ai_prog_oy+yy);//wangxf14_study
          byte  l  = alpha*(0.5+((((float) yy+1)/((float) ai_prog_oh))*0.5));
          byte  ralpha = 255 - l;
          byte r = (byte) (((((int) ag_r(ic[0])) * ralpha) + (((int) sg_r) * l)) >> 8);
          byte g = (byte) (((((int) ag_g(ic[0])) * ralpha) + (((int) sg_g) * l)) >> 8);
          byte b = (byte) (((((int) ag_b(ic[0])) * ralpha) + (((int) sg_b) * l)) >> 8);
          r  = min(r+er,255);
          g  = min(g+eg,255);
          b  = min(b+eb,255);
          byte nr  = ag_close_r(r);
          byte ng  = ag_close_g(g);
          byte nb  = ag_close_b(b);
          er = r-nr;
          eg = g-ng;
          eb = b-nb;
          ic[0]=ag_rgb(nr,ng,nb);
        }
      }
    }

#endif

    //ag_draw(NULL,ai_cv,0,0);
    //ag_sync();
    
    aw_draw(ai_win);
    ai_canvas_unlock();
    usleep(160);
  }
  return NULL;
}
/* end, lenovo-sw wangxf14 20130705 add, add for lenovo update progressthread */

static void *ac_progressthread(void *cookie){
  //-- COLORS
  dword hl1 = ag_calchighlight(acfg()->selectbg,acfg()->selectbg_g);
  byte sg_r = ag_r(acfg()->progressglow);
  byte sg_g = ag_g(acfg()->progressglow);
  byte sg_b = ag_b(acfg()->progressglow);
  sg_r = min(sg_r*1.4,255);
  sg_g = min(sg_g*1.4,255);
  sg_b = min(sg_b*1.4,255);
  
  while(ai_run){
    
    miui_debug("ai_progress_fract_n = %d, ai_progress_pos = %f", ai_progress_fract_n, ai_progress_pos);
    //-- CALCULATE PROGRESS BY TIME
    pthread_mutex_lock(&ai_progress_mutex);
    if(ai_progress_fract_n<0){
      long curtick  = alib_tick();
      int  targetc  = abs(ai_progress_fract_n);
      long  tickdiff = curtick - ai_progress_fract_l;
      if (tickdiff>0){
        long diffms          = tickdiff*10;
        ai_progress_fract_l  = curtick;
        ai_progress_fract_n += diffms;
        if (ai_progress_fract_n>=0){
          diffms-=ai_progress_fract_n;
          ai_progress_fract_n = 0;
        }
        float curradd        = ai_progress_fract*diffms;
        ai_progress_pos     += curradd;
      }
    }
    
    //-- Safe Progress
    if (ai_progress_pos>1) ai_progress_pos=1.0;
    if (ai_progress_pos<0) ai_progress_pos=0.0;
    int prog_g = ai_prog_w; //-(ai_prog_r*2);
    int prog_w = round(ai_prog_w*ai_progress_pos);

    miui_debug("prog_g = %d prog_w = %d\n", prog_g, prog_w);

    pthread_mutex_unlock(&ai_progress_mutex);

    //-- Percent Text
    float prog_percent = 100 * ai_progress_pos;
    char prog_percent_str[10];
    snprintf(prog_percent_str,9,"%0.2f%c",prog_percent,'%');
//wangxf14_pause	
    int ptxt_p = agdp()*5;
    int ptxt_y = ai_prog_oy-(ptxt_p+(ag_fontheight(0)*2));
    int ptxt_w = ag_txtwidth(prog_percent_str,0);
    int ptxt_x = (ai_prog_ox+ai_prog_ow)-(ptxt_w+ai_prog_or);
	
    miui_debug("ai_prog_ox = %d, ai_prog_oy = %d, ai_prog_ow = %d, ai_prog_or = %d \n", ai_prog_ox, ai_prog_oy, ai_prog_ow, ai_prog_or);
    miui_debug("ptxt_p = %d, ptxt_y = %d, ptxt_w = %d, ptxt_x = %d\n", ptxt_p, ptxt_y, ptxt_w, ptxt_x);
	
    int ptx1_x = ai_prog_ox+ai_prog_or;
    int ptx1_w = agw()-(agw()/3);

    miui_debug("ptx1_x = %d, ptx1_w = %d\n", ptx1_x, ptx1_w);

    miui_debug("ai_progress_w = %d, prog_w = %d\n", ai_progress_w, prog_w);
	
    if (ai_progress_w<prog_w){
      int diff       = ceil((prog_w-ai_progress_w)*0.1);
      ai_progress_w +=diff;
      if (ai_progress_w>prog_w) ai_progress_w=prog_w;
    }
    else if (ai_progress_w>prog_w){
      int diff       = ceil((ai_progress_w-prog_w)*0.1);
      ai_progress_w -=diff;
      if (ai_progress_w<prog_w) ai_progress_w=prog_w;
    }

    miui_debug("ai_progress_w = %d, (ai_prog_r*2) = %d\n", ai_progress_w, (ai_prog_r*2));	

    int issmall = -1;
    if (ai_progress_w<(ai_prog_r*2)){
      issmall = ai_progress_w;
      ai_progress_w = (ai_prog_r*2);
    }
    
    ai_canvas_lock();
    ag_draw_ex(ai_cv,ai_bg,0,ptxt_y,0,ptxt_y,agw(),agh()-ptxt_y);
    int curr_prog_w = round(ai_prog_ow*ai_progress_pos);

    miui_debug("curr_prog_w = %d\n", curr_prog_w);

    if (!atheme_draw("img.prograss.fill",ai_cv,ai_prog_ox,ai_prog_oy,curr_prog_w,ai_prog_oh)){
      ag_roundgrad(ai_cv,ai_prog_x,ai_prog_y,ai_progress_w,ai_prog_h,acfg()->selectbg,acfg()->selectbg_g,ai_prog_r);
      ag_roundgrad_ex(ai_cv,ai_prog_x,ai_prog_y,ai_progress_w,ceil((ai_prog_h)/2.0),LOWORD(hl1),HIWORD(hl1),ai_prog_r,2,2,0,0);
      if (issmall>=0){
        ag_draw_ex(ai_cv,ai_bg,ai_prog_x+issmall,ai_prog_oy,ai_prog_x+issmall,ai_prog_oy,(ai_prog_r*2),ai_prog_oh);
      }
    }

//wangxf14 pause    
    ag_textfs(ai_cv,ptx1_w,ptx1_x+1,ptxt_y+1,ai_progress_text,acfg()->winbg,0);
    ag_texts (ai_cv,ptx1_w,ptx1_x  ,ptxt_y  ,ai_progress_text,acfg()->winfg,0);
    ag_textfs(ai_cv,ai_prog_w-(ai_prog_or*2),ptx1_x+1,ptxt_y+1+ag_fontheight(0),ai_progress_info,acfg()->winbg,0);
    ag_texts (ai_cv,ai_prog_w-(ai_prog_or*2),ptx1_x  ,ptxt_y+ag_fontheight(0)+agdp(),ai_progress_info,acfg()->winfg_gray,0);

    ag_textfs(ai_cv,ptxt_w,ptxt_x+1,ptxt_y+1,prog_percent_str,acfg()->winbg,0);
    ag_texts (ai_cv,ptxt_w,ptxt_x,ptxt_y,prog_percent_str,acfg()->winfg,0);
    
    prog_g = ai_prog_w-(ai_prog_r*2);

    miui_debug("prog_g = %d\n", prog_g);
    
    if (++ai_progani_pos>60) ai_progani_pos=0;
    int x    = ai_progani_pos;
    int hpos = prog_g/2;
    int vpos = ((prog_g+hpos)*x) / 60;
    int hhpos= prog_g/4;
    int hph  = ai_prog_h/2;
    int xx;    
    int  sgmp = agdp()*40;

    miui_debug("vpos = %d, hpos = %d\n", vpos, hpos);
    
    if ((vpos>0)&&(hpos>0)){
      for (xx=0;xx<prog_g;xx++){
        int alp     = 255;
        float alx   = 1.0;
        int vn = (vpos-xx)-hhpos;
        if ((vn>0)){
          if (vn<hhpos){
            alp = (((hhpos-vn) * 255) / hhpos);
          }
          else if (vn<hpos){
            alp = (((vn-hhpos) * 255) / hhpos);
          }
        }

        if (xx<sgmp){
          alx = 1.0-(((float) (sgmp -xx)) / sgmp);
        }
        else if (xx>prog_g-sgmp){
          alx = 1.0-(((float) (xx-(prog_g-sgmp))) / sgmp);
        }
        int alpha = min(max(alx * (255-alp),0),255);

	 miui_debug("alpha = %d\n", alpha);

        int anix = ai_prog_x+ai_prog_r+xx;
        int yy;
        byte er = 0;
        byte eg = 0;
        byte eb = 0;
        for (yy=0;yy<ai_prog_oh;yy++){
          color * ic = agxy(ai_cv,anix,ai_prog_oy+yy);//wangxf14_study
          byte  l  = alpha*(0.5+((((float) yy+1)/((float) ai_prog_oh))*0.5));
          byte  ralpha = 255 - l;
          byte r = (byte) (((((int) ag_r(ic[0])) * ralpha) + (((int) sg_r) * l)) >> 8);
          byte g = (byte) (((((int) ag_g(ic[0])) * ralpha) + (((int) sg_g) * l)) >> 8);
          byte b = (byte) (((((int) ag_b(ic[0])) * ralpha) + (((int) sg_b) * l)) >> 8);
          r  = min(r+er,255);
          g  = min(g+eg,255);
          b  = min(b+eb,255);
          byte nr  = ag_close_r(r);
          byte ng  = ag_close_g(g);
          byte nb  = ag_close_b(b);
          er = r-nr;
          eg = g-ng;
          eb = b-nb;
          ic[0]=ag_rgb(nr,ng,nb);
        }
      }
    }
    
    //ag_draw(NULL,ai_cv,0,0);
    //ag_sync();
    
    aw_draw(ai_win);
    ai_canvas_unlock();
    usleep(160);
  }
  return NULL;
}

void miui_init_install(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph
){
  //-- Calculate Progress Location&Size
  
  pthread_mutex_lock(&ai_progress_mutex);
  ai_prog_oh = agdp()*10;
  ai_prog_oy = 0;
  ai_prog_ox = px;
  ai_prog_ow = pw;
  if (ai_prog_oh>ph) ai_prog_oh=ph;
  else{
    ai_prog_oy = (ph/2)-(ai_prog_oh/2);
  }
  ai_prog_oy += py;
  ai_prog_or = ai_prog_oh/2;

  miui_debug("ai_prog_oy = %d, ai_prog_oh = %d\n", ai_prog_oy, ai_prog_oh);

  //-- Draw Progress Holder Into BG
  dword hl1 = ag_calchighlight(acfg()->controlbg,acfg()->controlbg_g);
  
  if (!atheme_draw("img.progress",bg,px,ai_prog_oy,pw,ai_prog_oh)){
    ag_roundgrad(bg,px,ai_prog_oy,pw,ai_prog_oh,acfg()->border,acfg()->border_g,ai_prog_or);
    ag_roundgrad(bg,px+1,ai_prog_oy+1,pw-2,ai_prog_oh-2,
      ag_calculatealpha(acfg()->controlbg,0xffff,180),
      ag_calculatealpha(acfg()->controlbg_g,0xffff,160), ai_prog_or-1);
    ag_roundgrad(bg,px+2,ai_prog_oy+2,pw-4,ai_prog_oh-4,acfg()->controlbg,acfg()->controlbg_g,ai_prog_or-2);
    ag_roundgrad_ex(bg,px+2,ai_prog_oy+2,pw-4,ceil((ai_prog_oh-4)/2.0),LOWORD(hl1),HIWORD(hl1),ai_prog_or-2,2,2,0,0);
  }
  
  //-- Calculate Progress Value Locations
  int hlfdp  = ceil(((float) agdp())/2);
  ai_prog_x = px+(hlfdp+1);
  ai_prog_y = ai_prog_oy+(hlfdp+1);
  ai_prog_h = ai_prog_oh-((hlfdp*2)+2);
  ai_prog_w = pw-((hlfdp*2)+2);
  ai_prog_r = ai_prog_or-(1+hlfdp);

  miui_debug("ai_prog_x = %d, ai_prog_y = %d, ai_prog_h = %d, ai_prog_w = %d, ai_prog_r = %d", ai_prog_x, ai_prog_y, ai_prog_h, ai_prog_w, ai_prog_r);
  
  snprintf(ai_progress_text,63,"Initializing...");
  snprintf(ai_progress_info,100,"");
  pthread_mutex_unlock(&ai_progress_mutex);
  return ;
}

/* Begin, lenovo-sw wangxf14 20130705 add, add for lenovo init install */
void miui_lenovo_init_install(
  CANVAS * bg,
  int px, int py, int pw, int ph
){
  //-- Calculate Progress Location&Size
  pthread_mutex_lock(&ai_progress_mutex);

  ai_prog_oh = agdp()*10;
  ai_prog_oy = 0;
  ai_prog_ox = px;
  ai_prog_ow = pw;

//  ai_prog_oy += imgY + imgH + agdp() * 50;
  ai_prog_oy = agh() - agdp() * 50 - agdp() * 4;
  ai_prog_or = ai_prog_oh/2;

  //-- Draw Progress Holder Into BG
  dword hl1 = ag_calchighlight(acfg()->controlbg,acfg()->controlbg_g);
  
  if (!atheme_draw("img.progress.bg",bg,px,ai_prog_oy,pw,ai_prog_oh)){
  	miui_debug("wangxf14 failure atheme_draw img.progress.bg!\n");
    ag_roundgrad(bg,px,ai_prog_oy,pw,ai_prog_oh,acfg()->border,acfg()->border_g,ai_prog_or);
    ag_roundgrad(bg,px+1,ai_prog_oy+1,pw-2,ai_prog_oh-2,
      ag_calculatealpha(acfg()->controlbg,0xffff,180),
      ag_calculatealpha(acfg()->controlbg_g,0xffff,160), ai_prog_or-1);
    ag_roundgrad(bg,px+2,ai_prog_oy+2,pw-4,ai_prog_oh-4,acfg()->controlbg,acfg()->controlbg_g,ai_prog_or-2);
    ag_roundgrad_ex(bg,px+2,ai_prog_oy+2,pw-4,ceil((ai_prog_oh-4)/2.0),LOWORD(hl1),HIWORD(hl1),ai_prog_or-2,2,2,0,0);
  }
  
  //-- Calculate Progress Value Locations
  int hlfdp  = ceil(((float) agdp())/2);
  ai_prog_x = px+(hlfdp+1);
  ai_prog_y = ai_prog_oy+(hlfdp+1);
  ai_prog_h = ai_prog_oh-((hlfdp*2)+2);
  ai_prog_w = pw-((hlfdp*2)+2);
  ai_prog_r = ai_prog_or-(1+hlfdp);

  miui_debug("ai_prog_x = %d, ai_prog_y = %d, ai_prog_h = %d, ai_prog_w = %d, ai_prog_r = %d", ai_prog_x, ai_prog_y, ai_prog_h, ai_prog_w, ai_prog_r);
  
//  snprintf(ai_progress_text,63,"Initializing...");
//  snprintf(ai_progress_info,100,"");
  pthread_mutex_unlock(&ai_progress_mutex);
  return ;
}
/* End, lenovo-sw wangxf14 20130705 add, add for lenovo init install */

/* Begin, lenovo-sw wangxf14 20130709 add, add for lenovo init indeterminate progress */
void miui_lenovo_init_progress(
  CANVAS * bg,
  int px, int py, int pw, int ph
){
  //-- Calculate Progress Location&Size
  pthread_mutex_lock(&ai_progress_mutex);

  ai_indeterminate_prog_oh = agdp()*10;
  ai_indeterminate_prog_oy = 0;
  ai_indeterminate_prog_ox = px;
  ai_indeterminate_prog_ow = pw;

  miui_debug("indeterminate progress default ai_indeterminate_prog_ow = %d, ai_indeterminate_prog_oh = %d\n");

  PNGCANVAS ap;

  if ( apng_load(&ap,"themes/miui4/progressbar_indeterminate_holo1") )
  {
    miui_debug("indeterminate progress get png success ap.w = %d, ap.h = %d\n", ap.w, ap.h);
    ai_indeterminate_prog_ow = ap.w;
    ai_indeterminate_prog_oh = ap.h;

    ai_indeterminate_prog_ox = (pw - ai_indeterminate_prog_ow)/2 + ai_indeterminate_prog_ox;
    
  }
  else
  {
    miui_debug("indeterminate progress get png failured\n");
  }

  ai_indeterminate_prog_oy = agh() - agdp() * 50 - agdp() * 4;

  pthread_mutex_unlock(&ai_progress_mutex);
}
/* End, lenovo-sw wangxf14 20130709 add, add for lenovo init indeterminate progress */

void miuiInstall_reset_progress();
int miui_start_install(
  CANVAS * bg,
  int cx, int cy, int cw, int ch,
  int px, int py, int pw, int ph,
  CANVAS * cvf, int imgY, int chkFY, int chkFH,
  int echo
){
  int ai_return_status = 0;
  //-- Save Canvases
  ai_bg = bg;

  miui_debug("cx = %d, cy = %d, cw = %d, ch = %d, px = %d, py = %d, pw = %d, ph = %d, imgY = %d, chkFY = %d, chkFH = %d\n",cx, cy, cw,ch, px, py, pw, ph, imgY, chkFY, chkFH);

  unlink(MIUI_INSTALL_LOG);
  miuiInstall_reset_progress();
  ai_canvas_lock();
  miui_init_install(bg,cx,cy,cw,ch,px,py,pw,ph); 
  AWINDOWP hWin     = aw(bg);
  ai_win            = hWin;
  ai_cv             = &hWin->c;
  ai_progress_pos   = 0.0;
  ai_progress_w     = 0;
  ai_run            = 1;
  ai_buftxt         = actext(hWin,cx,cy+(agdp()*5),cw,ch-(agdp()*15),NULL,0);
  aw_set_on_dialog(1);
  ai_canvas_unlock();
  
  aw_show(hWin);
  
  pthread_t threadProgress, threadInstaller;
  pthread_create(&threadProgress, NULL, ac_progressthread, NULL);
  pthread_create(&threadInstaller, NULL, miui_install_package, NULL);
  byte ondispatch = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 16:{
        //install failed
        miuiInstall_set_text("Install failed!\n");
        ondispatch = 0;
        ai_return_status = -1;

      }
         break;
      case 15:{
        //install ok
        miuiInstall_set_text("Install successs!\n");
        ai_return_status = 0;
        ondispatch = 0;
      }
      break;
    }
  }
  ai_run = 0;
  hWin->isActived = 0;
  pthread_join(threadProgress,NULL);
  pthread_join(threadInstaller,NULL);
  pthread_detach(threadProgress);
  pthread_detach(threadInstaller);
  if (ai_return_status == -1 || 1 == echo)
  {
      int pad = agdp() * 4;
      
      ai_canvas_lock();
      miui_drawnav(bg, 0, py-pad, agw(), ph+(pad * 2));
      
      ag_draw_ex(bg, cvf, 0, imgY, 0, 0, cvf->w, cvf->h);
      ag_draw(&hWin->c, bg, 0, 0);
      ai_canvas_unlock();

      //Update Textbox
      ai_rebuildtxt(cx, chkFY, cw, chkFH);

      //Show Next Button
      ACONTROLP nxtbtn=acbutton(
        hWin,
        pad+(agdp()*2)+(cw/2),py,(cw/2)-(agdp()*2),ph,acfg()->text_next,0,
        6
      );
      
      // Show Dump Button
      acbutton(
        hWin,
        pad,py,(cw/2)-(agdp()*2),ph,"Save Logs",0,
        8
      );
      
      aw_show(hWin);
      aw_setfocus(hWin,nxtbtn);
      ondispatch = 1;
      while(ondispatch){
          dword msg = aw_dispatch(hWin);
          switch(aw_gm(msg))
          {
          case 8:
              ai_dump_logs();
              break;
          case 6:
              ondispatch = 0;
              break;
        }
      }
  }
  aw_set_on_dialog(0);
  aw_destroy(hWin);
  
  return WEXITSTATUS(ai_return_status);
}

/* Begin, lenovo-sw wangxf14 20130705 add, add for lenovo start install */
int miui_lenovo_start_install(
  CANVAS * bg,
  int x, int y, int w, int h,
  int echo
)
{
  int ai_return_status = 0;
  //-- Save Canvases
  ai_bg = bg;

  unlink(MIUI_INSTALL_LOG);
  miuiInstall_reset_progress();
  ai_canvas_lock();
  miui_lenovo_init_install(bg,x,y,w,h);
  AWINDOWP hWin     = aw(bg);
  ai_win            = hWin;
  ai_cv             = &hWin->c;
  ai_progress_pos   = 0.0;
  ai_progress_w     = 0;
  ai_run            = 1;
  //"<~wipe.name>"
//  ai_buftxt         = actext(hWin,cx,cy+(agdp()*5),cw,ch-(agdp()*15),NULL,0);

//  aw_set_on_dialog(1);
  ai_update = ac_update_text(hWin, x, y, w, h,"<~install.doing>", 0);
  ai_canvas_unlock();
  
  aw_show(hWin);

//  usleep(10*1000*1000);
  
  pthread_t threadProgress, threadInstaller;
  pthread_create(&threadProgress, NULL, ac_lenovo_progressthread, NULL);  
  pthread_create(&threadInstaller, NULL, miui_install_package, NULL);
  byte ondispatch = 1;
  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 16:{
        //install failed
        set_update_failure();
        ondispatch = 0;
        ai_return_status = 1;

      }
         break;
      case 15:{
        //install ok
        set_update_success();
        ai_return_status = 0;
        ondispatch = 0;
      }
      break;
    }
  }
  ai_run = 0;
  hWin->isActived = 0;
  pthread_join(threadProgress,NULL);
  pthread_join(threadInstaller,NULL);
  pthread_detach(threadProgress);
  pthread_detach(threadInstaller);

  if( (0 == echo) && (0 == ai_return_status) )
  {
      reset_update_status();
      aw_destroy(hWin);
      lenovo_set_update_result(ai_return_status);//lenovo-sw wangxf14, write update result
      //miuiIntent_send(INTENT_REBOOT, 1, "system1"); //lenovo_sw wangxf14, reset after auto update complete
#ifdef EASYIMAGE_SUPPORT    
	  clean_easyimage_file_if_need(pmiui_install->path);
#endif
      miuiIntent_send(INTENT_REBOOT, 1, "reboot");
  }

  if( 0 == ai_return_status )
  {
      ai_canvas_lock();
      if (!atheme_draw("img.background", bg, 0, agh() - agdp() * 50 - agdp() * 4, agw(),  agdp() * 50 + agdp() * 4))
      {
          miui_debug("wangxf14 failure reset img.progress.bg!\n");
      }
      ag_draw(&hWin->c, bg, 0, 0);
      ai_canvas_unlock();
	  
      ac_update_text_rebuild(ai_update, x, y, w, h, "<~install.success>", 0);
	  
	int btnW = 120 * agdp();
	int btnX = (w - btnW)/2 + x;
	int btnY1 = get_ac_update_text_imgY(ai_update)+ get_ac_update_text_imgH(ai_update) + 20*agdp();
	int btnH = 26 * agdp();

	int btnY2 = btnY1 + btnH + 30* agdp();

	miui_debug("update install finish btnY1 = %d, btnY2 = %d\n", btnY1, btnY2);

	ACONTROLP reset_btn = acbutton(hWin, btnX, btnY1, btnW, btnH,"<~reset.system>",0,6);
	acbutton(hWin, btnX, btnY2, btnW, btnH,"<~back.name>",0,8);

	aw_show(hWin);

	aw_setfocus(hWin, reset_btn);

      ondispatch = 1;
      while(ondispatch){
          dword msg = aw_dispatch(hWin);
          switch(aw_gm(msg))
          {
          case 8:		  	
              ondispatch = 0;
              break;
          case 6:
              ondispatch = 0;
              lenovo_set_update_result(ai_return_status);//lenovo-sw wangxf14, write update result
	       //miuiIntent_send(INTENT_REBOOT, 1, "system1");
#ifdef EASYIMAGE_SUPPORT    
			clean_easyimage_file_if_need(pmiui_install->path);
#endif
	       miuiIntent_send(INTENT_REBOOT, 1, "reboot");
              break;
        }
      }
  }
  else if ( 1 == ai_return_status )
  {
	ai_canvas_lock();
	if (!atheme_draw("img.background", bg, 0, agh() - agdp() * 50 - agdp() * 4, agw(),  agdp() * 50 + agdp() * 4))
	{
	  miui_debug("wangxf14 failure reset img.progress.bg!\n");
	}
	ag_draw(&hWin->c, bg, 0, 0);
	ai_canvas_unlock();

	ac_update_text_rebuild(ai_update, x, y, w, h, "<~install.failure>", 0);

	int btnW = 120 * agdp();
	int btnX = (w - btnW)/2 + x;
	int btnY1 = get_ac_update_text_imgY(ai_update)+ get_ac_update_text_imgH(ai_update) + 20*agdp();
	int btnH = 26 * agdp();

	ACONTROLP return_btn = acbutton(hWin, btnX, btnY1, btnW, btnH,"<~back.name>",0,8);

	aw_show(hWin);

	aw_setfocus(hWin, return_btn);

	ondispatch = 1;
	while(ondispatch)
	{
	  dword msg = aw_dispatch(hWin);
	  switch(aw_gm(msg))
	  {
	  case 8:		  	
	      ondispatch = 0;
	      break;
	  }
       }
  }

  reset_update_status();
  lenovo_set_update_result(ai_return_status);//lenovo-sw wangxf14, write update result
  
  aw_destroy(hWin);

  miui_debug("ai_return_status = %d\n", ai_return_status);
  
  return ai_return_status;
}
/* End, lenovo-sw wangxf14 20130705 add, add for lenovo start install */

STATUS miuiInstall_init(miuiInstall_fun fun, const char* path, int wipe_cache, const char* install_file)
{
    miui_debug("path = %s, wipe_cache = %d, install_file = %s \n", path, wipe_cache, install_file);

    pmiui_install->path = path;
    pmiui_install->wipe_cache = wipe_cache;
    pmiui_install->install_file = install_file;
    pmiui_install->pfun = fun;
    return RET_OK;
}
void miuiInstall_show_progress(float portion, int seconds)
{
    miui_debug("portion = %f, seconds = %d\n", portion, seconds);
    pthread_mutex_lock(&ai_progress_mutex); 
    float progsize = portion;
    ai_progress_fract_n = abs(seconds);
    ai_progress_fract_c = 0;
    ai_progress_fract_l = alib_tick();
    if (ai_progress_fract_n>0)
      ai_progress_fract = progsize/ai_progress_fract_n;
    else if(ai_progress_fract_n<0)
      ai_progress_fract = progsize/abs(ai_progress_fract_n);
    else{
/* Begin, lenovo-sw wangxf14 add 2013-08-13 for LenovoOTA full and diff package progress */
    if ( 1 == get_full_otapackage_flag() )
    {
        if( ai_progress_pos >= 0.5 )
    	 {
              ai_progress_fract = 0;
              ai_progress_pos   += progsize;
        }
	 else 
	 {
	       ai_progress_fract = 0;
              ai_progress_pos   = progsize;
	  }
    }
    else
    {
         ai_progress_fract = 0;
	  ai_progress_pos   += progsize;
    }
/* End, lenovo-sw wangxf14 add 2013-08-13 for LenovoOTA full and diff package progress */
    }
    pthread_mutex_unlock(&ai_progress_mutex); 
    miui_debug("ai_progress_fract = %f, ai_progress_pos = %f\n", ai_progress_fract, ai_progress_pos);
	
    return ;
}

void miuiInstall_set_progress(float fraction)
{
    miui_debug("fraction = %f\n", fraction);
    pthread_mutex_lock(&ai_progress_mutex);
    ai_progress_fract   = 0;
    ai_progress_fract_n = 0;
    ai_progress_fract_c = 0;
    ai_progress_pos     = fraction ;
    pthread_mutex_unlock(&ai_progress_mutex);
    return ;
}
void miuiInstall_reset_progress()
{
    pthread_mutex_lock(&ai_progress_mutex); 
    ai_progani_pos      = 0;
    ai_progress_pos     = 0;
	
    ai_progress_fract   = 0;
    ai_progress_fract_n = 0;
    ai_progress_fract_c = 0;
    ai_progress_fract_l = 0;
	
    ai_progress_w     = 0;
    ai_prog_x         = 0;
    ai_prog_y         = 0;
    ai_prog_w         = 0;
    ai_prog_h         = 0;	
    ai_prog_r         = 0;
	
    ai_prog_ox        = 0;
    ai_prog_oy        = 0;
    ai_prog_ow        = 0;
    ai_prog_oh        = 0;
    ai_prog_or        = 0;
    pthread_mutex_unlock(&ai_progress_mutex);
    return ;
}

void miuiInstall_set_text(char *str)
{
    ai_canvas_lock();
    actext_appendtxt(ai_buftxt, str);
    ai_canvas_unlock();
    FILE * install_log = fopen(MIUI_INSTALL_LOG, "ab+");
    if (install_log)
    {
        fputs(str, install_log);
        fputc('\n', install_log);
        fclose(install_log);
    }
    return ;
}

char * ai_fixlen(char * str,char * addstr){
  int maxw=ai_prog_w-(ai_prog_or*2)-ag_txtwidth(addstr,0);
  int clen=ag_txtwidth(str,0);
  if (clen<maxw) return NULL;
  int basepos = 0;
  int i=0;
  char basestr[64];
  char allstr[128];
  memset(basestr,0,64);
  for (i=strlen(str)-1;i>=0;i--){
    if (str[i]=='/'){
      basepos = i-2;
      snprintf(basestr,63,"%s",&(str[i]));
      if (i>0)
        snprintf(allstr,127,"/%c%c..%s",str[1],str[2],basestr);
      else
        snprintf(allstr,127,"%s",basestr);
      break;
    }
  }
  if (basepos>50) basepos=50;
  do{
    if (basepos<=0) break;
    char dirstr[64];
    memset(dirstr,0,64);
    memcpy(dirstr,str,basepos);
    snprintf(allstr,127,"%s..%s",dirstr,basestr);
    clen=ag_txtwidth(allstr,0);
    basepos--;
  }while(clen>=maxw);
  return strdup(allstr);
}
//echo text with progress item
void miuiInstall_set_info(char* file_name)
{

    char *filename = file_name;
    snprintf(ai_progress_info,100,"%s",filename);
    miui_debug("ai_progress_info = %s \n", ai_progress_info);
    return ;
}

/* Begin, lenovo-sw wangxf14 20130709 add, add for lenovo indeterminate progressthread */ //wangxf14_pause
static int gIndeterminateFrame = 0;

static void *ac_lenovo_indeterminate_progressthread(void *cookie)
{
  while(ai_progress_run){
    //-- Percent Text
    ai_canvas_lock();
    
    char filename[40];
    gIndeterminateFrame =  (gIndeterminateFrame + 1) % ai_indeterminate_frames;
    sprintf(filename, "img.pro.indeter.holo%02d", gIndeterminateFrame);

    if (!atheme_draw(filename, ai_progress_cv, ai_indeterminate_prog_ox, ai_indeterminate_prog_oy, ai_indeterminate_prog_ow, ai_indeterminate_prog_oh)){
      miui_debug("wangxf14 failure atheme draw %s\n", filename);
    }
    
    aw_draw(ai_progress_win);
    ai_canvas_unlock();
    usleep(160);
  }
  return NULL;
}

/* part Begin, lenovo-sw wangxf14 20140226 add, porting mtk6582 recovery optimize for data clean */
static void *miui_clean_process(void *cookie)
{
    int ret = -1;
    WIPE_CONTROLDP contrlp = (WIPE_CONTROLDP *) cookie;

    if( (NULL != contrlp->ctl1) && (strstr(contrlp->ctl1, "fuse_wipe_data") != NULL) )
    {
           miui_debug("do clean fuse_wipe_data ...\n");
           ret = lenovo_fuse_wipe_clean(contrlp->ctl1);
    }
    else
    {
        if( NULL != contrlp->ctl1 )
	    {
	       miui_debug("do clean ctl1 ...\n");
		   ret = lenovo_wipe_clean(contrlp->ctl1);
	    }

	    if( NULL != contrlp->ctl2 )
	    {
	       miui_debug("do clean ctl2 ...\n");    
		   ret = lenovo_wipe_clean(contrlp->ctl2);
	    }
    }

    miui_debug("the result of miui_clean_process ret = %d \n", ret);

    if( 0 == ret )
        aw_post(aw_msg(15, 0, 0, 0));
    else
        aw_post(aw_msg(16, 0, 0, 0));
    return NULL;
}
/* part End, lenovo-sw wangxf14 20140226 add, porting mtk6582 recovery optimize for data clean */
/* End, lenovo-sw wangxf14 20130709 add, add for lenovo indeterminate progressthread */

/* Begin, lenovo-sw wangxf14 20130709 add, add for lenovo start indeterminate progress */
int miui_lenovo_start_indeterminate_progress(
  CANVAS * bg,
  int x, int y, int w, int h,
  int echo, const char *ctl1, const char *ctl2
)
{
  int ai_return_status = 0;
  WIPE_CONTROLD contrl;
  //-- Save Canvases
  ai_progress_bg = bg;
  
  ai_canvas_lock();
  contrl.ctl1 = ctl1;
  contrl.ctl2 = ctl2;
  miui_lenovo_init_progress(bg,x,y,w,h);
  AWINDOWP hWin     = aw(bg);
  ai_progress_win            = hWin;
  ai_progress_cv             = &hWin->c;
  ai_progress_run            = 1;
  set_indeterminate_status();
  ai_indeterminate_progress = ac_update_text(hWin, x, y, w, h,"<~clean.doing>", 0);
  ai_canvas_unlock();
  
  aw_show(hWin);

//  usleep(10*1000*1000);
  
  pthread_t threadIndeterminateProgress, threadCleaner;
  pthread_create(&threadIndeterminateProgress, NULL, ac_lenovo_indeterminate_progressthread, NULL);
  pthread_create(&threadCleaner, NULL, miui_clean_process, &contrl);
  byte ondispatch = 1;

  while(ondispatch){
    dword msg=aw_dispatch(hWin);
    switch (aw_gm(msg)){
      case 16:{
        //wipe failed
        set_update_failure();
        ondispatch = 0;
        ai_return_status = 1;
      }
      break;		
      case 15:{
        //wipe ok
        set_update_success();
        ai_return_status = 0;
        ondispatch = 0;
      }
      break;
    }
  }
  ai_progress_run = 0;
  hWin->isActived = 0;
  pthread_join(threadIndeterminateProgress,NULL);
  pthread_join(threadCleaner,NULL);
  pthread_detach(threadIndeterminateProgress);
  pthread_detach(threadCleaner);

  if( (0 == echo) && (0 == ai_return_status) )
  {
	miui_debug("echo = 0!system auto doing...\n");
       reset_update_status();
       aw_destroy(hWin);
#ifdef EASYIMAGE_SUPPORT    
	   clean_easyimage_file_if_need(pmiui_install->path);
#endif
       miuiIntent_send(INTENT_REBOOT, 1, "reboot"); //lenovo_sw wangxf14, reset after auto clean complete	
  }

#ifdef LENOVO_FACTORY_WIPE_DATA_SHUTDOWN
  if( (9 == echo) && (0 == ai_return_status) )
  {
       miui_debug("echo = 9!system auto doing...\n");
       reset_update_status();
       aw_destroy(hWin);
       miuiIntent_send(INTENT_REBOOT, 1, "poweroff");
  }
#endif

  if( 0 == ai_return_status )
  {
      ai_canvas_lock();
      if (!atheme_draw("img.background", bg, 0, agh() - agdp() * 50 - agdp() * 4, agw(),  agdp() * 50 + agdp() * 4))
      {
          miui_debug("wangxf14 failure reset img.progress.bg!\n");
      }
      ag_draw(&hWin->c, bg, 0, 0);
      ai_canvas_unlock();
	  
      ac_update_text_rebuild(ai_indeterminate_progress, x, y, w, h, "<~clean.success>", 0);
	  
	int btnW = 120 * agdp();
	int btnX = (w - btnW)/2 + x;
	int btnY1 = get_ac_update_text_imgY(ai_indeterminate_progress)+ get_ac_update_text_imgH(ai_indeterminate_progress) + 20*agdp();
	int btnH = 26 * agdp();

	miui_debug("update install finish btnY1 = %d\n", btnY1);

	ACONTROLP return_btn = acbutton(hWin, btnX, btnY1, btnW, btnH,"<~back.name>",0,8);
	aw_show(hWin);

	aw_setfocus(hWin, return_btn);

      ondispatch = 1;
      while(ondispatch){
          dword msg = aw_dispatch(hWin);
          switch(aw_gm(msg))
          {
          case 8:		  	
              ondispatch = 0;
              break;
        }
      }
  }
  else if ( 1 == ai_return_status )
  {
	ai_canvas_lock();
	if (!atheme_draw("img.background", bg, 0, agh() - agdp() * 50 - agdp() * 4, agw(),  agdp() * 50 + agdp() * 4))
	{
	  miui_debug("wangxf14 failure reset img.progress.bg!\n");
	}
	ag_draw(&hWin->c, bg, 0, 0);
	ai_canvas_unlock();

	ac_update_text_rebuild(ai_indeterminate_progress, x, y, w, h, "<~clean.failure>", 0);

	int btnW = 120 * agdp();
	int btnX = (w - btnW)/2 + x;
	int btnY1 = get_ac_update_text_imgY(ai_indeterminate_progress)+ get_ac_update_text_imgH(ai_indeterminate_progress) + 20*agdp();
	int btnH = 26 * agdp();

	ACONTROLP return_btn = acbutton(hWin, btnX, btnY1, btnW, btnH,"<~back.name>",0,8);

	aw_show(hWin);

	aw_setfocus(hWin, return_btn);

	ondispatch = 1;
	while(ondispatch)
	{
	  dword msg = aw_dispatch(hWin);
	  switch(aw_gm(msg))
	  {
	  case 8:		  	
	      ondispatch = 0;
	      break;
	  }
       }
  }

  reset_update_status();
  aw_destroy(hWin);

  miui_debug("ai_return_status = %d\n", ai_return_status);
  
  return ai_return_status;
}
/* End, lenovo-sw wangxf14 20130709 add, add for lenovo start indeterminate progress */
