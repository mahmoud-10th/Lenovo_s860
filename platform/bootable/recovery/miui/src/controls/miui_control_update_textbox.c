/*
 * Copyright (C) 2014 lenovo MIUI
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
 * Lenovo Recovery UI: Update Base Window Control
 *
 */
//#define DEBUG
#include "../miui_inter.h"

/***************************[ UPDATE TEXTBOX ]**************************/
//static const char *image_normal = "@install.normal";
//static const char *image_success = "@install.success";
//static const char *image_failure = "@install.failure";

static int install_status = 0;
static int indeterminate_status = 0;

typedef struct{
  CANVAS    client;
  CANVAS    client_success;
  CANVAS    client_failure;

  byte      focused;

  int imgX;
  int imgY;
  int imgW;
  int imgH;

} AC_UPDATE_TEXTD, * AC_UPDATE_TEXTDP;

void reset_update_status(void)
{
    install_status = 0;
    indeterminate_status = 0;
}

void set_update_success(void)
{
    install_status = 1;
}

void set_update_failure(void)
{
    install_status = -1;
}

int get_update_status(void)
{
	return install_status;
}

void set_indeterminate_status(void)
{
    indeterminate_status = 1;
}

void ac_update_text_ondraw(void * x){
  ACONTROLP ctl= (ACONTROLP) x;
  AC_UPDATE_TEXTDP  d  = (AC_UPDATE_TEXTDP) ctl->d;
  CANVAS *  pc = &ctl->win->c;
  
  if (d->focused){
    miui_debug("wangxf14 look ac_update_text_ondraw foucused!\n");
    if ( 0 == get_update_status() )	
        ag_draw_ex(pc, &d->client, d->imgX, d->imgY, 0, 0, d->imgW, d->imgH);
    else if( 1 == get_update_status() )
        ag_draw_ex(pc, &d->client_success, d->imgX, d->imgY, 0, 0, d->imgW, d->imgH);
    else if( -1 == get_update_status() )
        ag_draw_ex(pc, &d->client_failure, d->imgX, d->imgY, 0, 0, d->imgW, d->imgH);
  }
  else
  {
    miui_debug("wangxf14 look ac_update_text_ondraw not foucused!\n");
    if( 1 == get_update_status() )
        ag_draw_ex(pc, &d->client_success, d->imgX, d->imgY, 0, 0, d->imgW, d->imgH);
    else if( -1 == get_update_status() )
        ag_draw_ex(pc, &d->client_failure, d->imgX, d->imgY, 0, 0, d->imgW, d->imgH);
  }

}
void ac_update_text_ondestroy(void * x){
  ACONTROLP ctl= (ACONTROLP) x;
  AC_UPDATE_TEXTDP  d  = (AC_UPDATE_TEXTDP) ctl->d;
  ag_ccanvas(&d->client_failure);
  ag_ccanvas(&d->client_success);
  ag_ccanvas(&d->client);
  free(ctl->d);
}
byte ac_update_text_onfocus(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  AC_UPDATE_TEXTDP   d  = (AC_UPDATE_TEXTDP) ctl->d;
  d->focused=1;
  ctl->ondraw(ctl);
  return 1;
}
void ac_update_text_onblur(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  AC_UPDATE_TEXTDP   d  = (AC_UPDATE_TEXTDP) ctl->d;
  d->focused=0;
  ctl->ondraw(ctl);
}

int get_ac_update_text_imgX(ACONTROLP ctl)
{
    AC_UPDATE_TEXTDP  d  = (AC_UPDATE_TEXTDP) ctl->d;
    return d->imgX;
}

int get_ac_update_text_imgY(ACONTROLP ctl)
{
    AC_UPDATE_TEXTDP  d  = (AC_UPDATE_TEXTDP) ctl->d;
    return d->imgY;
}

int get_ac_update_text_imgW(ACONTROLP ctl)
{
    AC_UPDATE_TEXTDP  d  = (AC_UPDATE_TEXTDP) ctl->d;
    return d->imgW;
}

int get_ac_update_text_imgH(ACONTROLP ctl)
{
    AC_UPDATE_TEXTDP  d  = (AC_UPDATE_TEXTDP) ctl->d;
    return d->imgH;
}

CANVAS get_ac_update_text_Client(ACONTROLP ctl)
{
    AC_UPDATE_TEXTDP  d  = (AC_UPDATE_TEXTDP) ctl->d;
    return d->client;
}

