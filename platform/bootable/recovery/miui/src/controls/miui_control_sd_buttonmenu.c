/*
 * Copyright (C) 2013 lenovo recovery ( http://lenovo.com/ )
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
 * lenovo-sw wangxf14 add 2013-06-20, add for language button menu control
 *
 */
//#define DEBUG //wangxf14_debug
#include "../miui_inter.h"

/***************************[ MENU BOX ]**************************/
typedef struct{
  char title[64];
  char desc[128];
  PNGCANVAS * img;
  int  id;
  int  h;
  int  y;
  
  /* Title & Desc Size/Pos */
  int  th;
  int  dh;
  int  ty;
  int  dy;
} ACBUTTONMAINMENUI, * ACBUTTONMAINMENUIP;
typedef struct{
  byte      acheck_signature;
  CANVAS    client;
  AKINETIC  akin;
  int       scrollY;
  int       maxScrollY;
  int       prevTouchY;
  int       invalidDrawItem;
  
  /* Client Size */
  int clientWidth;
  int clientHigh;
  int clientTextW;
  int clientTextX;
  int nextY;
  
  /* Items */
  ACBUTTONMAINMENUIP * items;
  int       itemn;
  int       touchedItem;
  int       focusedItem;
  int       draweditemn;
  int       selectedIndex;
  byte      touchmsg;
  /* Focus */
  byte      focused;
} ACBUTTONMAINMENUD, * ACBUTTONMAINMENUDP;
static void control_mainmenu_button_item_dump(ACBUTTONMAINMENUD * d)
{
    miui_debug("**************ACBUTTONMAINMENUD***************\n");
    miui_debug("itemn:%d,touchedItem:%d,focusedItem:%d,\n", d->itemn, d->touchedItem, d->focusedItem);
    miui_debug("drawedItemn:%d,selectIndex:%d,touchmsg:%x\n,", d->draweditemn, d->selectedIndex, d->touchmsg);
    miui_debug("******************END*******************\n");
}
void acbutton_sdmenu_ondestroy(void * x){
  ACONTROLP ctl= (ACONTROLP) x;
  ACBUTTONMAINMENUDP d  = (ACBUTTONMAINMENUDP) ctl->d;
  if (d->itemn>0){
    int i;
    for (i=0;i<d->itemn;i++){
      if (d->items[i]->img!=NULL){
        apng_close(d->items[i]->img);
        free(d->items[i]->img);
        d->items[i]->img=NULL;
      }
      free(d->items[i]);
    }
    free(d->items);
    ag_ccanvas(&d->client);
  }
  free(ctl->d);
}
void acbutton_sdmenu_redrawitem(ACONTROLP ctl, int index){
  ACBUTTONMAINMENUDP d = (ACBUTTONMAINMENUDP) ctl->d;
  if (d->acheck_signature != 144) return;   //-- Not Valid Signature
  if ((index>=d->itemn)||(index<0)) return; //-- Not Valid Index
  
  ACBUTTONMAINMENUIP p = d->items[index];
  CANVAS *  c = &d->client;
  
  color txtcolor = acfg()->textfg;
  color graycolor= acfg()->textfg_gray;
  byte isselectcolor=0;
  
  if (index==d->touchedItem){
    if (!atheme_draw("img.button.push", c,0,p->y,d->clientWidth,p->h)){
      color pshad = ag_calpushad(acfg()->selectbg_g);
      dword hl1 = ag_calcpushlight(acfg()->selectbg,pshad);
      ag_roundgrad(c,0,p->y+agdp(),d->clientWidth,p->h-(agdp()*2),acfg()->selectbg,pshad,(agdp()*acfg()->roundsz));
      ag_roundgrad(c,0,p->y+agdp(),d->clientWidth,(p->h-(agdp()*2))/2,LOWORD(hl1),HIWORD(hl1),(agdp()*acfg()->roundsz));
    }
    graycolor = txtcolor = acfg()->selectfg;
    isselectcolor=1;
  }
  else if ((index==d->focusedItem)&&(d->focused)){
    if (!atheme_draw("img.button.focus", c,0,p->y,d->clientWidth,p->h)){
      dword hl1 = ag_calchighlight(acfg()->selectbg,acfg()->selectbg_g);
      ag_roundgrad(c,0,p->y+agdp(),d->clientWidth,p->h-(agdp()*2),acfg()->selectbg,acfg()->selectbg_g,(agdp()*acfg()->roundsz));
      ag_roundgrad(c,0,p->y+agdp(),d->clientWidth,(p->h-(agdp()*2))/2,LOWORD(hl1),HIWORD(hl1),(agdp()*acfg()->roundsz));
    }
    graycolor = txtcolor = acfg()->selectfg;
    isselectcolor=1;
  }
  else 
  {
    miui_debug("acbutton_sdmenu_redrawitem menubox draw item button!\n");
    if(!atheme_draw("img.button", c,0,p->y,d->clientWidth,p->h))
    {
        miui_debug("acbutton menubox draw img.button failure!\n");
    }
  }

  int txtH    = p->th+p->dh;
  int txtAddY = 0;

  int tmpTitleTextW = ag_txtwidth(p->title, 0);//wangxf14_modify
  int tmpTitleTextX = (d->clientWidth - tmpTitleTextW)/2;
  int tmpTitleTextY = p->y + ( p->h -p->th)/2;

  //-- Now Draw The Text
  if (isselectcolor){
    ag_textf(c,d->clientTextW, tmpTitleTextX,tmpTitleTextY,p->title,acfg()->selectbg_g,0);//wangxf14 modify
  }
  ag_text(c,d->clientTextW, tmpTitleTextX-1,tmpTitleTextY-1,p->title,txtcolor,0);

}
void acbutton_sdmenu_redraw(ACONTROLP ctl){
  ACBUTTONMAINMENUDP d = (ACBUTTONMAINMENUDP) ctl->d;
  if (d->acheck_signature != 144) return; //-- Not Valid Signature
  if ((d->itemn>0)&&(d->draweditemn<d->itemn)) {
    ag_ccanvas(&d->client);
    ag_canvas(&d->client,d->clientWidth,d->clientHigh);//wangxf14_modify
#if 1
  //-- Cleanup Background
  if (!atheme_id_draw(0, &d->client, 0, 0, d->clientWidth,d->clientHigh)){//wangxf14_modify
    miui_debug("set button menu canvas background failure");
  }
#endif

    //-- Set Values
    d->scrollY     = 0;
    d->maxScrollY  = d->clientHigh-ctl->h;//wangxf14 debug
    if (d->maxScrollY<0) d->maxScrollY=0;

    //-- Draw Items
    int i;
    for (i=0;i<d->itemn;i++){
      acbutton_sdmenu_redrawitem(ctl,i);
    }
    d->draweditemn=d->itemn;
  }
  
}
int acbutton_sdmenu_getselectedindex(ACONTROLP ctl){
  ACBUTTONMAINMENUDP d = (ACBUTTONMAINMENUDP) ctl->d;
  if (d->acheck_signature != 144) return -1; //-- Not Valid Signature
  return d->selectedIndex < 0 ?0:d->selectedIndex;
}
//-- Add Item Into Control
byte acbutton_sdmenu_add(ACONTROLP ctl,char * title, char * desc, char * img){
  ACBUTTONMAINMENUDP d = (ACBUTTONMAINMENUDP) ctl->d;
  if (d->acheck_signature != 144) return 0; //-- Not Valid Signature
  
  //-- Allocating Memory For Item Data
  ACBUTTONMAINMENUIP newip = (ACBUTTONMAINMENUIP) malloc(sizeof(ACBUTTONMAINMENUI));
  if( NULL != title )
      snprintf(newip->title,64,"%s",title);
  if( NULL != desc )
      snprintf(newip->desc,128,"%s",desc);
  
  //-- Load Image
  if( NULL != img)
  {
	  newip->img      = (PNGCANVAS *) malloc(sizeof(PNGCANVAS));
	  memset(newip->img,0,sizeof(PNGCANVAS));
	  if (!apng_load(newip->img,img)){
	    free(newip->img);
	    newip->img=NULL;
	  }
  }
  
  newip->th       = ag_txtheight(d->clientTextW,newip->title,0);//wangxf14_modify small
  newip->ty       = agdp()*2;
  newip->h        = (agdp()*4) + newip->th;

  miui_debug("newip->h = %d , (agdp()*26) = %d \n", newip->h, agdp()*26);//wangxf14_modify

  if (newip->h<(agdp()*26)) newip->h = (agdp()*26);
  
  newip->id       = d->itemn;
  newip->y        = d->nextY + 30 * agdp();//wangxf14_modify
  d->nextY       += newip->h + 30 * agdp();//wangxf14_modify
  d->clientHigh   =  d->nextY;

  miui_debug("newip->id = %d, newip->y= %d, newip->h= %d \n", newip->id, newip->y, newip->h);
  
  
  if (d->itemn>0){
    int i;
    ACBUTTONMAINMENUIP * tmpitms   = d->items;
    d->items              = malloc( sizeof(ACBUTTONMAINMENUIP)*(d->itemn+1) );
    for (i=0;i<d->itemn;i++)
      d->items[i]=tmpitms[i];
    d->items[d->itemn] = newip;
    free(tmpitms);
  }
  else{
    d->items    = malloc(sizeof(ACBUTTONMAINMENUIP));
    d->items[0] = newip;
  }
  d->itemn++;
  return 1;
}
void acbutton_sdmenu_ondraw(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACBUTTONMAINMENUDP   d  = (ACBUTTONMAINMENUDP) ctl->d;
  CANVAS *    pc = &ctl->win->c;

  miui_debug("acbutton_sdmenu_ondraw\n");
  
  acbutton_sdmenu_redraw(ctl);
  if (d->invalidDrawItem!=-1){
    d->touchedItem = d->invalidDrawItem;
    acbutton_sdmenu_redrawitem(ctl,d->invalidDrawItem);
    d->invalidDrawItem=-1;
  }
  
  //-- Init Device Pixel Size
  int minpadding = max(acfg()->roundsz,4);
  int agdp3 = (agdp()*minpadding);
  int agdp6 = (agdp()*(minpadding*2));
  int agdpX = agdp6;

  int tmpDx = ctl->x + (agdp()*20);
  int tmpDy = ctl->y + agdp();//wangxf14

  miui_debug("d->scrollY = %d\n", d->scrollY);  
  if (d->focused){
    ag_draw_ex(pc, &d->client, tmpDx, tmpDy,0,d->scrollY,d->clientWidth,d->clientHigh);//wangxf14 need modify
  }
  else{
    ag_draw_ex(pc, &d->client, tmpDx, tmpDy,0,d->scrollY,d->clientWidth,d->clientHigh);
  }

  miui_debug("d->maxScrollY = %d\n", d->maxScrollY);
  
  if (d->maxScrollY>0){
    //-- Glow
    int i;
    byte isST=(d->scrollY>0)?1:0;
    byte isSB=(d->scrollY<d->maxScrollY)?1:0;
    int add_t_y = 1;
    if (d->focused)
      add_t_y = agdp();
    for (i=0;i<agdpX;i++){
      byte alph = 255-round((((float) (i+1))/ ((float) agdpX))*230);
      if (isST)
        ag_rectopa(pc,ctl->x+agdp3,ctl->y+i+add_t_y,ctl->w-agdpX,1,acfg()->textbg,alph);
      if (isSB)
        ag_rectopa(pc,ctl->x+agdp3,((ctl->y+ctl->h)-(add_t_y))-(i+1),ctl->w-agdpX,1,acfg()->textbg,alph);
    }
    
    //-- Scrollbar
    int newh = ctl->h - agdp6;
    float scrdif    = ((float) newh) / ((float) d->client.h);
    int  scrollbarH = round(scrdif * newh);
    int  scrollbarY = round(scrdif * d->scrollY) + agdp3;
    if (d->scrollY<0){
      scrollbarY = agdp3;
      int alp = (1.0 - (((float) abs(d->scrollY)) / (((float) ctl->h)/4))) * 255;
      if (alp<0) alp = 0;
      ag_rectopa(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar, alp);
    }
    else if (d->scrollY>d->maxScrollY){
      scrollbarY = round(scrdif * d->maxScrollY) + agdp3;
      int alp = (1.0 - (((float) abs(d->scrollY-d->maxScrollY)) / (((float) ctl->h)/4))) * 255;
      if (alp<0) alp = 0;
      ag_rectopa(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar, alp);
    }
    else{
      ag_rect(pc,(ctl->w-agdp()-2)+ctl->x,scrollbarY+ctl->y,agdp(),scrollbarH,acfg()->scrollbar);
    }
  }
}
dword acbutton_sdmenu_oninput(void * x,int action,ATEV * atev){
  ACONTROLP ctl= (ACONTROLP) x;
  ACBUTTONMAINMENUDP d  = (ACBUTTONMAINMENUDP) ctl->d;
  dword msg = 0;

  miui_debug("acbutton_sdmenu_oninput action = %d\n", action);
  
  switch (action){
    case ATEV_MOUSEDN:
      {
        d->prevTouchY  = atev->y;
        akinetic_downhandler(&d->akin,atev->y);
        
        int touchpos = atev->y - ctl->y + d->scrollY;
        int i;
        for (i=0;i<d->itemn;i++){
	   miui_debug("atev->y = %d, ctl->y = %d, d->scrollY = %d\n", atev->y, ctl->y, d->scrollY);
	   miui_debug("touchpos = %d, d->items[%d]->y = %d, d->items[%d]->h = %d\n", touchpos, i, d->items[i]->y, i, d->items[i]->h);
          if ((touchpos>=d->items[i]->y)&&(touchpos<d->items[i]->y+d->items[i]->h)
		 &&( atev->x > ctl->w/4 )
		 &&( atev->x < (ctl->w *3)/4)
		  ){ //wangxf14_modify pause
            ac_regpushwait(
              ctl,&d->prevTouchY,&d->invalidDrawItem,i
            );
            break;
          }
        }
      }
      break;
    case ATEV_MOUSEUP:
      {
        byte retmsgx = 0;
        if ((d->prevTouchY!=-50)&&(abs(d->prevTouchY-atev->y)<agdp()*5)){
          d->prevTouchY=-50;
          int touchpos = atev->y - ctl->y + d->scrollY;
          int i;
#ifdef DEBUG
          control_mainmenu_button_item_dump(d);
#endif
          for (i=0;i<d->itemn;i++){
	     miui_debug("atev->y = %d, ctl->y = %d, d->scrollY = %d\n", atev->y, ctl->y, d->scrollY);
	     miui_debug("touchpos = %d, d->items[%d]->y = %d, d->items[%d]->h = %d\n", touchpos, i, d->items[i]->y, i, d->items[i]->h);		  	
            if ((touchpos>=d->items[i]->y)&&(touchpos<d->items[i]->y+d->items[i]->h)
		 &&( atev->x > ctl->w/4 )
		 &&( atev->x < (ctl->w *3)/4)
			){//wangxf14_modify
              if ((d->touchedItem != -1)&&(d->touchedItem!=i)){
                int tmptouch=d->touchedItem;
                d->touchedItem = -1;
                acbutton_sdmenu_redrawitem(ctl,tmptouch);
              }
              if ((d->selectedIndex != -1)&&(d->selectedIndex!=i)){
                int tmpsidx=d->selectedIndex;
                d->selectedIndex = -1;
                acbutton_sdmenu_redrawitem(ctl,tmpsidx);
              }
              
              int prevfocus = d->focusedItem;
              d->focusedItem= i;
              d->touchedItem   = i;
              d->selectedIndex = i;
              if ((prevfocus!=-1)&&(prevfocus!=i)){
                acbutton_sdmenu_redrawitem(ctl,prevfocus);
              }
              
              acbutton_sdmenu_redrawitem(ctl,i);
              ctl->ondraw(ctl);
              aw_draw(ctl->win);
              vibrate(30);
              retmsgx = d->touchmsg;
              msg=aw_msg(retmsgx,1,0,0);
              break;//wangxf14_study
            }
          }
          miui_debug("d->scrollY = %d, d->maxScrollY = %d\n", d->scrollY, d->maxScrollY);
          if ((d->scrollY<0)||(d->scrollY>d->maxScrollY)){
            ac_regbounce(ctl,&d->scrollY,d->maxScrollY);
	     miui_debug("after ac_regbounce do : d->scrollY = %d, d->maxScrollY = %d\n", d->scrollY, d->maxScrollY);
          }
        }
        else{
          if (akinetic_uphandler(&d->akin,atev->y)){
            ac_regfling(ctl,&d->akin,&d->scrollY,d->maxScrollY);
          }
          else if ((d->scrollY<0)||(d->scrollY>d->maxScrollY)){
            ac_regbounce(ctl,&d->scrollY,d->maxScrollY);
          }
        }
#if 1
        if (d->touchedItem != -1){
          usleep(30);
          int tmptouch=d->touchedItem;
          d->touchedItem = -1;
          acbutton_sdmenu_redrawitem(ctl,tmptouch);
          ctl->ondraw(ctl);
          msg=aw_msg(retmsgx,1,0,0);
        }
#endif
      }
      break;
    case ATEV_MOUSEMV:
#if 0		
      {
        byte allowscroll=1;
        if (atev->y!=0){
          if (d->prevTouchY!=-50){
            if (abs(d->prevTouchY-atev->y)>=agdp()*5){
              d->prevTouchY=-50;
              if (d->touchedItem != -1){
                int tmptouch=d->touchedItem;
                d->touchedItem = -1;
                acbutton_sdmenu_redrawitem(ctl,tmptouch);
                ctl->ondraw(ctl);
                aw_draw(ctl->win);
              }
            }
            else
              allowscroll=0;
          }
          if (allowscroll){
            int mv = akinetic_movehandler(&d->akin,atev->y);
            if (mv!=0){
              if ((d->scrollY<0)&&(mv<0)){
                float dumpsz = 0.6-(0.6*(((float) abs(d->scrollY))/(ctl->h/4)));
                d->scrollY+=floor(mv*dumpsz);
              }
              else if ((d->scrollY>d->maxScrollY)&&(mv>0)){
                float dumpsz = 0.6-(0.6*(((float) abs(d->scrollY-d->maxScrollY))/(ctl->h/4)));
                d->scrollY+=floor(mv*dumpsz);
              }
              else
                d->scrollY+=mv;
  
              if (d->scrollY<0-(ctl->h/4)) d->scrollY=0-(ctl->h/4);
              if (d->scrollY>d->maxScrollY+(ctl->h/4)) d->scrollY=d->maxScrollY+(ctl->h/4);
              msg=aw_msg(0,1,0,0);
              ctl->ondraw(ctl);
            }
          }
        }
      }
#endif
      break;
      case ATEV_SELECT:
      {
        if ((d->focusedItem>-1)&&(d->draweditemn>0)){
          if (atev->d){
            if ((d->touchedItem != -1)&&(d->touchedItem!=d->focusedItem)){
		miui_debug("d->touchedItem = %d d->focusedItem = %d\n", d->touchedItem, d->focusedItem);
              int tmptouch=d->touchedItem;
              d->touchedItem = -1;
              acbutton_sdmenu_redrawitem(ctl,tmptouch);
            }
            vibrate(30);
            d->touchedItem=d->focusedItem;
            acbutton_sdmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            msg=aw_msg(0,1,0,0);
          }
          else{
            if ((d->touchedItem != -1)&&(d->touchedItem!=d->focusedItem)){
              int tmptouch=d->touchedItem;
              d->touchedItem = -1;
              acbutton_sdmenu_redrawitem(ctl,tmptouch);
            }
            if ((d->selectedIndex != -1)&&(d->selectedIndex!=d->focusedItem)){
              int tmpsidx=d->selectedIndex;
              d->selectedIndex = -1;
              acbutton_sdmenu_redrawitem(ctl,tmpsidx);
            }
            d->selectedIndex = d->focusedItem;
            d->touchedItem=-1;
            acbutton_sdmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            // msg=aw_msg(0,1,0,0);
            msg=aw_msg(d->touchmsg,1,0,0);
          }
        }
      }
      break;
      case ATEV_DOWN:
        {
          if ((d->focusedItem<d->itemn-1)&&(d->draweditemn>0)){
            int prevfocus = d->focusedItem;
            d->focusedItem++;
            acbutton_sdmenu_redrawitem(ctl,prevfocus);
            acbutton_sdmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            msg=aw_msg(0,1,1,0);

	     miui_debug("d->focusedItem = %d , round((ctl->h/2) = %d\n", d->focusedItem, round(ctl->h/2));
	     miui_debug("d->items[d->focusedItem]->y = %d, (d->items[d->focusedItem]->h/2) = %d\n", d->items[d->focusedItem]->y, (d->items[d->focusedItem]->h/2));
            
            int reqY = d->items[d->focusedItem]->y - round((ctl->h/2) - (d->items[d->focusedItem]->h/2));
            ac_regscrollto(
              ctl,
              &d->scrollY,
              d->maxScrollY,
              reqY,
              &d->focusedItem,
              d->focusedItem
            );
          }
        }
      break;
      case ATEV_UP:
        {
          if ((d->focusedItem>0)&&(d->draweditemn>0)){
            int prevfocus = d->focusedItem;
            d->focusedItem--;
            acbutton_sdmenu_redrawitem(ctl,prevfocus);
            acbutton_sdmenu_redrawitem(ctl,d->focusedItem);
            ctl->ondraw(ctl);
            msg=aw_msg(0,1,1,0);
            //wangxf14 study again
            int reqY = d->items[d->focusedItem]->y - round((ctl->h/2) - (d->items[d->focusedItem]->h/2));
            ac_regscrollto(
              ctl,
              &d->scrollY,
              d->maxScrollY,
              reqY,
              &d->focusedItem,
              d->focusedItem
            );
          }
        }
      break;
  }
  return msg;
}
byte acbutton_sdmenu_onfocus(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACBUTTONMAINMENUDP   d  = (ACBUTTONMAINMENUDP) ctl->d;

  miui_debug("acbutton_sdmenu_onfocus\n");
  
  d->focused=1;
  
  if ((d->focusedItem==-1)&&(d->itemn>0)){
    d->focusedItem=0;
  }

  miui_debug("d->focusedItem = %d d->draweditemn = %d\n", d->focusedItem, d->draweditemn);
  
  if ((d->focusedItem!=-1)&&(d->draweditemn>0)){
    acbutton_sdmenu_redrawitem(ctl,d->focusedItem);
  }
  ctl->ondraw(ctl);
  return 1;
}

