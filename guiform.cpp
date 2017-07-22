#include "guiform.h"
#include "math.h"
#include "gui.h"
#include "songmain.h"
#include "chunks.h"
#include "camxfile.h"

bool guiForm_Child::Disabled()
{
	if(deactivated==false && enable==false)
		return true;

	return false;
}

bool guiForm_Child::InUse()
{
	if(deactivated==true || enable==false)
		return false;

	return true;
}

guiForm_Child *guiForm_Child::NextHChildInUse()
{
	if(fx<form->forms_horz)
	{
		for(int x=fx+1;x<form->forms_horz;x++)
			if(form->forms[x][fy].deactivated==false && form->forms[x][fy].enable==true && (!(form->forms[x][fy].dock&DOCK_STATICWIDTH)))
				return &form->forms[x][fy];
	}

	return 0;
}



guiForm_Child *guiForm_Child::PrevVChildInUse()
{
	if(fy>0)
	{
		for(int iy=fy-1;iy>=0;iy--)
			if(form->forms[fx][iy].deactivated==false && form->forms[fx][iy].enable==true && (!(form->forms[fx][iy].dock&DOCK_TOOLBARTOP)) )
				return &form->forms[fx][iy];
	}

	return 0;
}

guiForm_Child *guiForm_Child::NextVChildInUse()
{
	if(fy<form->forms_vert)
	{
		for(int iy=fy+1;iy<form->forms_vert;iy++)
			if(form->forms[fx][iy].deactivated==false && form->forms[fx][iy].enable==true && (!(form->forms[fx][iy].dock&DOCK_STATICHEIGHT)) )
				return &form->forms[fx][iy];
	}

	return 0;
}

guiForm_Child *guiForm_Child::NextHChild()
{
	return fx<form->forms_horz-1?&form->forms[fx+1][fy]:0;
}

guiForm_Child *guiForm_Child::PrevVChild()
{
	return fy>0?&form->forms[fx][fy-1]:0;
}

guiForm_Child *guiForm_Child::NextVChild()
{
	return fy<form->forms_vert-1?&form->forms[fx][fy+1]:0;
}

int guiForm_Child::GetYSub(int subindex,int y2)
{
	if(!subindex)return y2;

	return (y2-1)-(subindex*maingui->GetButtonSizeY()+(subindex-1)*ADDYSPACE);
}

void guiForm_Child::Enable(bool on)
{
	//TRACE ("Enable Child Form x=%d y=%d [%d]\n",fx,fy,on);
	bool onu=deactivated;
	deactivated=on==true?false:true;

	if(onu!=deactivated)
	{
		forceredraw=true;
		form->ReDraw(true);
	}
}