void ac_update_text_rebuild(
  ACONTROLP ctl,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig
){
  AC_UPDATE_TEXTDP d = (AC_UPDATE_TEXTDP)ctl->d;
  AWINDOWP win = ctl->win;

  ag_ccanvas(&d->client);
  ag_ccanvas(&d->client_success);
  ag_ccanvas(&d->client_failure);
  memset(d, 0, sizeof(AC_UPDATE_TEXTDP));  

  //-- Validate Minimum Size
  if (h<agdp()*16) h=agdp()*16;
  if (w<agdp()*16) w=agdp()*16;
    
  //-- Initializing Client Area
  int minpadding = max(acfg()->roundsz,4);
  int cx = x;
  int cy = y + agdp() *20;
  int ch = 0;
  if (text!=NULL)
  {
    cx = (w - ag_txtwidth(text, isbig))/2 + cx;
    ch = ag_txtheight(w,text,isbig);
  }

  ag_text(&win->c,w,cx,cy,text,acfg()->textfg,isbig);

  cy = cy + ch + agdp() *20;

  //-- Calculate Progress Location&Size
//  pthread_mutex_lock(&ai_progress_mutex);

  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  
  int  imgX       = x;
  int  imgY       = cy;
//  char * icon_normal = LENOVO_INSTALL_NORMAL_ICON;

 // if (apng_load(&ap,image_normal)){
  if ( apng_load(&ap,"themes/miui4/icon.install.normal") ) {
    imgE  = 1;
    miui_debug("ap.w = %d, ap.h = %d\n", ap.w, ap.h);
    imgW  = max(ap.w,agdp()*50);
    imgH  = max(ap.h,agdp()*50);
    imgA  = imgW;
  }

  
  if (imgE){
    imgX = (w - imgW)/2 + imgX;
    miui_debug("imgX = %d, imgY = %d, imgW = %d, imgH = %d, imgA = %d\n", imgX, imgY, imgW, imgH, imgA);	
    //apng_draw_ex(&win->c, &ap, imgX, imgY, 0, 0, imgW, imgH);
    apng_close(&ap);
  }
  else 
  {
      miui_error("load icon.install.normal icon error!\n");
  }
  
  //-- Initializing Text Data
//  AC_UPDATE_TEXTDP d        = (AC_UPDATE_TEXTDP) malloc(sizeof(AC_UPDATE_TEXTD));
//  memset(d,0,sizeof(AC_UPDATE_TEXTD));
  
  //-- Initializing Canvas
  ag_canvas(&d->client,imgW,imgH);
  ag_canvas(&d->client_success,imgW,imgH);
  ag_canvas(&d->client_failure,imgW,imgH);

  //-- failure
  if (!atheme_draw("img.icon.install.failure", &d->client_failure, 0, 0, imgW, imgH)) {
    miui_debug("wangxf14 failure to set update_textbox failure bg!\n");
  }  

  //-- success
  if (!atheme_draw("img.icon.install.success", &d->client_success, 0, 0, imgW, imgH)) {
    miui_debug("wangxf14 failure to set update_textbox success bg!\n");
  }  

  //-- Background
  if (!atheme_draw("img.icon.install.normal", &d->client, 0, 0, imgW, imgH)) {
    miui_debug("wangxf14 failure to set update_textbox bg!\n");
  }

  if ( 1 != indeterminate_status)  	
  {
	  //-- Percent Text
	  int prog_percent = 0;
	  char prog_percent_str[10];
	  snprintf(prog_percent_str,9,"%2d%c",prog_percent,'%');

	  int prog_percent_h = ag_txtheight(imgW, prog_percent_str, 1);
	  int prog_percent_w = ag_txtwidth(prog_percent_str, 1);

	  int prog_percent_x = (imgW - prog_percent_w)/2;
	  int prog_percent_y = (imgH - prog_percent_h)/2;

	  miui_debug("prog_percent_x = %d, prog_percent_y = %d\n", prog_percent_x, prog_percent_y);

	  ag_textf(&d->client, imgW, prog_percent_x, prog_percent_y, prog_percent_str, acfg()->textfg, 1);
  }
  
  d->focused     = 0;
  d->imgX = imgX;
  d->imgY = imgY;
  d->imgW = imgW;
  d->imgH = imgH;

//  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
//  ctl->ondestroy= &ac_update_text_ondestroy;
//  ctl->ondraw   = &ac_update_text_ondraw;
//  ctl->onblur   = &ac_update_text_onblur;
//  ctl->onfocus  = &ac_update_text_onfocus;
//  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
//  ctl->d        = (void *) d;
//  aw_add(win,ctl);

//  return ctl;
  ctl->ondraw(ctl);
  aw_draw(ctl->win);
}