void acbutton_sdmenu_onblur(void * x){
  ACONTROLP   ctl= (ACONTROLP) x;
  ACBUTTONMAINMENUDP   d  = (ACBUTTONMAINMENUDP) ctl->d;
  
  miui_debug("acbutton_sdmenu_onblur\n");
  
  d->focused=0;
  
  if ((d->focusedItem!=-1)&&(d->draweditemn>0)){
    acbutton_sdmenu_redrawitem(ctl,d->focusedItem);
  }
  ctl->ondraw(ctl);
}
ACONTROLP acbuttonsdmenu(//wangxf14
  AWINDOWP win,
  int x,
  int y,
  int w,
  int h,
  byte touchmsg
){
  //-- Validate Minimum Size
  if (h<agdp()*16) h=agdp()*16;
  if (w<agdp()*20) w=agdp()*20;

  //-- Initializing Text Data
  ACBUTTONMAINMENUDP d        = (ACBUTTONMAINMENUDP) malloc(sizeof(ACBUTTONMAINMENUD));
  memset(d,0,sizeof(ACBUTTONMAINMENUD));

  //-- Set Signature
  d->acheck_signature = 144;
  
  int minpadding = max(acfg()->roundsz,4);

  miui_debug("acfg()->roundsz = %d, minpadding = %d acfg()->btnroundsz = %d\n", acfg()->roundsz, minpadding, acfg()->btnroundsz);
  
  //-- Initializing Client Size
  d->clientWidth  = w - (agdp()*40);
  d->clientTextW  = d->clientWidth - (agdp()*acfg()->btnroundsz*2);
  d->clientTextX  = (w - d->clientWidth)/2 + (agdp()*acfg()->btnroundsz);//wangxf14 menu center modify
  d->nextY       = 0;//wangxf14 debug
  d->clientHigh = 0;
  
  d->client.data=NULL;

  //-- Set Scroll Value
  d->scrollY     = 0;
  d->maxScrollY  = 0;
  d->prevTouchY  =-50;
  d->invalidDrawItem = -1;
  //-- Set Data Values
  d->items       = NULL;
  d->itemn       = 0;
  d->touchedItem = -1;
  d->focusedItem = -1;
  d->draweditemn = 0;
  d->selectedIndex = -1;
  d->touchmsg    = touchmsg;
  
  ACONTROLP ctl  = malloc(sizeof(ACONTROL));
  ctl->ondestroy= &acbutton_sdmenu_ondestroy;
  ctl->oninput  = &acbutton_sdmenu_oninput;
  ctl->ondraw   = &acbutton_sdmenu_ondraw;
  ctl->onblur   = &acbutton_sdmenu_onblur;
  ctl->onfocus  = &acbutton_sdmenu_onfocus;
  ctl->win      = win;
  ctl->x        = x;
  ctl->y        = y;
  ctl->w        = w;
  ctl->h        = h;
  ctl->forceNS  = 1;
  ctl->d        = (void *) d;
  aw_add(win,ctl);
  return ctl;
}
