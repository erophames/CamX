#include "defines.h"
#include "objectevent.h"
#include "MIDIhardware.h"


static char *sysid[]=
{
	"Sequential",
		"IDP",
		"Octave-Plateu",
		"Moog",
		"Passport",
		"Lexicon",
		"Kurzweil",
		"Fender",
		"Gulbrandsen",
		"AKG",
		"Voyce Music",
		"Waveframe",
		"ADA",
		"Garfield El.",
		"Ensoniq",
		"Oberheim",
		"Apple",
		"Grey Matter",
		"Digidesign",
		"Palm Tree",
		"JL Cooper",
		"Lowrey",
		"Adam-Smith",
		"Emu",
		"Harmony",
		"ART",
		"Baldwin",
		"Eventide",
		"Inventronics",
		"Key Concepts",
		"Clarity",
		"Passac",
		"SIEL",
		"Synthax",
		"Stepp",
		"Hohner",
		"Twister",
		"Solton",
		"Jellinghaus",
		"Southworth",
		"PPG",
		"JEN",
		"SSL",
		"Audio Strueven",
		"Neve",
		"Soundtracs",
		"Elka",
		"Dynacord",
		"Intercont. El.",
		"Drawmer",
		"t.c. electronic",
		"Audio Architecture",
		"General Music",
		"Cheetah",
		"C.T.M.",
		"Simmons",
		"Soundcraft",
		"Steinberg",
		"Wersi",
		"Avab",
		"Divigram",
		"Waldorf",
		"QuasiMIDI",
		"Kawai",
		"Roland",
		"Korg",
		"Yamaha",
		"Casio",
		"Moridaira",
		"Kamiya Studio",
		"Akai",
		"Japan Victor",
		"Meisosha",
		"Hoshino Gakki",
		"Fujitsu Elec.",
		"Sony",
		"Nissin Onpa",
		"Teac",
		"System Product",
		"Matsushita",
		"Fostex"
};

char *SysEx::GetSysExString()
{
	return mainMIDI->GetSysExString(data,length);
}