void guiForm::Form_OnCreate()
{
	// Calc New Form Sizes...

	switch(type)
	{
	case FORM_SCREEN:
		{
			form_screen=true;

			forms_horz=1;
			forms_vert=3;
			formcounter=3;

			subbottomheight=4*maingui->GetFontSizeY();

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT;
			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT;

			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;
			forms[0][2].height=subbottomheight;

			forms[0][0].vsizer=true;

			forms[0][0].percentofparentwindow_h=1;
			forms[0][0].percentofparentwindow_v=0.6;

			forms[0][1].percentofparentwindow_h=1;
			forms[0][1].percentofparentwindow_v=0.4;

			forms[0][2].percentofparentwindow_h=1;
		}
		break;

	case FORM_HORZ1x2SLIDERVTOOLBAR:
		{
			forms_horz=2;
			forms_vert=2;
			formcounter=3;

			topbaroffsety=PLUGINTOPBARHEIGHT;

			forms[1][0].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;
			//	forms[0][0].percentofparentwindow_h=1;
			forms[0][0].height=topbaroffsety;

			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM;
			forms[0][1].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[1][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;
			forms[1][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_PLUGIN:
		{
			forms_horz=1;
			forms_vert=2;
			formcounter=2;

			topbaroffsety=PLUGINTOPBARHEIGHT;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;
			forms[0][0].percentofparentwindow_h=1;
			forms[0][0].height=topbaroffsety;

			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM;
			forms[0][1].percentofparentwindow_h=1;
			forms[0][1].height=1;
			//forms[0][1].externwindow

			InitFormSize();
			EditForm(0,1,CHILD_HASWINDOW);
		}
		break;

	case FORM_PLAIN1x1:
		{
			forms_horz=1;
			forms_vert=1;
			formcounter=1;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM;
		}
		break;

	case FORM_VERT1x2_2small:
		{
			forms_horz=1;
			forms_vert=2;
			formcounter=2;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT;
			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][0].vsizer=true;
			forms[0][0].percentofparentwindow_h=1;
			forms[0][0].percentofparentwindow_v=0.8;

			forms[0][1].percentofparentwindow_h=1;
			forms[0][1].percentofparentwindow_v=0.2;
		}
		break;

	case FORM_VERT1x2:
		{
			forms_horz=1;
			forms_vert=2;
			formcounter=2;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT;
			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][0].vsizer=true;

			forms[0][0].percentofparentwindow_h=1;
			forms[0][0].percentofparentwindow_v=0.5;

			forms[0][1].percentofparentwindow_h=1;
			forms[0][1].percentofparentwindow_v=0.5;
		}
		break;

	case FORM_VERT1x3:
		{
			forms_horz=1;
			forms_vert=3;
			formcounter=3;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT;
			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT;
			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][0].percentofparentwindow_h=1;
			forms[0][0].percentofparentwindow_v=0.333;

			forms[0][1].percentofparentwindow_h=1;
			forms[0][1].percentofparentwindow_v=0.333;

			forms[0][2].percentofparentwindow_h=1;
			forms[0][2].percentofparentwindow_v=0.333;
		}
		break;


	case FORM_HORZ2x1SLIDERV: // Lists
		{
			forms_horz=2;
			forms_vert=1;
			formcounter=2;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_BOTTOM|DOCK_RIGHT;
			forms[1][0].dock=DOCK_TOP|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			forms[0][0].subwidth=EDITOR_SLIDER_SIZE_VERT;
			forms[1][0].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ2x1: // Settings
		{
			forms_horz=2;
			forms_vert=1;
			formcounter=2;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_BOTTOM;
			forms[1][0].dock=DOCK_TOP|DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][0].percentofparentwindow_h=0.3;
			forms[1][0].percentofparentwindow_h=0.7;

			forms[0][0].hsizer=true;

			EditForm(1,0,CHILD_HASWINDOW);
		}
		break;

	case FORM_HORZ2x1_SLIDERHV:
		{
			forms_horz=3;
			forms_vert=2;
			formcounter=3;

			forms[1][1].deactivated=forms[2][1].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_BOTTOM;
			forms[1][0].dock=DOCK_TOP|DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][0].percentofparentwindow_h=0.2;
			forms[1][0].percentofparentwindow_h=0.8;

			forms[0][0].hsizer=true;
			forms[0][0].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[0][0].subheight=forms[1][0].subheight=forms[2][0].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][1].subwidth=forms[1][0].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][1].dock|=DOCK_STATICHEIGHT;
			forms[2][0].dock|=DOCK_STATICWIDTH;

			forms[0][1].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[2][0].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZBAR_SLIDERHV:
		{
			//Sample Editor
			forms_horz=2;
			forms_vert=3;
			formcounter=3;

			forms[1][0].deactivated=true;
			forms[1][2].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM;
			forms[0][1].percentofparentwindow_h=1;

			forms[1][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			forms[0][1].subheight=forms[1][1].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][1].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;

			topbaroffsety=TOPBARHEIGHT_SMALL;

			forms[0][0].height=TOPBARHEIGHT_SMALL;

			forms[0][2].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[1][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ2x1BAR_SLIDERHV:
		{
			// Tempo Map
			forms_horz=3;
			forms_vert=3;
			formcounter=3;

			forms[1][0].deactivated=forms[2][0].deactivated=true;
			forms[1][2].deactivated=forms[2][2].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_BOTTOM;
			forms[1][1].dock=DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][1].percentofparentwindow_h=0.2;
			forms[1][1].percentofparentwindow_h=0.8;

			forms[0][1].hsizer=true;
			forms[0][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[0][1].subheight=forms[1][1].subheight=forms[2][1].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][2].subwidth=forms[1][1].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;
			forms[2][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			topbaroffsety=TOPBARHEIGHT_SMALL;

			forms[0][0].height=TOPBARHEIGHT_SMALL;
			forms[0][2].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[2][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ2x1BAR_EVENT_SLIDERHV:
		{
			forms_horz=3;
			forms_vert=3;
			formcounter=3;

			forms[1][0].deactivated=forms[2][0].deactivated=true;
			forms[1][2].deactivated=forms[2][2].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_BOTTOM;
			forms[1][1].dock=DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][1].percentofparentwindow_h=0.7;
			forms[1][1].percentofparentwindow_h=0.3;

			forms[0][1].hsizer=true;
			forms[0][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[0][1].subheight=forms[1][1].subheight=forms[2][1].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][2].subwidth=forms[1][1].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;
			forms[2][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			topbaroffsety=TOPBARHEIGHT;

			forms[0][0].height=TOPBARHEIGHT;
			forms[0][2].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[2][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ3x1BAR_SLIDERHV:
		{
			forms_horz=4;
			forms_vert=3;
			formcounter=4;

			subbottomheight=EDITOR_SLIDER_SIZE_VERT;

			forms[1][0].deactivated=forms[2][0].deactivated=forms[3][0].deactivated=true;
			forms[1][2].deactivated=forms[2][2].deactivated=forms[3][2].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_BOTTOM;
			forms[1][1].dock=DOCK_BOTTOM;
			forms[2][1].dock=DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][1].percentofparentwindow_h=0.1;
			forms[1][1].percentofparentwindow_h=0.2;
			forms[2][1].percentofparentwindow_h=0.7;

			forms[0][1].hsizer=true;
			forms[0][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[1][1].hsizer=true;
			forms[1][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			//	forms[0][1].subheight=forms[1][1].subheight=forms[2][1].subheight=forms[3][1].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][2].subwidth=forms[2][1].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;
			forms[3][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			topbaroffsety=2*maingui->GetButtonSizeY()+(2+3)*ADDYSPACE;

			forms[0][0].height=topbaroffsety;
			forms[0][2].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[3][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ4x1BAR_SLIDERHV: // Arrange
	case FORM_HORZ4x1BAR_SLIDERHV3: // Piano
		{
			forms_horz=5;
			forms_vert=3;
			formcounter=4;

			subbottomheight=EDITOR_SLIDER_SIZE_VERT;

			forms[1][0].deactivated=forms[2][0].deactivated=forms[3][0].deactivated=forms[4][0].deactivated=true;
			forms[1][2].deactivated=forms[2][2].deactivated=forms[3][2].deactivated=forms[4][2].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_BOTTOM;
			forms[1][1].dock=DOCK_BOTTOM;
			forms[2][1].dock=DOCK_BOTTOM;
			forms[3][1].dock=DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][1].percentofparentwindow_h=0.18;
			forms[1][1].percentofparentwindow_h=0.12;
			forms[2][1].percentofparentwindow_h=0.2;
			forms[3][1].percentofparentwindow_h=0.5;

			forms[0][1].hsizer=true;
			forms[0][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[1][1].hsizer=true;
			forms[1][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[2][1].hsizer=true;
			forms[2][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			//	forms[0][1].subheight=forms[1][1].subheight=forms[2][1].subheight=forms[3][1].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][2].subwidth=forms[3][1].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;
			forms[4][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			if(type==FORM_HORZ4x1BAR_SLIDERHV3)
			{
				topbaroffsety=maingui->GetButtonSizeY(3);
				topbaroffsety+=maingui->borderframey;
			}
			else
				topbaroffsety=maingui->GetButtonSizeY(3)+ADDYSPACE;

			forms[0][0].height=topbaroffsety;
			forms[0][2].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[4][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ3x2BAR_SLIDERHV:
		{
			// Drum Editor
			forms_horz=4;
			forms_vert=3;
			formcounter=4;

			subbottomheight=EDITOR_SLIDER_SIZE_VERT;

			forms[1][0].deactivated=forms[2][0].deactivated=forms[3][0].deactivated=true;
			forms[1][2].deactivated=forms[2][2].deactivated=forms[3][2].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_BOTTOM;
			forms[1][1].dock=DOCK_BOTTOM;
			forms[2][1].dock=DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][1].percentofparentwindow_h=0.1;
			forms[1][1].percentofparentwindow_h=0.2;
			forms[2][1].percentofparentwindow_h=0.7;

			forms[0][1].hsizer=true;
			forms[0][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[1][1].hsizer=true;
			forms[1][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			//	forms[0][1].subheight=forms[1][1].subheight=forms[2][1].subheight=forms[3][1].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][2].subwidth=forms[2][1].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][2].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;
			forms[3][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			topbaroffsety=maingui->GetButtonSizeY(3);
			topbaroffsety+=maingui->borderframey;

			forms[0][0].height=topbaroffsety;
			forms[0][2].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[3][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ1x2BAR_SLIDERV: // Mixer etc..
		{
			forms_horz=2;
			forms_vert=4;
			formcounter=3;

			subbottomheight=EDITOR_SLIDER_SIZE_VERT;

			forms[1][0].deactivated=true;
			forms[1][3].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT;
			forms[1][1].dock=forms[1][2].dock=DOCK_RIGHT|DOCK_STATICWIDTH;

			forms[0][2].dock=DOCK_BOTTOM|DOCK_LEFT|DOCK_RIGHT;
			forms[0][3].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;

			forms[0][1].bindtovertslider=forms[0][2].bindtovertslider=forms[1][1].bindtovertslider=forms[1][2].bindtovertslider=true;

			forms[0][1].percentofparentwindow_v=forms[1][1].percentofparentwindow_v=0.4;
			forms[0][2].percentofparentwindow_v=forms[1][2].percentofparentwindow_v=0.6;

			forms[0][1].percentofparentwindow_h=forms[0][2].percentofparentwindow_h=1;

			forms[0][1].vsizer=true;

			//	forms[0][2].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][1].subwidth=forms[0][2].subwidth=EDITOR_SLIDER_SIZE_VERT;

			topbaroffsety=(4*maingui->GetButtonSizeY()+(4+3)*ADDYSPACE);

			forms[0][0].height=topbaroffsety;
			forms[0][3].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[1][1].width=forms[1][2].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ1x2BAR_MIXER1TRACK:
		{
			forms_horz=2;
			forms_vert=3;
			formcounter=3;

			forms[1][0].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT|DOCK_RIGHT;
			forms[1][1].dock=forms[1][2].dock=DOCK_RIGHT|DOCK_STATICWIDTH;

			forms[0][2].dock=DOCK_BOTTOM|DOCK_LEFT|DOCK_RIGHT;
			//forms[0][3].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;

			forms[0][1].bindtovertslider=forms[0][2].bindtovertslider=forms[1][1].bindtovertslider=forms[1][2].bindtovertslider=true;

			forms[0][1].percentofparentwindow_v=forms[1][1].percentofparentwindow_v=0.3;
			forms[0][2].percentofparentwindow_v=forms[1][2].percentofparentwindow_v=0.7;

			forms[0][1].percentofparentwindow_h=forms[0][2].percentofparentwindow_h=1;

			forms[0][1].vsizer=true;

			//forms[0][2].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][1].subwidth=forms[0][2].subwidth=EDITOR_SLIDER_SIZE_VERT;

			topbaroffsety=2*maingui->GetButtonSizeY()+(2+3)*ADDYSPACE;

			//	topbaroffsety=maingui->GetButtonSizeY(2);

			forms[0][0].height=topbaroffsety;
			//forms[0][3].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[1][1].width=forms[1][2].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;

	case FORM_HORZ2x2BAR_SLIDERHV:
		{
			forms_horz=3;
			forms_vert=4;
			formcounter=3;

			subrightwidth=EDITOR_SLIDER_SIZE_HORZ;
			subbottomheight=EDITOR_SLIDER_SIZE_VERT;

			forms[1][0].deactivated=forms[2][0].deactivated=true;
			forms[2][2].deactivated=true;
			forms[1][3].deactivated=forms[2][3].deactivated=true;

			forms[0][0].dock=DOCK_TOP|DOCK_LEFT|DOCK_RIGHT|DOCK_TOOLBARTOP|DOCK_STATICHEIGHT;

			forms[0][1].dock=DOCK_LEFT;
			forms[1][1].dock=DOCK_RIGHT;

			forms[0][2].dock=DOCK_LEFT|DOCK_BOTTOM;
			forms[1][2].dock=DOCK_RIGHT|DOCK_BOTTOM;

			forms[0][2].bindtohorzslider=forms[1][2].bindtohorzslider=true;
			forms[0][1].bindtovertslider=forms[0][2].bindtovertslider=true;

			forms[0][1].percentofparentwindow_v=forms[1][1].percentofparentwindow_v=0.8;
			forms[0][1].percentofparentwindow_h=forms[0][2].percentofparentwindow_h=0.2;
			forms[1][1].percentofparentwindow_h=forms[1][1].percentofparentwindow_h=0.8;

			forms[0][2].percentofparentwindow_v=forms[1][2].percentofparentwindow_v=0.2;

			forms[0][1].hsizer=true;
			forms[0][1].sizerhorzsubh=EDITOR_SLIDER_SIZE_HORZ;

			forms[1][1].vsizer=true;
			forms[1][1].sizerversubw=EDITOR_SLIDER_SIZE_VERT;

			//	forms[0][2].subheight=forms[1][2].subheight=forms[2][1].subheight=EDITOR_SLIDER_SIZE_HORZ;
			forms[0][3].subwidth=forms[1][1].subwidth=forms[1][2].subwidth=EDITOR_SLIDER_SIZE_VERT;

			forms[0][3].dock=DOCK_LEFT|DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICHEIGHT;
			forms[2][1].dock=DOCK_RIGHT|DOCK_BOTTOM|DOCK_STATICWIDTH;

			topbaroffsety=maingui->GetButtonSizeY(3);
			topbaroffsety+=maingui->borderframey;

			forms[0][0].height=topbaroffsety;
			forms[0][3].height=EDITOR_SLIDER_SIZE_HORZ;
			forms[2][1].width=EDITOR_SLIDER_SIZE_VERT;
		}
		break;
	}

	RecalcForm();

	//if(formcounter>1)
	{
		// Sizer H
		for(int x=0;x<forms_horz;x++)
			for(int y=0;y<forms_vert;y++)
			{
				guiForm_Child *form=GetForm(x,y);

				//	TRACE ("Form H\n");

				if(form->deactivated==false)
				{
					if(form->hsizer==true)
					{
						int flag;

						if(form->NextHChildInUse())
						{
							form->horzsliderhide=false;
							flag=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|BS_OWNERDRAW;
						}
						else
						{
							form->horzsliderhide=true;
							flag=WS_CHILD|WS_CLIPSIBLINGS|BS_OWNERDRAW;
						}

						form->sizerhorz_hWnd=CreateWindowEx
							(
							WS_EX_NOPARENTNOTIFY,
							form_screen==true?"CAMXSCRZH":"CAMXZH", 
							0,
							flag,
							form->x+form->GetWidth(), 
							form->y, 
							SPLITTERHORIZON_W,
							((height-1)-form->sizerhorzsubh)-form->y, 
							hWnd,
							(HMENU)9999, 
							hInst, 
							0);

						if(!form->sizerhorz_hWnd)
							maingui->MessageBoxError(0,"CAMXZH");
					}

					if(form->vsizer==true)
					{
						int flag;

						if(form->NextVChildInUse())
						{
							form->vertsliderhide=false;
							flag=WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|BS_OWNERDRAW;
						}
						else
						{
							form->vertsliderhide=true;
							flag=WS_CHILD|WS_CLIPSIBLINGS|BS_OWNERDRAW;
						}

						form->sizervert_hWnd=CreateWindowEx
							(
							WS_EX_NOPARENTNOTIFY,
							form_screen==true?"CAMXSCRZV":"CAMXZV", 
							0,
							flag,
							form->x, 
							form->y+form->GetHeight(), 
							((width-1)-form->sizerversubw)-form->x,SPLITTERVERTICAL_H, 
							hWnd,
							(HMENU)9999, 
							hInst, 
							0);

						if(!form->sizervert_hWnd)
							maingui->MessageBoxError(0,"CAMXZV");
					}
				}
			}

			Form_CreateFormObjects();
	}
}

int guiForm::GetMaxWidth(bool subsizer)
{
	for(int x=0;x<forms_horz;x++)
		for(int y=0;y<forms_vert;y++)
		{
			guiForm_Child *fo=GetForm(x,y);

			if(fo->deactivated==false && fo->enable==true)
			{
				if((fo->dock&DOCK_RIGHT) && (fo->dock&DOCK_STATICWIDTH))
					return width-fo->GetWidth();
			}
		}

		return width;
}

void guiForm::GetMaxHeight(int *maxh,int *bottomh)
{
	*maxh=*bottomh=height;
	*maxh-=topbaroffsety+subbottomheight;

	//*bottomh-=subbottomheight;

	//bool bottom=false;

	for(int x=0;x<forms_horz;x++)
		for(int y=0;y<forms_vert;y++)
		{
			guiForm_Child *fo=GetForm(x,y);

			if(fo->deactivated==false && fo->enable==true)
			{
				if(fo->vsizer==true)
					*maxh-=SPLITTERVERTICAL_H;

				/*
				if(bottom==false && (fo->dock&DOCK_BOTTOM) && (fo->dock&DOCK_STATICHEIGHT))
				{
				*maxh-=fo->GetHeight();
				*bottomh-=fo->GetHeight();
				bottom=true;
				}
				*/

			}
		}

		//*maxh-=subbottomheight;
}

void guiForm::BufferOldPositions()
{
	for(int x=0;x<forms_horz;x++)
		for(int y=0;y<forms_vert;y++)
		{
			guiForm_Child *c=GetForm(x,y);

			if(c->deactivated==false)
			{
				c->ox=c->x;
				c->oy=c->y;
				c->owidth=c->width;
				c->oheight=c->height;
			}
		}
}

void guiForm::RefreshUseBuffer()
{
	for(int x=0;x<forms_horz;x++)
		for(int y=0;y<forms_vert;y++)
		{
			guiForm_Child *c=GetForm(x,y);

			if(c->InUse()==true && (c->ox!=c->x || c->oy!=c->y || c->owidth!=c->width || c->oheight!=c->height))	
			{
				RecalcChildPercent(c);
				c->ChangeChild();
			}
		}
}


void guiForm::FormEnable(int x,int y,bool enable)
{
	guiForm_Child *form=GetForm(x,y);

	if(form && form->deactivated==false && form->enable!=enable)
	{
		form->enable=enable;
		form->forceredraw=true;

		ReDraw(false);
	}
}

void guiForm::FormYEnable(int y,bool enable)
{
	bool redraw=false;

	double precent_h;

	guiForm_Child *hchild;

	for(int x=0;x<forms_horz;x++)
	{
		guiForm_Child *form=GetForm(x,y);

		if(enable==true)
		{
			if((hchild=form->NextVChildInUse()) && ((hchild->dock&DOCK_STATICHEIGHT)==0) && ((hchild->dock&DOCK_TOOLBARTOP)==0))
				precent_h=hchild->percentofparentwindow_h;
			else
				if((hchild=form->PrevVChildInUse()) && ((hchild->dock&DOCK_STATICHEIGHT)==0) && ((hchild->dock&DOCK_TOOLBARTOP)==0))
					precent_h=hchild->percentofparentwindow_h;
				else
					precent_h=1; // Default 1
		}

		if(form->deactivated==false && form->enable!=enable)
		{
			form->enable=enable;

			if(enable==false)
			{
				// Buffer existing Form Percents
				for(int x=0;x<forms_horz;x++)
					for(int y=0;y<forms_vert;y++)
					{
						//	form->percentofparentwindow_buffer_h[x][y]=forms[x][y].percentofparentwindow_h;
						form->percentofparentwindow_buffer_v[x][y]=forms[x][y].percentofparentwindow_v;
					}
			}
			else
			{
				// Reset Percents
				//	for(int x=0;x<forms_horz;x++)
				for(int y2=0;y2<forms_vert;y2++)
				{
					//	forms[x][y].percentofparentwindow_h=form->percentofparentwindow_buffer_h[x][y];
					if(y==y2)
						forms[x][y2].percentofparentwindow_h=precent_h;

					forms[x][y2].percentofparentwindow_v=form->percentofparentwindow_buffer_v[x][y2];
				}

				//if(form->NextVChildInUse()==true && form->NextVChildInUse()->dock&DOCK_STATICHEIGH
			}

			if(form->bindwindow)
			{
				// Window
				form->forceredraw=enable;
			}
			else
				// DB always refresh
				form->forceredraw=true;

			redraw=true;
		}
	}

	if(redraw==true)
		ReDraw(false);
}

void guiForm::ToggleForm(guiForm_Child *child,bool leftmouse) // Up/Down
{
	BufferOldPositions();

	if(child->percentofparentwindow_v_toggle==-1 && child->percentofparentwindow_v<1)
	{
		int maxy2,bottomh;
		GetMaxHeight(&maxy2,&bottomh);

		// Up
		child->percentofparentwindow_v_toggle=child->percentofparentwindow_v;
		SetChildHeight(child,leftmouse==true?0:maxy2);
	}
	else
	{
		// Down
		int maxy2,bottomh;
		GetMaxHeight(&maxy2,&bottomh);

		double h=maxy2;
		h*=child->percentofparentwindow_v_toggle;
		child->percentofparentwindow_v_toggle=-1;

		SetChildHeight(child,(int)h);
	}

	RefreshUseBuffer();
}

void guiForm::SetChildHeight(guiForm_Child *fo,int height)
{
	if(height<0)
		height=0;

	if(fo->height!=height)
	{
		fo->height=height;

		int x=fo->fx,y=fo->fy;

		for(int x2=x-1;x2>=0;x2--)
		{
			guiForm_Child *fo3=GetForm(x2,y);

			if(fo3->deactivated==false && fo3->bindtovertslider==true)
				fo3->height=fo->height;
		}

		for(int x2=x+1;x2<forms_horz;x2++)
		{
			guiForm_Child *fo3=GetForm(x2,y);

			if(fo3->deactivated==false && fo3->bindtovertslider==true)
				fo3->height=fo->height;
		}

		int ly=fo->y+fo->height+SPLITTERVERTICAL_H;

		if(y+1<forms_vert)
			for(int x2=0;x2<forms_horz;x2++)
			{
				guiForm_Child *fo2=GetForm(x2,y+1);

				if(fo2->deactivated==false /*&& fo2->bindtovertslider==true*/)
				{
					if(!(fo2->dock&DOCK_STATICHEIGHT))
					{
						int oy2=fo2->y+fo2->GetHeight();

						//	TRACE ("+++++ FO2 X %d Y %d \n",fo2->fx,fo2->fy);
						//	TRACE ("FO2  I Y %d H %d OY2 %d\n",fo2->y,fo2->height,oy2);

						fo2->y=ly;
						fo2->height=oy2-ly;

						//	TRACE ("FO2 H %d\n",fo2->height);

						if(fo2->height<0)
							fo2->height=0;
					}
				}
			}
	}
}

void guiForm::MoveForm()
{
	int maxx2=GetMaxWidth(true);

	BufferOldPositions();

	for(int x=0;x<forms_horz;x++)
		for(int y=0;y<forms_vert;y++)
		{
			guiForm_Child *fo=GetForm(x,y);

			if(fo->userselect==true && fo->InUse()==true)
			{
				if(fo->sizediffx && (fo->width>SPLITTERHORIZON_W || fo->sizediffx>0))
				{
					// Horz <<<->>>
					int nw=fo->width+fo->sizediffx;

					if(fo->x+nw>maxx2)
						nw=maxx2-fo->x;

					if(nw<SPLITTERHORIZON_W)
						nw=SPLITTERHORIZON_W;

					// Find Next Y Sizer
					int lx=fo->x+nw+SPLITTERHORIZON_W;

					for(int x2=x+1;x2<forms_horz;x2++)
					{
						for(int y2=0;y2<forms_vert;y2++)
						{
							guiForm_Child *fo2=GetForm(x2,y2);

							if(fo2->InUse()==true && fo2->hsizer==true)
							{
								if(lx>=fo2->x+fo2->GetWidth())
									return;

								goto check;
							}
						}
					}

check:
					if(nw!=fo->width)
					{
						fo->width=nw;

						for(int y2=0;y2<forms_vert;y2++)
						{
							guiForm_Child *fo2=GetForm(x,y2);

							if(fo2!=fo && fo2->InUse()==true && fo2->bindtohorzslider==true)
								fo2->width=fo->width;
						}

						for(int x2=x+1;x2<forms_horz;x2++)
						{
							bool found=false;

							for(int y2=0;y2<forms_vert;y2++)
							{
								guiForm_Child *fo3=GetForm(x2,y2);

								if(fo3->InUse()==true && /* fo3->bindtohorzslider==true && /* (!(fo3->dock&DOCK_RIGHT)) &&*/ (!(fo3->dock&DOCK_STATICWIDTH)) )
								{
									fo3->x=lx;
									fo3->width+=fo3->ox-fo3->x;
									found=true;
									//TRACE ("F3 X %d\n",lx);
									//TRACE ("F3 W %d\n",fo3->width);
								}
							}

							if(found==true)
								break;
						}
					}
				}// End Horz

				if(fo->sizediffy)
				{
					int maxy2,bottomh;
					GetMaxHeight(&maxy2,&bottomh);

					// Vert 
					int nh=fo->height+fo->sizediffy;

					if(nh>maxy2)
						nh=maxy2;

					// TRACE ("Changed Vert ... Y %d H %d Max %d\n",fo->y,fo->height,maxy2);
					SetChildHeight(fo,nh);

				}// End Vert
			}
		}

		RefreshUseBuffer();
}

int guiForm::InitY(int yp,int maxheight,int bottomy,guiForm_Child *c)
{
	if(c->dock&DOCK_TOOLBARTOP)
	{
		c->y=0;
		return c->height;
	}

	c->y=yp;

	if(c->dock&DOCK_BOTTOM)
	{
		if(c->dock&DOCK_STATICHEIGHT)
		{
			c->y=bottomy-c->height;
			return bottomy;
		}
		//else
		//	c->height=bottomy-(yp+subbottomheight);
	}

	//else
	{
		if(c->dock&DOCK_STATICHEIGHT)
		{
			c->y=bottomy-c->height;
		}
		else
			//if(!(c->dock&DOCK_STATICHEIGHT))
		{
			//c->y=yp;

#ifdef DEBUG
			if(c->percentofparentwindow_v<0 || c->percentofparentwindow_v>1)
				maingui->MessageBoxError(0,"c->percentofparentwindow_v");
#endif

			double oh=c->percentofparentwindow_v;
			int addy=0,prevnextfound=0;

			guiForm_Child *p=c->PrevVChild();

			while(p)
			{
				if(p->deactivated==false && (!(p->dock&DOCK_TOOLBARTOP)) && (!(p->dock&DOCK_STATICHEIGHT)))
					prevnextfound++;

				if(p->Disabled()==true)
					oh+=p->percentofparentwindow_v;

				p=p->PrevVChild();
			}

			p=c->NextVChild();

			while(p)
			{
				if(p->deactivated==false && (!(p->dock&DOCK_TOOLBARTOP)) && (!(p->dock&DOCK_STATICHEIGHT)))
					prevnextfound++;

				if(p->Disabled()==true)
				{
					oh+=p->percentofparentwindow_v;
				}
				else
				{
					if(!c->NextVChildInUse())
					{
						// Next Y Disable ?
						//if(c->vsizer==true && c->NextVChild())
						//	oh+=c->NextVChild()->percentofparentwindow_v;
						//else
						{
							if(c->NextHChildInUse() && c->NextHChildInUse()->vsizer==true && c->NextHChildInUse()->NextVChild())
							{
								//oh+=c->NextHChildInUse()->NextVChild()->percentofparentwindow_v;
								addy=SPLITTERVERTICAL_H;
							}
						}
					}
				}

				p=p->NextVChild();
			}

			if(prevnextfound==0)
				oh=1;

			if(oh>1)
			{
#ifdef DEBUG
				maingui->MessageBoxError(0,"oh>1");
#endif
				oh=1;
			}

			oh*=(double)maxheight/*-topbaroffsety)*/;

			int ih=(int)floor(oh+0.5);

			c->height=ih-c->subheight;

			if(c->vsizer==true)
			{
				if(c->NextVChildInUse())
					yp+=SPLITTERVERTICAL_H;
				else
					addy=SPLITTERVERTICAL_H;
			}

			c->height+=addy;
		}
	}

	if(c->height<0)
		c->height=0;

	return yp+c->height;
}

guiForm::guiForm()
{
	formcounter=0;
	forms_horz=forms_vert=0;
	topbaroffsety=0;

#ifdef WIN32
	hWnd=0;
#endif

	fx=fy=0;
	maximized=minimized=false;
	closeit=false;
	form_screen=false;
	autovscroll=false;

	maxheight=0;
	maxwidth=0;
	minwidth=0;
	minheight=0; // Set by Editor Class

	subrightwidth=subbottomheight=0;

	for(int x=0;x<MAXFORMCHILDS;x++)
		for(int y=0;y<MAXFORMCHILDS;y++)
		{
			forms[x][y].form=this;
			forms[x][y].fx=x;
			forms[x][y].fy=y;
		}

		Form_Create(); // Default 1x1
}

void guiForm::RecalcForm()
{
	int maxwidth=GetMaxWidth(true),maxheight,bottomheight;

	GetMaxHeight(&maxheight,&bottomheight);

	//TRACE ("Height %d\n",height);
	//TRACE ("Max Height %d\n",maxheight);
	//TRACE ("Bottom Height %d\n",bottomheight);

	for(int y=0;y<forms_vert;y++)
	{
		int xp=0;

		for(int x=0;x<forms_horz;x++)
		{
			guiForm_Child *c=GetForm(x,y);

			if(c->deactivated==true)
			{
				c->width=c->height=0;
			}
			else
				if(c->enable==false)
				{
					//c->width=c->height=1;
				}
				else
				{
					int dock=c->dock;

					if(!c->NextHChildInUse())
					{
						dock|=DOCK_RIGHT;
					}

					if(dock&DOCK_RIGHT)
					{
						if(dock&DOCK_STATICWIDTH)
						{					
							c->x=width-c->width;
							//	c->width=width-xp;
						}
						else
						{
							c->x=xp;
							c->width=(dock&DOCK_TOOLBARTOP)?width:maxwidth-xp;
							//c->width=(x==forms_horz-1 || (c->dock&DOCK_TOOLBARTOP))?width-(c->x+c->subwidth):maxwidth-(c->x+c->subwidth);
						}
					}
					else
					{
						c->x=xp;

						double oh=c->percentofparentwindow_h;
						oh*=maxwidth;

						int ih=(int)floor(oh+0.5);
						c->width=ih-c->subwidth;

						//if(c->hsizer==true && c->NextHChildInUse()==true)
						//	xp+=SPLITTERHORIZON_W;

						if(c->hsizer==true || c->bindtohorzslider==true)
						{
							if(c->NextHChildInUse())
								xp+=SPLITTERHORIZON_W;
							else
								c->width+=SPLITTERHORIZON_W;
						}
					}

					if(c->width<0)
						c->width=0;

					xp+=c->width;
				}
		}
	}

	for(int x=0;x<forms_horz;x++)
	{
		int yp=topbaroffsety;

		for(int y=0;y<forms_vert;y++)
		{
			guiForm_Child *c=GetForm(x,y);

			if(c->deactivated==true)
			{
				c->width=c->height=0;
			}
			else
				if(c->enable==false)
				{
					for(int xh=x+1;xh<forms_horz;xh++)
					{
						guiForm_Child *c2=GetForm(xh,y);

						if(c2->enable==true && c2->deactivated==false)
						{
							yp=InitY(yp,maxheight,bottomheight,c2);
							break;
						}
					}
				}
				else
					yp=InitY(yp,maxheight,bottomheight,c);
		}
	}
}

void guiForm::ReDraw(bool force)
{
	if(mainvar->exitprogram_flag==true)
		return;

	if(force==false)
		BufferOldPositions();

	RecalcForm();

	for(int x=0;x<forms_horz;x++)
		for(int y=0;y<forms_vert;y++)
		{
			guiForm_Child *c=GetForm(x,y);

			if(c->deactivated==false && (c->forceredraw==true || force==true || c->ox!=c->x || c->oy!=c->y || c->owidth!=c->width || c->oheight!=c->height))
			{
				c->ChangeChild();
				c->forceredraw=false;
			}
		}

		InitAutoVScroll();
}

void guiForm::Form_NewSize(int nwidth,int nheight,bool force) // OnCreate, else NewSize
{
	// Calc New Form Sizes...
	width=nwidth;
	height=nheight;

	ReDraw(force);

	//	if(autovscroll==true)
	//		InitAutoVScroll();
}

guiForm_Child::guiForm_Child()
{
	fhWnd=sizervert_hWnd=sizerhorz_hWnd=0;
	userselect=false;

	sizediffx=sizediffy=0;
	subwidth=subheight=0;

	bindtovertslider=bindtohorzslider=hsizer=vsizer=false;
	deactivated=sizechanged=false;

	gx=STARTOFGADGETSX;
	gy=STARTOFGADGETSY;

	dock=0;
	lastheight=0;
	x=y=0;

	maxx2=0;
	maxy2=0;

	sizerhorzsubh=sizerversubw=0;
	//	externwindow=false;
	screen=0;
	enable=true;
	forceredraw=false;
	horzsliderhide=vertsliderhide=true;
	flag=0;
	bindwindow=0;
	disablewithrecalc=false;
	fromdesktoptoscreen=false;

	percentofparentwindow_h=percentofparentwindow_v=1;
	percentofparentwindow_v_toggle=-1;
}

void guiForm_Child::BindWindow(guiWindow *win)
{
	if(win && (flag&CHILD_HASWINDOW))
	{
		bindwindow=win;
		win->bindtoform=this;

		guiWindowSetting settings;
		settings.bindtoform=this;
		settings.noactivate=true;
		maingui->OpenWindow(win,&settings,0,0);
	}
#ifdef DEBUG
	else if(win)
		maingui->MessageBoxError(0,"BindWindow");
#endif

}

void guiForm_Child::ChangeChild()
{
	sizechanged=true;

	// Enable/Disable H/V Slider

	if(sizerhorz_hWnd) // Top <-> Down
	{
		if(InUse()==true && NextHChildInUse())
		{
			MoveWindow(sizerhorz_hWnd,x+GetWidth(),y,SPLITTERHORIZON_W,(form->height-sizerhorzsubh)-y,TRUE);
			UpdateWindow(sizerhorz_hWnd);

			if(horzsliderhide==true)
			{
				horzsliderhide=false;
				ShowWindow(sizerhorz_hWnd,SW_SHOWNA);
			}
		}
		else
		{
			if(horzsliderhide==false)
			{
				horzsliderhide=true;
				ShowWindow(sizerhorz_hWnd,SW_HIDE);
			}
		}
	}

	if(sizervert_hWnd) // Left<->Right
	{
		if(InUse()==true && NextVChildInUse())
		{
			MoveWindow(sizervert_hWnd,x,y+GetHeight(),(form->width-sizerversubw)-x,SPLITTERVERTICAL_H,TRUE);
			UpdateWindow(sizervert_hWnd);

			if(vertsliderhide==true){
				vertsliderhide=false;
				ShowWindow(sizervert_hWnd,SW_SHOWNA);
			}
		}
		else
		{
			if(vertsliderhide==false){
				vertsliderhide=true;
				ShowWindow(sizervert_hWnd,SW_HIDE);
			}
		}
	}

	if(bindwindow)
	{
		// Window in Window

		if(bindwindow->hWnd)
		{
			if(InUse()==true)
			{
				if(forceredraw==true)
				{
					form->Form_RepaintChild(this); // Resize Gadgets etc..

					// Refresh All
					//bindwindow->Form_RepaintChild(this);
					//bindwindow->ReDraw(true);
					MoveWindow(bindwindow->hWnd,x,y,GetWidth(),GetHeight(),TRUE);
					//InvalidateRect(bindwindow->hWnd,0,TRUE); // Force Redraw
				}
				else
					MoveWindow(bindwindow->hWnd,x,y,GetWidth(),GetHeight(),TRUE);

				UpdateWindow(bindwindow->hWnd);
				ShowWindow(bindwindow->hWnd,SW_SHOWNA);

				bindwindow->hide=false;
			}
			else
			{
				//KillTimer(bindwindow->hWnd,NULL);
				ShowWindow(bindwindow->hWnd,SW_HIDE);
				MoveWindow(bindwindow->hWnd,0,0,0,0,FALSE);
				bindwindow->hide=true;
			}
		}

		return;
	}

	if(fhWnd)
	{
		if(InUse()==true)
		{
			form->Form_RepaintChild(this);

			guiWindow *w=maingui->FirstWindow();
			while(w)
			{
				if(w->hWnd==fhWnd)
				{
					w->hide=false;
					break;
				}

				w=w->NextWindow();
			}

			MoveWindow(fhWnd,x,y,GetWidth(),GetHeight(),TRUE);	
			UpdateWindow(fhWnd);
			ShowWindow(fhWnd,SW_SHOWNA);

		}
		else
		{
			form->Form_FreeChildObjects(this);
			ShowWindow(fhWnd,SW_HIDE);
			MoveWindow(fhWnd,0,0,0,0,FALSE);
		}
	}
}

void guiForm::ReadForm(camxFile *file)
{
	file->ReadAndAddClass((CPOINTER)this);

	file->ReadChunk(&width);
	file->ReadChunk(&height);
	file->ReadChunk(&maximized);
	file->ReadChunk(&minimized);
	file->ReadChunk(&fx);
	file->ReadChunk(&fy);

	file->CloseReadChunk();
}

void guiForm::SaveForm(camxFile *file)
{
	file->OpenChunk(CHUNK_FORM);

	file->Save_Chunk((CPOINTER)this);

	file->Save_Chunk(save_width);
	file->Save_Chunk(save_height);
	file->Save_Chunk(maximized);
	file->Save_Chunk(minimized);
	file->Save_Chunk(fx);
	file->Save_Chunk(fy);

	file->CloseChunk();
}

void guiForm::InitMaxInfo(LPARAM lParam)
{
		MINMAXINFO *info=(MINMAXINFO *)lParam;

				if(maxwidth || maxheight || minheight || minwidth)
				{
					RECT winrect;
					GetWindowRect(hWnd,&winrect);

					int winw=(winrect.right-winrect.left)+1,
						winh=(winrect.bottom-winrect.top)+1;

					RECT maxrect;
					GetClientRect(hWnd,&maxrect);

					int clientw=(maxrect.right-maxrect.left)+1;
					int clienth=(maxrect.bottom-maxrect.top)+1;

					if(minwidth)
						info->ptMinTrackSize.x=minwidth+(winw-clientw)-3;

					if(maxwidth)
						info->ptMaxTrackSize.x=maxwidth+(winw-clientw)-3;

					if(minheight)
						info->ptMinTrackSize.y=minheight+(winh-clienth)-3;

					if(maxheight)
						info->ptMaxTrackSize.y=maxheight+(winh-clienth)-3;
				}

}

void guiForm::SetFormXY(int x,int y)
{
	TRACE ("FX %d FY %d\n",x,y);

	if(x>-32000)
	fx=x;

	if(y>-32000)
	fy=y;
}

void guiForm::SetFormSizeFlags(WPARAM sflag,int nW,int nH)
{
	if(sflag&SIZE_MAXIMIZED)
	{
		maximized=true;
		minimized=false;
	}

	if(sflag&SIZE_MINIMIZED)
	{
		maximized=false;
		minimized=true;
	}

	if((!(sflag&SIZE_MAXIMIZED)) && (!(sflag&SIZE_MINIMIZED)) )
	{
		if(minimized==false)
		{
		save_width=nW;
		save_height=nH;

if(nH==0)
{
	int i=0;
}

		}
		else
		{
			int i=0;
		}

		//maximized=minimized=false;
	}
}

void guiForm::RecalcChildPercent(guiForm_Child *fo)
{
	double h=fo->GetWidth();
	double h2=GetMaxWidth(false);

	TRACE ("RecalcChildPercent %d %d \n",fo->fx,fo->fy);

	fo->percentofparentwindow_h=h2>0?h/h2:0;

	h=fo->height;
	int maxy2,bottomh;
	GetMaxHeight(&maxy2,&bottomh);

	h2=maxy2;
	fo->percentofparentwindow_v=h2>0?h/h2:0;

	if(fo->percentofparentwindow_h>1)
		fo->percentofparentwindow_h=1;

	if(fo->percentofparentwindow_v>1)
		fo->percentofparentwindow_v=1;

	/*
	if(fo->NextHChild() && fo->NextHChildInUse()==false)
	{
	fo->percentofparentwindow_h-=fo->NextHChild()->percentofparentwindow_h;
	}

	if(fo->NextVChild() && fo->NextVChildInUse()==false)
	{
	fo->percentofparentwindow_v-=fo->NextVChild()->percentofparentwindow_v;
	}
	*/

#ifdef DEBUG
	if(fo->percentofparentwindow_v<0 || fo->percentofparentwindow_v>1)
		maingui->MessageBoxError(0,"RecalcChildPercent y");
#endif

}