ACONTROLP ac_update_text(
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  char * text,
  byte isbig
){
  //-- Validate Minimum Size
  if (h<agdp()*16) h=agdp()*16;
  if (w<agdp()*16) w=agdp()*16;
    
  //-- Initializing Client Area
  int minpadding = max(acfg()->roundsz,4);
//  int cw            = w-(agdp()*(minpadding*2));
  int cx = x;
  int cy = y + agdp() *20;
  int ch = 0;
  if (text!=NULL)
  {
    cx = (w - ag_txtwidth(text, isbig))/2 + cx;
    ch = ag_txtheight(w,text,isbig);
  }

  ag_text(&win->c,w,cx,cy,text,acfg()->textfg,isbig);

  cy = cy + ch + agdp() *20;

  //-- Calculate Progress Location&Size
//  pthread_mutex_lock(&ai_progress_mutex);

  //-- Load Icon
  PNGCANVAS ap;
  byte imgE       = 0;
  int  imgA       = 0;
  int  imgW       = 0;
  int  imgH       = 0;
  
  int  imgX       = x;
  int  imgY       = cy;
//  char * icon_normal = LENOVO_INSTALL_NORMAL_ICON;

//  if (apng_load(&ap,image_normal)){
  if ( apng_load(&ap,"themes/miui4/icon.install.normal") ) {
    imgE  = 1;
    miui_debug("ap.w = %d, ap.h = %d\n", ap.w, ap.h);
    imgW  = max(ap.w,agdp()*50);
    imgH  = max(ap.h,agdp()*50);
    imgA  = imgW;
  }

  
  if (imgE){
    imgX = (w - imgW)/2 + imgX;
    miui_debug("imgX = %d, imgY = %d, imgW = %d, imgH = %d, imgA = %d\n", imgX, imgY, imgW, imgH, imgA);	
    //apng_draw_ex(&win->c, &ap, imgX, imgY, 0, 0, imgW, imgH);
    apng_close(&ap);
  }
  else 
  {
      miui_error("load install.normal icon error!\n");
  }
  
  //-- Initializing Text Data
  AC_UPDATE_TEXTDP d        = (AC_UPDATE_TEXTDP) malloc(sizeof(AC_UPDATE_TEXTD));
  memset(d,0,sizeof(AC_UPDATE_TEXTD));
  
  //-- Initializing Canvas
  ag_canvas(&d->client,imgW,imgH);
  ag_canvas(&d->client_success,imgW,imgH);
  ag_canvas(&d->client_failure,imgW,imgH);

  //-- failure
  if (!atheme_draw("img.icon.install.failure", &d->client_failure, 0, 0, imgW, imgH)) {
    miui_debug("wangxf14 failure to set update_textbox failure bg!\n");
  }  

  //-- success
  if (!atheme_draw("img.icon.install.success", &d->client_success, 0, 0, imgW, imgH)) {
    miui_debug("wangxf14 failure to set update_textbox success bg!\n");
  }  

  //-- Background
  if (!atheme_draw("img.icon.install.normal", &d->client, 0, 0, imgW, imgH)) {
    miui_debug("wangxf14 failure to set update_textbox bg!\n");
  }

  if ( 1 != indeterminate_status)
  {
	  //-- Percent Text
	  int prog_percent = 0;
	  char prog_percent_str[10];
	  snprintf(prog_percent_str,9,"%2d%c",prog_percent,'%');

	  int prog_percent_h = ag_txtheight(imgW, prog_percent_str, 1);
	  int prog_percent_w = ag_txtwidth(prog_percent_str, 1);

	  int prog_percent_x = (imgW - prog_percent_w)/2;
	  int prog_percent_y = (imgH - prog_percent_h)/2;

	  miui_debug("prog_percent_x = %d, prog_percent_y = %d\n", prog_percent_x, prog_percent_y);

	  ag_textf(&d->client, imgW, prog_percent_x, prog_percent_y, prog_percent_str, acfg()->textfg, 1);
  }
  
  d->focused     = 0;
  d->imgX = imgX;
  d->imgY = imgY;
  d->imgW = imgW;
  d->imgH = imgH;

  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy= &ac_update_text_ondestroy;
  ctl->ondraw   = &ac_update_text_ondraw;
  ctl->onblur   = &ac_update_text_onblur;
  ctl->onfocus  = &ac_update_text_onfocus;
  ctl->oninput = NULL;//fix abnormal interrupt on update doing by touch or input etc
  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 0;
  ctl->d        = (void *) d;
  aw_add(win,ctl);

  return ctl;
}