char *Get3SysEx (UBYTE *data)
{
	char *what = "?";
	UBYTE a = 0, id;
	UBYTE *check=data;
	
	/* F0 */
	/* 00 */
	/* 00/20h */
	/* nr */
	
	check += 2;
	
	if (*check == 00)              /* USA */
    {
		id = *(check+1);
		
		switch(id)
		{
		case 0x01:what ="Warner New Media";break;
		case 0x07:what ="Digital Music";break;
		case 0x08:what ="IOTA";break;
		case 0x09:what ="New England";break;
		case 0x0a:what ="Artisyn";break;
		case 0x0b:what ="IVL Tech.";break;
		case 0x0c:what ="Southern Music";break;
		case 0x0d:what ="Lake Butler";break;
		case 0x0e:what ="Alesis";break;
		case 0x10:what ="DOD";break;
		case 0x11:what ="Studer";break;
		case 0x14:what ="Jeff Tripp";break;
		case 0x15:what ="KAT";break;
		case 0x16:what ="Opcode";break;
		case 0x17:what ="Rane Corp.";break;
		case 0x18:what ="Spatial";break;
		case 0x19:what ="KMX";break;
		case 0x1a:what ="A&H Brenell";break;
		case 0x1b:what ="Peavey";break;
		case 0x1c:what ="360 System";break;
		case 0x1e:what ="Marquis";break;
		case 0x1f:what ="Zeta";break;
		case 0x20:what ="Axxes";break;
		case 0x21:what ="Orban";break;
		case 0x24:what ="KTI";break;
		case 0x25:what ="Breakaway";break;
		case 0x26:what ="CAE";break;
		case 0x29:what ="Rocktron";break;
		case 0x2a:what ="PianoDisc";break;
		case 0x2b:what ="Cannon Res.";break;
		case 0x2e:what ="Blue Sky";break;
		case 0x2d:what ="Rogers";break;
		case 0x2f:what ="Encore";break;
		case 0x30:what ="Uptown";break;
		case 0x31:what ="Voce";break;
		case 0x32:what ="CTI Audio";break;
		case 0x33:what ="S&S Res.";break;
		case 0x34:what ="Broderbund";break;
		case 0x35:what ="Allan Organ";break;
		case 0x37:what ="Music Quest";break;
		case 0x38:what ="Aphex";break;
		case 0x39:what ="Gallien Krueger";break;
		case 0x3a:what ="IBM";break;
		case 0x3c:what ="Hotz";break;
		case 0x3d:what ="ETA";break;
		case 0x3e:what ="NSI";break;
		case 0x3f:what ="Ad Lib";break;
		case 0x40:what ="Richmond";break;
		case 0x41:what ="Microsoft";break;
		case 0x42:what ="Toolworks";break;
		case 0x43:what ="RJMG/Niche";break;
		case 0x44:what ="Intone";break;
		case 0x47:what ="GT/Groove";break;
		case 0x4f:what ="InterMIDI";break;
		case 0x55:what ="Lone Wolf";break;
		case 0x64:what ="Musonix";break;
		}
    }
	else
		/* Europa */
    {
		id = *(check+1);
		
		switch(id)
		{
		case 0x00:what ="Dream";break;
		case 0x01:what ="Strand Lighting";break;
		case 0x02:what ="Amek";break;
		case 0x04:what ="Dr.Böhm";break;
		case 0x06:what ="Trident Audio";break;
		case 0x07:what ="Real World Studio";break;
		case 0x09:what ="Yes";break;
		case 0x0a:what ="Audiomatica";break;
		case 0x0b:what ="Bontempi";break;
		case 0x0c:what ="F.B.T.";break;
		case 0x0e:what ="Larking Audio";break;
		case 0x0f:what ="Zero 88";break;
		case 0x10:what ="Micon Audio";break;
		case 0x11:what ="Forefront";break;
		case 0x13:what ="Kenton";break;
		case 0x15:what ="ADB";break;
		case 0x16:what ="Marshall";break;
		case 0x17:what ="DDA";break;
		case 0x1f:what ="TC";break;
		}
    }
	
	return (what);
}

