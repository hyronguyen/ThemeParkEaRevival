#pragma once
#include <mmsystem.h>
struct ParkMusic {
    bool opened=false,enabled=true,focused=true;int track=1,volume=20;std::string error;
    bool command(const std::wstring&command){MCIERROR code=mciSendStringW(command.c_str(),nullptr,0,nullptr);if(code){wchar_t message[256];mciGetErrorStringW(code,message,256);error="Audio unavailable (code "+std::to_string(code)+")";return false;}return true;}
    void close(){if(opened)mciSendStringW(L"close themepark_music",nullptr,0,nullptr);opened=false;}
    bool open(const fs::path&path,bool silent=false){close();error.clear();if(!fs::is_regular_file(path)){error="Original music files missing";return false;}
        opened=command(L"open \""+path.wstring()+L"\" type mpegvideo alias themepark_music");if(!opened)return false;
        if(!command(L"setaudio themepark_music volume to "+std::to_wstring(silent?0:volume*10))){close();return false;}
        if(!command(L"play themepark_music repeat")){close();return false;}if(!enabled||!focused)command(L"pause themepark_music");return true;
    }
    void apply(){if(!opened)return;command(L"setaudio themepark_music volume to "+std::to_wstring(volume*10));command(enabled&&focused?L"resume themepark_music":L"pause themepark_music");}
};
static ParkMusic music;
// Independent PCM voices: SFX do not interrupt music or each other.
struct ParkSFX {
    struct Voice {HWAVEOUT device=nullptr;WAVEHDR header{};std::vector<char> pcm;std::string event;};
    std::array<Voice,8> voices{};std::map<std::string,std::vector<char>> cache;
    bool enabled=true,focused=true,silent=false;int volume=30,played=0;ULONGLONG lastTap=0;std::map<std::string,ULONGLONG> lastEvent;
    void reap(){for(auto&v:voices)if(v.device&&(v.header.dwFlags&WHDR_DONE)){waveOutUnprepareHeader(v.device,&v.header,sizeof(v.header));waveOutClose(v.device);v.device=nullptr;v.pcm.clear();}}
    void stop(){for(auto&v:voices)if(v.device){waveOutReset(v.device);waveOutUnprepareHeader(v.device,&v.header,sizeof(v.header));waveOutClose(v.device);v.device=nullptr;v.pcm.clear();}}
    bool play(const fs::path&folder,const std::string&event){
        if(!enabled||!focused)return false;reap();for(const auto&v:voices)if(v.device&&v.event==event)return false;ULONGLONG now=GetTickCount64();if(lastEvent.count(event)&&now-lastEvent[event]<(event=="click"?120:300))return false;
        auto it=cache.find(event);if(it==cache.end()){std::ifstream in(folder/(event+".wav"),std::ios::binary);if(!in)return false;std::vector<char> bytes((std::istreambuf_iterator<char>(in)),{});if(bytes.size()<44||std::string(bytes.data(),4)!="RIFF"||std::string(bytes.data()+36,4)!="data")return false;it=cache.emplace(event,std::move(bytes)).first;}
        auto free=std::find_if(voices.begin(),voices.end(),[](const Voice&v){return !v.device;});if(free==voices.end())return false;auto&v=*free;v.event=event;auto&b=it->second;
        auto u16=[&](int o){return unsigned((unsigned char)b[o])|(unsigned((unsigned char)b[o+1])<<8);};
        auto u32=[&](int o){return u16(o)|(u16(o+2)<<16);};
        if(u16(20)!=1||u16(22)!=1||u16(34)!=16||u32(40)!=b.size()-44)return false;
        WAVEFORMATEX format{};format.wFormatTag=WAVE_FORMAT_PCM;format.nChannels=1;format.nSamplesPerSec=u32(24);format.wBitsPerSample=16;format.nBlockAlign=2;format.nAvgBytesPerSec=format.nSamplesPerSec*2;
        v.pcm.assign(b.begin()+44,b.end());for(size_t i=0;i+1<v.pcm.size();i+=2){short value=short((unsigned char)v.pcm[i]|((unsigned char)v.pcm[i+1]<<8));value=short(int(value)*(silent?0:volume)/100);v.pcm[i]=char(value&255);v.pcm[i+1]=char((unsigned short)value>>8);}
        if(waveOutOpen(&v.device,WAVE_MAPPER,&format,0,0,CALLBACK_NULL)!=MMSYSERR_NOERROR){v.device=nullptr;return false;}
        v.header={};v.header.lpData=v.pcm.data();v.header.dwBufferLength=DWORD(v.pcm.size());
        if(waveOutPrepareHeader(v.device,&v.header,sizeof(v.header))!=MMSYSERR_NOERROR){waveOutClose(v.device);v.device=nullptr;return false;}
        if(waveOutWrite(v.device,&v.header,sizeof(v.header))!=MMSYSERR_NOERROR){waveOutUnprepareHeader(v.device,&v.header,sizeof(v.header));waveOutClose(v.device);v.device=nullptr;return false;}
        lastEvent[event]=now;if(event=="tap")lastTap=now;++played;return true;
    }
};
static ParkSFX sfx;