char *GetRealTime (UBYTE *data)
{
	char *r="?";
	UBYTE sub1, sub2;
	
	sub1 = *(data + 3);
	sub2 = *(data + 4);
	
	switch (sub1)
    {
    case 0x01:
		switch (sub2)
        {
        case 0x01:
			r = "MTC-Full Message";
			break;
        case 0x02:
			r = "MTC User Bits";
			break;
        }
		break;
		
		case 0x02:
			{
				UBYTE sub3 = *(data + 5);

				switch (sub3)
				{
				case 0x00:
					r = "Expansion";
					break;
					
				case 0x01:
					r = "Light";
					break;
					
				case 0x02:
					r = "Projector";
					
					break;
				case 0x03:
					r = "Colour";
					
					break;
				case 0x04:
					r = "Stroboskope";
					
					break;
				case 0x05:
					r = "Laser";
					
					break;
				case 0x06:
					r = "Pursuer";
					break;
					
				case 0x10:
					r = "Sound";
					break;
					
				case 0x11:
					r = "Music";
					break;
					
				case 0x12:
					r = "CD Player";
					break;
					
				case 0x13:
					r = "EPROM Playback";
					break;
					
				case 0x14:
					r = "Tape";
					break;
					
				case 0x15:
					r = "Intercoms";
					break;
					
				case 0x16:
					r = "Amplifier";
					break;
					
				case 0x17:
					r = "Effectmodule";
					break;
					
				case 0x18:
					r = "Equalicer";
					break;
					
				case 0x20:
					r = "Machine";
					break;
					
				case 0x21:
					r = "?";
					break;
					
				case 0x22:
					r = "Wing";
					break;
					
				case 0x23:
					r = "Lift";
					break;
					
				case 0x24:
					r = "Revoling Table";
					break;
					
				case 0x25:
					r = "Stage";
					break;
					
				case 0x26:
					r = "Robot";
					break;
					
				case 0x27:
					r = "Animation";
					break;
					
				case 0x28:
					r = "Swimmer";
					break;
					
				case 0x29:
					r = "Separation";
					break;
					
				case 0x2a:
					r = "Boots";
					break;
					
				case 0x30:
					r = "Video";
					break;
					
				case 0x31:
					r = "Videotape";
					break;
					
				case 0x32:
					r = "Videotapeger.";
					break;
					
				case 0x33:
					r = "Video Disc Player";
					break;
					
				case 0x34:
					r = "Controller";
					break;
					
				case 0x35:
					r = "Videoeffect";
					break;
					
				case 0x36:
					r = "Titlegenerator";
					break;
					
				case 0x37:
					r = "Picture-controller";
					break;
					
				case 0x38:
					r = "Monitor";
					break;
					
				case 0x40:
					r = "Projection";
					break;
					
				case 0x41:
					r = "Filmprojector";
					break;
					
				case 0x42:
					r = "Diaprojection";
					break;
					
				case 0x43:
					r = "Videoprojection";
					break;
					
				case 0x44:
					r = "Dissolvers";
					break;
					
				case 0x45:
					r = "LightsDown";
					break;
					
				case 0x50:
					r = "Prozesscontrol";
					break;
					
				case 0x51:
					r = "Oil";
					break;
					
				case 0x52:
					r = "H2O";
					break;
					
				case 0x53:
					r = "CO2";
					break;
					
				case 0x54:
					r = "Pneumatic hammer";
					break;
					
				case 0x55:
					r = "Gas";
					break;
					
				case 0x56:
					r = "Fog";
					break;
					
				case 0x57:
					r = "Smoke";
					break;
					
				case 0x58:
					r = "Steam";
					break;
					
				case 0x60:
					r = "Fire";
					break;
					
				case 0x61:
					r = "Firework";
					break;
					
				case 0x62:
					r = "Explosion";
					break;
					
				case 0x63:
					r = "Flames";
					break;
					
				case 0x64:
					r = "Smoke/Pots";
					break;
					
				case 0x7f:
					r = "Broadcast";
					break;
          }
      }
      break;
	  
    case 0x03:
		switch (sub2)
        {
        case 0x01:
			r = "Measurepointer";
			break;
        case 0x02:
			r = "Change Signature";
			break;
        case 0x42:
			r = "Change Sign./delayed";
			break;
        }
		break;
		
		case 0x04:
			switch (sub2)
			{
			case 0x01:
				r = "Mainvolume";
				break;
			case 0x02:
				r = "Mainbalance";
				break;
			}
			break;
			
			case 0x05:
				switch (sub2)
				{
				case 0x00:
					r = "Special";
					break;
				case 0x01:
					r = "Punch In";
					break;
				case 0x02:
					r = "Punch Out";
					break;
					
				case 0x03:
					r = "Delete Punch In";
					break;
					
				case 0x04:
					r = "Delete Punch Out";
					break;
					
				case 0x05:
					r = "Event-Start";
					break;
					
				case 0x06:
					r = "Event-Stop";
					break;
					
				case 0x07:
					r = "Event-Start+Info";
					break;
					
				case 0x08:
					r = "Event-Stop+Info";
					break;
					
				case 0x09:
					r = "Delete Event-Start";
					break;
					
				case 0x0a:
					r = "Delete Event-Stop";
					break;
					
				case 0x0b:
					r = "Cue Point";
					break;
					
				case 0x0c:
					r = "Cue Point+Info";
					break;
					
				case 0x0d:
					r = "Delete Cue Point";
					break;
					
				case 0x0e:
					r = "Event-Name+Info";
					break;
				}
				break;
				
				case 0x06:
//					help = 2;
					
					switch (sub2)
					{
					case 0x51:
						r = "Event";
						break;
						
					case 0x50:
						r = "Procedure";
						break;
						
					case 0x60:
						r = "Procedure Response";
						break;
						
					case 0x52:
						r = "Group";
						break;
					case 0x53:
						r = "Command Segment";
						break;
					case 0x7c:
						r = "Wait";
						break;
					case 0x7f:
						r = "Resume";
						break;
						
					case 0x0c:
						r = "Command Error Reset";
						break;
					case 0x40:
						r = "Write";
						break;
					case 0x41:
						r = "Masked Write";
						break;
					case 0x42:
						r = "Read";
						break;
					case 0x43:
						r = "Update";
						break;
						
					case 0x0b:
						r = "Chase(MCP)";
						break;
					case 0x49:
						r = "Assign System Master";
						break;
					case 0x4a:
						r = "Generator Command";
						break;
					case 0x4b:
						r = "MTC Command";
						break;
					case 0x4c:
						r = "Move";
						break;
					case 0x4d:
						r = "Add";
						break;
					case 0x4e:
						r = "Subtract";
						break;
					case 0x4f:
						r = "Drop Frame Adjust";
						break;
						
					case 0x01:
						r = "Stop";
						break;
					case 0x02:
						r = "Play";
						break;
					case 0x03:
						r = "Deferred Play";
						break;
					case 0x05:
						r = "REW";
						break;
					case 0x09:
						r = "Pause";
						break;
					case 0x0a:
						r = "Eject";
						break;
					case 0x45:
						r = "Var. Play";
						break;
					case 0x46:
						r = "Search";
						break;
					case 0x47:
						r = "Shuttle";
						break;
					case 0x48:
						r = "Step";
						break;
					case 0x54:
						r = "Deferred var. Play";
						break;
						
					case 0x04:
						r = "FF";
						break;
					case 0x06:
						r = "Record Store";
						break;
					case 0x07:
						r = "Record Exit";
						break;
					case 0x08:
						r = "Record Pause";
						break;
					case 0x0d:
						r = "MMC Reset";
						break;
					case 0x55:
						r = "Record Store var.";
						break;
        }
		break;
		
    case 0x07:
		{
			switch (sub2)
			{
			case 0x40:
				r = "MMC In Signature";
				break;
				
			case 0x41:
				r = "MMC In Update Rate";
				break;
			case 0x42:
				r = "MMC In Response Error";
				break;
			case 0x43:
				r = "MMC In Command Error";
				break;
			case 0x44:
				r = "MMC In Cmd Error Level";
				break;
				
			case 0x02:
				r = "MMC In Sel. Master Code";
				break;
			case 0x03:
				r = "MMC In Requested Offset";
				break;
			case 0x04:
				r = "MMC In Actuel Offset";
				break;
			case 0x05:
				r = "MMC In Lock Deviation";
				break;
			case 0x22:
				r = "MMC In Short sel. Master";
				break;
			case 0x23:
				r = "MMC In Short Req. Offset";
				break;
			case 0x24:
				r = "MMC In Short Actuel Offset";
				break;
			case 0x25:
				r = "MMC In Short Lock Deviation";
				break;
			case 0x59:
				r = "MMC In Resolved Play Mode";
				break;
			case 0x5a:
				r = "MMC In Chase Mode";
				break;
				
			case 0x06:
				r = "MMC In Gen. Time Code";
				break;
			case 0x26:
				r = "MMC In Short Gen. TC";
				break;
			case 0x5b:
				r = "MMC In Gen. CMD Tally";
				break;
			case 0x5c:
				r = "MMC In Gen. Setup";
				break;
			case 0x5d:
				r = "MMC In Gen. Userbits";
				break;
			case 0x63:
				r = "MMC In VITC Insert Enable";
				break;
				
			case 0x01:
				r = "MMC In Sel. Time Code";
				break;
			case 0x21:
				r = "MMC In Short Sel. TC";
				break;
			case 0x45:
				r = "MMC In Sel. Time Standard";
				break;
			case 0x46:
				r = "MMC In Sel. TC Source";
				break;
			case 0x47:
				r = "MMC In Sel. TC Userbits";
				break;
				
			case 0x07:
				r = "MMC In MTC Input";
				break;
			case 0x27:
				r = "MMC In Short MTC Input";
				break;
			case 0x5e:
				r = "MMC In MTC Command Tally";
				break;
			case 0x5f:
				r = "MMC In MTC Set Up";
				break;
				
			case 0x08:
				r = "MMC In GP0 LP";
				break;
			case 0x09:
				r = "MMC In GP1 LP";
				break;
			case 0x0a:
				r = "MMC In GP2 LP";
				break;
			case 0x0b:
				r = "MMC In GP3 LP";
				break;
			case 0x0c:
				r = "MMC In GP4 LP";
				break;
			case 0x0d:
				r = "MMC In GP5 LP";
				break;
			case 0x0e:
				r = "MMC In GP6 LP";
				break;
			case 0x0f:
				r = "MMC In GP7 LP";
				break;
				
			case 0x28:
				r = "MMC In S. GP0 LP";
				break;
			case 0x29:
				r = "MMC In S. GP1 LP";
				break;
			case 0x2a:
				r = "MMC In S. GP2 LP";
				break;
			case 0x2b:
				r = "MMC In S. GP3 LP";
				break;
			case 0x2c:
				r = "MMC In S. GP4 LP";
				break;
			case 0x2d:
				r = "MMC In S. GP5 LP";
				break;
			case 0x2e:
				r = "MMC In S. GP6 LP";
				break;
			case 0x2f:
				r = "MMC In S. GP7 LP";
				break;
				
			case 0x48:
				r = "MMC In Motion Control Tally";
				break;
			case 0x49:
				r = "MMC In Velocity Tally";
				break;
			case 0x4a:
				r = "MMC In Stop Mode";
				break;
			case 0x4b:
				r = "MMC In Fast Mode";
				break;
			case 0x4c:
				r = "MMC In Rec. Mode";
				break;
			case 0x4d:
				r = "MMC In Rec. Status";
				break;
			case 0x4e:
				r = "MMC In Track Rec. Status";
				break;
			case 0x4f:
				r = "MMC In Track Rec. Ready";
				break;
				
			case 0x50:
				r = "MMC In Global Monitor";
				break;
			case 0x51:
				r = "MMC In Rec. Monitor";
				break;
			case 0x52:
				r = "MMC In Track Sync Monitor";
				break;
			case 0x53:
				r = "MMC In Track Input Monitor";
				break;
			case 0x54:
				r = "MMC In Step Length";
				break;
			case 0x55:
				r = "MMC In Play Speed Ref.";
				break;
			case 0x56:
				r = "MMC In Fixed Speed";
				break;
			case 0x57:
				r = "MMC In Lifter Defeat";
				break;
			case 0x58:
				r = "MMC In Control Disable";
				break;
			case 0x62:
				r = "MMC In Track Mute";
				break;
			case 0x65:
				r = "MMC In Failure";
				break;
			}
        }
		break;
		
    case 0x08:
		r = "Change Pitch";
		break;
    }
	
	/*
	if (help)
    {
	
      switch (help)
	  {
	  case 1:
	  CopyMem ("MSC-", mmhelp, 4);
	  break;
	  case 2:
	  CopyMem ("MMC>", mmhelp, 4);
	  break;
	  case 3:
	  CopyMem ("MMC<", mmhelp, 4);
	  break;
	  }
      CopyMem (r, &mmhelp[4], str_len1 (r));
	  }
	  */
	  
	  return (r);
}

char *GetNonRealtime (UBYTE *data)
{
	char *r="?";
	UBYTE sub1, sub2;
	
	sub1 = *(data + 3);
	sub2 = *(data + 4);
	
	switch (sub1)
    {
    case 0x7b:
		r = "EOF";
		break;
		
    case 0x7c:
		r = "Wait";
		break;
		
    case 0x7d:
		r = "Cancel";
		break;
		
    case 0x7e:
		r = "NAK";
		break;
		
    case 0x7f:
		r = "ACK";
		break;
		
    case 0x01:
		r = "Sample-Dump-Header";
		break;
    case 0x02:
		r = "Sample-Dump-Data";
		break;
    case 0x03:
		r = "Sample-Dump-Request";
		break;
		
    case 0x04:
		{
			switch (sub2)
			{
			case 0x00:
				r = "Special";
				break;
			case 0x01:
				r = "Punch In";
				break;
			case 0x02:
				r = "Punch Out";
				break;
			case 0x03:
				r = "Lösche Punch In";
				break;
			case 0x04:
				r = "Lösche Punch Out";
				break;
				
			case 0x05:
				r = "Event-Start";
				break;
				
			case 0x06:
				r = "Event-Stop";
				break;
				
			case 0x07:
				r = "Event-Start+Info";
				break;
				
			case 0x08:
				r = "Event-Stop+Info";
				break;
				
			case 0x09:
				r = "Delete Event-Start";
				break;
				
			case 0x0a:
				r = "Delete Event-Stop";
				break;
				
			case 0x0b:
				r = "Cue Point";
				break;
				
			case 0x0c:
				r = "Cue Point+Info";
				break;
				
			case 0x0d:
				r = "Delete Cue Point";
				
				break;
				
			case 0x0e:
				r = "Event-Name+Info";
				break;
			}
		}
		break;
		
    case 0x05:
		switch (sub2)
        {
        case 0x01:
			r = "Multi Loop Point";
			break;
			
        case 0x02:
			r = "Loop Point Request";
			break;
        }
		break;
		
		case 0x06:
			switch (sub2)
			{
			case 0x01:
				r = "Identity Request";
				break;
				
			case 0x02:
				r = "Identity Reply";
				break;
			}
			break;
			
			case 0x07:
				switch (sub2)
				{
				case 0x01:
					r = "File Dump Header";
					break;
				case 0x02:
					r = "File Dump Data";
					break;
				case 0x03:
					r = "File Dump Request";
					break;
				}
				break;
				
				case 0x08:
					switch (sub2)
					{
					case 0x00:
						r = "Bulk Dump Request";
						break;
						
					case 0x01:
						r = "Bulk Dump";
						break;
					}
					break;
					
					case 0x09:
						switch (sub2)
						{
						case 0x01:
							r = "GM on";
							break;
							
						case 0x02:
							r = "GM off";
							break;
						}
						break;
    }
	
	return (r);
}

bool mainMIDIBase::CheckIfChannelModeMessage(UBYTE status,UBYTE byte1)
	{
		if((status&0xF0)==CONTROLCHANGE)
		{
			if(byte1>=0x7a && byte1<=0x7f)
				return true;
		}

		return false;
	}

char *mainMIDIBase::GetSysExString(UBYTE *data,int length)
{
	if((!data) || length<3)
		return "ND";

	UBYTE *xx = data + 1;
	UBYTE id = *(xx);
	char *string="?";
	
	if (id == 0x7e)
	{
		if(length>4)
		string = GetNonRealtime (data); // +3/4
		
		/*
		EsAt (et,efbx, y, "NTRM");
		
		  if (eventeditor.eventwave)
		  EsAt (et,egfxx, y, what);
		*/
	}
	else if (id == 0x7f)
	{
		if(length>5)
		string = GetRealTime (data);
		
		/*
		EsAt (et,efbx, y, "RTM");
		if (eventeditor.eventwave)
		EsAt (et,egfxx, y, what);
		*/
	}
	else if (id == 0x7d)
		string="Non-Com.";
	else if (!id)                 /* 3er SYSEX !!! */
	{
		if(length>3)
		string=Get3SysEx (data);
	}
	else if (id < 81)
	{
		string=sysid[--id];
	}

	return string;
}
